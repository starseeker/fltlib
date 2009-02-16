
/*****************************************************************************
 * FLT: An OpenFlight file loader
 *
 * Copyright (C) 2007  Michael M. Morrison   All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "flt.h"
#include "recnames.h"
#include "zmalloc.h"

static int debug = 0;

FltNode *
fltNewNode( FltFile * flt, uint32 size )
{
	FltNode * node;

	if( (node = zcalloc( 1, size )) ) {

		flt->allNodes = zrealloc( flt->allNodes, (1+flt->numNodes) * 
																									sizeof( FltNode * ) );
		if( flt->allNodes ) {
			flt->allNodes[ flt->numNodes ] = node;
			flt->numNodes++;

			return node;
		}
	}

	// need something better here.
	printf("Unable to malloc!\n");
	exit(0);

	// keep compiler quiet
	return 0;
}

char *
fltNodeName( FltNode * node )
{
	// check for nodes with id's
	if( FLTNODE_HASID( node->type ) ) {
		FltRecord * rec;
		FltNode * findNode;

		// first, check for "longid" field
		findNode = node->attr;

		while( findNode ) {
			if( findNode->type == FLTRECORD_LONGID ) {
				FltLongID * lid = (FltLongID *)findNode;
				return lid->text;
			}

			findNode = findNode->next;
		}

		// if no longid, return regular ID field
		if( (rec = fltRecordGetDefinition( (uint16)node->type )) ) {
			int x = 0;

			while( rec->entry[x].type ) {
				if( !strcmp( rec->entry[x].name, "ID" ) ) {
					return (char *)( (char *)node + rec->entry[x].offset );
				}
				x ++;
			}

		}

		return NULL;

	/* special cases */
	} else if( node->type == FLTRECORD_EXTERNALREFERENCE ) {

		// temporary buffer
		static char fname[256];

		/* name it the same as the flt file in the path */
		FltExternalReference * extref = (FltExternalReference *)node;

		/* first, unix naming */
		char * ptr = strrchr( extref->path, '/' );

		if( ptr ) {
			char * ptr2;

			ptr++;
			/* next, make sure no winblows paths */
			ptr2 = strrchr( extref->path, '\\' );

			if( ptr2 )
				ptr = ++ptr2;

		} else {
			/* only winblows paths */
			ptr = strrchr( extref->path, '\\' );
			if( ptr )
				ptr++;
			else
				ptr = extref->path;
		}

		strcpy( fname, ptr );

		ptr = strrchr( fname, '.' );

		if( ptr )
			*ptr = 0;

		return fname;

	} else
		return NULL;
}

char *
fltSafeNodeName( FltNode * node )
{
	static char unnamed[] = {"Unnamed"};
	static char null[] = {"Null"};
	char * ret;

	if( node ) {
		ret = fltNodeName( node );

		if( ret )
			return ret;
		else
			return unnamed;
	} else
		return null;
}

void
fltNodeAddChild( FltNode * parent, FltNode * child )
{
	assert( parent );

	parent->child = zrealloc( parent->child, (1+parent->numChildren) *
																									sizeof( FltNode * ) );
	parent->child[ parent->numChildren ] = child;
	parent->numChildren++;
}

static void *
fltPushLevel( FLTRECORDFUNC_ARGLIST )
{
	assert( flt );

	FltStackPush( flt->stack, flt->lastParent );

	if( debug )
		printf("Pushing: '%s' is parent.\n", fltSafeNodeName( FLT_GETPARENT(flt) ));

	flt->lastNode = NULL;

	flt->treeDepth++;

  flt->flags |= FLTFILE_FLAGS_FIRSTPUSHSEEN;

	return NULL;
}

static void *
fltPopLevel( FLTRECORDFUNC_ARGLIST )
{
	assert( flt );

	// pop off last node
	// we set flt->lastNode here, so that the next node has this node
	// as its neighbor.

	if( FLT_GETPARENT(flt) == NULL )
	{
		printf( "Warning in fltlib: attempted to pop the stack on a top-level node.\n"
			"\tThis indicates that THE FLT FILE IS MALFORMED.\n"
			"\tIgnoring the pop opcode, WITH NO GUARANTEE OF CORRECTNESS.\n" );
	}
	else
	{
		flt->lastNode = FltStackPop( flt->stack );

		if( debug ) {
			if( FLT_GETPARENT(flt) == NULL )
				printf("Popping: At top level.\n" );
			else
				printf("Popping: '%s' is new parent.\t", 
																fltSafeNodeName( FLT_GETPARENT(flt) ) );

			printf("lastNode: %s\n", fltSafeNodeName( flt->lastNode ) );
		}

		flt->treeDepth--;
	}
	return NULL;
}

static void *
fltPushExtension( FLTRECORDFUNC_ARGLIST )
{
  char xbuf[20];

	assert( flt );

  // read reserved string (probably name?)
  fltReadBlock( flt, xbuf, 18 );
  // read vertex reference index
  (void)fltReadUInt16( flt );

	if( debug )
		printf("Pushing Extension: '%s' is parent.\n", 
                                      fltSafeNodeName( FLT_GETPARENT(flt) ));
	flt->ignoreExtension++;

	return NULL;
}

static void *
fltPopExtension( FLTRECORDFUNC_ARGLIST )
{
  char xbuf[20];

  assert( flt );

  // read reserved string (probably name?)
  fltReadBlock( flt, xbuf, 18 );
  // read vertex reference index
  (void)fltReadUInt16( flt );

	if( debug )
			printf("Popping Extension.\n" );

	flt->ignoreExtension--;

	return NULL;
}

static void *
fltHeader( FLTRECORDFUNC_ARGLIST )
{
	char path[MAX_PATHLEN];
	char * ptr;

	assert( flt );
	assert( flt->stdFile );

	flt->header = (FltHeader *) fltNewNode( flt, sizeof( FltHeader ) );

	fltReadBlock( flt, flt->header->ID, 8 );

	// byte swap important fields

	flt->header->formatRevision = fltReadUInt32( flt );
	flt->header->editRevision   = fltReadUInt32( flt );

	fltReadBlock( flt, flt->header->dateTime, 32 );

	flt->header->nextGroupNodeID  = fltReadUInt16( flt );
	flt->header->nextLODNodeID    = fltReadUInt16( flt );
	flt->header->nextObjectNodeID = fltReadUInt16( flt );
	flt->header->nextFaceNodeID   = fltReadUInt16( flt );
	flt->header->unitMultiplier   = fltReadUInt16( flt );
	flt->header->coordUnits       = fltReadUInt8( flt );
	flt->header->setTexWhiteOnNewFaces  = fltReadUInt8( flt );
	flt->header->flags            = fltReadUInt32( flt );

	fltReadBlock( flt, flt->header->reserved0, sizeof(uint32)*6 );

	flt->header->projectionType		= fltReadUInt32( flt );

	fltReadBlock( flt, flt->header->reserved1, sizeof(uint32)*7 );

	flt->header->nextDOFNodeID		= fltReadUInt16( flt );
	flt->header->vertexStorageLength	= fltReadUInt16( flt );
	flt->header->databaseOrigin		= fltReadUInt32( flt );

	flt->header->swDBCoordX		= fltReadReal64( flt );
	flt->header->swDBCoordY		= fltReadReal64( flt );
	flt->header->deltaXPlace	= fltReadReal64( flt );
	flt->header->deltaYPlace	= fltReadReal64( flt );

	flt->header->nextSoundNodeID	= fltReadUInt16( flt );
	flt->header->nextPathNodeID		= fltReadUInt16( flt );

	flt->header->reserved2[0]			= fltReadUInt32( flt );
	flt->header->reserved2[1]			= fltReadUInt32( flt );

	flt->header->nextClipNodeID		= fltReadUInt16( flt );
	flt->header->nextTextNodeID		= fltReadUInt16( flt );
	flt->header->nextBSPNodeID		= fltReadUInt16( flt );
	flt->header->nextSwitchNodeID	= fltReadUInt16( flt );
	flt->header->reserved3				= fltReadUInt32( flt );

	/* these are now necessary for the UTM coord conversion */
	flt->header->swDBLatitude			= fltReadReal64( flt );
	flt->header->swDBLongitude		= fltReadReal64( flt );
	flt->header->neDBLatitude			= fltReadReal64( flt );
	flt->header->neDBLongitude		= fltReadReal64( flt );
	flt->header->originDBLatitude	= fltReadReal64( flt );
	flt->header->originDBLongitude		= fltReadReal64( flt );
	flt->header->lambertUpperLatitude = fltReadReal64( flt );
	flt->header->lambertLowerLatitude = fltReadReal64( flt );

	flt->header->nextLightSourceNodeID = fltReadUInt16( flt );
	flt->header->nextLightPointNodeID = fltReadUInt16( flt );
	flt->header->nextRoadNodeID = fltReadUInt16( flt );
	flt->header->nextCATNodeID = fltReadUInt16( flt );
	flt->header->reserved4 = fltReadUInt16( flt );
	flt->header->reserved5 = fltReadUInt16( flt );
	flt->header->reserved6 = fltReadUInt16( flt );
	flt->header->reserved7 = fltReadUInt16( flt );

	flt->header->earthEllipsoidModel = fltReadUInt32( flt );

	flt->header->nextAdaptiveNodeID = fltReadUInt16( flt );
	flt->header->nextCurveNodeID = fltReadUInt16( flt );

	flt->header->utmZone = fltReadInt16( flt );

	/* ignore the rest for now.. */

  if( FLT_IS_V13X( flt ) )
  {
    // only 1 flag valid in flags for 13.x
    flt->header->flags &= FLTHDRFLAGS_SAVE_VTX_NORMALS;
  }

	if( debug )
		printf("FLT: %s: is a version %d flt file: coords: ", flt->fileName, 
																								flt->header->formatRevision );

	if( flt->header->projectionType == FLT_PROJECTION_UTM )
	{
		printf("%s: Projection is UTM\n", flt->fileName );
		//printf("sw corner: %f %f\n", flt->header->swDBLatitude,
	  //														flt->header->swDBLongitude );
		flt->coordScale = 1.0;
	}
	else
	{ 
		if( flt->header->projectionType != FLT_PROJECTION_FLATEARTH )
		{
			printf(
	"FLT: WARNING: database projection type is UNSUPPORTED - USING FLATEARTH\n");
		}

		/* set up coord units system (default is METERS)*/
		if( flt->xformUnits )
		{

		// check coord conversions
		switch( flt->header->coordUnits )
		{
			case FLT_COORDS_METERS:
				if( debug ) printf("Meters\n");
				flt->coordScale = 1.0f;
				break;

			case FLT_COORDS_KILOMETERS:
				if( debug ) printf("Kilometers\n");
				flt->coordScale = 1000.0f;
				break;

			case FLT_COORDS_FEET:
				if( debug ) printf("Feet\n");
				flt->coordScale = 0.3048f;
				break;

			case FLT_COORDS_INCHES:
				if( debug ) printf("Inches\n");
				flt->coordScale = (0.3048f/12.0f);
				break;

			case FLT_COORDS_NAUTICALMILES:
				if( debug ) printf("Nautical Miles\n");
				flt->coordScale = (1.1516f*5280.f*0.3048f); /* NM->M->FT->METERS */
				break;

			default:
				printf("FLT: COORD UNITS UNKNOWN!!\n");
				flt->coordScale = 1.0f;
				break;
		}

		// since we are converting to meters, set the header appropriately
		flt->header->coordUnits = FLT_COORDS_METERS;
		}
	}
	// automagically add the path to this file
	strcpy( path, flt->fileName );
	if( (ptr = strrchr( path, '/' )) ) 
		*ptr = 0;
	else if( (ptr = strrchr( path, '\\' )) )
		*ptr = 0;
	else
		strcpy( path, "." );
	
	fltAddSearchPath( flt, path ); 

	return flt->header;
}

static void *
fltExternalReference( FLTRECORDFUNC_ARGLIST )
{
	FltExternalReference * ext;

	assert( flt );
	assert( flt->stdFile );

	ext = (FltExternalReference *) fltNewNode( flt, sizeof(FltExternalReference));

	fltReadBlock( flt, ext->path, 200 );

	ext->reserved0 = fltReadUInt8( flt );
	ext->reserved1 = fltReadUInt8( flt );
	ext->reserved2 = fltReadUInt16( flt );

	ext->flags = fltReadUInt32( flt );

	ext->reserved3 = fltReadUInt16( flt );

	return ext;
}

static void *
fltMatrix( FLTRECORDFUNC_ARGLIST )
{
	FltMatrix * mat;
	int i;

	assert( flt );
	assert( flt->stdFile );

	mat = (FltMatrix *) fltNewNode( flt, sizeof(FltMatrix));

	// read matrix
	for(i=0;i<16;i++)
		mat->matrix[i] = fltReadReal32( flt );

	// scale translation
	mat->matrix[12] *= flt->coordScale;
	mat->matrix[13] *= flt->coordScale;
	mat->matrix[14] *= flt->coordScale;

	return mat;
}

static void *
fltGeneralMatrix( FLTRECORDFUNC_ARGLIST )
{
	FltGeneralMatrix * mat;
	int i;

	assert( flt );
	assert( flt->stdFile );

	mat = (FltGeneralMatrix *) fltNewNode( flt, sizeof(FltGeneralMatrix));

	// read matrix
	for(i=0;i<16;i++)
		mat->matrix[i] = fltReadReal32( flt );

	// scale translation
	mat->matrix[12] *= flt->coordScale;
	mat->matrix[13] *= flt->coordScale;
	mat->matrix[14] *= flt->coordScale;

	return mat;
}

static void *
fltNonuniformScale( FLTRECORDFUNC_ARGLIST )
{
	FltNonuniformScale * nus;

	assert( flt );
	assert( flt->stdFile );

	nus = (FltNonuniformScale *) fltNewNode( flt, sizeof(FltNonuniformScale));

	nus->reserved = fltReadUInt32( flt );
	nus->centerX  = fltReadReal64( flt ) * flt->coordScale;
	nus->centerY  = fltReadReal64( flt ) * flt->coordScale;
	nus->centerZ  = fltReadReal64( flt ) * flt->coordScale;
	nus->scaleX   = fltReadReal32( flt );
	nus->scaleY   = fltReadReal32( flt );
	nus->scaleZ   = fltReadReal32( flt );

	return nus;
}

static void *
fltTranslate( FLTRECORDFUNC_ARGLIST )
{
	FltTranslate * trans;

	assert( flt );
	assert( flt->stdFile );

	trans = (FltTranslate *) fltNewNode( flt, sizeof(FltTranslate));

	trans->reserved = fltReadUInt32( flt );
	trans->fromX  = fltReadReal64( flt ) * flt->coordScale;
	trans->fromY  = fltReadReal64( flt ) * flt->coordScale;
	trans->fromZ  = fltReadReal64( flt ) * flt->coordScale;
	trans->deltaX   = fltReadReal64( flt ) * flt->coordScale;
	trans->deltaY   = fltReadReal64( flt ) * flt->coordScale;
	trans->deltaZ   = fltReadReal64( flt ) * flt->coordScale;

	return trans;
}

static void *
fltReplicate( FLTRECORDFUNC_ARGLIST )
{
	FltReplicate * rep;

	assert( flt );
	assert( flt->stdFile );

	rep = (FltReplicate *) fltNewNode( flt, sizeof(FltReplicate));

	rep->replications = fltReadUInt16( flt );
	rep->reserved     = fltReadUInt16( flt );

	return rep;
}

static void *
fltExternalReferencePostProcess( FLTPOSTPROCESSFUNC_ARGLIST )
{
	FltExternalReference * extref = (FltExternalReference *)node;

	assert( flt );

	// go find the files, and reference (later)
	(void)(extref);

	return 0;
}

static void *
fltLightPoint( FLTRECORDFUNC_ARGLIST )
{
	FltLightPoint * lp;

	assert( flt );
	assert( flt->stdFile );

	lp = (FltLightPoint *) fltNewNode( flt, sizeof(FltLightPoint) );

  // only support name
  fltReadBlock( flt, lp->ID, 8 );

	return lp;
}

static void *
fltLightPointSystem( FLTRECORDFUNC_ARGLIST )
{
	FltLightPointSystem * lp;

	assert( flt );
	assert( flt->stdFile );

	lp = (FltLightPointSystem *) fltNewNode( flt, sizeof(FltLightPointSystem) );

  fltReadBlock( flt, lp->ID, 8 );
  lp->intensity = fltReadReal32( flt );
  lp->animationState = fltReadUInt32( flt );
  lp->flags = fltReadUInt32( flt );

	return lp;
}

static void *
fltVertexPalette( FLTRECORDFUNC_ARGLIST )
{
	assert( flt );
	assert( flt->stdFile );

	flt->vertPalette = (FltVertexPalette *) 
																fltNewNode( flt, sizeof( FltVertexPalette ) );

	// may be needed?
	//len = fltReadUInt32( flt );

	(void)fltReadUInt32( flt );

	flt->vertPalette->numVerts = 0;

	return flt->vertPalette;
}

static void *
fltColorPalette( FLTRECORDFUNC_ARGLIST )
{
	uint32 i;

	assert( flt );
	assert( flt->stdFile );

	flt->colorPalette = (FltColorPalette *) 
																fltNewNode( flt, sizeof( FltColorPalette ) );

	// skip reserved block 
	fltSkipBlock( flt, 128 );

	for(i=0;i<1024;i++)
		flt->colorPalette->color[i] = fltReadUInt32( flt );

	return flt->colorPalette;
}

static void *
fltVertexPalettePostProcess( FLTPOSTPROCESSFUNC_ARGLIST )
{
	uint32 i;

	assert( flt );

	// adjust byte-offset of the record to anchor from VertexPalette, rather
	// than start of file

	for(i=0; i<flt->vertPalette->numVerts; i++) {
		FltVertex *v = flt->vertPalette->verts[i];

		v->node.byteOffset -= flt->vertPalette->node.byteOffset;
	}

	//
	// set the number of "search" verts to be the number originally
	// read from the file.  We add verts later during post processing
	// and it messes up the search.
	//
	flt->vertPalette->numSearchVerts = flt->vertPalette->numVerts;

	return 0;
}

static void *
fltSwitch( FLTRECORDFUNC_ARGLIST )
{
	uint32 i;
	FltSwitch * sw;

	assert( flt );
	assert( flt->stdFile );

	sw = (FltSwitch *) fltNewNode( flt, sizeof( FltSwitch ) );

	fltReadBlock( flt, sw->ID, 8 );

	sw->reserved0 = fltReadUInt32( flt );
	sw->currentMask = fltReadUInt32( flt );
	//
	// NOTE: The docs must be wrong.  They state that the order is
	// num32bitInts/mask first, then numMasks.
	// Also: The bit pattern appears to be inverted as well.
	//
	sw->numMasks = fltReadUInt32( flt );
	sw->numUInt32sPerMask = fltReadUInt32( flt );

	sw->masks = (uint32 *)zcalloc( 1, sw->numUInt32sPerMask * 
																				sw->numMasks * sizeof(uint32) );

//	printf("Switch: '%s' - numMasks: %d (%d/mask)\n", sw->ID, sw->numMasks,
//																										sw->numUInt32sPerMask);

	for(i=0;i<sw->numMasks*sw->numUInt32sPerMask;i++) {
		sw->masks[i] = fltReadUInt32( flt );
		//printf("\t%d = %08x\n", i, sw->masks[i] );
	}

	return sw;
}

static void *
fltComment( FLTRECORDFUNC_ARGLIST )
{
	FltComment * comment;

	assert( flt );
	assert( flt->stdFile );

	comment = (FltComment *) fltNewNode( flt, sizeof( FltComment ) );

	fltReadBlock( flt, comment->text, 
				(recLength > FLTRECORD_COMMENT_MAX_LEN) ? FLTRECORD_COMMENT_MAX_LEN-1 :
						recLength );
	// null term
	comment->text[FLTRECORD_COMMENT_MAX_LEN-1] = 0;

	return comment;
}

static void *
fltLongID( FLTRECORDFUNC_ARGLIST )
{
	FltLongID * longid;

	assert( flt );
	assert( flt->stdFile );

	longid = (FltLongID *) fltNewNode( flt, sizeof( FltLongID ) );

	fltReadBlock( flt, longid->text, 
				(recLength > FLTRECORD_LONGID_MAX_LEN) ? FLTRECORD_LONGID_MAX_LEN-1 :
						recLength );
	// null term
	longid->text[FLTRECORD_LONGID_MAX_LEN-1] = 0;

	return longid;
}

static void *
fltLOD( FLTRECORDFUNC_ARGLIST )
{
	FltLOD * lod;

	assert( flt );
	assert( flt->stdFile );

	lod = (FltLOD *) fltNewNode( flt, sizeof( FltLOD ) );

	fltReadBlock( flt, lod->ID, 8 );

	lod->reserved0 = fltReadUInt32( flt );
	lod->switchInDistance = fltReadReal64( flt ) * flt->coordScale;
	lod->switchOutDistance = fltReadReal64( flt ) * flt->coordScale;
	lod->specialEffectID1 = fltReadUInt16( flt );
	lod->specialEffectID2 = fltReadUInt16( flt );
	lod->flags = fltReadUInt32( flt );
	lod->centerX = fltReadReal64( flt ) * flt->coordScale;
	lod->centerY = fltReadReal64( flt ) * flt->coordScale;
	lod->centerZ = fltReadReal64( flt ) * flt->coordScale;
	lod->transitionRange = fltReadReal64( flt ) * flt->coordScale;

	return lod;
}

void
fltAddVertexToPalette( FltVertexPalette * palette, FltVertex * vert )
{
	assert( palette );
	
	palette->verts = zrealloc( palette->verts, (palette->numVerts+1) * 
																									sizeof( FltVertex * ) );

	palette->verts[ palette->numVerts ] = vert;
	vert->indx = palette->numVerts;

	palette->numVerts++;
}

void
fltAddLightSourceToPalette( FltFile * flt, FltLightSourcePaletteEntry * entry )
{
  FltLightSourcePalette * palette = flt->lightSourcePalette;

	if( !palette )
	{
    palette = flt->lightSourcePalette = 
                            zcalloc( 1, sizeof( FltLightSourcePalette ) );
	}

	palette->lights = zrealloc( palette->lights, (palette->numLights+1) * 
																	sizeof( FltLightSourcePaletteEntry * ) );

	palette->lights[ palette->numLights ] = entry;
	palette->numLights++;
}

static void *
fltLightSourcePaletteEntry( FLTRECORDFUNC_ARGLIST )
{
	FltLightSourcePaletteEntry * light;

	assert( flt );
	assert( flt->stdFile );

	light = (FltLightSourcePaletteEntry *) fltNewNode( flt, 
													sizeof( FltLightSourcePaletteEntry ) );

	light->index = fltReadUInt32( flt );
	fltSkipBlock( flt, 2*sizeof(uint32) );

	fltReadBlock( flt, light->ID, 20 );

	light->reserved1 = fltReadUInt32( flt );

	light->ambient[0] = fltReadReal32( flt );
	light->ambient[1] = fltReadReal32( flt );
	light->ambient[2] = fltReadReal32( flt );
	light->ambient[3] = fltReadReal32( flt );

	light->diffuse[0] = fltReadReal32( flt );
	light->diffuse[1] = fltReadReal32( flt );
	light->diffuse[2] = fltReadReal32( flt );
	light->diffuse[3] = fltReadReal32( flt );

	light->specular[0] = fltReadReal32( flt );
	light->specular[1] = fltReadReal32( flt );
	light->specular[2] = fltReadReal32( flt );
	light->specular[3] = fltReadReal32( flt );

	light->type = fltReadUInt32( flt );

	fltSkipBlock( flt, sizeof(uint32)*10 );

	light->spotExponent = fltReadReal32( flt );
	light->spotCutoff   = fltReadReal32( flt );

	light->yaw     = fltReadReal32( flt );
	light->pitch   = fltReadReal32( flt );

	light->attenC   = fltReadReal32( flt );
	light->attenL   = fltReadReal32( flt );
	light->attenQ   = fltReadReal32( flt );

	light->activeDuringModeling = fltReadUInt32( flt );

	// skip the rest as it is reserved anyway

	fltAddLightSourceToPalette( flt, light );

	return light;
}

static void *
fltLightSource( FLTRECORDFUNC_ARGLIST )
{
	FltLightSource * light;

	assert( flt );
	assert( flt->stdFile );

	light = (FltLightSource *) fltNewNode( flt, sizeof( FltLightSource ) );

  fltReadBlock( flt, light->ID, 8 );
	light->reserved0    = fltReadUInt32( flt );
	light->paletteIndex = fltReadUInt32( flt );
	light->reserved1    = fltReadUInt32( flt );
	light->flags        = fltReadUInt32( flt );
	light->reserved2    = fltReadUInt32( flt );

	light->position[0] = fltReadReal64( flt );
	light->position[1] = fltReadReal64( flt );
	light->position[2] = fltReadReal64( flt );

	light->yaw   = fltReadReal32( flt );
	light->pitch = fltReadReal32( flt );

	return light;
}

static void
checkExtents( FltFile * flt, FltVertex * vert )
{
	if( vert->x < flt->extents[0] )
		flt->extents[0] = vert->x;
	if( vert->x > flt->extents[2] )
		flt->extents[2] = vert->x;

	if( vert->y < flt->extents[1] )
		flt->extents[1] = vert->y;
	if( vert->y > flt->extents[3] )
		flt->extents[3] = vert->y;

	if( vert->z < flt->extents[4] )
		flt->extents[4] = vert->z;
	if( vert->z > flt->extents[5] )
		flt->extents[5] = vert->z;
}

static void *
fltVertexWithColor( FLTRECORDFUNC_ARGLIST )
{
	FltVertex * vert;

	assert( flt );
	assert( flt->stdFile );

	vert = (FltVertex *) fltNewNode( flt, sizeof( FltVertex ) );

	vert->colorNameIndex = fltReadUInt16( flt );
	vert->flags = fltReadUInt16( flt );

	vert->x = fltReadReal64( flt ) * flt->coordScale;
	vert->y = fltReadReal64( flt ) * flt->coordScale;
	vert->z = fltReadReal64( flt ) * flt->coordScale;

	checkExtents( flt, vert );

  // set default normal in case it is read
  // i,j will be zero
  vert->k = 1.f;

	vert->packedColor = fltReadUInt32( flt );

  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
    vert->colorIndex  = vert->colorNameIndex;
  else
    vert->colorIndex  = fltReadUInt32( flt );

	vert->localFlags = FVHAS_COLOR;

	fltAddVertexToPalette( flt->vertPalette, vert );

	return vert;
}

static void *
fltVertexWithColorNormal( FLTRECORDFUNC_ARGLIST )
{
	FltVertex * vert;

	assert( flt );
	assert( flt->stdFile );

	vert = (FltVertex *) fltNewNode( flt, sizeof( FltVertex ) );

	vert->colorNameIndex = fltReadUInt16( flt );
	vert->flags = fltReadUInt16( flt );

	vert->x = fltReadReal64( flt ) * flt->coordScale;
	vert->y = fltReadReal64( flt ) * flt->coordScale;
	vert->z = fltReadReal64( flt ) * flt->coordScale;

	checkExtents( flt, vert );

	vert->i = fltReadReal32( flt );
	vert->j = fltReadReal32( flt );
	vert->k = fltReadReal32( flt );

	vert->packedColor = fltReadUInt32( flt );

  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
    vert->colorIndex  = vert->colorNameIndex;
  else
    vert->colorIndex  = fltReadUInt32( flt );

	vert->localFlags = FVHAS_COLOR | FVHAS_NORMAL;

	fltAddVertexToPalette( flt->vertPalette, vert );

	return vert;
}

static void *
fltVertexWithColorTexture( FLTRECORDFUNC_ARGLIST )
{
	FltVertex * vert;

	assert( flt );
	assert( flt->stdFile );

	vert = (FltVertex *) fltNewNode( flt, sizeof( FltVertex ) );

	vert->colorNameIndex = fltReadUInt16( flt );
	vert->flags = fltReadUInt16( flt );

	vert->x = fltReadReal64( flt ) * flt->coordScale;
	vert->y = fltReadReal64( flt ) * flt->coordScale;
	vert->z = fltReadReal64( flt ) * flt->coordScale;

	checkExtents( flt, vert );

  // set default normal in case it is read.
  // i,j will be zeroed already
  vert->k = 1.f;

	vert->u = fltReadReal32( flt );
	vert->v = fltReadReal32( flt );

	vert->packedColor = fltReadUInt32( flt );

  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
    vert->colorIndex  = vert->colorNameIndex;
  else
    vert->colorIndex  = fltReadUInt32( flt );

	vert->localFlags = FVHAS_COLOR | FVHAS_TEXTURE;

	fltAddVertexToPalette( flt->vertPalette, vert );

	return vert;
}

static void *
fltVertexWithColorNormalTexture( FLTRECORDFUNC_ARGLIST )
{
	FltVertex * vert;

	assert( flt );
	assert( flt->stdFile );

	vert = (FltVertex *) fltNewNode( flt, sizeof( FltVertex ) );

	vert->colorNameIndex = fltReadUInt16( flt );
	vert->flags = fltReadUInt16( flt );

	vert->x = fltReadReal64( flt ) * flt->coordScale;
	vert->y = fltReadReal64( flt ) * flt->coordScale;
	vert->z = fltReadReal64( flt ) * flt->coordScale;

	checkExtents( flt, vert );

	vert->i = fltReadReal32( flt );
	vert->j = fltReadReal32( flt );
	vert->k = fltReadReal32( flt );

	vert->u = fltReadReal32( flt );
	vert->v = fltReadReal32( flt );

	vert->packedColor = fltReadUInt32( flt );

  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
    vert->colorIndex  = vert->colorNameIndex;
  else
    vert->colorIndex  = fltReadUInt32( flt );

	vert->localFlags = FVHAS_COLOR | FVHAS_TEXTURE | FVHAS_NORMAL;

	fltAddVertexToPalette( flt->vertPalette, vert );

	return vert;
}

static void *
fltVertexList( FLTRECORDFUNC_ARGLIST )
{
	FltVertexList * list;
	uint32 numVerts = recLength/4;
	uint32 i;

	assert( flt );
	assert( flt->stdFile );

	list = (FltVertexList *) fltNewNode( flt, sizeof( FltVertexList ) );

	list->numVerts = numVerts;
	list->list = (FltVertex **) zcalloc( 1, (sizeof(FltVertex *) * numVerts) );
	list->indexList = (uint32 *) zcalloc( 1, (sizeof(uint32 *) * numVerts) );

	// read in the ints, we later convert them to pointers
	for(i=0;i<numVerts;i++)
		list->list[i] = (FltVertex *)fltReadUInt32( flt );

	return list;
}

static int
vertFunc( const void * xkey, const void * member )
{
	FltVertex * v = *((FltVertex **)member);
	uint32 off = *(uint32 *)xkey;

	if( off > v->node.byteOffset )
		return 1;
	else if( off < v->node.byteOffset )
		return -1;
	else
		return 0;
}

static void *
findVert( FltFile * flt, uint32 *offset )
{
	FltVertex ** vert;

	vert = (FltVertex **) bsearch( offset,
																flt->vertPalette->verts,
																flt->vertPalette->numSearchVerts,
																sizeof( FltVertex * ),
																vertFunc );

	if( vert ) {
		*offset = (*vert)->indx;
		return *vert;
	} else	
		return 0;
}

static void *
fltVertexListPostProcess( FLTPOSTPROCESSFUNC_ARGLIST )
{
	uint32 i, offset;
	FltVertex * idx;
	FltVertexList * list = (FltVertexList *)node;
	FltUVList * uvList = 0;
	uint32 numTex = 0, checkForMultiTexture = 1;

	// check for multitexture attributes
	uvList = (FltUVList *)fltFindFirstAttrNode( node, FLTRECORD_UVLIST );

	// get number of textures
	if( uvList )
	{
		if( uvList->mask & FLTMT_HASLAYER1 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER2 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER3 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER4 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER5 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER6 )
			numTex++;
		if( uvList->mask & FLTMT_HASLAYER7 )
			numTex++;
	}

	// connect all "byte offsets" from vertex palette into this list
	// we assume that the PostProcessFunc from vertexPalette has already
	// been executed.

	for(i=0;i<list->numVerts;i++) 
	{
		offset = (uint32)list->list[i];

		idx = findVert( flt, &offset );

		if( idx ) 
		{
			if( debug )
				printf("FLT: vertex record at offset: %p found (%p)\n", list->list[i],
																																	idx );
			if( checkForMultiTexture && uvList )
			{
				FltVertex * newVert = 0;
				uint32 mtOffset, j;

				//
				// We manufacture new indices if they have multitexture coords
				// so as to simplify things later.
				//

				// alloc new vert
				newVert = (FltVertex *)fltNewNode( flt, sizeof( FltVertex ) );

				// copy old vert over
				memcpy( newVert, idx, sizeof( FltVertex ) );

				// set mask so we know how many texcoords we have
				newVert->mtMask = uvList->mask;

				mtOffset = numTex * 2 * i;
				
				for(j=0;j<numTex;j++)
				{
					newVert->mtU[j] = uvList->uvValues[mtOffset++];
					newVert->mtV[j] = uvList->uvValues[mtOffset++];
				}

				// add the vert to palette
				fltAddVertexToPalette( flt->vertPalette, newVert );

				list->list[i] = newVert;
				list->indexList[i] = flt->vertPalette->numVerts - 1;
			}
			else
			{
				list->list[i] = idx;
				list->indexList[i] = offset;
			}
		} else 
			printf("FLT: vertex record at offset: %p not found!\n", list->list[i] );
	}

	return 0;
}


static void *
fltGroup( FLTRECORDFUNC_ARGLIST )
{
	FltGroup * group;

	assert( flt );
	assert( flt->stdFile );

  if( !(flt->flags & FLTFILE_FLAGS_FIRSTPUSHSEEN) )
  {
    printf(
    "WARNING: This OpenFlight file is improperly formatted.  Hierarchy \n"
    "         records preceed the first PUSH record.\n" );
  }

	group = (FltGroup *) fltNewNode( flt, sizeof( FltGroup ) );

	fltReadBlock( flt, group->ID, 8 * sizeof(char) );

	group->relativePriority = fltReadUInt16( flt );
	group->reserved0 = fltReadUInt16( flt );
	group->flags = fltReadUInt32( flt );
	group->specialEffectID1 = fltReadUInt16( flt );
	group->specialEffectID2 = fltReadUInt16( flt );
	group->significance = fltReadUInt16( flt );
	group->layerCode = fltReadUInt8( flt );
	group->reserved1 = fltReadUInt8( flt );
	group->reserved2 = fltReadUInt32( flt );

	return group;
}

static void *
fltInstanceDefinition( FLTRECORDFUNC_ARGLIST )
{
	FltInstanceDefinition * instDef;

	assert( flt );
	assert( flt->stdFile );

	instDef = (FltInstanceDefinition *) 
													fltNewNode( flt, sizeof( FltInstanceDefinition ) );

	instDef->reserved0 = fltReadUInt16( flt );
	instDef->instance  = fltReadUInt16( flt );

	return instDef;
}

static void *
fltInstanceReference( FLTRECORDFUNC_ARGLIST )
{
	FltInstanceReference * instRef;

	assert( flt );
	assert( flt->stdFile );

	instRef = (FltInstanceReference *) 
													fltNewNode( flt, sizeof( FltInstanceReference ) );

	instRef->reserved0 = fltReadUInt16( flt );
	instRef->instance  = fltReadUInt16( flt );

	return instRef;
}

static void *
fltDOF( FLTRECORDFUNC_ARGLIST )
{
	FltDOF * dof;

	assert( flt );
	assert( flt->stdFile );

	dof = (FltDOF *) fltNewNode( flt, sizeof( FltDOF ) );

	fltReadBlock( flt, dof->ID, 8 * sizeof(char) );

	dof->reserved0 = fltReadUInt32( flt );

	dof->localOriginX = fltReadReal64( flt ) * flt->coordScale;
	dof->localOriginY = fltReadReal64( flt ) * flt->coordScale;
	dof->localOriginZ = fltReadReal64( flt ) * flt->coordScale;

	dof->localPointX = fltReadReal64( flt ) * flt->coordScale;
	dof->localPointY = fltReadReal64( flt ) * flt->coordScale;
	dof->localPointZ = fltReadReal64( flt ) * flt->coordScale;
	
	dof->localPlanePointX = fltReadReal64( flt ) * flt->coordScale;
	dof->localPlanePointY = fltReadReal64( flt ) * flt->coordScale;
	dof->localPlanePointZ = fltReadReal64( flt ) * flt->coordScale;
	
	dof->localMinZ = fltReadReal64( flt ) * flt->coordScale;
	dof->localMaxZ = fltReadReal64( flt ) * flt->coordScale;
	dof->localCurZ = fltReadReal64( flt ) * flt->coordScale;
	dof->localIncZ = fltReadReal64( flt ) * flt->coordScale;

	dof->localMinY = fltReadReal64( flt ) * flt->coordScale;
	dof->localMaxY = fltReadReal64( flt ) * flt->coordScale;
	dof->localCurY = fltReadReal64( flt ) * flt->coordScale;
	dof->localIncY = fltReadReal64( flt ) * flt->coordScale;

	dof->localMinX = fltReadReal64( flt ) * flt->coordScale;
	dof->localMaxX = fltReadReal64( flt ) * flt->coordScale;
	dof->localCurX = fltReadReal64( flt ) * flt->coordScale;
	dof->localIncX = fltReadReal64( flt ) * flt->coordScale;

	dof->localMinPitch = fltReadReal64( flt );
	dof->localMaxPitch = fltReadReal64( flt );
	dof->localCurPitch = fltReadReal64( flt );
	dof->localIncPitch = fltReadReal64( flt );

	dof->localMinRoll = fltReadReal64( flt );
	dof->localMaxRoll = fltReadReal64( flt );
	dof->localCurRoll = fltReadReal64( flt );
	dof->localIncRoll = fltReadReal64( flt );

	dof->localMinYaw = fltReadReal64( flt );
	dof->localMaxYaw = fltReadReal64( flt );
	dof->localCurYaw = fltReadReal64( flt );
	dof->localIncYaw = fltReadReal64( flt );

	dof->localMinScaleZ = fltReadReal64( flt );
	dof->localMaxScaleZ = fltReadReal64( flt );
	dof->localCurScaleZ = fltReadReal64( flt );
	dof->localIncScaleZ = fltReadReal64( flt );

	dof->localMinScaleY = fltReadReal64( flt );
	dof->localMaxScaleY = fltReadReal64( flt );
	dof->localCurScaleY = fltReadReal64( flt );
	dof->localIncScaleY = fltReadReal64( flt );

	dof->localMinScaleX = fltReadReal64( flt );
	dof->localMaxScaleX = fltReadReal64( flt );
	dof->localCurScaleX = fltReadReal64( flt );
	dof->localIncScaleX = fltReadReal64( flt );

	dof->flags = fltReadUInt32( flt );

	return dof;
}


static void *
fltObject( FLTRECORDFUNC_ARGLIST )
{
	FltObject * obj;

	assert( flt );
	assert( flt->stdFile );

	obj = (FltObject *) fltNewNode( flt, sizeof( FltObject ) );

	fltReadBlock( flt, obj->ID, 8 * sizeof(char) );

	obj->flags = fltReadUInt32( flt );
	obj->relativePriority = fltReadUInt16( flt );
	obj->transparency = fltReadUInt16( flt );
	obj->specialEffectID1 = fltReadUInt16( flt );
	obj->specialEffectID2 = fltReadUInt16( flt );
	obj->significance = fltReadUInt16( flt );
	obj->reserved0 = fltReadUInt16( flt );

	return obj;
}

void
fltAddMaterialToPalette( FltFile * flt, FltMaterial * mat )
{
	assert( flt );

	if( flt->materialPalette == 0 )
		flt->materialPalette = zcalloc( 1, sizeof( FltMaterialPalette ) ); 
	
	flt->materialPalette->material = zrealloc( flt->materialPalette->material, 
															(flt->materialPalette->numMaterials+1) * 
																									sizeof( FltMaterial * ) );

	flt->materialPalette->material[ flt->materialPalette->numMaterials++ ] = mat;
}

static void *
fltMaterial( FLTRECORDFUNC_ARGLIST )
{
	FltMaterial * mat;

	assert( flt );
	assert( flt->stdFile );

	mat = (FltMaterial *) fltNewNode( flt, sizeof( FltMaterial ) );

	mat->index = fltReadUInt32( flt );
	fltReadBlock( flt, mat->ID, 12 * sizeof(char) );
	mat->flags = fltReadUInt32( flt );

	mat->ambientRed = fltReadReal32( flt );
	mat->ambientGreen = fltReadReal32( flt );
	mat->ambientBlue = fltReadReal32( flt );

	mat->diffuseRed = fltReadReal32( flt );
	mat->diffuseGreen = fltReadReal32( flt );
	mat->diffuseBlue = fltReadReal32( flt );

	mat->specularRed = fltReadReal32( flt );
	mat->specularGreen = fltReadReal32( flt );
	mat->specularBlue = fltReadReal32( flt );

	mat->emissiveRed = fltReadReal32( flt );
	mat->emissiveGreen = fltReadReal32( flt );
	mat->emissiveBlue = fltReadReal32( flt );

	mat->shininess = fltReadReal32( flt );
	mat->alpha = fltReadReal32( flt );

	fltAddMaterialToPalette( flt, mat );

	return mat;
}

static void *
fltMaterialTable( FLTRECORDFUNC_ARGLIST )
{
	FltMaterial * mat;
  int i;

	assert( flt );
	assert( flt->stdFile );

  //
  // Material table defines 64 materials, so we read the
  // entries in, and submit them just like it was a newer
  // material palette entry.
  //

  for(i=0;i<64;i++)
  {
    uint32 junk[28];

    mat = (FltMaterial *) fltNewNode( flt, sizeof( FltMaterial ) );

    mat->ambientRed = fltReadReal32( flt );
    mat->ambientGreen = fltReadReal32( flt );
    mat->ambientBlue = fltReadReal32( flt );

    mat->diffuseRed = fltReadReal32( flt );
    mat->diffuseGreen = fltReadReal32( flt );
    mat->diffuseBlue = fltReadReal32( flt );

    mat->specularRed = fltReadReal32( flt );
    mat->specularGreen = fltReadReal32( flt );
    mat->specularBlue = fltReadReal32( flt );

    mat->emissiveRed = fltReadReal32( flt );
    mat->emissiveGreen = fltReadReal32( flt );
    mat->emissiveBlue = fltReadReal32( flt );

    mat->shininess = fltReadReal32( flt );
    mat->alpha = fltReadReal32( flt );

    mat->flags = fltReadUInt32( flt );
    mat->index = i;

    // read name
    fltReadBlock( flt, mat->ID, 12 * sizeof(char) );

    // read spares at end
    fltReadBlock( flt, junk, 28 * sizeof( uint32 ) );

    fltAddMaterialToPalette( flt, mat );
  }

	return mat;
}

void
fltAddLineStyleToPalette( FltFile * flt, FltLineStyle * style )
{
	assert( flt );

	if( flt->lineStylePalette == 0 )
		flt->lineStylePalette = zcalloc( 1, sizeof( FltLineStylePalette ) ); 
	
	flt->lineStylePalette->styles = zrealloc( flt->lineStylePalette->styles, 
															(flt->lineStylePalette->numStyles+1) * 
																									sizeof( FltLineStyle * ) );

	flt->lineStylePalette->styles[ flt->lineStylePalette->numStyles++ ] = style;
}

static void *
fltLineStyle( FLTRECORDFUNC_ARGLIST )
{
	FltLineStyle * style;

	assert( flt );
	assert( flt->stdFile );

	style = (FltLineStyle *) fltNewNode( flt, sizeof( FltLineStyle ) );

	style->index       = fltReadUInt16( flt );
	style->patternMask = fltReadUInt16( flt );
	style->lineWidth   = fltReadUInt32( flt );

	fltAddLineStyleToPalette( flt, style );

	return style;
}

void
fltAddTextureMappingToPalette( FltFile * flt, 
															 FltTextureMappingPaletteEntry * mapping )
{
	assert( flt );

	if( flt->textureMappingPalette == 0 )
		flt->textureMappingPalette = zcalloc(1, sizeof( FltTextureMappingPalette)); 
	
	flt->textureMappingPalette->mappings = 
                      zrealloc( flt->textureMappingPalette->mappings, 
															(flt->textureMappingPalette->numMappings+1) * 
																	sizeof( FltTextureMappingPaletteEntry * ) );

	flt->textureMappingPalette->
              mappings[ flt->textureMappingPalette->numMappings++ ] = mapping;
}

static void *
fltTextureMapping( FLTRECORDFUNC_ARGLIST )
{
	FltTextureMappingPaletteEntry * mapping;

	assert( flt );
	assert( flt->stdFile );

	mapping = (FltTextureMappingPaletteEntry *) 
									fltNewNode( flt, sizeof( FltTextureMappingPaletteEntry ) );

	mapping->reserved    = fltReadUInt32( flt );
	mapping->index       = fltReadUInt32( flt );
	fltReadBlock( flt, mapping->ID, 20 * sizeof(char) );
	mapping->type        = fltReadUInt32( flt );
	mapping->warped      = fltReadUInt32( flt );

  // skip everything else

	fltAddTextureMappingToPalette( flt, mapping );

	return mapping;
}

void
fltAddShaderToPalette( FltFile * flt, FltShader * shader )
{
	assert( flt );

	if( flt->shaderPalette == 0 )
		flt->shaderPalette = zcalloc( 1, sizeof( FltShaderPalette ) ); 
	
	flt->shaderPalette->shaders = zrealloc( flt->shaderPalette->shaders, 
															(flt->shaderPalette->numShaders+1) * 
																									sizeof( FltShader * ) );

	flt->shaderPalette->shaders[ flt->shaderPalette->numShaders++ ] = shader;
}

static void *
fltShader( FLTRECORDFUNC_ARGLIST )
{
	FltShader * shader;

	assert( flt );
	assert( flt->stdFile );

	shader = (FltShader *) fltNewNode( flt, sizeof( FltShader ) );

	shader->index       = fltReadUInt32( flt );
	shader->type        = fltReadUInt32( flt );
	fltReadBlock( flt, shader->ID, 1024 * sizeof(char) );

  // really only support Cg/CgFX right now
  if( shader->type == FLTSHADER_CG || shader->type == FLTSHADER_CGFX )
  {
    fltReadBlock( flt, shader->vertexProgramFile, 1024 * sizeof(char) );
    fltReadBlock( flt, shader->fragmentProgramFile, 1024 * sizeof(char) );

    shader->vertexProgramProfile   = fltReadUInt32( flt );
    shader->fragmentProgramProfile = fltReadUInt32( flt );

    fltReadBlock( flt, shader->vertexProgramEntry, 256 * sizeof(char) );
    fltReadBlock( flt, shader->fragmentProgramEntry, 256 * sizeof(char) );
  }

	fltAddShaderToPalette( flt, shader );

	return shader;
}


static void *
fltTexture( FLTRECORDFUNC_ARGLIST )
{
	FltTexture * tex;

	assert( flt );
	assert( flt->stdFile );

	tex = (FltTexture *) fltNewNode( flt, sizeof( FltTexture ) );

	fltReadBlock( flt, tex->ID, 200 * sizeof(char) );

#if 0
	{
		int i;
		printf("TEXTURE: %s\n", tex->ID ); 
		for(i=0;i<200;i++)
			printf("%02x ", tex->ID[i]&0xff );
		printf("\n");
	}
#endif

	tex->index = fltReadUInt32( flt );
	tex->xloc = fltReadUInt32( flt );
	tex->yloc = fltReadUInt32( flt );

	return tex;
}

static void *
fltRoadSegment( FLTRECORDFUNC_ARGLIST )
{
	FltRoadSegment * rs;

	assert( flt );
	assert( flt->stdFile );

	rs = (FltRoadSegment *) fltNewNode( flt, sizeof( FltRoadSegment ) );

	fltReadBlock( flt, rs->ID, 8 * sizeof(char) );

	return rs;
}

static void *
fltRoadPath( FLTRECORDFUNC_ARGLIST )
{
	FltRoadPath * rp;

	assert( flt );
	assert( flt->stdFile );

	rp = (FltRoadPath *) fltNewNode( flt, sizeof( FltRoadPath ) );

	fltReadBlock( flt, rp->ID, 8 * sizeof(char) );

	rp->reserved0 = fltReadUInt32( flt );
	
	fltReadBlock( flt, rp->pathName, 120 * sizeof( char ) );

	rp->speedLimit = fltReadReal64( flt );
	rp->noPassing  = fltReadUInt32( flt );
	rp->vertexNormalType  = fltReadUInt32( flt );

	fltReadBlock( flt, rp->spare, 480 * sizeof( char ) );

	return rp;
}

static void *
fltRoadConstruction( FLTRECORDFUNC_ARGLIST )
{
	FltRoadConstruction * rc;

	assert( flt );
	assert( flt->stdFile );

	rc = (FltRoadConstruction *) fltNewNode( flt, sizeof( FltRoadConstruction ) );

	fltReadBlock( flt, rc->ID, 8 * sizeof(char) );

	rc->reserved0 = fltReadUInt32( flt );
	rc->roadType  = fltReadUInt32( flt );
	rc->roadtoolsVersion  = fltReadUInt32( flt );

	rc->entryX = fltReadReal64( flt ) * flt->coordScale;
	rc->entryY = fltReadReal64( flt ) * flt->coordScale;
	rc->entryZ = fltReadReal64( flt ) * flt->coordScale;

	rc->alignmentX = fltReadReal64( flt ) * flt->coordScale;
	rc->alignmentY = fltReadReal64( flt ) * flt->coordScale;
	rc->alignmentZ = fltReadReal64( flt ) * flt->coordScale;

	rc->exitX = fltReadReal64( flt ) * flt->coordScale;
	rc->exitY = fltReadReal64( flt ) * flt->coordScale;
	rc->exitZ = fltReadReal64( flt ) * flt->coordScale;

	rc->arcRadius = fltReadReal64( flt ) * flt->coordScale;
	rc->entrySpiralLength = fltReadReal64( flt ) * flt->coordScale;
	rc->exitSpiralLength = fltReadReal64( flt ) * flt->coordScale;
	rc->superelevation = fltReadReal64( flt );

	rc->spiralType = fltReadUInt32( flt );
	rc->verticalParabolaFlag = fltReadUInt32( flt );

	rc->verticalCurveLength = fltReadReal64( flt ) * flt->coordScale;
	rc->minimumCurveLength = fltReadReal64( flt ) * flt->coordScale;

	rc->entrySlope = fltReadReal64( flt ) * flt->coordScale;
	rc->exitSlope = fltReadReal64( flt ) * flt->coordScale;

	return rc;
}


static void *
fltBSP( FLTRECORDFUNC_ARGLIST )
{
	FltBSP * bsp;

	assert( flt );
	assert( flt->stdFile );

	bsp = (FltBSP *) fltNewNode( flt, sizeof( FltBSP ) );

	fltReadBlock( flt, bsp->ID, 8 * sizeof(char) );

	bsp->reserved0 = fltReadUInt32( flt );
	bsp->coefA = fltReadReal64( flt );
	bsp->coefB = fltReadReal64( flt );
	bsp->coefC = fltReadReal64( flt );
	bsp->coefD = fltReadReal64( flt );

	return bsp;
}

static void *
fltFace( FLTRECORDFUNC_ARGLIST )
{
	FltFace * face;

	assert( flt );
	assert( flt->stdFile );

	face = (FltFace *) fltNewNode( flt, sizeof( FltFace ) );

	fltReadBlock( flt, face->ID, 8 * sizeof(char) );

	face->irColorCode = fltReadUInt32( flt );
	face->relativePriority = fltReadUInt16( flt );
	face->drawType = fltReadUInt8( flt );
	face->textureWhite = fltReadUInt8( flt );
	face->colorNameIndex = fltReadUInt16( flt );
	face->alternateColorNameIndex = fltReadUInt16( flt );
	face->reserved0 = fltReadUInt8( flt );
	face->billboardFlags = fltReadUInt8( flt );
	face->detailTexturePatternIndex = fltReadInt16( flt );
	face->texturePatternIndex = fltReadInt16( flt );
	face->materialIndex = fltReadInt16( flt );
	face->surfaceMaterialCode = fltReadUInt16( flt );
	face->featureID = fltReadUInt16( flt );
	face->irMaterialCode = fltReadUInt32( flt );
	face->transparency = fltReadUInt16( flt );
	face->LODGenerationControl = fltReadUInt8( flt );
	face->lineStyleIndex = fltReadUInt8( flt );
	face->miscFlags = fltReadUInt32( flt );
  
  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
  {
    // v14 does not have hidden flag
    face->miscFlags &= ~FLTFACEMF_HIDDEN;
  }

	face->lightMode = fltReadUInt8( flt );
	face->reserved1 = fltReadUInt8( flt );
	face->reserved2 = fltReadUInt16( flt );
	face->reserved3 = fltReadUInt32( flt );
	face->packedColorPrimary = fltReadUInt32( flt );
	face->packedColorAlternate = fltReadUInt32( flt );

  if( FLT_IS_V14X( flt ) || FLT_IS_V13X( flt ) )
  {
    // these are not present
    face->textureMappingIndex = -1;
    face->reserved4 = 0;
    face->primaryColorIndex = face->colorNameIndex;
    face->alternateColorIndex = face->alternateColorNameIndex;
    face->reserved5 = 0;
    face->shaderIndex = -1;
  }
  else
  {
    face->textureMappingIndex = fltReadUInt16( flt );
    face->reserved4 = fltReadUInt16( flt );
    face->primaryColorIndex = fltReadUInt32( flt );
    face->alternateColorIndex = fltReadUInt32( flt );
    face->reserved5 = fltReadUInt16( flt );
    face->shaderIndex = fltReadUInt16( flt );
  }

  // set this just in case of old files
  if( !FLT_IS_V16X( flt ) )
    face->shaderIndex = -1;

	return face;
}

static void *
fltMesh( FLTRECORDFUNC_ARGLIST )
{
	FltMesh * mesh;

	assert( flt );
	assert( flt->stdFile );

	mesh = (FltMesh *) fltNewNode( flt, sizeof( FltMesh ) );

	fltReadBlock( flt, mesh->ID, 8 * sizeof(char) );

	mesh->irColorCode = fltReadInt32( flt );
	mesh->relativePriority = fltReadInt16( flt );
	mesh->drawType = fltReadInt8( flt );
	mesh->textureWhite = fltReadUInt8( flt );
	mesh->colorNameIndex = fltReadUInt16( flt );
	mesh->alternateColorNameIndex = fltReadUInt16( flt );
	mesh->reserved0 = fltReadUInt8( flt );
	mesh->billboardFlags = fltReadUInt8( flt );
	mesh->detailTexturePatternIndex = fltReadInt16( flt );
	mesh->texturePatternIndex = fltReadInt16( flt );
	mesh->materialIndex = fltReadInt16( flt );
	mesh->surfaceMaterialCode = fltReadUInt16( flt );
	mesh->featureID = fltReadUInt16( flt );
	mesh->irMaterialCode = fltReadUInt32( flt );
	mesh->transparency = fltReadUInt16( flt );
	mesh->LODGenerationControl = fltReadUInt8( flt );
	mesh->lineStyleIndex = fltReadUInt8( flt );
	mesh->miscFlags = fltReadUInt32( flt );
	mesh->lightMode = fltReadUInt8( flt );
	mesh->reserved1 = fltReadUInt8( flt );
	mesh->reserved2 = fltReadUInt16( flt );
	mesh->reserved3 = fltReadUInt32( flt );
	mesh->packedColorPrimary = fltReadUInt32( flt );
	mesh->packedColorAlternate = fltReadUInt32( flt );
	mesh->textureMappingIndex = fltReadUInt16( flt );
	mesh->reserved4 = fltReadUInt16( flt );
	mesh->primaryColorIndex = fltReadUInt32( flt );
	mesh->alternateColorIndex = fltReadUInt32( flt );
	mesh->reserved5 = fltReadUInt16( flt );
	mesh->shaderIndex = fltReadUInt16( flt );

  // set this just in case of old files
  if( flt->header->formatRevision < 1600 )
    mesh->shaderIndex = -1;

	return mesh;
}

static void *
fltMeshPrimitive( FLTRECORDFUNC_ARGLIST )
{
	FltMeshPrimitive * meshPrim;
	unsigned int i;

	assert( flt );
	assert( flt->stdFile );

	meshPrim = (FltMeshPrimitive *) fltNewNode( flt, sizeof( FltMeshPrimitive ) );

	meshPrim->primitiveType = fltReadUInt16( flt );
	meshPrim->vertIndexLength = fltReadUInt16( flt );
	meshPrim->numVerts = fltReadInt32( flt );

	// free me on the way out!
	meshPrim->indices = zcalloc( meshPrim->numVerts, sizeof( uint32 ) );

	switch( meshPrim->vertIndexLength )
	{
		case 1:
			for(i=0;i<meshPrim->numVerts;i++)
				meshPrim->indices[i] = (uint32) fltReadUInt8( flt );
		break;

		case 2:
			for(i=0;i<meshPrim->numVerts;i++)
				meshPrim->indices[i] = (uint32) fltReadUInt16( flt );
		break;

		case 4:
			for(i=0;i<meshPrim->numVerts;i++)
				meshPrim->indices[i] = fltReadUInt32( flt );
		break;

		default:
			break;
	}

	return meshPrim;
}

static void *
fltLocalVertexPool( FLTRECORDFUNC_ARGLIST )
{
	FltLocalVertexPool * lvp;
	unsigned int i;

	assert( flt );
	assert( flt->stdFile );

	lvp = (FltLocalVertexPool *) fltNewNode( flt, 
																		sizeof( FltLocalVertexPool ) );

	lvp->numVerts = fltReadUInt32( flt );
	lvp->attrMask = fltReadUInt32( flt );

	// free me on the way out!
	// Note that this is wasteful
	lvp->entries = zcalloc( lvp->numVerts, sizeof( FltLVPEntry ) );

	for(i=0;i<lvp->numVerts;i++)
	{
		FltLVPEntry * e = &lvp->entries[i];

		if( lvp->attrMask & FLTLVPATTR_POSITION )
		{
			e->x = fltReadReal64( flt ) * flt->coordScale;
			e->y = fltReadReal64( flt ) * flt->coordScale;
			e->z = fltReadReal64( flt ) * flt->coordScale;
		}

		if( lvp->attrMask & (FLTLVPATTR_COLORINDEX | FLTLVPATTR_PACKEDCOLOR) )
			e->color = fltReadUInt32( flt );

		if( lvp->attrMask & FLTLVPATTR_NORMAL )
		{
			e->i = fltReadReal32( flt );
			e->j = fltReadReal32( flt );
			e->k = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV0 )
		{
			e->u0 = fltReadReal32( flt );
			e->v0 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV1 )
		{
			e->u1 = fltReadReal32( flt );
			e->v1 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV2 )
		{
			e->u2 = fltReadReal32( flt );
			e->v2 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV3 )
		{
			e->u3 = fltReadReal32( flt );
			e->v3 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV4 )
		{
			e->u4 = fltReadReal32( flt );
			e->v4 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV5 )
		{
			e->u5 = fltReadReal32( flt );
			e->v5 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV6 )
		{
			e->u6 = fltReadReal32( flt );
			e->v6 = fltReadReal32( flt );
		}

		if( lvp->attrMask & FLTLVPATTR_UV7 )
		{
			e->u7 = fltReadReal32( flt );
			e->v7 = fltReadReal32( flt );
		}
	}

	return lvp;
}

static void *
fltUVList( FLTRECORDFUNC_ARGLIST )
{
	FltUVList * mt;

	assert( flt );
	assert( flt->stdFile );

	mt = (FltUVList *) fltNewNode( flt, 
																		sizeof( FltUVList ) );

	// num floats is recLength - 4 (mask) / 4 (floats)

	mt->mask = fltReadUInt32( flt );
	mt->numValues = (recLength - 4) / sizeof(real32);

	mt->uvValues = (real32 *)zmalloc( sizeof(real32) * mt->numValues );

	if( mt->uvValues )
	{
		unsigned int i;

		for(i=0;i<mt->numValues;i++)
			mt->uvValues[i] = fltReadReal32( flt );
	}

	return mt;
}

static void *
fltMultiTexture( FLTRECORDFUNC_ARGLIST )
{
	FltMultiTexture * mt;

	assert( flt );
	assert( flt->stdFile );

	mt = (FltMultiTexture *) fltNewNode( flt, 
																		sizeof( FltMultiTexture ) );

	mt->mask = fltReadUInt32( flt );

	if( mt->mask & FLTMT_HASLAYER1 )
	{
		mt->layer[0].index   = fltReadUInt16( flt );
		mt->layer[0].effect  = fltReadUInt16( flt );
		mt->layer[0].mapping = fltReadUInt16( flt );
		mt->layer[0].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER2 )
	{
		mt->layer[1].index   = fltReadUInt16( flt );
		mt->layer[1].effect  = fltReadUInt16( flt );
		mt->layer[1].mapping = fltReadUInt16( flt );
		mt->layer[1].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER3 )
	{
		mt->layer[2].index   = fltReadUInt16( flt );
		mt->layer[2].effect  = fltReadUInt16( flt );
		mt->layer[2].mapping = fltReadUInt16( flt );
		mt->layer[2].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER4 )
	{
		mt->layer[3].index   = fltReadUInt16( flt );
		mt->layer[3].effect  = fltReadUInt16( flt );
		mt->layer[3].mapping = fltReadUInt16( flt );
		mt->layer[3].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER5 )
	{
		mt->layer[4].index   = fltReadUInt16( flt );
		mt->layer[4].effect  = fltReadUInt16( flt );
		mt->layer[4].mapping = fltReadUInt16( flt );
		mt->layer[4].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER6 )
	{
		mt->layer[5].index   = fltReadUInt16( flt );
		mt->layer[5].effect  = fltReadUInt16( flt );
		mt->layer[5].mapping = fltReadUInt16( flt );
		mt->layer[5].data    = fltReadUInt16( flt );
	}
	if( mt->mask & FLTMT_HASLAYER7 )
	{
		mt->layer[6].index   = fltReadUInt16( flt );
		mt->layer[6].effect  = fltReadUInt16( flt );
		mt->layer[6].mapping = fltReadUInt16( flt );
		mt->layer[6].data    = fltReadUInt16( flt );
	}

	return mt;
}


// record types
static FltRecord FlightRecords[] = {
	{ 1, "Header", fltHeader, FltHeaderEntryName }, 
	{ 2, "Group", fltGroup, FltGroupEntryName },
	{ 4, "Object", fltObject, FltObjectEntryName }, 
	{ 5, "Face", fltFace, FltFaceEntryName }, 
	{ 10, "Push Level", fltPushLevel }, 
	{ 11, "Pop Level", fltPopLevel }, 
	{ 14, "Degree of Freedom", fltDOF, FltDOFEntryName }, 
	{ 19, "Push Subface", fltPushLevel }, 
	{ 20, "Pop Subface", fltPopLevel }, 
	{ 21, "Push Extension", fltPushExtension }, 
	{ 22, "Pop Extension", fltPopExtension }, 
	{ 23, "Continuation", NULL }, 
	{ 31, "Text Comment", fltComment, FltCommentEntryName }, 
	{ 32, "Color Palette", fltColorPalette, FltColorPaletteEntryName }, 
	{ 33, "Long ID", fltLongID, FltLongIDEntryName }, 
	{ 49, "Transformation Matrix", fltMatrix, FltMatrixEntryName }, 
	{ 50, "Vector", NULL }, 
	{ 52, "MultiTexture", fltMultiTexture, FltMultiTextureEntryName }, 
	{ 53, "UV List", fltUVList, FltUVListEntryName }, 
	{ 55, "Binary Separating Plane", fltBSP, FltBSPEntryName }, 
	{ 60, "Replicate", fltReplicate, FltReplicateEntryName }, 
	{ 61, "Instance Reference", fltInstanceReference, 
															FltInstanceReferenceEntryName },
	{ 62, "Instance Definition", fltInstanceDefinition,
															 FltInstanceDefinitionEntryName }, 
	{ 63, "External Reference", fltExternalReference, 
																					FltExternalReferenceEntryName,
																					fltExternalReferencePostProcess }, 
	{ 64, "Texture Reference", fltTexture, FltTextureEntryName }, 
	{ 66, "Material Table", fltMaterialTable, FltMaterialTableEntryName }, 
	{ 67, "Vertex Palette", fltVertexPalette, FltVertexPaletteEntryName, 
																									fltVertexPalettePostProcess}, 
	{ 68, "Vertex with Color", fltVertexWithColor, FltVertexEntryName }, 
	{ 69, "Vertex with Color and Normal", fltVertexWithColorNormal, 
																												FltVertexEntryName }, 
	{ 70, "Vertex with Color, Normal and UV", fltVertexWithColorNormalTexture,
																											FltVertexEntryName }, 
	{ 71, "Vertex with Color and UV", fltVertexWithColorTexture,
																											FltVertexEntryName }, 
	{ 72, "Vertex List", fltVertexList, FltVertexListEntryName,
																										fltVertexListPostProcess},
	{ 73, "Level of Detail", fltLOD, FltLODEntryName }, 
	{ 74, "Bounding Box", NULL }, 
	{ 76, "Rotate about Edge", NULL }, 
	{ 78, "Translate", fltTranslate, FltTranslateEntryName }, 
	{ 79, "Scale (Nonuniform)", fltNonuniformScale, FltNonuniformScaleEntryName}, 
	{ 80, "Rotate about Point", NULL }, 
	{ 81, "Rotate and/or Scale to", NULL }, 
	{ 82, "Put Transform", NULL }, 
	{ 83, "Eyepoint and Trackplane", NULL }, 
	{ 84, "Mesh", fltMesh, FltMeshEntryName }, 
	{ 85, "Local Vertex Pool", fltLocalVertexPool, FltLocalVertexPoolEntryName }, 
	{ 86, "Mesh Primitive", fltMeshPrimitive, FltMeshPrimitiveEntryName }, 
	{ 87, "Road Segment", fltRoadSegment, FltRoadSegmentEntryName }, 
	{ 88, "Road Zone", NULL }, 
	{ 89, "Morph Vertex List", NULL }, 
	{ 90, "Behavior (Linkage) Palette", NULL }, 
	{ 91, "Sound", NULL },
	{ 92, "Road Path", fltRoadPath, FltRoadPathEntryName }, 
	{ 93, "Sound Palette", NULL }, 
	{ 94, "General Matrix", fltGeneralMatrix, FltGeneralMatrixEntryName }, 
	{ 95, "Text", NULL },
	{ 96, "Switch", fltSwitch, FltSwitchEntryName }, 
	{ 97, "Line Style", fltLineStyle, FltLineStyleEntryName }, 
	{ 98, "Clip Region", NULL }, 
	{ 100, "Extension", NULL }, 
	{ 101, "Light Source", fltLightSource, FltLightSourceEntryName }, 
	{ 102, "Light Source Palette", fltLightSourcePaletteEntry, 
																		FltLightSourcePaletteEntryName }, 
	{ 103, "Reserved", NULL }, 
	{ 104, "Reserved", NULL }, 
	{ 105, "Bounding Sphere", NULL }, 
	{ 106, "Bounding Cylinder", NULL }, 
	{ 107, "Reserved", NULL }, 
	{ 108, "Bounding Volume Center", NULL }, 
	{ 109, "Bounding Volume Orientation", NULL }, 
	{ 111, "Light Point", fltLightPoint, FltLightPointEntryName }, 
	{ 112, "Texture Mapping Palette", fltTextureMapping, 
																					FltTextureMappingPaletteEntryName }, 
	{ 113, "Material Palette", fltMaterial, FltMaterialEntryName }, 
	{ 114, "Color Name Palette", NULL }, 
	{ 115, "Continuously Adaptive", NULL }, 
	{ 116, "CAT Data", NULL }, 
	{ 117, "Reserved", NULL }, 
	{ 118, "Reserved", NULL }, 
	{ 119, "Reserved", NULL },
	{ 120, "Reserved", NULL }, 
	{ 121, "Reserved", NULL }, 
	{ 122, "Push Attribute", NULL }, 
	{ 123, "Pop Attribute", NULL }, 
	{ 124, "Reserved", NULL }, 
	{ 125, "Adaptive Attribute", NULL },
	{ 126, "Curve Node", NULL },
	{ 127, "Road Construction Node", fltRoadConstruction, 
																								FltRoadConstructionEntryName },
	{ 130, "Indexed Light Point", fltLightPoint, FltLightPointEntryName }, 
	{ 131, "Light Point System", fltLightPointSystem, 
                                                FltLightPointSystemEntryName }, 
	{ 133, "Shader Palette Node", fltShader, FltShaderEntryName }
};
#define FLTRECORDS_NUM ( sizeof( FlightRecords ) / sizeof( FltRecord ) )

// obsolete
static FltRecord ObsoleteRecords[] = {
	{ 3, "Level of Detail", NULL }, 
	{ 6, "Vertex with ID", NULL }, 
	{ 7, "ShortVertex", NULL }, 
	{ 8, "Vertex with Color", NULL }, 
	{ 9, "Vertex with Color and Normal", NULL }, 
	{ 12, "Translate", NULL }, 
	{ 13, "Degree of Freedom", NULL }, 
	{ 16, "Instance Reference", NULL }, 
	{ 17, "Instance Definition", NULL }, 
	{ 40, "Translate", NULL }, 
	{ 41, "Rotate about Point", NULL }, 
	{ 42, "Rotate about Edge", NULL }, 
	{ 43, "Scale", NULL }, 
	{ 44, "Translate", NULL }, 
	{ 45, "Scale (Nonuniform)", NULL }, 
	{ 46, "Rotate about Point", NULL }, 
	{ 47, "Rotate and/or Scale to Point", NULL }, 
	{ 48, "Put Transform", NULL }, 
	{ 51, "Bounding Box", NULL }, 
	{ 65, "Eyepoint Palette", NULL }, 
//	{ 66, "Material Palette", NULL },  Obsolete, but supported.
	{ 77, "Scale", NULL }, 
	{ 110, "Histogram Bounding Volume", NULL }, 
};
#define OBSOLETERECORDS_NUM ( sizeof( ObsoleteRecords ) / sizeof( FltRecord ) )

static int
compareFunc( const void * xkey, const void * member )
{
	FltRecord * rec = (FltRecord *)member;
	uint16 key = *(uint16 *)xkey;

	if( key > rec->opcode )
		return 1;
	else if( key < rec->opcode )
		return -1;
	else
		return 0;
}

int
fltRegisterRecordUserCallback( uint16 record, FltRecordUserCallback cb, 
																														void * userData )
{
	FltRecord * rec;

	rec = (FltRecord *) bsearch(	&record, 
																FlightRecords, 
																FLTRECORDS_NUM,
																sizeof( FltRecord ),
																compareFunc );

	if( rec ) {
		rec->userFunc = cb;
		rec->userData = userData;
		return 1;
	} else {
		printf("fltRegisterRecordUserCallback: unknown record type: %d\n", record);
		return 0;
	}

}

FltRecord *
fltRecordGetDefinition( uint16 type )
{
	FltRecord * rec;

	rec = (FltRecord *) bsearch(	&type, 
																FlightRecords, 
																FLTRECORDS_NUM,
																sizeof( FltRecord ),
																compareFunc );

	// may be null
	return rec;
}

static void
fltPostProcess( FltFile * flt )
{
	uint32 i;
	FltRecord * rec;

	if( debug )
		printf("Post Processing Begin\n");

	for(i=0;i<flt->numNodes;i++) {
		rec = fltRecordGetDefinition( (uint16)flt->allNodes[i]->type );
		if( rec && rec->postProcessFunc )
			rec->postProcessFunc( flt, flt->allNodes[i] );
	}

	if( debug )
		printf("Post Processing End\n");
}

int
fltParse( FltFile * flt, uint32 skip )
{
	uint16 length, type;
	FltRecord * rec;

	while( fltReadRecordAttr( flt, &type, &length ) ) 
  {
		rec = (FltRecord *) bsearch(	&type, 
																	FlightRecords, 
																	FLTRECORDS_NUM,
																	sizeof( FltRecord ),
																	compareFunc );

		if( rec ) 
    { 
			if( rec->func ) 
      {
				FltNode * node;

				if( debug )
					printf("supported node type: %s len: %d\n", rec->name, length );

				// if we are skipping, then continue
				if( skip ) {
					fltSkipRecord( flt, length );
					continue;
				}

				// call the internal function for this record type
				node = (FltNode *) rec->func( flt, length );

				// set state on the returned node
        // if we are inside an extension, we skip everything as
        // the interior may contain nodes like LongID and whatnot
				if( node  ) 
        {
          if( flt->ignoreExtension )
          {
            continue;
          }

					node->type = type;
					node->length = length;
					node->parent = FLT_GETPARENT( flt );

					// note: we want the byte-offset for the start of record
					node->byteOffset = flt->byteOffset - length - 4;

					if( FLTNODE_ISATTRIBUTE( node->type ) ) {

						//
						// is an attribute/ancillary record
						//

						// if lastNode available, apply attr
						if( debug )
							printf("ancillary node: %s len: %d parent: %s\n", rec->name, 
											length, fltSafeNodeName( flt->lastNode ));

						if( flt->lastNode ) {

							// set up cross linking

							node->prev = NULL;
							node->next = flt->lastNode->attr;

							if( node->next )
								node->next->prev = node;

							flt->lastNode->attr = node;

							node->treeDepth = flt->treeDepth;

						} else {

							if( 1 || debug )
								printf("hanging attribute of type \"%s\" length: %d, not"
												" attached to parent node..\n", rec->name, length);

						}

						// 
						// we do not reset flt->lastNode here so that we can catch all
						// additional attributes that follow
						//

					} else {

						//
						// was not an attribute
						//

						// if node has a parent, add this one as a child
						if( node->parent )
							fltNodeAddChild( node->parent, node );

						// set up cross linking
						// popping sets lastNode to be the parent node

						node->next = NULL;
						node->prev = flt->lastNode;

						if( node->prev )
							node->prev->next = node;

						if( FLTNODE_CANPARENT( node->type ) )
							flt->lastParent = node;

						node->treeDepth = flt->treeDepth;

						// can be null, which is OK
						flt->lastNode = node;
					}
				}

				// call user func, if available
				if( rec->userFunc )
					rec->userFunc( flt, (void *)node, rec->userData );

			} else {
				if( debug )
					printf("unsupported node of type \"%s\" length: %d\n", rec->name, 
																																			length);
				fltSkipRecord( flt, length );
			}
		} else { 
			rec = (FltRecord *) bsearch(	&type, 
																		ObsoleteRecords, 
																		OBSOLETERECORDS_NUM,
																		sizeof( FltRecord ),
																		compareFunc );

			if( rec )
				printf("obsolete node of type \"%s\" found.\n", rec->name ); 
			else
				printf("Found undefined node: %d, length: %d\n", type, length );

			fltSkipRecord( flt, length );
		}

	}

	// run post-processing step
	fltPostProcess( flt );

	return 1;
}



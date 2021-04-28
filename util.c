
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include "flt.h"
#include "zmalloc.h"

FltType
fltStringToType( char * string )
{
    FltType t = FLT_TYPE_NULL;

    if( strchr( string, '[' ) )
	t |= FLT_ARRAY;

    if( strstr( string, "**" ) )
	t |= FLT_POINTER2;
    else if( strchr( string, '*' ) )
	t |= FLT_POINTER;

    if( !strncmp( string, "char", strlen( "char" ) ) )
	return t | FLT_CHAR;

    if( !strncmp( string, "uint8", strlen( "uint8" ) ) )
	return t | FLT_UINT8;

    if( !strncmp( string, "uint16", strlen( "uint16" ) ) )
	return t | FLT_UINT16;

    if( !strncmp( string, "uint32", strlen( "uint32 " ) ) )
	return t | FLT_UINT32;

    if( !strncmp( string, "int8", strlen( "int8" ) ) )
	return t | FLT_INT8;

    if( !strncmp( string, "int16", strlen( "int16" ) ) )
	return t | FLT_INT16;

    if( !strncmp( string, "int32", strlen( "int32 " ) ) )
	return t | FLT_INT32;

    if( !strncmp( string, "real32", strlen( "real32" ) ) )
	return t | FLT_REAL32;

    if( !strncmp( string, "real64", strlen( "real64 " ) ) )
	return t | FLT_REAL64;

    return t | FLT_UNKNOWN;
}

FltNode *
fltFindFirstAttrNode( FltNode * node, uint32 type )
{
    if (node && node->attr)
    {
	FltNode * find_node = node->attr;

	while (find_node)
	{
	    if (find_node->type == type)
		return find_node;

	    find_node = find_node->next;
	}
    }

    return 0;
}

//
// Ease of use functions.  Definintely not the most optimal.
//
int
fltGetMultiTextureCount( FltVertexList * vlist )
{
    FltUVList * list = (FltUVList*)fltFindFirstAttrNode(
	    (FltNode*)vlist, FLTRECORD_UVLIST);
    int count = 0;

    if( list )
    {
	if( list->mask & FLTMT_HASLAYER1 )
	    count++;
	if( list->mask & FLTMT_HASLAYER2 )
	    count++;
	if( list->mask & FLTMT_HASLAYER3 )
	    count++;
	if( list->mask & FLTMT_HASLAYER4 )
	    count++;
	if( list->mask & FLTMT_HASLAYER5 )
	    count++;
	if( list->mask & FLTMT_HASLAYER6 )
	    count++;
	if( list->mask & FLTMT_HASLAYER7 )
	    count++;
    }

    return count;
}

float *
fltGetMultiTextureCoords( FltVertexList * vlist, int idx )
{
    FltUVList * list = (FltUVList*)fltFindFirstAttrNode(
	    (FltNode*)vlist, FLTRECORD_UVLIST);

    if( list )
    {
	int count = fltGetMultiTextureCount( vlist );

	if( count )
	{
	    int offset = idx * 2 * count;

	    return &list->uvValues[ offset ];
	}
    }

    return 0;
}

int
fltArrayLength( char * string )
{
    char * start = strchr( string, '[' );
    char buf[32];
    char * bufptr = buf;

    if( start )
    {
	start++;
	while( *start != 0 && *start != ']' )
	    *bufptr++ = *start++;

	*bufptr = 0;

	return atoi( buf );
    }

    return 0;
}

void
fltRecordEntryNameToString( char * string, FltNode * node,
	FltRecordEntryName * e )
{
    // null string
    string[0] = 0;

    switch( fltStringToType( e->type ) ) {
	case FLT_UINT8:
	    sprintf( string, "%s: (%s) %02x", e->name, e->type,
		    *((uint8 *) ((char *)node +
			    e->offset ) ) & 0xff );
	    break;

	case FLT_UINT16:
	    sprintf( string, "%s: (%s) %04x", e->name, e->type,
		    *((uint16 *) ((char *)node +
			    e->offset ) ) & 0xffff);
	    break;

	case FLT_UINT32:
	    sprintf( string, "%s: (%s) %08x", e->name, e->type,
		    *((uint32 *) ((char *)node +
			    e->offset ) ) );
	    break;

	case FLT_INT8:
	    sprintf( string, "%s: (%s) %02x", e->name, e->type,
		    *((int8 *) ((char *)node +
			    e->offset ) ) & 0xff );
	    break;

	case FLT_INT16:
	    sprintf( string, "%s: (%s) %04x", e->name, e->type,
		    *((int16 *) ((char *)node +
			    e->offset ) ) & 0xffff );
	    break;
	case FLT_INT32:
	    sprintf( string, "%s: (%s) %08x", e->name, e->type,
		    *((int32 *) ((char *)node +
			    e->offset ) ) );
	    break;

	case FLT_REAL64:
	    sprintf( string, "%s: (%s) %f", e->name, e->type,
		    *((real64 *) ((char *)node +
			    e->offset ) ) );
	    break;

	case FLT_REAL32:
	    sprintf( string, "%s: (%s) %f", e->name, e->type,
		    *((real32 *) ((char *)node +
			    e->offset ) ) );
	    break;

	case FLT_REAL32 | FLT_ARRAY:
	    {
		real32 * rptr = (real32 *)((char *)node + e->offset );
		int len = fltArrayLength( e->type );
		char tbuf[80];

		sprintf( string, "%s: (%s)\n [", e->name, e->type );

		if( len == 16 )
		{
		    /* assume matrix */
		    sprintf(tbuf, " %f %f %f %f\n", rptr[0], rptr[1], rptr[2], rptr[3]);
		    strcat( string, tbuf );
		    sprintf(tbuf, " %f %f %f %f\n", rptr[4], rptr[5], rptr[6], rptr[7]);
		    strcat( string, tbuf );
		    sprintf(tbuf, " %f %f %f %f\n", rptr[8], rptr[9], rptr[10], rptr[11]);
		    strcat( string, tbuf );
		    sprintf(tbuf, " %f %f %f %f", rptr[12], rptr[13], rptr[14], rptr[15]);
		    strcat( string, tbuf );
		}
		else
		{
		    while( len-- )
		    {
			sprintf(tbuf, " %f", *rptr++ );
			strcat( string, tbuf );
		    }
		}

		strcat( string, " ]" );

		break;
	    }


	case FLT_CHAR | FLT_ARRAY:
	    sprintf( string, "%s: (%s) \"%s\"", e->name, e->type,
		    ((char *) ((char *)node +
			e->offset ) ) );
	    break;

	default:
	    sprintf( string, "%s: (%s)", e->name, e->type);
	    break;
    }
}

/* returns swX swY neX neY */
void
fltGetExtentsLatLon( FltFile * flt, real64 extents[4] )
{
    extents[0] = flt->header->swDBLatitude;
    extents[1] = flt->header->swDBLongitude;
    extents[2] = flt->header->neDBLatitude;
    extents[3] = flt->header->neDBLongitude;
}

void
fltGetOriginLatLon( FltFile * flt, real64 origin[2] )
{
    origin[0] = flt->header->originDBLatitude;
    origin[1] = flt->header->originDBLongitude;
}

/* returns swX swY neX neY zMin zMax*/
void
fltGetExtents( FltFile * flt, real64 extents[6] )
{
    memcpy( extents, flt->extents, sizeof( real64 ) * 4 );
}

uint32
fltGetProjection( FltFile * flt )
{
    return flt->header->projectionType;
}

uint32
fltGetEllipsoidModel( FltFile * flt )
{
    return flt->header->earthEllipsoidModel;
}

void
fltGetUTMZoneAndHemisphere( FltFile * flt, int * zone, int * hemisphere )
{
    *zone = (flt->header->utmZone > 0)?
	flt->header->utmZone : -flt->header->utmZone;

    // if zone > 0, northern
    *hemisphere = (flt->header->utmZone > 0)?0:1;
}

void
fltDumpNode( FltNode * node )
{
    char string[300];
    FltRecord * rec;
    int x = 0;

    if( (rec = fltRecordGetDefinition( (uint16) node->type )) ) {
	while( rec->entry[x].type ) {
	    fltRecordEntryNameToString( string, node, &rec->entry[x] );
	    printf("%s\n", string);
	    x ++;
	}
    }
}

uint32
fltLookupColor( FltFile * flt, uint32 colorIndex, real32 *r, real32 *g,
	real32 *b, real32 *a )
{
    uint32 idx = colorIndex / 128;
    real32 intensity = (float)(colorIndex % 128) / 127.0f;

    if( !flt->colorPalette )
	return 0;

    if( idx > 1023 ) {
	// return white

	*r = 1.0;
	*g = 1.0;
	*b = 1.0;
	*a = 1.0;

	return 1;
    }

    *r = (real32)( flt->colorPalette->color[ idx ] & 0x000000ff ) / 255.0f;
    *g = (real32)((flt->colorPalette->color[ idx ] >> 8) & 0x000000ff) / 255.0f;
    *b = (real32)((flt->colorPalette->color[ idx ] >> 16) & 0x000000ff) / 255.0f;
    *a = (real32)((flt->colorPalette->color[ idx ] >> 24) & 0x000000ff) / 255.0f;

    *r = *r * intensity;
    *g = *g * intensity;
    *b = *b * intensity;
    *a = *a * intensity;

    return 1;
}

FltLightSourcePaletteEntry *
fltLookupLightSource( FltFile * flt, uint32 lightIndex )
{
    uint32 i;

    if( !flt->lightSourcePalette )
	return 0;

    for(i=0;i<flt->lightSourcePalette->numLights;i++)
    {
	if( flt->lightSourcePalette->lights[i]->index == lightIndex )
	    return flt->lightSourcePalette->lights[i];
    }

    return 0;
}

FltLineStyle *
fltLookupLineStyle( FltFile * flt, uint16 idx )
{
    uint32 i;

    if( !flt->lineStylePalette )
	return 0;

    for(i=0;i<flt->lineStylePalette->numStyles;i++)
    {
	if( flt->lineStylePalette->styles[i]->index == idx )
	    return flt->lineStylePalette->styles[i];
    }

    return 0;
}

FltShader *
fltLookupShader( FltFile * flt, uint32 idx )
{
    uint32 i;

    if( !flt->shaderPalette )
	return 0;

    for(i=0;i<flt->shaderPalette->numShaders;i++)
    {
	if( flt->shaderPalette->shaders[i]->index == idx )
	    return flt->shaderPalette->shaders[i];
    }

    return 0;
}

FltTextureMappingPaletteEntry *
fltLookupTextureMapping( FltFile * flt, uint32 idx )
{
    uint32 i;

    if( !flt->textureMappingPalette )
	return 0;

    for(i=0;i<flt->textureMappingPalette->numMappings;i++)
    {
	if( flt->textureMappingPalette->mappings[i]->index == idx )
	    return flt->textureMappingPalette->mappings[i];
    }

    return 0;
}

//
// Find the FltTexture Record associated with an index
//
FltTexture *
fltLookupTexture( FltFile * flt, uint16 idx )
{
    uint32 i;

    for (i=0;i<flt->numNodes;i++) {

	if (flt->allNodes[i]->type == FLTRECORD_TEXTURE) {
	    if (((FltTexture *)flt->allNodes[i])->index == idx)
		return (FltTexture *)flt->allNodes[i];
	}

    }

    return 0;
}

//
// Find the FltMaterial Record associated with an index
//
FltMaterial *
fltLookupMaterial( FltFile * flt, uint16 idx )
{
    uint32 i;

    if( flt->materialPalette )
    {
	for (i=0;i<flt->materialPalette->numMaterials;i++)
	{
	    if (flt->materialPalette->material[i]->index == idx)
		return flt->materialPalette->material[i];
	}
    }

    return 0;
}

//
// Find the FltInstance Record associated with an index
//
FltInstanceDefinition *
fltLookupInstance( FltFile * flt, uint16 idx )
{
    uint32 i;

    for (i=0;i<flt->numNodes;i++) {

	if (flt->allNodes[i]->type == FLTRECORD_INSTANCEDEFINITION) {
	    if (((FltInstanceDefinition *)flt->allNodes[i])->instance == idx)
		return (FltInstanceDefinition *)flt->allNodes[i];
	}

    }

    return 0;
}

static uint32
attrReadUInt32( FILE * f, uint32 * ret )
{
    uint32 uint;

    if( fread( &uint, 1, sizeof( uint32 ), f ) != sizeof( uint32 ) )
	return 0;

    ENDIAN_32( uint );

    *ret = uint;

    return 1;
}

FltTxAttributes *
fltLoadAttributes( const char * file )
{
    FILE * f;
    FltTxAttributes * attrs;

    if(!( f = fopen( file, "rb" ) ))
	return (FltTxAttributes *)NULL;

    if(!( attrs = zmalloc( sizeof( FltTxAttributes ) ) )) {
	fclose( f );
	return (FltTxAttributes *)NULL;
    }

    // read first set of attrs from file
    // todo: check for errors
    attrReadUInt32( f, &attrs->uTexels );
    attrReadUInt32( f, &attrs->vTexels );
    attrReadUInt32( f, &attrs->realWorldU );
    attrReadUInt32( f, &attrs->realWorldV );
    attrReadUInt32( f, &attrs->upVectorX );
    attrReadUInt32( f, &attrs->upVectorY );
    attrReadUInt32( f, &attrs->fileFormat );
    attrReadUInt32( f, &attrs->minificationFilter );
    attrReadUInt32( f, &attrs->magnificationFilter );
    attrReadUInt32( f, &attrs->repetitionType );
    attrReadUInt32( f, &attrs->repetitionU );
    attrReadUInt32( f, &attrs->repetitionV );
    attrReadUInt32( f, &attrs->modifyFlag );
    attrReadUInt32( f, &attrs->xPivot );
    attrReadUInt32( f, &attrs->yPivot );
    attrReadUInt32( f, &attrs->environmentType );
    attrReadUInt32( f, &attrs->isIntensity );

    // scan forward to get detail texture offsets
    fseek( f, 340, SEEK_SET );

    attrReadUInt32( f, &attrs->detailTexture );
    attrReadUInt32( f, &attrs->detailJ );
    attrReadUInt32( f, &attrs->detailK );
    attrReadUInt32( f, &attrs->detailM );
    attrReadUInt32( f, &attrs->detailN );
    attrReadUInt32( f, &attrs->detailScramble );

    fclose( f );
    return attrs;
}

void
fltFreeAttributes( FltTxAttributes * attrs )
{
    zfree( attrs );
}

static char *
mstrtok( MStrtokCtxt * ctxt, char * initstr, char * sep )
{
    int i, sepLen, eol = 0, found = 0;

    assert( ctxt );
    assert( sep );

    sepLen = strlen( sep );

    if( initstr ) {

	// reset everything
	ctxt->head = initstr;
	ctxt->tail = initstr;
	ctxt->last = 0;

    } else {

	// index curPtr
	if( ctxt->last )
	    return 0;

	ctxt->tail++;
	ctxt->head = ctxt->tail;
    }


    // search for next separator, using tail
    while( 1 ) {
	if( *ctxt->tail ) {

	    for(i=0;i<sepLen;i++) {
		if( *ctxt->tail == sep[i] ) {
		    found++;
		    break;
		}
	    }

	    if( found )
		break;

	} else {

	    eol++;
	    break;

	}

	ctxt->tail++;
    }

    if( eol ) {
	ctxt->last = 1;
	return ctxt->head;
    }

    // null term, and return
    if( found ) {
	*ctxt->tail = 0;
	return ctxt->head;
    }

    // sep not found
    ctxt->last = 1;
    return 0;
}

//
// helper func for fltFindFile
// note, you must now send this routine a copy of the "envPath" string, because
// mstrtok changes it.
//

static int
buildPath( FltFile * flt, char * buf, char * filePath, char * envPath, int pass )
{
    char * ptr = 0;

    // if no envpath, nothing to do
    if( envPath == 0 ) {
	flt->pathInfo.first = 0;
	return 0;
    }

    if( flt->pathInfo.first == 0 ) {

	if(!( ptr = mstrtok( &flt->pathInfo.stc, envPath, ";" ) ))
	    // only 1 listed
	    ptr = envPath;

	flt->pathInfo.first = 1;

    } else
	ptr = mstrtok( &flt->pathInfo.stc, 0, ";" );

    if( ptr ) {

	strcpy( buf, ptr );
	//if( buf[ strlen( buf ) ] != '/' )
	strcat( buf, "/" );

	if( pass > 0 )
	{
	    // if on second pass, do not remove leading dots
	    strcat( buf, filePath );
	}
	else
	{
	    // check dos and unix sep
	    if( strrchr( filePath, '/' ) )
		strcat( buf, strrchr( filePath, '/' ) + 1 );
	    else if( strrchr( filePath, '\\' ) )
		strcat( buf, strrchr( filePath, '\\' ) + 1 );
	    else
		strcat( buf, filePath );
	}

    } else {

	flt->pathInfo.first = 0;

	return 0;

    }

    return 1;
}

void
fltAddSearchPath( FltFile * flt, char * path )
{
    int first = ( flt->pathInfo.searchPath == 0 );
    int size = strlen(path) + strlen( ";" );
    char * buf = 0;

    if( !first )
	size += strlen( flt->pathInfo.searchPath );

    // null term
    size++;

    //
    // assume that these allocs won't fail.
    //
    buf = (char *) malloc( size );

    //
    // add new path to the front of the list
    //
    strcpy( buf, path );
    strcat( buf, ";" );

    if( !first )
    {
	strcat( buf, flt->pathInfo.searchPath );
    }
    else
    {
	char * txtpath;

	// automatically append TXTPATH as well
	txtpath = getenv( "TXTPATH" );

	if( txtpath )
	{
	    buf = realloc( buf, size + strlen(txtpath) + strlen(";") );

	    strcat( buf, txtpath );
	    strcat( buf, ";" );
	}
    }

    // remove old, add new
    if( flt->pathInfo.searchPath )
	free( flt->pathInfo.searchPath );

    flt->pathInfo.searchPath = buf;
}

void
fltCopySearchPaths( FltFile * to, FltFile * from )
{
    // copy one to the other
    if( to->pathInfo.searchPath )
	free( to->pathInfo.searchPath );

    to->pathInfo.searchPath = strdup( from->pathInfo.searchPath );
}

//
// Find a file based on TXTPATH variable, separated by ';' (due to Creator
// for windows drive letters).
//

static int
fileExists( char * file )
{
#ifdef _WIN32
    DWORD fileAttr = GetFileAttributes( file );
    if( fileAttr == 0xFFFFFFFF )
    {
	return -1;
    }
    else
    {
	return 0;
    }
#else
    struct stat st;
    return stat( file, &st );
#endif
}

int
fltFindFile ( FltFile * flt, char * inFileName, char *outFileName )
{
    int result;
    char tmpBuf[MAX_PATHLEN];
    char * txtpath;

    //printf("looking for: %s\n", inFileName );

    result = fileExists( inFileName );

    if( result < 0 )
    {
	txtpath = flt->pathInfo.searchPath;
	//printf("search path: %s\n", txtpath );
	if( txtpath )
	{
	    int i, pass = 0;

	    if( inFileName[0] == '.' )
		pass = 2;
	    else
		pass = 1;

	    for(i=0; i<pass; i++ )
	    {
		// must copy the string because buildPath/mstrtok changes it
		txtpath = strdup( flt->pathInfo.searchPath );

		while( buildPath( flt, tmpBuf, inFileName, txtpath, i ) )
		{
		    if( fileExists( tmpBuf ) < 0 )
		    {
			//printf("FLT: Unable to find file in path: \"%s\"\n", tmpBuf );
			continue;
		    }
		    else
		    {
			//printf("FLT: file located: \"%s\"\n", tmpBuf );
			strcpy( outFileName, tmpBuf );
			// reset buildpath
			buildPath( flt, 0, 0, 0, 0 );

			free( txtpath );
			return 1;
		    }
		}

		free( txtpath );
		txtpath = 0;
	    }
	}
    } else {
	strcpy( outFileName, inFileName );
	return 1;
    }

    //printf("FLT: file NOT located: \"%s\"\n", inFileName );

    return 0;
}

//
// check for a finite value (ie: not NAN or INF), as stored in the flight file
//
int
fltFinite64( real64 value )
{
    // IEEE 754 values that will be stored in the flight file
    static unsigned char
	posInf[] = { 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char
	negInf[] = { 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char
	posNan[] = { 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char
	negNan[] = { 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    unsigned char * valPtr;

    // if little endian, convert to big
    ENDIAN_64r( value );

    valPtr = (unsigned char *)&value;

    if(	!memcmp( negNan, valPtr, 8 ) || !memcmp( posNan, valPtr, 8 ) ||
	    !memcmp( negInf, valPtr, 8 ) || !memcmp( posInf, valPtr, 8 )  )
	return 0;
    else
	return 1;
}

// get/set the file identifier
void
fltSetFileID( FltFile * flt, uint32 id )
{
    assert( flt );

    flt->fileID = id;
}

uint32
fltGetFileID( FltFile * flt )
{
    assert( flt );

    return flt->fileID;
}


/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */

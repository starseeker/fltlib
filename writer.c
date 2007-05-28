
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
#include <time.h>
#include "flt.h"
#include "zmalloc.h"

uint32
fltWriteHeader( FILE * flt, FltHeader * header )
{
	uint16 type, length;

	assert( flt );
	assert( header );

	type = FLTRECORD_HEADER;
	length = 2 + 2 + 8 + 4 + 4 + 32 + 2 + 2 + 2 + 2 + 2 + 1 + 1 + 4 + (4*6) +
						4 + (4*7) + 2 + 2 + 4 + (4*8) + 2 + 2 + (4*2) +
						2 + 2 + 2 + 2 + 4 + (8*8) + (2*8) + 4 + 2 + 2 + 2 + 6 + 8 + 8 + 
						2 + 2; 

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// swap written fields
	ENDIAN_32( header->formatRevision );
	ENDIAN_16( header->unitMultiplier );
	ENDIAN_16( header->vertexStorageLength );
	ENDIAN_32( header->databaseOrigin );

	// write it all out
	
	fwrite( header->ID, 8, 1, flt );
	fwrite( &header->formatRevision, sizeof( header->formatRevision ), 1, flt );
	fwrite( &header->editRevision, sizeof( header->editRevision ), 1, flt );
	fwrite( header->dateTime, 32, 1, flt );
	fwrite( &header->nextGroupNodeID, sizeof( header->nextGroupNodeID ), 1, flt );
	fwrite( &header->nextLODNodeID, sizeof( header->nextLODNodeID ), 1, flt );
	fwrite( &header->nextObjectNodeID, sizeof( header->nextObjectNodeID ),1,flt );
	fwrite( &header->nextFaceNodeID, sizeof( header->nextFaceNodeID ), 1, flt );
	fwrite( &header->unitMultiplier, sizeof( header->unitMultiplier ), 1, flt );
	fwrite( &header->coordUnits, sizeof( header->coordUnits ), 1, flt );
	fwrite( &header->setTexWhiteOnNewFaces, 
														sizeof( header->setTexWhiteOnNewFaces ), 1, flt );
	fwrite( &header->flags, sizeof( header->flags ), 1, flt );
	fwrite( header->reserved0, 6*sizeof( uint32 ), 1, flt );
	fwrite( &header->projectionType, sizeof( header->projectionType ), 1, flt );
	fwrite( header->reserved1, 7*sizeof( uint32 ), 1, flt );
	fwrite( &header->nextDOFNodeID, sizeof( header->nextDOFNodeID ), 1, flt );
	fwrite( &header->vertexStorageLength, sizeof( header->vertexStorageLength ), 
																																		1, flt );
	fwrite( &header->databaseOrigin, sizeof( header->databaseOrigin ), 1, flt );
	fwrite( &header->swDBCoordX, sizeof( header->swDBCoordX ), 1, flt );
	fwrite( &header->swDBCoordY, sizeof( header->swDBCoordY ), 1, flt );
	fwrite( &header->deltaXPlace, sizeof( header->deltaXPlace ), 1, flt );
	fwrite( &header->deltaYPlace, sizeof( header->deltaYPlace ), 1, flt );
	fwrite( &header->nextSoundNodeID, sizeof( header->nextSoundNodeID ), 1, flt );
	fwrite( &header->nextPathNodeID, sizeof( header->nextPathNodeID ), 1, flt );
	fwrite( header->reserved2, 2*sizeof( uint32 ), 1, flt );
	fwrite( &header->nextClipNodeID, sizeof( header->nextClipNodeID ), 1, flt );
	fwrite( &header->nextTextNodeID, sizeof( header->nextTextNodeID ), 1, flt );
	fwrite( &header->nextBSPNodeID, sizeof( header->nextBSPNodeID ), 1, flt );
	fwrite( &header->nextSwitchNodeID, sizeof( header->nextSwitchNodeID ),1,flt );
	fwrite( &header->reserved3, sizeof( header->reserved3 ), 1, flt );
	fwrite( &header->swDBLatitude, sizeof( header->swDBLatitude ), 1, flt );
	fwrite( &header->swDBLongitude, sizeof( header->swDBLongitude ), 1, flt );
	fwrite( &header->neDBLatitude, sizeof( header->neDBLatitude ), 1, flt );
	fwrite( &header->neDBLongitude, sizeof( header->neDBLongitude ), 1, flt );
	fwrite( &header->originDBLatitude, sizeof( header->originDBLatitude ),1,flt );
	fwrite( &header->originDBLongitude, sizeof( header->originDBLongitude ),
																																			1, flt );
	fwrite( &header->lambertUpperLatitude, sizeof( header->lambertUpperLatitude ),
																																	1, flt );
	fwrite( &header->lambertLowerLatitude, 
														sizeof( header->lambertLowerLatitude ), 1, flt );
	fwrite( &header->nextLightSourceNodeID, 
														sizeof( header->nextLightSourceNodeID ), 1, flt );
	fwrite( &header->nextLightPointNodeID, 
														sizeof( header->nextLightPointNodeID ), 1, flt );
	fwrite( &header->nextRoadNodeID, sizeof( header->nextRoadNodeID ), 1, flt );
	fwrite( &header->nextCATNodeID, sizeof( header->nextCATNodeID ), 1, flt );
	fwrite( &header->reserved4, sizeof( header->reserved4 ), 1, flt );
	fwrite( &header->reserved5, sizeof( header->reserved5 ), 1, flt );
	fwrite( &header->reserved6, sizeof( header->reserved6 ), 1, flt );
	fwrite( &header->reserved7, sizeof( header->reserved7 ), 1, flt );
	fwrite( &header->earthEllipsoidModel, sizeof( header->earthEllipsoidModel ), 
																																			1, flt );
	fwrite( &header->nextAdaptiveNodeID, sizeof( header->nextAdaptiveNodeID ), 1,
																																				flt );
	fwrite( &header->nextCurveNodeID, sizeof( header->nextCurveNodeID ), 1, flt );
	// utmZone is not really in 15.7
	fwrite( &header->utmZone, sizeof( header->utmZone ), 1, flt );
	fwrite( &header->reserved8, 6*sizeof(uint8), 1, flt );
	fwrite( &header->deltaZPlace, sizeof( header->deltaZPlace ), 1, flt );
	fwrite( &header->dbRadius, sizeof( header->dbRadius ), 1, flt );
	fwrite( &header->nextMeshNodeID, sizeof( header->nextMeshNodeID ), 1, flt );
	// neither is this one, but makes length match
	fwrite( &header->nextLightPointSystemID, 
										sizeof( header->nextLightPointSystemID ), 1, flt );

	// whew!!

	return 1;
}

uint32
fltWritePushLevel( FILE * flt )
{
	uint16 type, length;

	assert( flt );

	type = 10; // no enum?
	length = 2 + 2;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	return 1;
}

uint32
fltWritePopLevel( FILE * flt )
{
	uint16 type, length;

	assert( flt );

	type = 11; // no enum?
	length = 2 + 2;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	return 1;
}

FltHeader *
fltBuildHeader( void )
{
	FltHeader * header = (FltHeader *) zcalloc( 1, sizeof( FltHeader ) );
	time_t tm;
	char * dateStr;

	if( header ) 
	{
		// only set relevent fields
		sprintf( header->ID, "db" );
		header->formatRevision = 1570;	// version 15.7 (due to len)
		header->unitMultiplier = 1;			// according to docs
		header->coordUnits = 0;					// meters
		header->vertexStorageLength = 1;// only option?
		header->databaseOrigin = 100;		// OpenFLT

		// get current time
		time( &tm );
		dateStr = ctime( &tm );

		// "dont" remove \n
	//	dateStr[24] = 0;

		strcpy( header->dateTime, dateStr );
	}

	return header;
}

uint32
fltWriteExternalReference( FILE * flt, FltExternalReference * extref )
{
	uint16 type, length;

	assert( flt );
	assert( extref );

	type = FLTRECORD_EXTERNALREFERENCE;
	length = 2 + 2 + 200 + 1 + 1 + 2 + 4 + 2 + 2;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// since all data is either 0, bytes, or all 1's, no need to byte swap
	//
	fwrite( extref->path, 200, 1, flt );
	fwrite( &extref->reserved0, sizeof( extref->reserved0 ), 1, flt );
	fwrite( &extref->reserved1, sizeof( extref->reserved1 ), 1, flt );
	fwrite( &extref->reserved2, sizeof( extref->reserved2 ), 1, flt );
	fwrite( &extref->flags, sizeof( extref->flags ), 1, flt );
	fwrite( &extref->reserved3, sizeof( extref->reserved3 ), 1, flt );
	fwrite( &extref->reserved4, sizeof( extref->reserved4 ), 1, flt );

	return 1;
}

FltExternalReference *
fltBuildExternalReference( char * refpath )
{
	FltExternalReference * extref = (FltExternalReference *) 
																	zcalloc( 1, sizeof( FltExternalReference ) );

	if( extref ) {
		// copy path
		strncpy( extref->path, refpath, 199 );

		// set override bits (to use this files attrs)
		//extref->flags = 0xffffffff;

		// make big endian?
		extref->flags = 
				FLTEXTREF_COLORPALETTE_OVERRIDE |
				FLTEXTREF_MATERIALPALETTE_OVERRIDE |
				FLTEXTREF_TEXTUREPALETTE_OVERRIDE |
				FLTEXTREF_LINESTYLEPALETTE_OVERRIDE |
				FLTEXTREF_SOUNDPALETTE_OVERRIDE |
				FLTEXTREF_LIGHTSOURCEPALETTE_OVERRIDE |
				FLTEXTREF_LIGHTPOINTPALETTE_OVERRIDE |
				FLTEXTREF_SHADERPALETTE_OVERRIDE;
	}

	return extref;
}

uint32
fltWriteMatrix( FILE * flt, FltMatrix * mat )
{
	uint16 type, length;
	uint32 i;

	assert( flt );
	assert( mat );

	type = FLTRECORD_MATRIX;
	length = 2 + 2 + (16 * sizeof( real32 ));

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// endian swap it all, and write
	for(i=0;i<16;i++) {
		ENDIAN_32r( mat->matrix[i] );
		fwrite( &mat->matrix[i], sizeof( real32 ), 1, flt );
	}

	return 1;
}

/*
 * Build rotation and translation matrix
 */
FltMatrix *
fltBuildMatrix( real32 x, real32 y, real32 z, real32 h, real32 p, real32 r )
{
	FltMatrix * mat = (FltMatrix *) zcalloc( 1, sizeof( FltMatrix ) );

	if( mat ) {

		// rotation (taken from sglMat4.hpp)
		real32	sr = (real32) sin(p),
						cr = (real32) cos(p),
						sh = (real32) sin(r),
						ch = (real32) cos(r),
						sp = (real32) sin(h),
						cp = (real32) cos(h);

		mat->matrix[4*0+0] = -sp*sr*sh + cp*ch;
		mat->matrix[4*0+1] =  cp*sr*sh + sp*ch;
		mat->matrix[4*0+2] = -cr*sh;
		mat->matrix[4*0+3] =  0.0f;

		mat->matrix[4*1+0] = -sp*cr;
		mat->matrix[4*1+1] =  cp*cr;
		mat->matrix[4*1+2] =  sr;
		mat->matrix[4*1+3] =  0.0f;

		mat->matrix[4*2+0] =  sp*sr*ch + cp*sh;
		mat->matrix[4*2+1] = -cp*sr*ch + sp*sh;
		mat->matrix[4*2+2] =  cr*ch;
		mat->matrix[4*2+3] =  0.0f;

		// translation
		mat->matrix[4*3+0] = x;
		mat->matrix[4*3+1] = y;
		mat->matrix[4*3+2] = z;
		mat->matrix[4*3+3] = 1.0f;
	}

	return mat;
}

/*
 * Alloc matrix
 */
FltMatrix *
fltBuildMatrix4x4( float cmat[4][4] )
{
	FltMatrix * mat = (FltMatrix *) zcalloc( 1, sizeof( FltMatrix ) );

	if( mat ) {
		mat->matrix[4*0+0] = cmat[0][0];
		mat->matrix[4*0+1] = cmat[0][1];
		mat->matrix[4*0+2] = cmat[0][2];
		mat->matrix[4*0+3] = cmat[0][3];

		mat->matrix[4*1+0] = cmat[1][0];
		mat->matrix[4*1+1] = cmat[1][1];
		mat->matrix[4*1+2] = cmat[1][2];
		mat->matrix[4*1+3] = cmat[1][3];

		mat->matrix[4*2+0] = cmat[2][0];
		mat->matrix[4*2+1] = cmat[2][1];
		mat->matrix[4*2+2] = cmat[2][2];
		mat->matrix[4*2+3] = cmat[2][3];

		mat->matrix[4*3+0] = cmat[3][0];
		mat->matrix[4*3+1] = cmat[3][1];
		mat->matrix[4*3+2] = cmat[3][2];
		mat->matrix[4*3+3] = cmat[3][3];
	}

	return mat;
}


/*
 * Build rot, trans, scale matrix with vector args
 */
FltMatrix *
fltBuildMatrixSv( real32 pos[3], real32 ori[3], real32 sc[3] )
{
	FltMatrix * mat = (FltMatrix *) zcalloc( 1, sizeof( FltMatrix ) );
	real32 scale[3*3], rot[3*3];
	real32 sr, cr, sh, ch, sp, cp;
	int i,j;

	// zero final mat
	memset( mat->matrix, 0x00, 16 * sizeof( float ) );

	// build scale matrix
	scale[3*0+0] = sc[0];
	scale[3*0+1] = 0.0f;
	scale[3*0+2] = 0.0f;

	scale[3*1+0] = 0.0f;
	scale[3*1+1] = sc[1];
	scale[3*1+2] = 0.0f;

	scale[3*2+0] = 0.0f;
	scale[3*2+1] = 0.0f;
	scale[3*2+2] = sc[2];

	// rotation (taken from sglMat4.hpp)
	sr = (real32) sin(ori[1]),
	cr = (real32) cos(ori[1]),
	sh = (real32) sin(ori[2]),
	ch = (real32) cos(ori[2]),
	sp = (real32) sin(ori[0]),
	cp = (real32) cos(ori[0]);

	rot[3*0+0] = -sp*sr*sh + cp*ch;
	rot[3*0+1] =  cp*sr*sh + sp*ch;
	rot[3*0+2] = -cr*sh;

	rot[3*1+0] = -sp*cr;
	rot[3*1+1] =  cp*cr;
	rot[3*1+2] =  sr;

	rot[3*2+0] =  sp*sr*ch + cp*sh;
	rot[3*2+1] = -cp*sr*ch + sp*sh;
	rot[3*2+2] =  cr*ch;

	// multiply S*R
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			mat->matrix[4*i+j] = scale[3*i+0] * rot[3*0+j] +
														scale[3*i+1] * rot[3*1+j] +
														scale[3*i+2] * rot[3*2+j];
		}
	}

	// translation
	mat->matrix[4*3+0] = pos[0];
	mat->matrix[4*3+1] = pos[1];
	mat->matrix[4*3+2] = pos[2];
	mat->matrix[4*3+3] = 1.0f;

	return mat;
}

FltGroup * 
fltBuildGroup( char * id )
{
	FltGroup * group = (FltGroup *) zcalloc( 1, sizeof( FltGroup ) );

	if( group )
	{
		strncpy( group->ID, id, 8 );

		group->relativePriority = 0;
		group->reserved0 = 0;
		group->flags = 0;
		group->specialEffectID1 = 0;
		group->specialEffectID2 = 0;
		group->significance = 0;
		group->layerCode = 0;
		group->reserved1 = 0;
		group->reserved2 = 0;
	}

	return group;
}

uint32
fltWriteGroup( FILE * flt, FltGroup * group )
{
	uint16 type, length;

	assert( flt );
	assert( group );

	type = FLTRECORD_GROUP;
	length = 2 + 2 + 8 + 2 + 2 + 4 + 2 + 2 + 2 + 1 + 1 + 4;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// since all data is either 0, or bytes, no need to byte swap for now
	//
	fwrite( group->ID, 8, 1, flt );
	fwrite( &group->relativePriority, sizeof( group->relativePriority ), 1, flt );
	fwrite( &group->reserved0, sizeof( group->reserved0 ), 1, flt );
	fwrite( &group->flags, sizeof( group->flags ), 1, flt );
	fwrite( &group->specialEffectID1, sizeof( group->specialEffectID1 ),1,flt );
	fwrite( &group->specialEffectID2, sizeof( group->specialEffectID2 ),1,flt );
	fwrite( &group->significance, sizeof( group->significance ), 1, flt );
	fwrite( &group->layerCode, sizeof( group->layerCode ), 1, flt );
	fwrite( &group->reserved1, sizeof( group->reserved1 ), 1, flt );
	fwrite( &group->reserved2, sizeof( group->reserved2 ), 1, flt );

	return 1;
}

uint32
fltWriteDefaultVertexPalette( FILE * flt )
{
	uint16 type, length;
	uint32 palLength = 8;

	assert( flt );

	type = FLTRECORD_VERTEXPALETTE;
	length = 2 + 2 + 4;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	ENDIAN_32( palLength );
	fwrite( &palLength, sizeof( palLength ), 1, flt );

	return 1;
}

uint32
fltWriteDefaultColorPalette( FILE * flt )
{
	uint16 type, length;
	static char res[128] = {0};
	int i;

	assert( flt );

	type = FLTRECORD_COLORPALETTE;
	length = 2 + 2 + 128 + 4*1024;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	// write header
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// write reserved
	fwrite( res, 128, 1, flt );

	// write entries (all zeroed)
	for(i=0;i<(4*1024)/128;i++)
		fwrite( res, 128, 1, flt );

	// should write numNames == 0 ?

	return 1;
}

uint32
fltWriteDefaultLightSourcePalette( FILE * flt )
{
	uint32 dummy = 0;
	uint16 type, length;
	int i;

	assert( flt );

	type = 102;
	length = 240;

	// byte swap
	ENDIAN_16( type );
	ENDIAN_16( length );
	
	// write header
	fwrite( &type, sizeof( uint16 ), 1, flt ); 
	fwrite( &length, sizeof( uint16 ), 1, flt ); 

	// write entries (all zeroed)
	for(i=0;i<236/sizeof(dummy);i++)
		fwrite( &dummy, sizeof( dummy ), 1, flt );

	return 1;
}

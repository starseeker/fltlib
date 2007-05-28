
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
#include <assert.h>
#include <math.h>
#include <string.h>
#include "flt.h"
#include "zmalloc.h"

int
fltReadRecordAttr( FltFile * flt, uint16 * type, uint16 * length )
{
	int result;
	uint16 ltype, llength;
	uint8 * bufPtr;

	assert( flt );
	assert( type );
	assert( length );

	if( fread( &ltype, 1, sizeof( uint16 ), flt->stdFile ) != sizeof(uint16) )
		// eof
		return 0;

	// 
	// check for EOF - not sure if this is necessary
	//
	if( feof( flt->stdFile ) )
	{
		return 0;
	}

	flt->byteOffset += sizeof( uint16 );

	// bail out on NULL type
	if( ltype == 0 ) {
		printf("FLT: Caught NULL node.. bailing out.\n");
		return 0;
	}

#if 0
	// hack -- for extra zeroes (doesn't seem to work in all cases)
	if( ltype == 0 ) {
		uint8 dummy;

		printf("FLT: Caught NULL node, applying Hack.\n");

		// read again
		if( fread( &dummy, 1, sizeof( uint8 ), flt->stdFile ) != sizeof(uint8) ) {
			printf("ONE.5\n");
			return 0;
		}

		flt->byteOffset += sizeof( uint16 );
	}
#endif

//	printf("BO llength: %x\n", flt->byteOffset );

	if( fread( &llength, 1, sizeof( uint16), flt->stdFile ) != sizeof(uint16) ) {
//		printf("TWO\n");
		return 0;
	}

//	printf("llength: %04x\n", llength );

	flt->byteOffset += sizeof( uint16 );

	ENDIAN_16( ltype );
	ENDIAN_16( llength );

	// for header
	if( llength)
		llength -= 4;

	*type = ltype;
	*length = llength;

	// read the block into memory
	bufPtr = FltBufferResize( flt->buffer, llength );

	if( llength ) {
		result = fread( bufPtr, 1, llength, flt->stdFile );
		if( result != llength )  {
			printf("FLT: Unexpected EOF: Type: %d Attempted: %d Actual: %d Off: %x\n",
											ltype, llength, result, flt->byteOffset );
		printf("FLT: %04x %04x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
										ltype & 0xffff,
										llength & 0xffff,
										bufPtr[0]&0xff,
										bufPtr[1]&0xff,
										bufPtr[2]&0xff,
										bufPtr[3]&0xff,
										bufPtr[4]&0xff,
										bufPtr[5]&0xff,
										bufPtr[6]&0xff,
										bufPtr[7]&0xff,
										bufPtr[8]&0xff,
										bufPtr[9]&0xff );
			return 0;
		}
	}

	flt->byteOffset += llength;

	FltBufferResetWithLength( flt->buffer, llength );

	return 1;
}

void
fltSkipRecord( FltFile * flt, uint16 length )
{
	assert( flt );

//	return fseek( flt->stdFile, length, SEEK_CUR ) + 1;
}

FltFile *
fltOpen( const char * name )
{
	FltFile * f;
	char * option = 0;

	assert( name );

	f = zcalloc( 1, sizeof( FltFile ) );
	if(!f) {
		printf("unable to malloc..\n");
		return 0;
	}

	f->stdFile = fopen( name, "rb" );
	if( !f->stdFile ) {
		printf("unable to find file: %s\n", name );
		zfree( f );
		return 0;
	}

	strcpy( f->fileName, name );

	FltBufferAlloc( f );
	FltStackAlloc( f );

  // set fileID to all f's at init
  fltSetFileID( f, 0xffffffff );

	/* check for local options */
	if( (option = getenv("FLTLIB_COORDCONV")) && *option == '0' )
		f->xformUnits = 0;
	else
		f->xformUnits = 1;

	return f;
}

void
fltClose( FltFile * file )
{
	assert( file );

	if( file->stdFile )
		fclose( file->stdFile );

	file->stdFile = 0;
}

void
fltFileFree( FltFile * flt )
{
	unsigned int i;

	//
	// Free all resources
	//

	if( flt->stdFile )
		fltClose( flt );

	// rundown tree, freeing all nodes as we go
	for(i=0;i<flt->numNodes;i++) {
		FltNode * node = flt->allNodes[i];

		// free nodes with special attrs
		switch( node->type ) {

			case FLTRECORD_VERTEXLIST: {
				FltVertexList * l = (FltVertexList *)node;
				zfree( l->list );
				zfree( l->indexList );
				break;
			}

			case FLTRECORD_VERTEXPALETTE: {
				FltVertexPalette * p = (FltVertexPalette *)node;
				zfree( p->verts );
				break;
			}

			case FLTRECORD_SWITCH: {
				FltSwitch * s = (FltSwitch *)node;
				zfree( s->masks );
				break;
			}

      case FLTRECORD_LOCALVERTEXPOOL: {
         FltLocalVertexPool * l = (FltLocalVertexPool *)node;
         zfree( l->entries );
         break;
       }

       case FLTRECORD_MESHPRIMITIVE: {
         FltMeshPrimitive * m = (FltMeshPrimitive *)node;
         zfree( m->indices );
         break;
       }
		}

		if( node->child )
			zfree( node->child );
		zfree( node );
	}

	if( flt->lightSourcePalette ) 
  {
		if( flt->lightSourcePalette->lights )
			zfree( flt->lightSourcePalette->lights );
		zfree( flt->lightSourcePalette );
	}

	if( flt->materialPalette ) 
  {
		if( flt->materialPalette->material )
			zfree( flt->materialPalette->material );
		zfree( flt->materialPalette );
	}

	if( flt->lineStylePalette ) 
  {
    if( flt->lineStylePalette->styles )
      zfree( flt->lineStylePalette->styles );
    zfree( flt->lineStylePalette );
  }

	if( flt->shaderPalette ) 
  {
    if( flt->shaderPalette->shaders )
      zfree( flt->shaderPalette->shaders );
    zfree( flt->shaderPalette );
  }

	if( flt->textureMappingPalette ) 
  {
    if( flt->textureMappingPalette->mappings )
      zfree( flt->textureMappingPalette->mappings );
    zfree( flt->textureMappingPalette );
  }

	FltBufferFree( flt );
	FltStackFree( flt );

	if( flt->allNodes )
		zfree( flt->allNodes );

	zfree( flt );

  // free rest
  zmallocFreeAll();
}

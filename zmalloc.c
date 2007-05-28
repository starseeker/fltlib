
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

/*
 * zmalloc.c
 *
 * Author: Mike Morrison
 *         October 2002
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zmalloc.h"

#define ZMALLOC_MAGIC  0xf10203f4
#define ZMALLOC_MAGIC0 0xf1f2f3f4

#define ZMALLOC_PAGE_INC (4096)
#define ZMALLOC_ALIGN 32

#define USE_LOCALALLOC

#ifdef USE_LOCALALLOC

#define ZMALLOC_SLABSIZE ( 8192 * 1024)
#define ZMALLOC_MAX_REALLOC ( 4 * 1024 * 1024)

typedef struct {
	unsigned char * ptr;
	unsigned int length;
	unsigned int used;
	unsigned int numAllocs;
} slab;

typedef struct {
	unsigned int magic;
	unsigned int numAllocs;
	unsigned int bufferLength;
	unsigned int initialSize;
	slab       * slab;
	unsigned int pad;
	unsigned int pad1;
	unsigned int pad2;
} MHeader;

static slab ** slabs = 0;
static unsigned int numSlabs = 0;
static slab * curSlab = 0;

static void
addSlab( const char * file, int line )
{
	slabs = (slab **)realloc( slabs, (numSlabs+1)*sizeof( slab * ) );
	if( slabs )
	{
		slabs[ numSlabs ] = (slab *)malloc( sizeof( slab ) );
		if( slabs[ numSlabs ] == 0 )
		{
			printf("Error allocating new slab: %d, slabsize: %d, total: %u\n" 
						 "Location: %s:%d\n",
															numSlabs, ZMALLOC_SLABSIZE, 
															numSlabs*ZMALLOC_SLABSIZE,
															file, line );
			exit(0);
		}
		else
		{
			curSlab = slabs[ numSlabs ];
			numSlabs++;

			curSlab->ptr = (unsigned char *)calloc( 1, ZMALLOC_SLABSIZE );
			if( curSlab->ptr == 0 )
			{
				printf("Error allocating new slab: %d, slabsize: %d, total: %u\n" 
							 "Location: %s:%d\n",
																numSlabs, ZMALLOC_SLABSIZE, 
																numSlabs*ZMALLOC_SLABSIZE,
																file, line );
				exit(0);
			}

			curSlab->length = ZMALLOC_SLABSIZE;
			curSlab->used   = 0;
			curSlab->numAllocs = 0;

			//printf("Adding Slab: #%d: %p\n", numSlabs, curSlab->ptr );
		}

	}
	else
	{
		printf("slab alloc failure..\n");
		exit(0);
	}
}

static void
freeSlab( slab * theSlab )
{
	//printf("Freeing Slab: %p\n", theSlab->ptr );

	if( theSlab->ptr )
		free( theSlab->ptr );

	theSlab->ptr = 0;

	if( theSlab == curSlab )
		curSlab = 0;
}


// we may need to worry about thread safety..
static void *
_localAlloc( int size, slab ** whichSlab, const char * file, int line )
{
	unsigned char * ptr;
	unsigned long offset;

	if( size > ZMALLOC_SLABSIZE )
	{
		ptr = calloc( 1, size );
		if( ptr )
		{
			*whichSlab = 0;
			return ptr;
		}
		else
		{
			printf("Unable to malloc!\n");
			exit(0);
		}
	}

	if( curSlab == 0 )
		addSlab( file, line );

	// check to see if we have enough space
	if( (curSlab->used + size + ZMALLOC_ALIGN) >= curSlab->length )
		addSlab( file, line );
	
	ptr = curSlab->ptr + curSlab->used;

	// warning assumes PTR == sizeof(unsigned long)
	offset = (unsigned long)ptr;

	// align appropriately
	offset %= ZMALLOC_ALIGN;
	if( offset )
		offset = ZMALLOC_ALIGN - offset;

	ptr += offset;

	curSlab->used = (ptr - curSlab->ptr) + size;
	curSlab->numAllocs++;

	*whichSlab = curSlab;

	return ptr;
}

#endif /* USE_LOCALALLOC */

/*
 * If this function is not called, a small amount of mem will
 * be leaked.  It must be called if we expect to use zfuncs to alloc again.
 */
void
zmallocFreeAll( void )
{
	unsigned int i;

	// ONLY FREES SLAB MEMORY..

	for(i=0;i<numSlabs;i++)
	{
		if( slabs[i]->ptr )
			free( slabs[i]->ptr );
		free( slabs[i] );
	}

	free( slabs );

  // reset these for next time
  slabs = 0;
  numSlabs = 0;
  curSlab = 0;
}

void *
_zrealloc( void * ptr, unsigned int newSize, char * file, int line )
{
	MHeader * hdr;
	unsigned int /*div,*/ allocsize;

	if( ptr )
	{
		/* allocate memory from the end, if we can */
		hdr = (MHeader *)( (((unsigned char *)ptr) - sizeof( MHeader )) );

		if( hdr->magic == ZMALLOC_MAGIC )
		{
			if( newSize < hdr->bufferLength )
			{
				hdr->numAllocs++;
				return ptr;
			}
			else
			{

#if 0
				//printf("had to realloc: %p: %d allocs, initialize: %d cursize: %d\n", ptr,
				//													hdr->numAllocs, hdr->initialSize,
				//													hdr->bufferLength );
				
				// if we are reallocating "frequently", build a bigger buffer
				if( hdr->numAllocs > 10 )
				{
					hdr->initialSize *= 2;
					hdr->numAllocs = 1;
				}
				else if( hdr->initialSize == hdr->bufferLength )
				{
					hdr->initialSize *= 20;
				}

				div = newSize / hdr->initialSize;
				div++;

				allocsize = div * hdr->initialSize;
#else
				hdr->initialSize *= 2;
				if( hdr->initialSize > ZMALLOC_MAX_REALLOC )
					hdr->initialSize = ZMALLOC_MAX_REALLOC;

				allocsize = hdr->bufferLength + hdr->initialSize;

				while( allocsize < newSize ) 
					allocsize += ZMALLOC_PAGE_INC;

				if( allocsize > 10*1024*1024 )
				printf("realloc: %s:%d %d allocs, initialize: %d cursize: %d "
										"newsize: %d\n", 
																	file, line,
																	hdr->numAllocs, hdr->initialSize,
																	hdr->bufferLength, allocsize );
#endif

				hdr = (MHeader *) realloc( hdr, sizeof(MHeader) + allocsize ); 

				if( hdr )
				{
					hdr->bufferLength = allocsize;
					hdr->numAllocs++;

					return (void *)( (((unsigned char *)hdr) + sizeof( MHeader )) );
				}
				else
				{
					printf( "Unable to realloc: %p (%d bytes) for: %s::%d\n", 
										ptr, sizeof(MHeader)+ allocsize, file,line );
					exit(0);
					return 0;
				}
			}
		}
		else if( hdr->magic == ZMALLOC_MAGIC0 )
		{
			printf( "Tried to realloc mem that started elsewhere: %p (%d bytes) for: %s::%d\n", 
																ptr, newSize, file, line );
			exit(0);
			return 0;
		}
		else
		{
			printf( "Memory corruption in realloc: %p (%d bytes) for: %s::%d\n", 
																ptr, newSize, file, line );
			exit(0);
			return 0;
		}
	}

	hdr = (MHeader *) malloc( newSize + sizeof( MHeader ) );

	if( hdr )
	{
		hdr->magic = ZMALLOC_MAGIC;
		hdr->numAllocs    = 1;
		hdr->bufferLength = newSize;
		hdr->initialSize  = hdr->bufferLength;

		//printf("INITIAL REALLOC: %p for %d (%s::%d)\n", hdr, newSize, file, line );

		return (void *)( (((unsigned char *)hdr) + sizeof( MHeader )) );
	}
	else
	{
		printf("Unable to realloc: block: %d bytes for: %s::%d\n", newSize,
																							file, line );
		exit(0);
		return 0;
	}
}

void *
_zmalloc( unsigned int size, char * file, int line )
{
	MHeader * ptr;

#ifdef USE_LOCALALLOC
	slab * theSlab;
	ptr = (MHeader *) _localAlloc( size + sizeof( MHeader ), &theSlab, file,line);
#else
	ptr = (MHeader *) malloc( size + sizeof( MHeader ) );
#endif

	if( ptr )
	{
#ifdef USE_LOCALALLOC
		ptr->magic = ZMALLOC_MAGIC0;
		ptr->slab  = theSlab;
#else
		ptr->magic = ZMALLOC_MAGIC;
#endif
		ptr->numAllocs    = 1;
		ptr->bufferLength = size;
		ptr->initialSize  = size;

		return (void *)( (((unsigned char *)ptr) + sizeof( MHeader )) );
	}
	else
	{
		printf("Unable to malloc for: %s::%d\n", file, line);
		exit(0);
		return 0;
	}
}

void *
_zcalloc( unsigned int nelem, unsigned int size, char * file, int line )
{
	MHeader * hdr;

#ifdef USE_LOCALALLOC
	// localalloc'd ram is all zeroed first
	slab * theSlab;
	hdr = (MHeader *) _localAlloc( (nelem*size) + sizeof( MHeader ), &theSlab,
																								file, line);
#else
	hdr = (MHeader *) calloc( 1, (nelem*size) + sizeof( MHeader ) );
#endif

	if( hdr )
	{
#ifdef USE_LOCALALLOC
		hdr->magic = ZMALLOC_MAGIC0;
		hdr->slab  = theSlab;
#else
		hdr->magic = ZMALLOC_MAGIC;
#endif
		hdr->numAllocs    = 1;
		hdr->bufferLength = size;
		hdr->initialSize = size;

		return (void *)( (((unsigned char *)hdr) + sizeof( MHeader )) );
	}
	else
	{
		printf("Unable to calloc for: %s::%d\n", file, line);
		exit(0);
		return 0;
	}
}

void
_zfree( void * ptr, char * file, int line )
{
	MHeader * hdr;

	if( ptr )
	{
		//printf("ZFree: %p\n", ptr );

		hdr = (MHeader *)( (((unsigned char *)ptr) - sizeof( MHeader )) );

		if( hdr->magic == ZMALLOC_MAGIC )
		{
			free( hdr );
		}
#ifdef USE_LOCALALLOC
		else if( hdr->magic == ZMALLOC_MAGIC0 )
		{
			if( hdr->slab )
			{
				if( hdr->slab->ptr )
				{
					hdr->slab->numAllocs--;

					//printf("Freeing: %p: %d allocs left in slab\n", ptr, hdr->slab->numAllocs );
					if( hdr->slab->numAllocs == 0 )
					{
						freeSlab( hdr->slab );
					}
				}
				else
				{
					printf("Slab Corrupt, mem likely free'd twice:"
									" %s::%d\n", file, line);
					return;
				}
			}
			else
				/* mem was bigger than a slab */
				free( hdr );
		}
#endif
		else
		{
			printf("Attempt to free Non-Zmalloc'd memory (or corruption) from:"
							" %s::%d\n", file, line);
			return;
		}
	}
	else
	{
		printf("Attempt to free NULL pointer!\n");
		return;
	}
}


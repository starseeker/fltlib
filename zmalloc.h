
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



#ifndef _ZMALLOC_H_
#define _ZMALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

#undef USE_ZMALLOC

#ifdef USE_ZMALLOC

#define zmalloc( a ) _zmalloc( (a), __FILE__, __LINE__ )
#define zrealloc( a, b ) _zrealloc( (a), (b), __FILE__, __LINE__ )
#define zcalloc( a, b ) _zcalloc( (a), (b), __FILE__, __LINE__ )
#define zfree( a ) _zfree( (a), __FILE__, __LINE__ )

FLTLIB_API void * _zmalloc( unsigned int size, char *, int);
FLTLIB_API void * _zrealloc( void * ptr, unsigned int newSize, char *, int);
FLTLIB_API void * _zcalloc( unsigned int size, unsigned int nelem, char *, int);
FLTLIB_API void _zfree( void * ptr, char *, int );
FLTLIB_API void zmallocFreeAll( void );

#else

#define zmalloc( a ) malloc( (a) )
#define zrealloc( a, b ) realloc( (a), (b) )
#define zcalloc( a, b ) calloc( (a), (b) )
#define zfree( a ) free( (a) )
#define zmallocFreeAll(x)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _ZMALLOC_H_ */

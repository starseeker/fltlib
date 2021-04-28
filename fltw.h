
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

#ifndef _FLTW_H_
#define _FLTW_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "flt.h"

#ifdef __cplusplus
extern "C" {
#endif

    FLTLIB_API uint32 fltWriteHeader( FILE * flt, FltHeader * header );
    FLTLIB_API uint32 fltWriteGroup( FILE * flt, FltGroup * group );
    FLTLIB_API FltHeader * fltBuildHeader( void );
    FLTLIB_API FltGroup * fltBuildGroup( char * name );
    FLTLIB_API uint32 fltWriteExternalReference( FILE * flt, FltExternalReference * extref );
    FLTLIB_API FltExternalReference * fltBuildExternalReference( char * refpath );
    FLTLIB_API uint32 fltWriteMatrix( FILE * flt, FltMatrix * mat );
    FLTLIB_API FltMatrix *
	fltBuildMatrix( real32 x, real32 y, real32 z, real32 h, real32 p, real32 r );
    FLTLIB_API FltMatrix * fltBuildMatrix4x4( float cmat[4][4] );
    FLTLIB_API FltMatrix * fltBuildMatrixSv( real32 *pos, real32 *ori,
	    real32 *scale );
    FLTLIB_API uint32 fltWritePushLevel( FILE * flt );
    FLTLIB_API uint32 fltWritePopLevel( FILE * flt );
    FLTLIB_API uint32 fltWriteDefaultVertexPalette( FILE * flt );
    FLTLIB_API uint32 fltWriteDefaultColorPalette( FILE * flt );
    FLTLIB_API uint32 fltWriteDefaultLightSourcePalette( FILE * flt );

#ifdef __cplusplus
}
#endif


#endif /* _FLTW_H_ */

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */

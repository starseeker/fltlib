
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
 * OpenFlight Parser
 * 
 * Author: Mike Morrison
 *         morrison@users.sourceforge.net
 */

#ifndef _FLTMATH_H_
#define _FLTMATH_H_

#include "flt.h"

#ifdef __cplusplus
extern "C" {
#endif

FLTLIB_API void FltMatrixLoadIdentity( FltMatrix * mat );
FLTLIB_API void FltBuildMatrix( FltMatrix * mat, real32 x, real32 y, real32 z, 
                            real32 h, real32 p, real32 r );
FLTLIB_API void FltMatrixMultiply( FltMatrix * first, FltMatrix * second );
FLTLIB_API void FltMatrixCopy( FltMatrix * dest, FltMatrix * src );
FLTLIB_API void FltMatrixVertMultiply( float vert[3], FltMatrix * mat );
FLTLIB_API void FltMatrixInverseTranspose( FltMatrix * mat );
FLTLIB_API void FltMatrixNormMultiply( float vert[3], FltMatrix * mat );

#ifdef __cplusplus
}
#endif


#endif /* _FLTMATH_H_ */


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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fltMath.h"

void
FltMatrixLoadIdentity( FltMatrix * mat )
{
    memset( mat->matrix, 0x00, sizeof( float ) * 16 );

    mat->matrix[0*4+0] = 1.0f;
    mat->matrix[1*4+1] = 1.0f;
    mat->matrix[2*4+2] = 1.0f;
    mat->matrix[3*4+3] = 1.0f;
}

void
FltBuildMatrix( FltMatrix * mat, real32 x, real32 y, real32 z, real32 h,
	real32 p, real32 r )
{
    if( mat )
    {
	// rotation (taken from sglMat4.hpp)
	real32	sr = sin(p),
	cr = cos(p),
	sh = sin(r),
	ch = cos(r),
	sp = sin(h),
	cp = cos(h);

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
}

/*
 * Multiply first*second, result goes to first
 */
void
FltMatrixMultiply( FltMatrix * first, FltMatrix * second )
{
    FltMatrix tmp;
    unsigned int i, j;

    for(i=0;i<4;i++)
    {
	for(j=0;j<4;j++)
	{
	    tmp.matrix[i*4+j] = ( first->matrix[i*4+0]*second->matrix[0*4+j] +
		    first->matrix[i*4+1]*second->matrix[1*4+j] +
		    first->matrix[i*4+2]*second->matrix[2*4+j] +
		    first->matrix[i*4+3]*second->matrix[3*4+j] );
	}
    }

    memcpy( first, &tmp, sizeof( FltMatrix ) );
}

/*
 * Copy src to dest
 */
void
FltMatrixCopy( FltMatrix * dest, FltMatrix * src )
{
    memcpy( dest, src, sizeof( FltMatrix ) );
}

/*
 * Multiply vertex vert by matrix mat
 */
void
FltMatrixVertMultiply( float vert[3], FltMatrix * mat )
{
    // vert->w = 1.0
    float tmp[3] = {vert[0], vert[1], vert[2]};

    vert[0] = tmp[0] * mat->matrix[0*4+0] +
	tmp[1] * mat->matrix[1*4+0] +
	tmp[2] * mat->matrix[2*4+0] +
	mat->matrix[3*4+0];

    vert[1] = tmp[0] * mat->matrix[0*4+1] +
	tmp[1] * mat->matrix[1*4+1] +
	tmp[2] * mat->matrix[2*4+1] +
	mat->matrix[3*4+1];

    vert[2] = tmp[0] * mat->matrix[0*4+2] +
	tmp[1] * mat->matrix[1*4+2] +
	tmp[2] * mat->matrix[2*4+2] +
	mat->matrix[3*4+2];
}

/*
 * Compute the transpose( inverse(M) ) of the upper 3x3 of the matrix
 * by computing the transpose of the adjoint of the matrix
 */
void
FltMatrixInverseTranspose( FltMatrix * mat )
{
    /*
     * taken from article at: http://www.gignews.com/realtime020100.htm on
     * transformation of normals.
     */

    /* cofactor for each element */

    mat->inverseTranspose[0*3+0] = mat->matrix[1*4+1] * mat->matrix[2*4+2] -
	mat->matrix[1*4+2] * mat->matrix[2*4+1] ;
    mat->inverseTranspose[0*3+1] = mat->matrix[1*4+2] * mat->matrix[2*4+0] -
	mat->matrix[1*4+0] * mat->matrix[2*4+2] ;
    mat->inverseTranspose[0*3+2] = mat->matrix[1*4+0] * mat->matrix[2*4+1] -
	mat->matrix[1*4+1] * mat->matrix[2*4+0] ;
    mat->inverseTranspose[1*3+0] = mat->matrix[2*4+1] * mat->matrix[0*4+2] -
	mat->matrix[2*4+2] * mat->matrix[0*4+1] ;
    mat->inverseTranspose[1*3+1] = mat->matrix[2*4+2] * mat->matrix[0*4+0] -
	mat->matrix[2*4+0] * mat->matrix[0*4+2] ;
    mat->inverseTranspose[1*3+2] = mat->matrix[2*4+0] * mat->matrix[0*4+1] -
	mat->matrix[2*4+1] * mat->matrix[0*4+0] ;
    mat->inverseTranspose[2*3+0] = mat->matrix[0*4+1] * mat->matrix[1*4+2] -
	mat->matrix[0*4+2] * mat->matrix[1*4+1] ;
    mat->inverseTranspose[2*3+1] = mat->matrix[0*4+2] * mat->matrix[1*4+0] -
	mat->matrix[0*4+0] * mat->matrix[1*4+2] ;
    mat->inverseTranspose[2*3+2] = mat->matrix[0*4+0] * mat->matrix[1*4+1] -
	mat->matrix[0*4+1] * mat->matrix[1*4+0] ;
}

/*
 * Multiply vertex normal by inverse transpose of matrix mat
 */
void
FltMatrixNormMultiply( float vert[3], FltMatrix * mat )
{
    // vert->w = 1.0
    float tmp[3] = { vert[0], vert[1], vert[2] };
    float norm;

    //
    // inverse transpose is a 3x3 matrix
    //
    vert[0] = tmp[0] * mat->inverseTranspose[0*3+0] +
	tmp[1] * mat->inverseTranspose[1*3+0] +
	tmp[2] * mat->inverseTranspose[2*3+0];

    vert[1] = tmp[0] * mat->inverseTranspose[0*3+1] +
	tmp[1] * mat->inverseTranspose[1*3+1] +
	tmp[2] * mat->inverseTranspose[2*3+1];

    vert[2] = tmp[0] * mat->inverseTranspose[0*3+2] +
	tmp[1] * mat->inverseTranspose[1*3+2] +
	tmp[2] * mat->inverseTranspose[2*3+2];

    //
    // now renormalize
    //
    norm = sqrt(vert[0]*vert[0]+vert[1]*vert[1]+vert[2]*vert[2]);

    if( norm > 0.00001f )
    {
	vert[0] /= norm;
	vert[1] /= norm;
	vert[2] /= norm;
    }
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

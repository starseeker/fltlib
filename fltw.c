
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
#include "flt.h"
#include "fltw.h"

int
main( int argc, char **argv )
{
    FILE * fltOut = fopen( "tst.flt", "w" );
    FltHeader * header = fltBuildHeader();
    FltExternalReference * extref = fltBuildExternalReference( "extref.flt" );
    FltMatrix * mat = fltBuildMatrix( 0, 0, 0, 0, 0, 0 );
    FltMatrix * mat2 = fltBuildMatrix( 200, 0, 0, 3.141592, 0, 0 );

    // first, write header
    fltWriteHeader( fltOut, header );

    // push level
    fltWritePushLevel( fltOut );

    // then reference
    fltWriteExternalReference( fltOut, extref );

    // then matrix attr
    fltWriteMatrix( fltOut, mat );

    // then second reference
    fltWriteExternalReference( fltOut, extref );

    // then second matrix attr
    fltWriteMatrix( fltOut, mat2 );

    // pop level
    fltWritePopLevel( fltOut );

    fclose( fltOut );

    return 0;
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

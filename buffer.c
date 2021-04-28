
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

void *
FltBufferResize( FltBuffer * buf, uint32 size )
{
    assert( buf );

    if( buf->bufferSize < size ) {
	buf->buffer = zrealloc( buf->buffer, size );
	buf->bufferSize = size;
    }

    return buf->buffer;
}

void
FltBufferRewind( FltBuffer * buf )
{
    assert( buf );

    buf->bytesRead = 0;
    buf->bytesRemaining = buf->readLength;
    buf->curPtr = buf->buffer;
}

void
FltBufferResetWithLength( FltBuffer * buf, uint32 len )
{
    assert( buf );

    buf->readLength = len;

    FltBufferRewind( buf );
}

FltBuffer *
FltBufferAlloc( FltFile * file )
{
    file->buffer = zcalloc( 1, sizeof( FltBuffer ) );

    return file->buffer;
}

void
FltBufferFree( FltFile * flt )
{
    if( flt->buffer ) {
	if( flt->buffer->buffer )
	    zfree( flt->buffer->buffer );
	zfree( flt->buffer );
    }
}

static void
bufferReadBytes( FltBuffer * buf, void * dest, uint32 len )
{
    memcpy( dest, buf->curPtr, len );

    // check below in skip block too!
    buf->curPtr += len;
    buf->bytesRead += len;
    buf->bytesRemaining -= len;
}

static int
bufferCheckLength( FltBuffer * buf, uint32 len )
{
    if( buf->bytesRemaining < len )
	return 0;
    else
	return 1;
}

int
fltSkipBlock( FltFile * flt, uint32 len )
{
    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, len ) ) {
	flt->buffer->curPtr += len;
	flt->buffer->bytesRead += len;
	flt->buffer->bytesRemaining -= len;
	return 1;
    } else
	return 0;
}

int
fltReadBlock( FltFile * flt, void * data, uint32 length )
{
    assert( flt );
    assert( flt->buffer );
    assert( data );

    if( bufferCheckLength( flt->buffer, length ) ) {
	bufferReadBytes( flt->buffer, data, length );
	return 1;
    } else
	return 0;
}

uint32
fltReadUInt32( FltFile * flt )
{
    uint32 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_32( d );

    return d;
}

int32
fltReadInt32( FltFile * flt )
{
    int32 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_32( d );

    return d;
}


uint16
fltReadUInt16( FltFile * flt )
{
    uint16 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_16( d );

    return d;
}

int16
fltReadInt16( FltFile * flt )
{
    int16 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_16( d );

    return d;
}

uint8
fltReadUInt8( FltFile * flt )
{
    uint8 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    return d;
}

int8
fltReadInt8( FltFile * flt )
{
    int8 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    return d;
}

real32
fltReadReal32( FltFile * flt )
{
    real32 d;

    assert( flt );
    assert( flt->buffer );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_32r( d );

    return d;
}

real64
fltReadReal64( FltFile * flt )
{
    real64 d;

    assert( flt );
    assert( flt->stdFile );

    if( bufferCheckLength( flt->buffer, sizeof( d ) ) )
	bufferReadBytes( flt->buffer, &d, sizeof( d ) );
    else
	// flag error
	return 0;

    ENDIAN_64( d );

    return d;
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

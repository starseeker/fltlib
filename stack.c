
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
#include "flt.h"
#include "zmalloc.h"

void 
FltStackPush( FltStack * stack, void * entry )
{
	assert( stack );

	if( stack->stackPtr >= stack->stackSize ) {
		// alloc an additional 10 entries
		stack->stack = zrealloc( stack->stack, (stack->stackSize + 10) * 
																								sizeof( void * ) );
		stack->stackSize += 10;
	}

	stack->stack[ stack->stackPtr++ ] = entry;
}

void * 
FltStackPop( FltStack * stack )
{
	assert( stack );
	assert( stack->stackPtr ); // check for underrun

	return stack->stack[ --stack->stackPtr ];
}

void * 
FltStackGetCurrent( FltStack * stack )
{
	assert( stack );
//	assert( stack->stackPtr ); // check for underrun

	if( stack->stackPtr )
		return stack->stack[ (stack->stackPtr-1) ];
	else	
		return NULL;
}

void * 
FltStackAlloc( FltFile * flt )
{
	assert( flt );

	flt->stack = zcalloc( 1, sizeof( FltStack ) );

	// start it off with 10 entries
	flt->stack->stack = zrealloc( 0, 10 * sizeof( void * ) );
	flt->stack->stackSize = 10;

	return flt->stack;
}

void
FltStackFree( FltFile * flt )
{
	if( flt->stack ) {
		if( flt->stack->stack )
			zfree( flt->stack->stack );
		zfree( flt->stack );
	}
}


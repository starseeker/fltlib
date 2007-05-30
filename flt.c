
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

void *
callback( FLTRECORDUSERCALLBACK_ARGLIST )
{
	FltVertex * vert = (FltVertex *)recData;

	printf("v: (%f,%f,%f)\n", vert->x, vert->y, vert->z );

	return 0;
}

void *
callbackList( FLTRECORDUSERCALLBACK_ARGLIST )
{
	uint32 i;
	FltVertexList * list = (FltVertexList *)recData;

	printf("VList: ");
	for(i=0;i<list->numVerts;i++)
		printf("%p ", list->list[i] );
	printf("\n");

	return 0;
}

void
printRecs( FltNode * node )
{
	char string[300];
	FltRecord * rec;
	int x = 0;

	if( (rec = fltRecordGetDefinition( node->type )) ) {
		char * nn = fltSafeNodeName( node );
		printf("Node Type: '%s': %s\n", rec->name, nn );
		while( rec->entry[x].type ) {
			fltRecordEntryNameToString( string, node, &rec->entry[x] );
			printf("%s\n", string);
			x ++;
		}
	}
}

void
descendChildren( FltNode * node )
{
	unsigned int i;

	if( FLTNODE_CANPARENT( node->type ) ) {
		printf("%d: %s <- %s -> %s {\n", node->treeDepth,
							fltSafeNodeName( node->prev ),
							fltSafeNodeName( node ),
							fltSafeNodeName( node->next ) );
		printRecs( node );
		printf("}\n");
	} else {
		printf("%d: leaf {\n", node->treeDepth );

		printRecs( node );

		printf("}\n");
	}

	for( i=0;i<node->numChildren;i++ )
		descendChildren( node->child[i] );
}

void
walkLinear( FltFile * flt )
{
	unsigned int i;
	FltNode * rec;

	for(i=0;i<flt->numNodes;i++) {
		printRecs( flt->allNodes[i] );

		rec = flt->allNodes[i]->attr;

		if( rec ) {
			printf("--ATTR--\n");
			while( rec ) {
				printRecs( rec );
				rec = rec->next;
			}
			printf("--END ATTR--\n");
		}
		printf("\n");
	}
}

int
main( int argc, char **argv )
{
	FltFile * flt = 0;
	FltTxAttributes * attrs = 0;

   if( argc < 2 ) {
      printf("Usage: %s flt_file, or %s -a attr_file\n", argv[0], argv[0] );
      exit(0);
   }

   if( argv[1][0] == '-' && argv[1][1] == 'a' && argc < 3 ) {
      printf("Usage: %s flt_file, or %s -a attr_file\n", argv[0], argv[0] );
      exit(0);
   }

	if( !strcmp( argv[1], "-a" ) ) {
		attrs = fltLoadAttributes( argv[2] );
		if( attrs ) {
			printf("uTexels: %d\n", attrs->uTexels );
			printf("vTexels: %d\n", attrs->vTexels );
			printf("minificationFilter: %d\n", attrs->minificationFilter );
			printf("magnificationFilter: %d\n", attrs->magnificationFilter );
			printf("repetitionType: %d\n", attrs->repetitionType );
			printf("environmentType: %d\n", attrs->environmentType );
			printf("detailTexture: %d\n", attrs->detailTexture );
			printf("detailJ: %d\n", attrs->detailJ );
			printf("detailK: %d\n", attrs->detailK );
			printf("detailM: %d\n", attrs->detailM );
			printf("detailN: %d\n", attrs->detailN );
			printf("scramble: %d\n", attrs->detailScramble );
		} else
			printf("unable to open: %s\n", argv[2] );
		fltFreeAttributes( attrs );
	} else {

		flt = fltOpen( argv[1] );
		printf("flt->fileName: %s\n", flt->fileName );
		fflush(stdout);

	//	fltRegisterRecordUserCallback( 68, callback, NULL );
	//	fltRegisterRecordUserCallback( 69, callback, NULL );
	//	fltRegisterRecordUserCallback( 70, callback, NULL );
	//	fltRegisterRecordUserCallback( 71, callback, NULL );

	//	fltRegisterRecordUserCallback( 72, callbackList, NULL );

		if( argc > 2 && argv[2][0] == 's' )
			fltParse( flt, 1 );
		else
			fltParse( flt, 0 );

		fltClose( flt );

		fflush(stdout);
		walkLinear( flt );

		printf("\n\n------TREE-------\n\n");
		descendChildren( (FltNode *)flt->header );

	}

	fltFileFree( flt );

	return 0;
}

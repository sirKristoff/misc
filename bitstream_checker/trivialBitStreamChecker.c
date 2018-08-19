/*
 * trivialBitStreamChecker.c
 *
 *  Created on: 19.08.2018
 *      Author: Krzysztof Lasota
 */

#include <stdlib.h>

#include "trivialBitStreamChecker.h"

#define BIT_LOW  -1
#define BIT_HIGH  1
#define SEQ_NO_LIMIT 3


struct StreamCtxt
{
	uint8_t streamNo;
	int8_t bitSequenceState;
};

static struct StreamCtxt* ctxtList = 0;
static size_t ctxtListSize = 0;

static void freeStreamCtxtList(void)
{
	free(ctxtList);
	ctxtList = 0;
	ctxtListSize = 0;
}

static int8_t* getBitSequenceState(uint8_t streamNo)
{
	if( ! ctxtList ){
		if( (ctxtList = malloc(sizeof(struct StreamCtxt))) ){
			atexit(freeStreamCtxtList);
			ctxtListSize = 1u;
			ctxtList->streamNo = streamNo;
			ctxtList->bitSequenceState = 0;
		}
		else
			return 0;  // failure of context allocation
	}

	// find context for streamNo on list
	struct StreamCtxt* ctxt = 0;
	for( size_t idx = 0 ; idx < ctxtListSize ; ++idx ){
		if( streamNo == ctxtList[idx].streamNo ){
			ctxt = &ctxtList[idx];
			break;
		}
	}

	if( ! ctxt )
	{  // no context for streamNo on list - add new context to list
		struct StreamCtxt* const reallocatedCtxtList =
			realloc(ctxtList, (ctxtListSize+1)*sizeof(struct StreamCtxt));
		if( reallocatedCtxtList ){
			ctxtList = reallocatedCtxtList;
			ctxtList[ctxtListSize].streamNo = streamNo;
			ctxtList[ctxtListSize].bitSequenceState = 0;
			ctxt = &ctxtList[ctxtListSize];
			++ctxtListSize;
		} else
			return 0;  // failure of context reallocation
	}

	return &ctxt->bitSequenceState;
}

static int8_t getBitState(uint8_t bit)
{
	switch ( bit & 0x01 ) {
		case 1:
			return BIT_HIGH;
		case 0:
			return BIT_LOW;
	}
	return 0;  // should never happen
}

bool verify(uint8_t bit, uint8_t streamNo)
{
	int8_t bitState = getBitState(bit);
	int8_t* bitSequenceState = getBitSequenceState(streamNo);

	if( 0 <= (*bitSequenceState * bitState) )
	{ // current bit with this same state as last sequence
		*bitSequenceState += bitState;
	}
	else
	{ // current bit state and last sequence differ - change sequence
		*bitSequenceState = bitState;
	}

	if( SEQ_NO_LIMIT <= abs(*bitSequenceState) )
	{
		// prevent bitSequenceState value to exceed limit of int8_t
		*bitSequenceState -= bitState;
		return false;
	}
	else
		return true;
}

#undef BIT_LOW
#undef BIT_HIGH
#undef SEQ_NO_LIMIT

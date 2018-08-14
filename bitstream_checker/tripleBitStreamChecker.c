/*
 * tripleBitStreamChecker.c
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */

#include "tripleBitStreamChecker.h"

#include <stdlib.h>

struct TripleBitStreamCheckerCtxt
{
	BitChunk lastChunk;
};


// hidden functions as implementations for public class methods
static void tbsc_free(void* self);
static void tbsc_clean(void* self);
static bool tbsc_verify_first_call(struct TripleBitStreamChecker* self, BitChunk chunk);
static bool tbsc_verify(struct TripleBitStreamChecker* self, BitChunk chunk);


void* alloc_TripleBitStreamChecker(void)
{
	TripleBitStreamChecker* obj =
			(TripleBitStreamChecker*)malloc(sizeof(TripleBitStreamChecker));
	init_TripleBitStreamChecker(obj);
	return obj;
}
void init_TripleBitStreamChecker(TripleBitStreamChecker* obj)
{
	init_BitStreamChecker((BitStreamChecker*)obj);

	// allocate context if needed
	obj->ctxt =
			(TripleBitStreamCheckerCtxt*)malloc(sizeof(TripleBitStreamCheckerCtxt));
	// initialize context
	obj->ctxt->lastChunk = 0;

	// bind destructor
	obj->free_self = tbsc_free;
	obj->clean_self = tbsc_clean;
	// bind methods
	obj->verify = tbsc_verify_first_call;
}

static void tbsc_clean(void* self)
{
	TripleBitStreamChecker* obj = (TripleBitStreamChecker*) self;

	// clean context
	// free context and null
	free(obj->ctxt);
	obj->ctxt = 0;

	// call cleanup for Super class
	clean_BitStreamChecker((BitStreamChecker*) self);
}

static void tbsc_free(void* self)
{
	tbsc_clean(self);
	free(self);
}


#define NO_OF_BITS_IN_MASK  3u
#define MASK  0x07u  // 0000 0111

static bool tbsc_verify_first_call(struct TripleBitStreamChecker* self, BitChunk chunk)
{
	BitChunk mask = MASK;

	self->verify = tbsc_verify;
	self->ctxt->lastChunk = chunk;

	// move bits in mask by one position to the left
	for( unsigned i = NO_OF_BITS_IN_MASK-1 ; i < self->sCtxt->bitChunkNoOfBits ; ++i, mask<<=1 )
	{
		// if all bits on 'mask' position are set in chunk
		if( (chunk&mask) == mask )
			return false;
		// if all bits on 'mask' position are not set in chunk
		else if( (~chunk&mask) == mask )
			return false;
	}
	return true;
}

static bool tbsc_verify(struct TripleBitStreamChecker* self, BitChunk chunk)
{
  { // Checking of bits from last chunk
	BitChunk mask = MASK;
	BitChunk lastChunk = self->ctxt->lastChunk;
	self->ctxt->lastChunk = chunk;  // store current chunk for future

	const unsigned nBitsToCheckInLastChunk = NO_OF_BITS_IN_MASK-1;
	// move N oldest bits to the right, N = number_of_bits_in_mask - 1
	lastChunk >>= self->sCtxt->bitChunkNoOfBits-nBitsToCheckInLastChunk;
	// append bits from chunk to checkable bits from last chunk
	lastChunk |= chunk << nBitsToCheckInLastChunk;

	for( unsigned i = 0 ; i < nBitsToCheckInLastChunk ; ++i, mask<<=1 )
	{
		// if all bits on 'mask' position are set in chunk
		if( (lastChunk&mask) == mask )
			return false;
		// if all bits on 'mask' position are not set in chunk
		else if( (~lastChunk&mask) == mask )
			return false;
	}
  }

  { // Checking of bits from current chunk
	BitChunk mask = MASK;
	// move bits in mask by one position to the left
	for( unsigned i = NO_OF_BITS_IN_MASK-1 ; i < self->sCtxt->bitChunkNoOfBits ; ++i, mask<<=1 )
	{
		// if all bits on 'mask' position are set in chunk
		if( (chunk&mask) == mask )
			return false;
		// if all bits on 'mask' position are not set in chunk
		else if( (~chunk&mask) == mask )
			return false;
	}
	return true;
  }
}

#undef NO_OF_BITS_IN_MASK
#undef MASK

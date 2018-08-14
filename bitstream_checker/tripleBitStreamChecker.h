/*
 * tripleBitStreamChecker.h
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */

#ifndef TRIPLEBITSTREAMCHECKER_H_
#define TRIPLEBITSTREAMCHECKER_H_

#include "bitStreamChecker.h"


typedef StaticBitStreamChecker StaticTripleBitStreamChecker;

typedef struct TripleBitStreamCheckerCtxt TripleBitStreamCheckerCtxt;


typedef struct TripleBitStreamChecker
{
	StaticTripleBitStreamChecker* sCtxt;
	TripleBitStreamCheckerCtxt* ctxt;  // private

	void (*free_self)(void* self);
	void (*clean_self)(void* self);

	bool (*verify)(struct TripleBitStreamChecker* self, BitChunk chunk);
} TripleBitStreamChecker;

void* alloc_TripleBitStreamChecker(void);
void init_TripleBitStreamChecker(TripleBitStreamChecker* obj);


#endif /* TRIPLEBITSTREAMCHECKER_H_ */

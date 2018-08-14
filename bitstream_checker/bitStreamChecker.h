/*
 * BitStreamChecker.h
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */

#ifndef BITSTREAMCHECKER_H_
#define BITSTREAMCHECKER_H_

#include <stdbool.h>
#include <stdint.h>


typedef uint8_t BitChunk;

typedef struct StaticBitStreamChecker
{
	unsigned bitChunkNoOfBits;
} StaticBitStreamChecker;
extern StaticBitStreamChecker sBitStreamChecker;

typedef struct BitStreamCheckerCtxt BitStreamCheckerCtxt;


typedef struct BitStreamChecker
{
	StaticBitStreamChecker* sCtxt;
	BitStreamCheckerCtxt* ctxt;

	void (*free_self)(void* self);
	void (*clean_self)(void* self);

	bool (*verify)(struct BitStreamChecker* self, BitChunk in);
} BitStreamChecker;

void* alloc_BitStreamChecker(void);
void init_BitStreamChecker(BitStreamChecker* obj);
void clean_BitStreamChecker(BitStreamChecker* obj);


#endif /* BITSTREAMCHECKER_H_ */

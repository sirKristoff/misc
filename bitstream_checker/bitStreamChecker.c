/*
 * BitStreamChecker.c
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */

#include "bitStreamChecker.h"

#include <stdlib.h>

struct StaticBitStreamChecker sBitStreamChecker = {8u};


// hidden functions as implementations for public class methods
static void bsc_free(void* self);
static void bsc_clean(void* self);


void* alloc_BitStreamChecker(void)
{
	BitStreamChecker* obj = (BitStreamChecker*)malloc(sizeof(BitStreamChecker));
	init_BitStreamChecker(obj);
	return obj;
}
void init_BitStreamChecker(BitStreamChecker* obj)
{
	obj->sCtxt = &sBitStreamChecker;
	// allocate context if needed
	// initialize context
	obj->ctxt = 0; // no context in this class

	// bind destructor
	obj->free_self = bsc_free;
	obj->clean_self = bsc_clean;
	// bind methods
	obj->verify = 0; // pure virtual method
}

void clean_BitStreamChecker(BitStreamChecker* obj)
{
	// clean context
	// free context and null
	obj->ctxt = 0;  // no context to free
}


static void bsc_clean(void* self)
{
	clean_BitStreamChecker((BitStreamChecker*) self);
}

static void bsc_free(void* self)
{
	bsc_clean(self);
	free(self);
}


/*
 * main.c
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */
#include <stdbool.h>

#include "tripleBitStreamChecker.h"


const unsigned streamSize = 6;
BitChunk stream[6/*streamSize*/] =
	{0x55u,   0xAAu,   0x33u,   0xCCu,   0xDD,    0x5Au};
//   01010101 10101010 00110011 11001100 11011101 01011010

/**
 * @brief main application function
 */
int main()
{
	BitStreamChecker* bsc = (BitStreamChecker*)alloc_TripleBitStreamChecker();

	for( unsigned i = 0 ; i<streamSize ; ++i ){
		if( ! bsc->verify(bsc, stream[i]) ){
			bsc->free_self(bsc);
			return i+1;
		}
	}

	bsc->free_self(bsc);
	return 0;
}





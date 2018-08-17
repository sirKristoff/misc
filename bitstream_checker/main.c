/*
 * main.c
 *
 *  Created on: 13.08.2018
 *      Author: Krzysztof Lasota
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "tripleBitStreamChecker.h"
#include "vectorContainer.h"


BitChunk getChunk(size_t id);


typedef struct ContainerElement
{
	unsigned key;
	BitStreamChecker* checker;
} ContainerElement;
void freeContainerElement(void* obj);

unsigned registerBitStreamChecker(SetContainer* container, BitStreamChecker* bsc);

/**
 * @brief main application function
 */
int main()
{
	VectorContainer container;
	init_VectorContainer(&container, freeContainerElement);

	unsigned checkerId[2] =
		{
		registerBitStreamChecker((SetContainer*) &container,
				alloc_TripleBitStreamChecker()),
		registerBitStreamChecker((SetContainer*) &container,
				alloc_TripleBitStreamChecker())
		};


	unsigned streamId, chunkNo;
	for( chunkNo = 0, streamId = 0; chunkNo<10 ; ++chunkNo, streamId= (streamId+1) % 2 ){
		BitStreamChecker* bsc =
				((ContainerElement*) container.find(&container, checkerId[streamId]))->checker;
		if( ! bsc->verify(bsc, getChunk(streamId)) ){
			clean_VectorContainer(&container);
			return chunkNo+1 + streamId*100;
		}
	}

	clean_VectorContainer(&container);
	return 0;
}


BitChunk getChunkFrom0(void)
{
	const unsigned streamSize = 6;
	static size_t idx = 0;
	static BitChunk stream[6/*streamSize*/] =
		{0x55u,   0xAAu,   0x33u,   0xCCu,   0xDD,    0x5Au};
//       01010101 10101010 00110011 11001100 11011101 01011010

	if( idx == streamSize )
		return 0u;

	return stream[idx++];
}

BitChunk getChunkFromDefault(void)
{
	return 0x95u;
}

BitChunk getChunk(size_t id)
{
	switch (id) {
		case 0:
			return getChunkFrom0();
			break;
		default:
			return getChunkFromDefault();
			break;
	}
}


void freeContainerElement(void* obj)
{
	ContainerElement* element = (ContainerElement*) obj;
	element->checker->free_self(element->checker);
	free(obj);
}


unsigned registerBitStreamChecker(SetContainer* container, BitStreamChecker* bsc)
{
	ContainerElement* element = malloc(sizeof(ContainerElement));
	element->checker = bsc;
	return container->insert(container, (SetElement*) element)->key;
}

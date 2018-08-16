/*
 * setContainer.h
 *
 *  Created on: 14.08.2018
 *      Author: Krzysztof Lasota
 */

#ifndef SETCONTAINER_H_
#define SETCONTAINER_H_

#include <stddef.h>

/**
 * Abstract element storeable in SetContainer
 */
typedef struct SetElement
{
	unsigned key;
	// Value of Element at end of struct
} SetElement;

typedef struct SetContainerCtxt SetContainerCtxt; //!< Abstract context for SetContainer


typedef struct SetContainer
{
	SetContainerCtxt* ctxt;

	void (*free_self)(void* self);
	void (*clean_self)(void* self);


	size_t (*capacity)(struct SetContainer* self);

	/**
	 * @brief Removes all elements from the container.
	 * Leaving the container with a size of @c 0.
	 */
	void (*clear)(struct SetContainer* self);

	/**
	 * @brief Searches the container for an element associated to @a key
	 * @return Element if found, otherwise NULL.
	 */
	SetElement* (*find)(struct SetContainer* self, unsigned key);

	/** Removes from the container a single element.
	 */
	void (*erase)(struct SetContainer* self, const SetElement*);

	/**
	 * @brief Extends the container by inserting new element.
	 * @return An already stored element in container, on failure return NULL
	 */
	SetElement* (*insert)(struct SetContainer* self, SetElement* newElement);

	/** Returns the number of elements in the container.
	 */
	size_t (*size)(struct SetContainer* self);
} SetContainer;

#endif /* SETCONTAINER_H_ */

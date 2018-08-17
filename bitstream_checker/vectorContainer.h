/*
 * vectorContainer.h
 *
 *  Created on: 14.08.2018
 *      Author: Krzysztof Lasota
 */

#ifndef VECTORCONTAINER_H_
#define VECTORCONTAINER_H_

#include "setContainer.h"


typedef void(*FreeSetElementFun)(void*); //!< Function type for free SetElement

typedef struct VectorContainerCtxt VectorContainerCtxt;


typedef struct  VectorContainer
{
	VectorContainerCtxt* ctxt;

	void (*free_self)(void* self);
	void (*clean_self)(void* self);


	size_t (*capacity)(struct VectorContainer* self);

	/** @brief Removes all elements from the container.
	 *  Leaving the container with a size of @c 0.
	 *  @note For each element @a FreeSetElementFun will be called.
	 */
	void (*clear)(struct VectorContainer* self);

	/** @brief Searches the container for an element associated to @a key
	 * @return Element if found, otherwise NULL.
	 */
	SetElement* (*find)(struct VectorContainer* self, unsigned key);

	/**
	 * @brief Removes from the container a single element.
	 * @note @a FreeSetElementFun will NOT be called for element.
	 */
	void (*erase)(struct VectorContainer* self, const SetElement*);

	/**
	 * @brief Extends the container by inserting new element.
	 * @return An already stored element in container, on failure return NULL.
	 * @note If newElement does not contain key (equal 0),
	 *       first available key is assigned to newElement will.
	 * @note If newElement's key is associated with already existing @a oldElement
	 *       in container, the old one will be replaced and @a FreeSetElementFun
	 *       will be called on it.
	 *
	 */
	SetElement* (*insert)(struct VectorContainer* self, SetElement* newElement);

	/**
	 * @brief Returns the number of elements in the container.
	 * @note size include NULL-elements.
	 */
	size_t (*size)(struct VectorContainer* self);
} VectorContainer;

void* alloc_VectorContainer(FreeSetElementFun freeOperator);
void init_VectorContainer(VectorContainer* obj, FreeSetElementFun freeOperator);
void clean_VectorContainer(VectorContainer* obj);

#endif /* VECTORCONTAINER_H_ */

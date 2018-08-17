/*
 * vectorContainer.c
 *
 *  Created on: 14.08.2018
 *      Author: Krzysztof Lasota
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "vectorContainer.h"

struct VectorContainerCtxt
{
	size_t size;  //!< number of elements stored in container
	size_t capacity;  //!< number of elements allocated in container
	SetElement** elements;  //!< pointers to elements stored in container
	FreeSetElementFun freeElementOperator;  // function for destructing elements
};

// hidden functions as implementations for public class methods
static void vc_free(void* self);
static void vc_clean(void* self);

static size_t vc_capacity(VectorContainer* self);
static void vc_clear(VectorContainer* self);
static SetElement* vc_find(VectorContainer* self, unsigned key);
static void vc_erase(VectorContainer* self, const SetElement* element);
static SetElement* vc_insert(VectorContainer* self, SetElement* newElement);
static size_t vc_size(VectorContainer* self);

static bool vc_isKeyElementSpecified(const SetElement* const element);
static size_t vc_elementIdx(VectorContainerCtxt* ctxt, const SetElement* const element);


void* alloc_VectorContainer(FreeSetElementFun freeOperator)
{
	VectorContainer* obj = (VectorContainer*) malloc(sizeof(VectorContainer));
	init_VectorContainer(obj, freeOperator);
	return obj;
}

void init_VectorContainer(VectorContainer* obj, FreeSetElementFun freeOperator)
{
	obj->ctxt = (VectorContainerCtxt*) malloc(sizeof(VectorContainerCtxt));
	obj->ctxt->size = 0;
	obj->ctxt->capacity = 1;
	obj->ctxt->elements =
			(SetElement**) malloc(sizeof(SetElement*) * obj->ctxt->capacity);
	obj->ctxt->freeElementOperator = freeOperator;

	// bind destructor
	obj->free_self = vc_free;
	obj->clean_self = vc_clean;
	// bind methods
	obj->capacity = vc_capacity;
	obj->clear = vc_clear;
	obj->find = vc_find;
	obj->erase = vc_erase;
	obj->insert = vc_insert;
	obj->size = vc_size;
}

void clean_VectorContainer(VectorContainer* obj)
{
	// clean Context
	if( obj->ctxt ){
		vc_clear(obj);
		free(obj->ctxt->elements);
	}
	free(obj->ctxt);
	obj->ctxt = 0;
}

static void vc_free(void* self)
{
	vc_clean(self);
	free(self);
}
static void vc_clean(void* self)
{
	clean_VectorContainer((VectorContainer*) self);
}


static size_t vc_capacity(VectorContainer* self)
{
	return self->ctxt->capacity;
}

static void vc_clear(VectorContainer* self)
{
	if( self->ctxt->freeElementOperator ){
		FreeSetElementFun freeOp = self->ctxt->freeElementOperator;
		for( size_t i = 0 ; i<self->ctxt->size ; ++i )
			if( self->ctxt->elements[i] )
				freeOp(self->ctxt->elements[i]);
	}
	self->ctxt->size = 0;
	// TODO: change capacity and allocation of elements
}

static SetElement* vc_find(VectorContainer* self, unsigned key)
{
	SetElement** elements = self->ctxt->elements;
//	for( size_t idx = 0 ; idx < self->ctxt->size ; ++idx )
	SetElement elementToFind  = {key};
	size_t idx = vc_elementIdx(self->ctxt, &elementToFind);
		if( 0 != elements[idx] && key == elements[idx]->key )
			return elements[idx];
	return 0;
}

static void vc_erase(VectorContainer* self, const SetElement* element)
{
	VectorContainerCtxt* ctxt = self->ctxt;
	SetElement** elements = ctxt->elements;
	size_t index = vc_elementIdx(ctxt, element); //!< index of element in elements table

	// element not in container
	if( ! (index < ctxt->size) || ! elements[index] )
		return;

	// remove element from container
	elements[index] = 0;

	// reduce size if element is on back or last element is NULL
	while( ctxt->size && !elements[index] && ((index+1) == ctxt->size) ){
		ctxt->size--;
		--index;
	}

	// reduce capacity
	if( (2*ctxt->size) < ctxt->capacity ){
		size_t newCapacity = (ctxt->size ? ctxt->size : 1);

		if( ctxt->capacity == newCapacity )
			return;  // no reallocation needed

		// reallocate elements
		SetElement** realocatedElements =
			realloc(elements, newCapacity*sizeof(SetElement*));
		if( realocatedElements ){
			ctxt->capacity = newCapacity;
			ctxt->elements = realocatedElements;
		}
	}
}

/**
 * @returns false if element's key is 0
 */
static bool vc_isKeyElementSpecified(const SetElement* const element)
{
	if( element->key )
		return true;
	else
		return false;
}

/** Get index of element in elements table located in context
 * @return index of element or next available index in container if key not specified
 * @note keys are numbered from 1
 */
static size_t vc_elementIdx(VectorContainerCtxt* ctxt, const SetElement* const element)
{
	if( vc_isKeyElementSpecified(element) )
		return element->key - 1;
	return ctxt->size;
}

static SetElement* vc_insert(VectorContainer* self, SetElement* newElement)
{
	VectorContainerCtxt* ctxt = self->ctxt;
	SetElement** elements = ctxt->elements;
	size_t index = vc_elementIdx(ctxt, newElement); //!< index of newElement in elements table

	// if newElement not contains key
	if( ! vc_isKeyElementSpecified(newElement) )
		newElement->key = index+1; // chose next available key

	// if capacity to low - reallocate vector with elements
	if( ctxt->capacity <= index ){
		size_t newCapacity = ((ctxt->capacity==index) ? 2*index : index+1);
		SetElement** realocatedElements =
			realloc(ctxt->elements, newCapacity*sizeof(SetElement*));
		if( realocatedElements ){
			ctxt->capacity = newCapacity;
			ctxt->elements = realocatedElements;
			elements = ctxt->elements;
		}else{
			return 0; // reallocation failure
		}
	}

	// if newElement already exists in container - replace
	if( index < ctxt->size && 0 != elements[index] ) {
		ctxt->freeElementOperator(elements[index]);
		elements[index] = 0;
	}

	// if vector become sparse after insert - assign NULL to non-existing elements
	if( ctxt->size < index )
		memset(&elements[ctxt->size], 0, (index-ctxt->size)*sizeof(SetElement*));

	// resize required
	if( ctxt->size <= index )
		ctxt->size = index + 1;

	// insert new element on proper position
	elements[index] = newElement;

	return newElement;
}

static size_t vc_size(VectorContainer* self)
{
	return self->ctxt->size;
}

/**
 ******************************************************************************
 * @file      IShape.h
 *
 * @brief     Common container for geometric shapes defined by a number of
 *            coordinates.
 ******************************************************************************
 */

#ifndef ISHAPE_H
#define ISHAPE_H


/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
 */

#include <stddef.h>

#include "RoboticTypes.h"


/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
 */


/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
 */

/**
 * Common container type for geometric shapes, containing a number of coordinates
 */
typedef struct
{
    uint16          __private_size__;     /*< Current number of vertices */
    uint16          __private_alloced__;  /*< Maximum number of vertices */
    tCoordinate2D*  __private_vertices__; /*< Pointer to data buffer */
    tDistance       __private_width__;    /*< The total width with half distance on each side of the polyline, eg Path */

} tIShape;


/*
 ------------------------------------------------------------------------------
    Interface functions
 ------------------------------------------------------------------------------
 */

/**
 ******************************************************************************
 * @brief   Initializes the shape by providing the memory buffer for the
 *          shape's vertices.
 *          NOTE! This function is somewhat like the std::deque::deque(), but
 *          not really, as the user provides the memory buffer.
 *          NOTE: SOFTWARE_ASSERTs that pointers are not null
 * @param   pShape
 *          shape struct to initialize
 * @param   capacity
 *          number of bytes in the provided buffer (i.e. MaxSize() will not be
 *          == capacity, but rather capacity / sizeof( element ) afterwards)
 * @param   pMem
 *          pointer to the provided memory buffer, where elements will be
 *          placed when e.g. PushBack() is called.
 ******************************************************************************
 */
void IShape_Init( tIShape * const pShape, const size_t capacity, void * const pMem );

/**
 ******************************************************************************
 * @brief   De-initializes the shape.
 *          NOTE! This function is somewhat like the std::deque::~deque(), but
 *          not really, as the user provides the memory buffer.
 *          NOTE! In case malloc-ed memory is provided in the Init() function
 *          call, the caller **MUST** free it before this function
 *          is called, as the pointer to the buffer is cleared!
 * @param   pShape
 *          shape struct to de-initialize
 ******************************************************************************
 */
void IShape_DeInit( tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Returns a copy of the element at position n in the shape
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          the shape object
 * @param   n
 *          position of an element in the shape
 * @return  A copy of the n-th element in the shape, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D IShape_At( const tIShape* const pShape, const size_t n );

/**
 ******************************************************************************
 * @brief   Returns a copy of the first coordinate of a shape
 *          Same as calling IShape_At( pShape, 0 )
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          the shape object
 * @return  A copy of the first coordinate in the shape, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D IShape_Front( const tIShape * const pShape );

/**
 ******************************************************************************
 * @brief   Returns a copy of the last coordinate of a shape
 *          Can be used to access the last element of a shape.
 *          Useful if long variable names and to not mess up
 *          indices.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          the shape object
 * @return  A copy of the last coordinate in the shape, nullCoord2D otherwise
 ******************************************************************************
 */
tCoordinate2D IShape_Back( const tIShape * const pShape );

/**
 ******************************************************************************
 * @brief   Sets size and width of shape to zero
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to clear
 * @return  -
 ******************************************************************************
 */
void IShape_Clear( tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Copies the content and current size from one shape to another.
 *          NOTE! This functions is somewhat like the std::deque::operator=(),
 *          but since IShape does not allocate any memory the user must
 *          ensure that the maxSize of pShapeTo is at least as large as the
 *          current size of pShapeFrom or the operation will fail.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pSrc
 *          shape to copy data from
 * @param   pDst
 *          shape to copy data from
 * @return  true if shape was successfully copied, false otherwise
 ******************************************************************************
 */
bool IShape_Copy( const tIShape* const pSrc, tIShape* const pDst );

/**
 ******************************************************************************
 * @brief   Returns whether the shape is empty (i.e. whether its size is 0)
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to check for empty
 * @return  true if the shape size is 0, false otherwise
 ******************************************************************************
 */
bool IShape_IsEmpty( const tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Checks if shape is the null shape. Using other functions on null
 *          shapes will likely result in SOFTWARE_EXCEPTION
 * @param   pShape
 *          shape to check
 * @return  true if vertices is NULL.
 ******************************************************************************
 */
bool IShape_IsNull( const tIShape * const pShape );

/**
 ******************************************************************************
 * @brief   Returns a pointer to the elements (coordinates) array in the shape
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to get pointer from
 * @return  A pointer to the elements (coordinates) array in the shape
 ******************************************************************************
 */
tCoordinate2D* IShape_GetElements( const tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Inserts a coordinate at the back of a shape
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to add on
 * @param   pCoordinate
 *          what coordinate to add
 * @return  true if memory available
 ******************************************************************************
 */
bool IShape_PushBack( tIShape* const pShape, const tCoordinate2D * const pCoordinate );

/**
 ******************************************************************************
 * @brief   Removes the last element of the shape
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to remove from
 * @return  -
 ******************************************************************************
 */
void IShape_PopBack( tIShape *pShape );

/**
 ******************************************************************************
 * @brief   Replaces the element at an index with a new one
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to modify
 * @param   index
 *          index of the element to replace
 * @param   pCoord
 *          new coordinate to replace with
 * @return  false if index was invalid
 ******************************************************************************
 */
bool IShape_Replace( const tIShape* const pShape, const uint16 index, const tCoordinate2D* const pCoord );

/**
 ******************************************************************************
 * @brief   Reduces the Shape's MaxSize() value to the current Size() value.
 *          NOTE: This does not free any allocated memory, only prohibits users
 *          from adding more vertices to the shape.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to reduce the MaxSize for
 * @return  -
 ******************************************************************************
 */
void IShape_ShrinkToFit( tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Returns the number of elements in the shape
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to get size for
 * @return  The number of elements in the shape
 ******************************************************************************
 */
uint16 IShape_Size( const tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Returns the maximum number of elements that the shape can hold
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to get max size for
 * @return  The maximum number of elements a shape can hold as content
 ******************************************************************************
 */
uint16 IShape_MaxSize( const tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Insert the elements of one shape at end of another shape.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShapeDst
 *          pointer to the destination shape to be extended
 * @param   pShapeSrc
 *          pointer to the source shape which elements will be inserted into
 *          the destination shape
 * @returns true if successful, false otherwise
 ******************************************************************************
 */
bool IShape_Extend( tIShape * const pShapeDst, const tIShape * const pShapeSrc );

/**
 ******************************************************************************
 * @brief   Returns the line-width for a shape (not to be mixed up with e.g.
 *          the height or width of the bounding box of a shape).
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to get line-width from
 * @return  The line-width of the shape
 ******************************************************************************
 */
tDistance IShape_GetLineWidth( const tIShape* const pShape );

/**
 ******************************************************************************
 * @brief   Sets the line-width for a shape (not to be mixed up with e.g. the
 *          height or width of the bounding box of a shape).
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized
 * @param   pShape
 *          shape to set line-width for
 * @param   lineWidth
 *          the line-width to set for this shape
 * @return  -
 ******************************************************************************
 */
void IShape_SetLineWidth( tIShape* const pShape, const tDistance lineWidth );

/* ****************************************************************************
* Function(s) related to Minimal Covering Rectangle (MCR) for a shape.
* The MCR will be referred to as a bounding box for the coordinates.
* Picture below shows an example of this with polygon points marked as o1..o8
* o2 and o7 defines the minimum and maximum X values respectively.
* o1 and o5 defines the minimum and maximum Y values respectively.
*
* Y
* ^
* |  +------o5-----+
* |  |  o4      o6 |
* |  |     o3      o7
* |  o2         o8 |
* |  +-o1----------+
* +------------------>X
* ****************************************************************************
*/

/**
******************************************************************************
* @brief   Calculate a bounding box around a shape.
*          Returns the minimal covering rectangle (MCR) as two coordinates
*          (min-min and max-max corners). For a shape with single coordinate
*          min-min and max-max will be the same.
*          Function(s) related to Minimal Covering Rectangle (MCR) for a shape.
*          The MCR will be referred to as a bounding box for the coordinates.
*          Picture below shows an example of this with polygon points marked as o1..o8
*          2 and o7 defines the minimum and maximum X values respectively.
*          o1 and o5 defines the minimum and maximum Y values respectively.
*
*         Y
*         ^
*         |  +------o5-----+
*         |  |  o4      o6 |
*         |  |     o3      o7
*         |  o2         o8 |
*         |  +-o1----------+
*         +------------------>X
*
*          NOTE: SOFTWARE_ASSERTs that pointers are initialized
* ****************************************************************************
* @param   pShape
*          pointer to the polygon
* @param   pMinMin
*          pointer for the coordinate for lower-left corner (min-x and min-y)
* @param   pMaxMax
*          pointer for the coordinate for upper-right corner (max-x and max-y)
* @return  true if BB is returned successfully, false otherwise
******************************************************************************
*/
bool IShape_BoundingBox( const tIShape * const pShape,
                          tCoordinate2D *pMinMin,
                          tCoordinate2D *pMaxMax );


#endif /* ISHAPE_H */

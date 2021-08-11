/**
 ******************************************************************************
 * @file        IVector2d.h
 *
 * @brief       Interface.
 *
 ******************************************************************************
 */
#ifndef IVECTOR2D_H
#define IVECTOR2D_H


/*
-------------------------------------------------------------------------------
    Include files
-------------------------------------------------------------------------------
*/
#include "RoboticTypes.h"


/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
*/
#pragma pack(1)

typedef struct
{
    tDistance x;
    tDistance y;
} tIVector2d;

#pragma pack()

typedef struct
{
    float x;
    float y;
} tIVector2dFloat;

typedef struct
{
    float aLenbLen;    // |A||B|
    float cosv;
    sint64 cross;      // AxB = |A||B|sin(v),  0 < AxB if yaw increases
    sint64 dot;        // AB = |A||B|cos(v)    0 < AB if line to line has a angle above 90 "none pointy angle" (not the projected vector)
    float sinv;
    float v;            // angle in rad between vector A and B
} tIVector2dCrossDot;

typedef struct
{
    tCoordinate2D end;      // end point of vector
    float len;              // length of vector
    float sqLen;            // length * length
    tCoordinate2D start;    // start point of vector
    tIVector2d v;           // vector
} tIVector2dObject;


/*
-------------------------------------------------------------------------------
    Public function prototypes
-------------------------------------------------------------------------------
*/
/**
 ***************************************************************************************************
 * @brief       Initialises a vector cross dot object,
 *              vector object order is important
 * @param[out]  me
 *              the vector cross dot object
 * @param[in]   a
 *              pre calculated vector object
 * @param[in]   b
 *              pre calculated vector object
 * @returns     -
 ***************************************************************************************************
 */
void IVector2d_CrossDotInit( tIVector2dCrossDot* const me, const tIVector2dObject* const  a, const tIVector2dObject* const b );

/**
 ***************************************************************************************************
 * @brief       Cross product a x b = |a|*|b|*sin(v) just the z part x = y = 0,
 *              cross dot is the area use or
 *              length of the vector the sign can be used for left or right oriented system
 * @param[in]   a
 * @param[in]   b
 * @returns     a x b
 ***************************************************************************************************
 */
sint64 IVector2d_Cross( const tIVector2d* const a, const tIVector2d* const b );

/**
 ***************************************************************************************************
 * @brief       Sin(v) using cross product
 *              sin(v) = a x b / |a|*|b|
 * @param[in]   cross
 * @param[in]   aLenbLen
 * @returns     sin(v)
 ***************************************************************************************************
 */
float IVector2d_CrossSinv( const float cross, const float aLenbLen );

/**
 ***************************************************************************************************
 * @brief       Dot product
 *              ab = |a||b|cos(v)
 * @param[in]   a
 * @param[in]   b
 * @returns     ab
 ***************************************************************************************************
 */
sint64 IVector2d_Dot( const tIVector2d* const a, const tIVector2d* const b );

/**
 ***************************************************************************************************
 * @brief       Cos(v) using the dot product
 *              cos(v) = ab / |a||b|
 * @param[in]   dot
 * @param[in]   aLenbLen
 * @returns     cos(v)
 ***************************************************************************************************
 */
float IVector2d_DotCosv( const float dot, const float aLenbLen );

/**
 ***************************************************************************************************
 * @brief       Return a vector from 2 points.
 * @param[in]   start
 * @param[in]   end
 * @returns     vector
 ***************************************************************************************************
 */
tIVector2d IVector2d_FromPoints( const tCoordinate2D* const start, const tCoordinate2D* const end );

/**
 ***************************************************************************************************
 * @brief       Squared length to length.
 * @param[in]   sqLength
 * @returns     length
 ***************************************************************************************************
 */
float IVector2d_Length( const float sqLength );

/**
 ***************************************************************************************************
 * @brief       Normal vector that is unit in length.
 * @param[in]   v
 *              vector
 * @param[in]   len
 *              vector length
 * @returns     normal vector
 ***************************************************************************************************
 */
tIVector2dFloat IVector2d_Normal( const tIVector2d* const v, const float len );

/**
 ***************************************************************************************************
 * @brief       Init vector object with lengths and vector.
 * @param[in]   obj
 *              start and end points needs to be initialised before call
 * @returns     -
 ***************************************************************************************************
 */
void IVector2d_ObjectInit( tIVector2dObject* const obj );

/**
 ***************************************************************************************************
 * @brief       Normal vector that is unit in length.
 * @param[in]   a
 *              vector that is projected on the vector object
 * @param[in]   obj
 *              the initialised vector object
 * @returns     length of the vector projection
 ***************************************************************************************************
 */
float IVector2d_ProjectionLength( const tIVector2d* const a, const tIVector2dObject* const obj );

/**
 ***************************************************************************************************
 * @brief       Squared vector length.
 * @param[in]   vx
 *              x length
 * @param[in]   vy
 *              y length
 * @returns     squared vector length
 ***************************************************************************************************
 */
float IVector2d_SqLen( const float vx, const float vy );

/**
 ***************************************************************************************************
 * @brief       Creates a point from start with a length and direction from obj
 * @param[in]   start
 * @param[in]   obj
 * @param[in]   length
 * @returns     end point
 ***************************************************************************************************
 */
tCoordinate2D IVector2d_ToPoint( const tCoordinate2D* const start, const tIVector2dObject* const obj, const float length );


#endif

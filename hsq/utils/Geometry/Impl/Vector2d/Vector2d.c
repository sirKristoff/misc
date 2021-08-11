/**
 ******************************************************************************
 * @file        Vector2d.c
 *
 * @brief       implementation.
 *
 ******************************************************************************
 */
/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */
#include "math.h"
#include "RoboticTypes.h"
#include "RoboticUtils.h"
#include "IVector2d.h"


/*
 ------------------------------------------------------------------------------
 public
 ------------------------------------------------------------------------------
 */
void IVector2d_CrossDotInit( tIVector2dCrossDot* const me, const tIVector2dObject* const  a, const tIVector2dObject* const b )
{
    me->aLenbLen = a->len * b->len;

    // AxB = |A||B|sin(v) ie the area or the vector normal * length
    me->cross = IVector2d_Cross(&a->v, &b->v);
    me->sinv = IVector2d_CrossSinv(me->cross, me->aLenbLen);
    me->v = asinf(me->sinv);

    // ab = |a||b|cos(v) projection
    me->dot = IVector2d_Cross(&a->v, &b->v);
    me->cosv = IVector2d_DotCosv(me->dot, me->aLenbLen);
}


sint64 IVector2d_Cross( const tIVector2d* const a, const tIVector2d* const b )
{
    return (sint64)a->x*b->y - (sint64)a->y*b->x;
}


float IVector2d_CrossSinv( const float cross, const float aLenbLen )
{
    return cross / aLenbLen;
}


sint64 IVector2d_Dot( const tIVector2d* const a, const tIVector2d* const b )
{
    return (sint64)a->x*b->x + (sint64)a->y*b->y;
}


float IVector2d_DotCosv( const float dot, const float aLenbLen )
{
    return dot / aLenbLen;
}


tIVector2d IVector2d_FromPoints( const tCoordinate2D* const start, const tCoordinate2D* const end )
{
    return (tIVector2d){ .x = end->x - start->x, .y = end->y - start->y};
}


float IVector2d_Length( const float sqLength )
{
    return sqrtf(sqLength);
}


tIVector2dFloat IVector2d_Normal( const tIVector2d* const v, const float len )
{                                          // 90
    float nx = -(float)v->y / len;         // cosv  -sinv
    float ny =  (float)v->x / len;         // sinv   cosv
    return (tIVector2dFloat) { .x = nx, .y= ny };
}


void IVector2d_ObjectInit( tIVector2dObject* const obj )
{
    obj->v = IVector2d_FromPoints(&obj->start, &obj->end);
    obj->sqLen = IVector2d_SqLen(obj->v.x, obj->v.y);
    obj->len = IVector2d_Length(obj->sqLen);
}


float IVector2d_ProjectionLength( const tIVector2d* const a, const tIVector2dObject* const obj )
{
    sint64 dot = IVector2d_Dot(a,&(obj->v));
    // ab = |a||b|cos(v)
    // after line end, lLen < aLen*cos(v) = aLen*dot/lLen
    //                 obj->sqLen <= dot
    return (float)dot / obj->len;
}


float IVector2d_SqLen( const float vx, const float vy )
{
    return vx*vx + vy*vy;
}


tCoordinate2D IVector2d_ToPoint( const tCoordinate2D* const start, const tIVector2dObject* const obj, const float length )
{
    // lineStart + lineVector/lineVectorLen * dot/lineVectorLen
    // start     + direction (unit vector)  * length
    float x = (float)start->x + length*(float)(obj->v.x) / obj->len;
    float y = (float)start->y + length*(float)(obj->v.y) / obj->len;
    tCoordinate2D p = { .x = FloatToS32(x), .y = FloatToS32(y) };
    return p;
}


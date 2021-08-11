/**
 ******************************************************************************
 * @file      EventQueue.h
 *
 * @brief     Header file for EventQueue implementation
 ******************************************************************************
 */

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"
#include "SweepLine.h"

/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
*/

#define EVENTQUEUE_EVENT_INTERSECTIONS ( 2 )

/*
 ------------------------------------------------------------------------------
    Type definitions
 ------------------------------------------------------------------------------
*/

typedef enum
{
    EVENTQUEUE_TYPE_LEFT,          /* Left segment end point event type */
    EVENTQUEUE_TYPE_RIGHT,         /* Right segment end point event type */
    EVENTQUEUE_TYPE_INTERSECTION   /* Intersection event type */
} tEventQueueType;

typedef struct tEventQueueElement
{
    uint16 segmentIndex;
    uint16 sortIndex;
    tEventQueueType eventType;
    tCoordinate2D eventPoint;
    tSweepLineSegment* segment;
    tSweepLineSegment* intersectingSegments[ EVENTQUEUE_EVENT_INTERSECTIONS ];
    struct tEventQueueElement* otherEndPoint;
} tEventQueueElement;

typedef struct tEventQueue
{
    uint16 index;
    uint16 maxSize;
    uint16 size;
    tEventQueueElement* eventData;
    tEventQueueElement** eventPointers;
} tEventQueue;

/*
 ------------------------------------------------------------------------------
    Public function prototypes
 ------------------------------------------------------------------------------
*/

/**
 ******************************************************************************
 * @brief   Initialize event queue based on polygon vertices.
 * @param   pEventQueue
 *          Pointer to event queue.
 * @param   pPolygon
 *          Pointer to polygon.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool EventQueue_InitializeFromPolygon( tEventQueue* pEventQueue,
                                       const tIPolygon* pPolygon );

/**
 ******************************************************************************
 * @brief   Initialize event queue based on line segments.
 * @param   pEventQueue
 *          Pointer to event queue.
 * @param   pLineSegments
 *          Pointer to line segments.
 * @param   lineSegmentsSize
 *          Number of line segments in pLineSegments.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool EventQueue_InitializeFromLineSegments( tEventQueue* pEventQueue,
                                            const tILine* pLineSegments,
                                            const uint16 lineSegmentsSize );

/**
 ******************************************************************************
 * @brief   Add an intersection as new event in event queue.
 * @param   pEventQueue
 *          Pointer to event queue.
 * @param   intersectionPoint
 *          The new intersection point.
 * @param   pIntersectSegmentAbove
 *          Pointer to line segment in sweep line that is 'above' intersection.
 * @param   pIntersectSegmentBelow
 *          Pointer to line segment in sweep line that is 'below' intersection.
 * @param   segmentIndex
 *          Index of segment where intersection was found.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool EventQueue_AddIntersection( tEventQueue* pEventQueue,
                                 tCoordinate2D intersectionPoint,
                                 tSweepLineSegment* pIntersectSegmentAbove,
                                 tSweepLineSegment* pIntersectSegmentBelow,
                                 uint16 segmentIndex );

/**
 ******************************************************************************
 * @brief   Get next event element from event queue.
 * @param   pEventQueue
 *          Pointer to event queue.
 * @return  Pointer to next event element if existing, otherwise NULL.
 ******************************************************************************
 */
tEventQueueElement* EventQueue_GetNext( tEventQueue* pEventQueue );

/**
 ******************************************************************************
 * @brief   Determine lexicographical order (based on x- and y-coordinates)
 *          of two event element points.
 * @param   pointA
 *          Event point for order comparison.
 * @param   pointB
 *          Event point for order comparison.
 * @return  +1 if pointA > pointB.
 *          -1 if pointA < pointB.
 *          0 if pointA == pointB
 ******************************************************************************
 */
int EventQueue_GetEventPointOrder( tCoordinate2D pointA, tCoordinate2D pointB );

#endif /* EVENTQUEUE_H */

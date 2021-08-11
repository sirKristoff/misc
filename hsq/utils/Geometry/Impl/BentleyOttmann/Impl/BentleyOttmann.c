/**
 ******************************************************************************
 * @file      BentelyOttmann.c
 *
 * @brief     Implementation file for BentelyOttmann
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
*/

#include "BentleyOttmann.h"
#include "RoboticTypes.h"
#include "SweepLine.h"
#include "EventQueue.h"
#include "IBinarySearchTree.h"
#include "ISoftwareException.h"
#include "ILog.h"

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
*/

static bool isActive = false;

/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * @brief   Process all events in event queue one by one.
 ******************************************************************************
 */
static bool ProcessEventQueue( tEventQueue* pEventQueue,
                               tSweepLine* pSweepLine,
                               tCoordinate2D* pIntersections,
                               uint16 intersectionsSize,
                               uint16* pFoundIntersections );

/*
-------------------------------------------------------------------------------
    Public functions
-------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool BentleyOttmann_GetPolygonIntersections( const tIPolygon* pPolygon,
                                             tCoordinate2D* pIntersectionsOut,
                                             uint16* pIntersectionsSizeOut,
                                             uint16 intersectionsSizeIn )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pPolygon );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsOut );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsSizeOut );

    *pIntersectionsSizeOut = 0;

    if ( isActive )
    {
        ILOG( ILOG_LEVEL_DEBUG, "Currently active, wait for calculations to finish", "^" );
        return false;
    }

    isActive = true;

    /* Set up event queue */
    tEventQueue eventQueue = { 0 };

    if ( !EventQueue_InitializeFromPolygon( &eventQueue, pPolygon ) )
    {
        isActive = false;
        return false;
    }

    /* Set up sweep line */
    tIBinarySearchTree BST = { 0 };
    tSweepLine sweepLine   = { .polygon = pPolygon,
                               .lineSegments = NULL,
                               .lineSegmentsSize = 0 };

    SweepLine_Initialize( &sweepLine, &BST );

    if ( !ProcessEventQueue( &eventQueue,
                             &sweepLine,
                             pIntersectionsOut,
                             intersectionsSizeIn,
                             pIntersectionsSizeOut ) )
    {
        isActive = false;
        return false;
    }

    isActive = false;
    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool BentleyOttmann_GetLineIntersections( const tILine* pLineSegments,
                                          const uint16 linesSegmentsSize,
                                          tCoordinate2D* pIntersectionsOut,
                                          uint16* pIntersectionsSizeOut,
                                          uint16 intersectionsSizeIn )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pLineSegments );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsOut );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pIntersectionsSizeOut );

    *pIntersectionsSizeOut = 0;

    if ( isActive )
    {
        ILOG( ILOG_LEVEL_DEBUG, "Currently active, wait for calculations to finish", "^" );
        return false;
    }

    isActive = true;

    /* Set up event queue */
    tEventQueue eventQueue = { 0 };

    if ( !EventQueue_InitializeFromLineSegments( &eventQueue,
                                                 pLineSegments,
                                                 linesSegmentsSize ) )
    {
        isActive = false;
        return false;
    }

    /* Set up sweep line */
    tIBinarySearchTree BST = { 0 };
    tSweepLine sweepLine   = { .polygon = NULL,
                               .lineSegments = pLineSegments,
                               .lineSegmentsSize = linesSegmentsSize };

    SweepLine_Initialize( &sweepLine, &BST );

    if ( !ProcessEventQueue( &eventQueue,
                             &sweepLine,
                             pIntersectionsOut,
                             intersectionsSizeIn,
                             pIntersectionsSizeOut ) )
    {
        isActive = false;
        return false;
    }

    isActive = false;
    return true;
}

/*
-------------------------------------------------------------------------------
    Private functions
-------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool ProcessEventQueue( tEventQueue* pEventQueue,
                               tSweepLine* pSweepLine,
                               tCoordinate2D* pIntersections,
                               uint16 intersectionsSize,
                               uint16* pFoundIntersections )
{
    tEventQueueElement* event = EventQueue_GetNext( pEventQueue );

    /* Process all events in the event queue one by one */
    while ( NULL != event )
    {
        if ( EVENTQUEUE_TYPE_LEFT == event->eventType )
        {
            /* Current event is left end point */
            tSweepLineSegment* segment = NULL;

            /* Create sweep line segment from event and add to sweep line */
            if ( !SweepLine_Add( pSweepLine, event->segmentIndex, &segment ) )
            {
                return false;
            }

            event->segment = segment;

            /* Check for intersection with 'above' sweep line segment */
            const tCoordinate2D intersectionAbove = SweepLine_Intersection( pSweepLine,
                                                                            segment,
                                                                            segment->segmentAbove );

            /* Check for intersection with 'below' sweep line segment */
            const tCoordinate2D intersectionBelow = SweepLine_Intersection( pSweepLine,
                                                                            segment,
                                                                            segment->segmentBelow );

            if ( !ICoordinate2D_IsNull( intersectionAbove ) )
            {
                /* Add intersection to event queue */
                if ( !EventQueue_AddIntersection( pEventQueue,
                                                  intersectionAbove,
                                                  segment->segmentAbove,
                                                  segment,
                                                  event->segmentIndex ) )
                {
                    return false;
                }
            }

            if ( !ICoordinate2D_IsNull( intersectionBelow ) )
            {
                /* Add intersection to event queue */
                if ( !EventQueue_AddIntersection( pEventQueue,
                                                  intersectionBelow,
                                                  segment,
                                                  segment->segmentBelow,
                                                  event->segmentIndex ) )
                {
                    return false;
                }
            }
        }
        else if ( EVENTQUEUE_TYPE_RIGHT == event->eventType )
        {
            /* Current event is a right end point (reached end of segment) */
            tSweepLineSegment* segment = event->otherEndPoint->segment;

            /* Check for intersection with segment's adjacent segments ('above' and 'below') */
            const tCoordinate2D intersection = SweepLine_Intersection( pSweepLine,
                                                                       segment->segmentAbove,
                                                                       segment->segmentBelow );

            if ( !ICoordinate2D_IsNull( intersection ) )
            {
                /* Add intersection to event queue */
                if ( !EventQueue_AddIntersection( pEventQueue,
                                                  intersection,
                                                  segment->segmentAbove,
                                                  segment->segmentBelow,
                                                  event->segmentIndex ) )
                {
                    return false;
                }
            }

            /* Remove from sweep line */
            SweepLine_Remove( pSweepLine, segment );
        }
        else
        {
            /* Current event is an intersection,
             * add as a valid intersection */
            if ( *pFoundIntersections >= intersectionsSize )
            {
                return false;
            }
            else
            {
                pIntersections[ (*pFoundIntersections)++ ] = event->eventPoint;
            }

            /* Swap 'above-below' relations of the two intersecting segments after intersection point */
            tSweepLineSegment* pSegAbove = event->intersectingSegments[ 0 ];
            tSweepLineSegment* pSegBelow = event->intersectingSegments[ 1 ];

            pSegAbove->segmentBelow = pSegBelow->segmentBelow;
            pSegBelow->segmentAbove = pSegAbove->segmentAbove;

            pSegAbove->segmentAbove = pSegBelow;
            pSegBelow->segmentBelow = pSegAbove;

            /* Check for intersection with 'above' sweep line segment */
            const tCoordinate2D intersectionAbove = SweepLine_Intersection( pSweepLine,
                                                                            pSegBelow->segmentAbove,
                                                                            pSegBelow );

            /* Check for intersection with 'below' sweep line segment */
            const tCoordinate2D intersectionBelow = SweepLine_Intersection( pSweepLine,
                                                                            pSegAbove,
                                                                            pSegAbove->segmentBelow );

            if ( !ICoordinate2D_IsNull( intersectionAbove ) )
            {
                /* Add intersection to event queue */
                if ( !EventQueue_AddIntersection( pEventQueue,
                                                  intersectionAbove,
                                                  pSegBelow->segmentAbove,
                                                  pSegBelow,
                                                  event->segmentIndex ) )
                {
                    return false;
                }
            }
            if ( !ICoordinate2D_IsNull( intersectionBelow ) )
            {
                /* Add intersection to event queue */
                if ( !EventQueue_AddIntersection( pEventQueue,
                                                  intersectionBelow,
                                                  pSegAbove,
                                                  pSegAbove->segmentBelow,
                                                  event->segmentIndex ) )
                {
                    return false;
                }
            }
        }

        event = EventQueue_GetNext( pEventQueue );
    }

    return true;
}

/**
 ******************************************************************************
 * @file      EventQueue.c
 *
 * @brief     Implementation file for EventQueue
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"
#include "EventQueue.h"
#include "ISoftwareException.h"

/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
*/

#define EVENTQUEUE_BUFFER_MAX_SIZE     ( 500 )

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
*/

static tEventQueueElement  eventQueueBuffer[ EVENTQUEUE_BUFFER_MAX_SIZE ] STATIC_EXT_VAR;         /* Memory pool for event queue */
static tEventQueueElement* eventQueuePointerBuffer[ EVENTQUEUE_BUFFER_MAX_SIZE ] STATIC_EXT_VAR;  /* Memory pool for event queue pointers */

/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * @brief   Initialize event queue structure.
 ******************************************************************************
 */
static bool InitializeEventQueue( tEventQueue* pEventQueue, const uint16 nrOfEvents );

/*
 ******************************************************************************
 * @brief   Add elements to event queue.
 ******************************************************************************
 */
static void AddElementsToEventQueue( tEventQueue* pEventQueue,
                                     const uint16 index,
                                     const tCoordinate2D endPointA,
                                     const tCoordinate2D endPointB );

/*
 ******************************************************************************
 * @brief   Sort event queue.
 ******************************************************************************
 */
static void SortEventQueue( tEventQueue* pEventQueue );

/*
 ******************************************************************************
 * @brief   Compare order of two event elements (used for qsort).
 ******************************************************************************
 */
static int CompareEvents( const void* pEventA, const void* pEventB );

/*
 ------------------------------------------------------------------------------
    Public functions
 ------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool EventQueue_InitializeFromPolygon( tEventQueue* pEventQueue,
                                       const tIPolygon* pPolygon )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pEventQueue );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pPolygon );

    /* Each polygon edge yields two events (one for each vertex) */
    const uint16 nrOfEvents = IShape_Size( pPolygon ) * 2;

    if ( !InitializeEventQueue( pEventQueue, nrOfEvents ) )
    {
        return false;
    }

    for ( uint16 i = 0; i < IShape_Size( pPolygon ); i++ )
    {
        const tCoordinate2D endPointA = IShape_At( pPolygon, i );
        const tCoordinate2D endPointB = ( i + 1 < IShape_Size( pPolygon ) ) ?
                                        IShape_At( pPolygon, i + 1 ) : IShape_At( pPolygon, 0 );

        AddElementsToEventQueue( pEventQueue, i,
                                 endPointA, endPointB );
    }

    SortEventQueue( pEventQueue );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool EventQueue_InitializeFromLineSegments( tEventQueue* pEventQueue,
                                            const tILine* pLineSegments,
                                            const uint16 lineSegmentsSize )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pEventQueue );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pLineSegments );

    /* Each line segment yields two events (one for each end point) */
    const uint16 nrOfEvents = lineSegmentsSize * 2;

    if ( !InitializeEventQueue( pEventQueue, nrOfEvents ) )
    {
        return false;
    }

    for ( uint16 i = 0; i < lineSegmentsSize; i++ )
    {
        const tCoordinate2D endPointA = pLineSegments[ i ].A;
        const tCoordinate2D endPointB = pLineSegments[ i ].B;

        AddElementsToEventQueue( pEventQueue, i,
                                 endPointA, endPointB );
    }

    SortEventQueue( pEventQueue );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool EventQueue_AddIntersection( tEventQueue* pEventQueue,
                                 tCoordinate2D intersectionPoint,
                                 tSweepLineSegment* pIntersectSegmentAbove,
                                 tSweepLineSegment* pIntersectSegmentBelow,
                                 uint16 segmentIndex )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pEventQueue );

    if ( pEventQueue->size >= pEventQueue->maxSize )
    {
        /* Event queue too small, abort */
        return false;
    }

    /* Check if intersection already in event queue */
    for ( uint16 i = 0; i < pEventQueue->size; i++ )
    {
        if ( pEventQueue->eventPointers[ i ]->eventType ==
             EVENTQUEUE_TYPE_INTERSECTION )
        {
            if ( ICoordinate2D_IsEqual( intersectionPoint,
                                        pEventQueue->eventPointers[ i ]->eventPoint ) )
            {
                /* Intersection already in event queue */
                return true;
            }
        }
    }

    pEventQueue->eventPointers[ pEventQueue->size ]->segmentIndex              = segmentIndex;
    pEventQueue->eventPointers[ pEventQueue->size ]->sortIndex                 = pEventQueue->size;
    pEventQueue->eventPointers[ pEventQueue->size ]->eventType                 = EVENTQUEUE_TYPE_INTERSECTION;
    pEventQueue->eventPointers[ pEventQueue->size ]->eventPoint                = intersectionPoint;
    pEventQueue->eventPointers[ pEventQueue->size ]->otherEndPoint             = NULL;
    pEventQueue->eventPointers[ pEventQueue->size ]->segment                   = NULL;
    pEventQueue->eventPointers[ pEventQueue->size ]->intersectingSegments[ 0 ] = pIntersectSegmentAbove;
    pEventQueue->eventPointers[ pEventQueue->size ]->intersectingSegments[ 1 ] = pIntersectSegmentBelow;

    pEventQueue->size++;

    /* Sort event queue when new event has been added */
    SortEventQueue( pEventQueue );

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tEventQueueElement* EventQueue_GetNext( tEventQueue* pEventQueue )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pEventQueue );

    if ( pEventQueue->index >= pEventQueue->size )
    {
        return NULL;
    }

    return pEventQueue->eventPointers[ pEventQueue->index++ ];
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
int EventQueue_GetEventPointOrder( tCoordinate2D pointA, tCoordinate2D pointB )
{
    /* Prioritize x-coordinate for determining order */
    if ( pointA.x > pointB.x )
    {
        return 1;
    }
    if ( pointA.x < pointB.x )
    {
        return -1;
    }
    /* Continue checking y-coordinate if x-coordinate is identical */
    if ( pointA.y > pointB.y )
    {
        return 1;
    }
    if ( pointA.y < pointB.y )
    {
        return -1;
    }

    /* Event points are identical */
    return 0;
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
static bool InitializeEventQueue( tEventQueue* pEventQueue, const uint16 nrOfEvents )
{
    memset( eventQueueBuffer, 0, sizeof( eventQueueBuffer ) );
    memset( eventQueuePointerBuffer, 0, sizeof( eventQueuePointerBuffer ) );

    pEventQueue->size = 0;
    pEventQueue->index = 0;
    pEventQueue->maxSize = EVENTQUEUE_BUFFER_MAX_SIZE;
    pEventQueue->eventData = eventQueueBuffer;
    pEventQueue->eventPointers = eventQueuePointerBuffer;

    if ( pEventQueue->maxSize < nrOfEvents )
    {
        /* Event queue too small, abort */
        return false;
    }

    /* Initialize event queue pointers */
    for ( uint16 i = 0; i < pEventQueue->maxSize; i++ )
    {
        pEventQueue->eventPointers[ i ] = &( pEventQueue->eventData[ i ] );
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void AddElementsToEventQueue( tEventQueue* pEventQueue,
                                     const uint16 index,
                                     const tCoordinate2D endPointA,
                                     const tCoordinate2D endPointB )
{
    const uint16 indexA = index * 2;
    const uint16 indexB = ( index * 2 ) + 1;

    pEventQueue->eventPointers[ indexA ]->segmentIndex = index;
    pEventQueue->eventPointers[ indexB ]->segmentIndex = index;

    pEventQueue->eventPointers[ indexA ]->eventPoint = endPointA;
    pEventQueue->eventPointers[ indexB ]->eventPoint = endPointB;

    pEventQueue->eventPointers[ indexA ]->otherEndPoint = pEventQueue->eventPointers[ indexB ];
    pEventQueue->eventPointers[ indexB ]->otherEndPoint = pEventQueue->eventPointers[ indexA ];

    pEventQueue->eventPointers[ indexA ]->segment = NULL;
    pEventQueue->eventPointers[ indexB ]->segment = NULL;

    for ( uint16 j = 0; j < EVENTQUEUE_EVENT_INTERSECTIONS; j++ )
    {
        pEventQueue->eventPointers[ indexA ]->intersectingSegments[ j ] = NULL;
        pEventQueue->eventPointers[ indexB ]->intersectingSegments[ j ] = NULL;
    }

    if ( EventQueue_GetEventPointOrder( endPointA, endPointB ) < 0 )
    {
        pEventQueue->eventPointers[ indexA ]->eventType = EVENTQUEUE_TYPE_LEFT;
        pEventQueue->eventPointers[ indexB ]->eventType = EVENTQUEUE_TYPE_RIGHT;
    }
    else
    {
        pEventQueue->eventPointers[ indexA ]->eventType = EVENTQUEUE_TYPE_RIGHT;
        pEventQueue->eventPointers[ indexB ]->eventType = EVENTQUEUE_TYPE_LEFT;
    }

    pEventQueue->size += 2;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void SortEventQueue( tEventQueue* pEventQueue )
{
    /* Sort event queue by increasing x and y */
    qsort( pEventQueue->eventPointers,
           pEventQueue->size,
           sizeof( tEventQueueElement* ),
           CompareEvents );

    for ( uint16 i = 0; i < pEventQueue->size; i++ )
    {
        pEventQueue->eventPointers[ i ]->sortIndex = i;
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static int CompareEvents( const void* pEventA, const void* pEventB )
{
    tEventQueueElement** ppEventA = (tEventQueueElement**) pEventA;
    tEventQueueElement** ppEventB = (tEventQueueElement**) pEventB;

    int result = EventQueue_GetEventPointOrder( (*ppEventA)->eventPoint, (*ppEventB)->eventPoint );

    if ( result == 0 )
    {
        if ( (*ppEventA)->eventType == (*ppEventB)->eventType )
        {
            if ( (*ppEventA)->sortIndex < (*ppEventB)->sortIndex )
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
        if ( (*ppEventA)->eventType == EVENTQUEUE_TYPE_LEFT )
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

    return result;
}

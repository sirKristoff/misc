/**
 ******************************************************************************
 * @file      SweepLine.c
 *
 * @brief     Implementation file for SweepLine
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"
#include "SweepLine.h"
#include "EventQueue.h"
#include "IShape.h"
#include "IBinarySearchTree.h"
#include "ISoftwareException.h"

/*
 ------------------------------------------------------------------------------
    Defines
 ------------------------------------------------------------------------------
*/

#define SWEEPLINE_BUFFER_MAX_SIZE     ( 500 )

/*
 ------------------------------------------------------------------------------
    Private data
 ------------------------------------------------------------------------------
*/

static tIBinarySearchTree_Node BSTBuffer[ SWEEPLINE_BUFFER_MAX_SIZE ] STATIC_EXT_VAR;        /* Memory pool for BST */
static tSweepLineSegment       sweepLineBuffer[ SWEEPLINE_BUFFER_MAX_SIZE ] STATIC_EXT_VAR;  /* Memory pool for sweep line */

/*
 ------------------------------------------------------------------------------
    Private function prototypes
 ------------------------------------------------------------------------------
*/

/*
 ******************************************************************************
 * @brief   Check if a sweep line segment is considered being 'below' another
 *          (in terms 'above-below' relationship
 *          defined by Bentley-Ottman algorithm).
 ******************************************************************************
 */
static tIBinarySearchTree_CompareResult CompareSegments( const void *pSegmentA,
                                                         const void *pSegmentB );

/*
 ******************************************************************************
 * @brief   Compare two sweep line segments.
 *          Used for sorting elements contained in BST.
 ******************************************************************************
 */
static bool IsSegmentBelow( const tSweepLineSegment* pSegmentA,
                            const tSweepLineSegment* pSegmentB );

/*
 ******************************************************************************
 * @brief   Test if a point is 'left' relative to reference line.
 ******************************************************************************
 */
static inline sint64 IsLeft( tCoordinate2D lineA,
                             tCoordinate2D lineB,
                             tCoordinate2D point )
{
    return ( (sint64) lineB.x - (sint64) lineA.x ) * ( (sint64) point.y - (sint64) lineA.y ) -
           ( (sint64) point.x - (sint64) lineA.x ) * ( (sint64) lineB.y - (sint64) lineA.y );
}

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
void SweepLine_Initialize( tSweepLine* pSweepLine,
                           tIBinarySearchTree* pBST )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pSweepLine );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );

    memset( BSTBuffer, 0, sizeof( BSTBuffer ) );
    memset( sweepLineBuffer, 0, sizeof( sweepLineBuffer ) );

    /* Initialize BST for sweep line */
    IBinarySearchTree_Init( pBST, SWEEPLINE_BUFFER_MAX_SIZE,
                            BSTBuffer, CompareSegments );

    pSweepLine->index            = 0;
    pSweepLine->maxSize          = SWEEPLINE_BUFFER_MAX_SIZE;
    pSweepLine->segmentBuffer    = sweepLineBuffer;
    pSweepLine->BST              = pBST;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
bool SweepLine_Add( tSweepLine* pSweepLine,
                    uint16 index,
                    tSweepLineSegment** ppSegment )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pSweepLine );

    if ( pSweepLine->index < pSweepLine->maxSize )
    {
        *ppSegment = &( pSweepLine->segmentBuffer[ pSweepLine->index++ ] );
    }
    else
    {
        return false;
    }

    (*ppSegment)->segmentIndex = index;
    (*ppSegment)->segmentAbove = NULL;
    (*ppSegment)->segmentBelow = NULL;

    /* Must determine end point having 'left' event type */
    tCoordinate2D endPointA = nullCoord2D;
    tCoordinate2D endPointB = nullCoord2D;

    /* Check if based on polygon or general set of line segments */
    if ( NULL != pSweepLine->polygon )
    {
        endPointA = IShape_At( pSweepLine->polygon, (*ppSegment)->segmentIndex );
        endPointB = ( (*ppSegment)->segmentIndex + 1 < IShape_Size( pSweepLine->polygon ) ) ?
                    IShape_At( pSweepLine->polygon, (*ppSegment)->segmentIndex + 1 ) :
                    IShape_At( pSweepLine->polygon, 0 );
    }
    else if ( NULL != pSweepLine->lineSegments )
    {
        endPointA = pSweepLine->lineSegments[ (*ppSegment)->segmentIndex ].A;
        endPointB = pSweepLine->lineSegments[ (*ppSegment)->segmentIndex ].B;
    }
    else
    {
        return false;
    }

    if ( EventQueue_GetEventPointOrder( endPointA, endPointB ) < 0 )
    {
        (*ppSegment)->leftEndPoint  = endPointA;
        (*ppSegment)->rightEndPoint = endPointB;
    }
    else
    {
        (*ppSegment)->leftEndPoint  = endPointB;
        (*ppSegment)->rightEndPoint = endPointA;
    }

    /* Update BST with sweep line segment */
    tIBinarySearchTree_Node* pInsertedNode = IBinarySearchTree_Insert( pSweepLine->BST, *ppSegment );
    tIBinarySearchTree_Node* pNextNode     = IBinarySearchTree_Next( pSweepLine->BST, pInsertedNode );
    tIBinarySearchTree_Node* pPrevNode     = IBinarySearchTree_Previous( pSweepLine->BST, pInsertedNode );

    /* Update segment's 'above-below' relationships */
    if ( NULL != pNextNode )
    {
        (*ppSegment)->segmentAbove               = (tSweepLineSegment*) pNextNode->data;
        (*ppSegment)->segmentAbove->segmentBelow = *ppSegment;
    }
    if ( NULL != pPrevNode )
    {
        (*ppSegment)->segmentBelow               = (tSweepLineSegment*) pPrevNode->data;
        (*ppSegment)->segmentBelow->segmentAbove = *ppSegment;
    }

    return true;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void SweepLine_Remove( tSweepLine* pSweepLine,
                       tSweepLineSegment* pSegment )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pSweepLine );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pSegment );

    tIBinarySearchTree_Node* pFoundNode = IBinarySearchTree_Search( pSweepLine->BST, pSegment );

    if ( NULL == pFoundNode )
    {
        /* Node not found in BST */
        return;
    }

    /* Get above and below segments */
    tIBinarySearchTree_Node* pNextNode = IBinarySearchTree_Next( pSweepLine->BST, pFoundNode );
    tIBinarySearchTree_Node* pPrevNode = IBinarySearchTree_Previous( pSweepLine->BST, pFoundNode );

    if ( NULL != pNextNode )
    {
        tSweepLineSegment* nextNodeSLSegment = (tSweepLineSegment*) pNextNode->data;
        nextNodeSLSegment->segmentBelow      = pSegment->segmentBelow;
    }
    if ( NULL != pPrevNode )
    {
        tSweepLineSegment* prevNodeSLSegment = (tSweepLineSegment*) pPrevNode->data;
        prevNodeSLSegment->segmentAbove      = pSegment->segmentAbove;
    }

    /* Now safe to remove node */
    IBinarySearchTree_Remove( pSweepLine->BST, pFoundNode->data );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tCoordinate2D SweepLine_Intersection( tSweepLine* pSweepLine,
                                      tSweepLineSegment *pSegmentA,
                                      tSweepLineSegment *pSegmentB )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pSweepLine );

    if ( NULL == pSegmentA || NULL == pSegmentB )
    {
        return nullCoord2D;
    }

    /* Check if based on polygon,
     * and if so check for consecutive edges in polygon */
    if ( NULL != pSweepLine->polygon )
    {
        uint16 segmentIndexA = pSegmentA->segmentIndex;
        uint16 segmentIndexB = pSegmentB->segmentIndex;

        if ( ( ( segmentIndexA + 1 ) % IShape_Size( pSweepLine->polygon ) == segmentIndexB ) ||
             ( ( segmentIndexB + 1 ) % IShape_Size( pSweepLine->polygon ) == segmentIndexA ) )
        {
            /* No non-simple intersection since edges are consecutive */
            return nullCoord2D;
        }
    }

    const tILine segmentA = { .A = pSegmentA->leftEndPoint, .B = pSegmentA->rightEndPoint };
    const tILine segmentB = { .A = pSegmentB->leftEndPoint, .B = pSegmentB->rightEndPoint };

    return ILine_Intersection( &segmentA, &segmentB );
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
static tIBinarySearchTree_CompareResult CompareSegments( const void *pSegmentA,
                                                         const void *pSegmentB )
{
    const tSweepLineSegment* pSLSegmentA = ( tSweepLineSegment* ) pSegmentA;
    const tSweepLineSegment* pSLSegmentB = ( tSweepLineSegment* ) pSegmentB;

    /* Check equality based one edge index */
    if ( pSLSegmentA->segmentIndex == pSLSegmentB->segmentIndex )
    {
        /* Segments are equal, comparison done */
        return BST_COMPARE_EQ;
    }

    /* If not same edge index, check relative orientation of segments */
    if ( IsSegmentBelow( pSLSegmentA, pSLSegmentB ) )
    {
        /* pSegmentA is 'below' pSegmentB */
        return BST_COMPARE_MIN;
    }
    else
    {
        /* pSegmentA is 'above' pSegmentB */
        return BST_COMPARE_MAX;
    }
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static bool IsSegmentBelow( const tSweepLineSegment* pSegmentA,
                            const tSweepLineSegment* pSegmentB )
{
    if ( pSegmentA->leftEndPoint.x <= pSegmentB->leftEndPoint.x )
    {
        sint64 leftCheck = IsLeft( pSegmentA->leftEndPoint,
                                   pSegmentA->rightEndPoint,
                                   pSegmentB->leftEndPoint );

        if ( leftCheck != 0 )
        {
            return leftCheck > 0;
        }
        else
        {
            /* Special case of vertical line */
            if ( pSegmentA->leftEndPoint.x == pSegmentA->rightEndPoint.x )
            {
                return pSegmentA->leftEndPoint.y < pSegmentB->leftEndPoint.y;
            }
            else
            {
                leftCheck = IsLeft( pSegmentA->leftEndPoint,
                                    pSegmentA->rightEndPoint,
                                    pSegmentB->rightEndPoint );

                return leftCheck > 0;
            }
        }
    }
    else
    {
        sint64 leftCheck = IsLeft( pSegmentB->leftEndPoint,
                                   pSegmentB->rightEndPoint,
                                   pSegmentA->leftEndPoint );

        if ( leftCheck != 0 )
        {
            return leftCheck < 0;
        }
        else
        {
            leftCheck = IsLeft( pSegmentB->leftEndPoint,
                                pSegmentB->rightEndPoint,
                                pSegmentA->rightEndPoint );

            return leftCheck < 0;
        }
    }
}

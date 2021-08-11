/**
 ******************************************************************************
 * @file      SweepLine.h
 *
 * @brief     Header file for SweepLine implementation
 ******************************************************************************
 */

#ifndef SWEEPLINE_H
#define SWEEPLINE_H

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"
#include "IBinarySearchTree.h"
#include "IPolygon.h"

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

typedef struct tSweepLineSegment
{
    uint16 segmentIndex;
    tCoordinate2D leftEndPoint;
    tCoordinate2D rightEndPoint;
    struct tSweepLineSegment* segmentAbove;
    struct tSweepLineSegment* segmentBelow;
} tSweepLineSegment;

typedef struct tSweepLine
{
    uint16 index;
    uint16 maxSize;
    tSweepLineSegment* segmentBuffer;
    tIBinarySearchTree* BST;
    const tILine* lineSegments;
    const uint16 lineSegmentsSize;
    const tIPolygon* polygon;
} tSweepLine;

/*
 ------------------------------------------------------------------------------
    Public function prototypes
 ------------------------------------------------------------------------------
*/

/**
 ******************************************************************************
 * @brief   Initialize sweep line structure.
 * @param   pSweepLine
 *          Pointer to sweep line structure.
 * @param   pBST
 *          Pointer to BST structure.
 ******************************************************************************
 */
void SweepLine_Initialize( tSweepLine* pSweepLine,
                           tIBinarySearchTree* pBST );

/**
 ******************************************************************************
 * @brief   Create new sweep line segment from event and
 *          add it to sweep line structure.
 * @param   pSweepLine
 *          Pointer to sweep line structure.
 * @param   index
 *          Event index.
 * @param   ppSegment
 *          Pointer to added sweep line segment if successful.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool SweepLine_Add( tSweepLine* pSweepLine,
                    uint16 index,
                    tSweepLineSegment** ppSegment );

/**
 ******************************************************************************
 * @brief   Remove sweep line segment from sweep line structure.
 * @param   pSweepLine
 *          Pointer to sweep line structure.
 * @param   pSegment
 *          Pointer to segment that shall be removed.
 ******************************************************************************
 */
void SweepLine_Remove( tSweepLine* pSweepLine,
                       tSweepLineSegment* pSegment );

/**
 ******************************************************************************
 * @brief   Check if two sweep line segments intersect at any point.
 * @param   pSweepLine
 *          Pointer to sweep line structure.
 * @param   pSegmentA
 *          Pointer to segment.
 * @param   pSegmentB
 *          Pointer to segment.
 ******************************************************************************
 */
tCoordinate2D SweepLine_Intersection( tSweepLine* pSweepLine,
                                      tSweepLineSegment *pSegmentA,
                                      tSweepLineSegment *pSegmentB );

#endif /* SWEEPLINE_H */

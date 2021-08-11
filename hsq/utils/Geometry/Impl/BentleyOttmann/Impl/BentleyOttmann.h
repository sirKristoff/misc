/**
 ******************************************************************************
 * @file      BentleyOttmann.h
 *
 * @brief     Header file for BentleyOttmann implementation
 ******************************************************************************
 */

#ifndef BENTLEYOTTMANN_H
#define BENTLEYOTTMANN_H

/*
 ------------------------------------------------------------------------------
    Include files
 ------------------------------------------------------------------------------
*/

#include "RoboticTypes.h"
#include "IPolygon.h"
#include "ILine.h"

/*
 ------------------------------------------------------------------------------
    Public function prototypes
 ------------------------------------------------------------------------------
*/

/**
 ******************************************************************************
 * @brief   Find intersection points among all edges of a polygon.
 *          NOTE: SOFTWARE_ASSERTs that pointers are not null.
 * @param   pPolygon
 *          Pointer to polygon.
 * @param   pIntersectionsOut
 *          Pointer to found intersection points, must be initialized.
 * @param   pIntersectionsSizeOut
 *          Pointer to number of intersection points found.
 * @param   intersectionsSizeIn
 *          Total size allocated for pIntersectionsOut.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool BentleyOttmann_GetPolygonIntersections( const tIPolygon* pPolygon,
                                             tCoordinate2D* pIntersectionsOut,
                                             uint16* pIntersectionsSizeOut,
                                             uint16 intersectionsSizeIn );

/**
 ******************************************************************************
 * @brief   Find intersection points among a set of line segments.
 *          NOTE: SOFTWARE_ASSERTs that pointers are not null.
 * @param   pLineSegments
 *          Pointer to set of line segments.
 * @param   linesSegmentsSize
 *          Number of line segments in pLineSegments.
 * @param   pIntersectionsOut
 *          Pointer to found intersection points, must be initialized.
 * @param   pIntersectionsSizeOut
 *          Pointer to number of intersection points found.
 * @param   intersectionsSizeIn
 *          Total size allocated for pIntersectionsOut.
 * @return  True if successful, otherwise false.
 ******************************************************************************
 */
bool BentleyOttmann_GetLineIntersections( const tILine* pLineSegments,
                                          const uint16 linesSegmentsSize,
                                          tCoordinate2D* pIntersectionsOut,
                                          uint16* pIntersectionsSizeOut,
                                          uint16 intersectionsSizeIn );

#endif /* BENTLEYOTTMANN_H */

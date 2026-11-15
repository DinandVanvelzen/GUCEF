/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_GEOMETRY_ETYPES_H
#define GUCEF_GEOMETRY_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_ETYPES_H
#include "gucefMATH_ETypes.h"
#define GUCEF_MATH_ETYPES_H
#endif /* GUCEF_MATH_ETYPES_H ? */

#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef MATH::UInt8 UInt8;
typedef MATH::Int8 Int8;
typedef MATH::UInt16 UInt16;
typedef MATH::Int16 Int16;
typedef MATH::UInt32 UInt32;
typedef MATH::Int32 Int32;
typedef MATH::UInt64 UInt64;
typedef MATH::Int64 Int64;
typedef MATH::Float32 Float32;
typedef MATH::Float64 Float64;

/*-------------------------------------------------------------------------*/

/**
 *  The shapes of the library. The values index the pair dispatch table of the intersection tests, new shapes are only
 *  ever added before GEOSHAPE_COUNT.
 */
enum EGeoShapeType
{
    GEOSHAPE_UNKNOWN = 0         ,
    GEOSHAPE_RAY                 ,
    GEOSHAPE_LINE                ,
    GEOSHAPE_SEGMENT             ,
    GEOSHAPE_PLANE               ,
    GEOSHAPE_SPHERE              ,
    GEOSHAPE_AXIS_ALIGNED_BOX    ,
    GEOSHAPE_TRIANGLE            ,
    GEOSHAPE_CAPSULE             ,
    GEOSHAPE_CIRCLE              ,
    GEOSHAPE_RECTANGLE           ,
    GEOSHAPE_ORIENTED_RECTANGLE  ,
    GEOSHAPE_TRIANGLE_2D         ,
    GEOSHAPE_POLYGON             ,
    GEOSHAPE_POLYGON_WITH_HOLES  ,
    GEOSHAPE_POLYLINE_2D         ,
    GEOSHAPE_POLYLINE_3D         ,
    GEOSHAPE_ORIENTED_BOX        ,
    GEOSHAPE_CYLINDER            ,
    GEOSHAPE_CONE                ,
    GEOSHAPE_FRUSTUM             ,
    GEOSHAPE_PLANE_BOUNDED_VOLUME,
    GEOSHAPE_CONVEX_HULL         ,

    GEOSHAPE_COUNT                 /**< the number of shapes, not a shape */
};
typedef enum EGeoShapeType TGeoShapeType;

/** The frame that a translation or rotation of a geometry space is given in */
enum EGeoSpaceRelativeTo
{
    GEOSPACE_RELATIVETO_LOCAL = 0 ,
    GEOSPACE_RELATIVETO_PARENT    ,
    GEOSPACE_RELATIVETO_WORLD
};
typedef enum EGeoSpaceRelativeTo TGeoSpaceRelativeTo;

/** Result of testing a volume against a frustum */
enum EFrustumClassification
{
    FRUSTUMCLASSIFICATION_OUTSIDE = 0 ,
    FRUSTUMCLASSIFICATION_INTERSECTING ,
    FRUSTUMCLASSIFICATION_INSIDE
};
typedef enum EFrustumClassification TFrustumClassification;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_GEOMETRY_ETYPES_H ? */

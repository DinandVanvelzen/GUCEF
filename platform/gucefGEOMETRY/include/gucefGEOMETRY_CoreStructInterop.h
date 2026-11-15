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

#ifndef GUCEF_GEOMETRY_CORESTRUCTINTEROP_H
#define GUCEF_GEOMETRY_CORESTRUCTINTEROP_H

/*
 *  Conversions between the rectangle and box structures of gucefCORE (EStructs.h) and the gucefGEOMETRY shapes.
 *  Free functions, so that CORE types never appear inside the class templates:
 *  - TArea2D <-> TFloat32Rectangle ( CTRectangle< Float32 > ): ToShape, ToCore;
 *  - TArea3D <-> TFloat32AxisAlignedBox3D ( CTAxisAlignedBox3D< Float32 > ): ToShape, ToCore.
 *  All functions are exact and realtime-safe. The point forms are in gucefMATH_CoreStructInterop.h.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The rectangle of the area ( min, max ) */
GUCEF_GEOMETRY_FORCE_INLINE CTRectangle< Float32 >
ToShape( const CORE::TArea2D& area ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTRectangle< Float32 >( MATH::CTPoint2D< Float32 >( area.min[ 0 ], area.min[ 1 ] ), MATH::CTPoint2D< Float32 >( area.max[ 0 ], area.max[ 1 ] ) );
}

/*-------------------------------------------------------------------------*/

/** The area of the rectangle; an empty rectangle gives min above max, as it is in the rectangle */
GUCEF_GEOMETRY_FORCE_INLINE CORE::TArea2D
ToCore( const CTRectangle< Float32 >& rectangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CORE::TArea2D area;
    area.min[ 0 ] = rectangle.min.x;
    area.min[ 1 ] = rectangle.min.y;
    area.max[ 0 ] = rectangle.max.x;
    area.max[ 1 ] = rectangle.max.y;
    return area;
}

/*-------------------------------------------------------------------------*/

/** The box of the area ( min, max ) */
GUCEF_GEOMETRY_FORCE_INLINE CTAxisAlignedBox3D< Float32 >
ToShape( const CORE::TArea3D& area ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTAxisAlignedBox3D< Float32 >( MATH::CTPoint3D< Float32 >( area.min[ 0 ], area.min[ 1 ], area.min[ 2 ] ),
                                          MATH::CTPoint3D< Float32 >( area.max[ 0 ], area.max[ 1 ], area.max[ 2 ] ) );
}

/*-------------------------------------------------------------------------*/

/** The area of the box; an empty box gives min above max, as it is in the box */
GUCEF_GEOMETRY_FORCE_INLINE CORE::TArea3D
ToCore( const CTAxisAlignedBox3D< Float32 >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CORE::TArea3D area;
    area.min[ 0 ] = box.min.x;
    area.min[ 1 ] = box.min.y;
    area.min[ 2 ] = box.min.z;
    area.max[ 0 ] = box.max.x;
    area.max[ 1 ] = box.max.y;
    area.max[ 2 ] = box.max.z;
    return area;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CORESTRUCTINTEROP_H ? */

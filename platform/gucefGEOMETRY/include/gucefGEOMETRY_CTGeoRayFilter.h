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

#ifndef GUCEF_GEOMETRY_CTGEORAYFILTER_H
#define GUCEF_GEOMETRY_CTGEORAYFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTGeoObjectBase;

/*-------------------------------------------------------------------------*/

/**
 *  Optional custom filter of the ray caster queries (section 7.7 of the design): a small interface called once per
 *  candidate object that passed the layer mask, before its shape is tested (coarse granularity, one virtual call per
 *  object). Accept returns false to skip the object. Called on the thread that runs the query, possibly on several
 *  threads at once for concurrent queries, so implementations must be safe for concurrent const use.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoRayFilter
{
    public:

    virtual ~CTGeoRayFilter() {}

    /** True when the object takes part in the query */
    virtual bool Accept( const CTGeoObjectBase< T >& object ) const = 0;
};

typedef CTGeoRayFilter< Float32 > CGeoRayFilter;
typedef CTGeoRayFilter< Float64 > CGeoRayFilter64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEORAYFILTER_H ? */

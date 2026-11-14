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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefGEOMETRY_CModule.h"

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#include "gucefGEOMETRY_CGeometryGlobal.h"
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_GEOMETRY_STRINGIFY2( x ) #x
#define GUCEF_GEOMETRY_STRINGIFY( x ) GUCEF_GEOMETRY_STRINGIFY2( x )

/*-------------------------------------------------------------------------*/

static bool g_moduleLoaded = false;

/*-------------------------------------------------------------------------*/

bool
CModule::Load( void )
{
    g_moduleLoaded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModule::Unload( void )
{
    g_moduleLoaded = false;
    CGeometryGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
GeometryModuleLoadProbe( void )
{
    return g_moduleLoaded;
}

/*-------------------------------------------------------------------------*/

bool
GeometryModulePublicSpecProbe( void )
{
    #ifdef GUCEF_GEOMETRY_BUILD_MODULE
    return 0 == strcmp( GUCEF_GEOMETRY_STRINGIFY( GUCEF_GEOMETRY_PUBLIC_CPP ), GUCEF_GEOMETRY_STRINGIFY( GUCEF_EXPORT ) );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

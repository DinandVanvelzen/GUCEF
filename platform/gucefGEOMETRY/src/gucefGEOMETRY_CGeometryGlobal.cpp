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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#include "gucefGEOMETRY_CGeometryGlobal.h"

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

static CGeometryGlobal* g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CGeometryGlobal*
CGeometryGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        // Ensure the systems of the modules this module depends on are initialized first
        CORE::CCoreGlobal::Instance();
        MATH::CMathGlobal::Instance();

        g_instance = GUCEF_NEW CGeometryGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CGeometryGlobal::Deinstance( void )
{GUCEF_TRACE;

    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CGeometryGlobal::CGeometryGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeometryGlobal::~CGeometryGlobal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGeometryGlobal::Initialize( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

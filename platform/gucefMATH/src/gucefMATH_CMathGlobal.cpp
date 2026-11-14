/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#include "gucefMATH_CMathGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CMathGlobal* g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CMathGlobal*
CMathGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        // Ensure the Core systems are initialized first since this module depends on Core
        CORE::CCoreGlobal::Instance();

        g_instance = GUCEF_NEW CMathGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CMathGlobal::Deinstance( void )
{GUCEF_TRACE;

    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMathGlobal::CMathGlobal( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMathGlobal::~CMathGlobal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMathGlobal::Initialize( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

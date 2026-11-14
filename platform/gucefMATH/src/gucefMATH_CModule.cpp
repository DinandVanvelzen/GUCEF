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

#include <string.h>

#include "gucefMATH_CModule.h"

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

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

#define GUCEF_MATH_STRINGIFY2( x ) #x
#define GUCEF_MATH_STRINGIFY( x ) GUCEF_MATH_STRINGIFY2( x )

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
    CMathGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
MathModuleLoadProbe( void )
{
    return g_moduleLoaded;
}

/*-------------------------------------------------------------------------*/

bool
MathModulePublicSpecProbe( void )
{
    #ifdef GUCEF_MATH_BUILD_MODULE
    return 0 == strcmp( GUCEF_MATH_STRINGIFY( GUCEF_MATH_PUBLIC_CPP ), GUCEF_MATH_STRINGIFY( GUCEF_EXPORT ) );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

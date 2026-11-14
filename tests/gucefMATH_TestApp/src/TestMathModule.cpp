/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_H
#include "gucefMATH.h"
#define GUCEF_MATH_H
#endif /* GUCEF_MATH_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#include "TestMathModule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_MATH_STATIC_ASSERT( 1, GucefMathTestStaticAssertOk );
GUCEF_MATH_STATIC_ASSERT( sizeof( MATH::Float32 ) == 4 && sizeof( MATH::Float64 ) == 8, GucefMathTestFloatSizes );

/*-------------------------------------------------------------------------*/

static GUCEF_MATH_FORCE_INLINE int
ForceInlinedSum( int a, int b )
{
    return a + b;
}

/*-------------------------------------------------------------------------*/

static int
RealtimeContextFunction( int a ) GUCEF_MATH_REALTIME_CONTEXT;

static int
RealtimeContextFunction( int a )
{
    return a * 2;
}

/*-------------------------------------------------------------------------*/

void
PerformMathModuleTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "MathModule: The global singleton exists and is stable" )
    try
    {
        MATH::CMathGlobal* global = MATH::CMathGlobal::Instance();
        ASSERT_TRUE( GUCEF_NULL != global );
        ASSERT_TRUE( global == MATH::CMathGlobal::Instance() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    /*
     *  Only the Microsoft Windows loader calls the module entry point of gucef_DLLInit.h. On the other platforms the
     *  module, like every GUCEF module, does not rely on CModule::Load() having run.
     */
    GUCEF_TESTFW_TESTCASE( "MathModule: Load ran when the library was loaded" )
    try
    {
        ASSERT_TRUE( MATH::MathModuleLoadProbe() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    #endif /* GUCEF_PLATFORM_MSWIN ? */

    GUCEF_TESTFW_TESTCASE( "MathModule: GUCEF_MATH_PUBLIC_CPP is the export specification inside the module" )
    try
    {
        // Callable from here at all means the function is exported. The module itself checks what the macro expanded to.
        ASSERT_TRUE( MATH::MathModulePublicSpecProbe() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MathModule: Macros GUCEF_MATH_REALTIME_CONTEXT, FORCE_INLINE, STATIC_ASSERT and ASSERT" )
    try
    {
        #ifndef GUCEF_MATH_REALTIME_CONTEXT
        ERRORHERE;
        #endif
        #ifndef GUCEF_MATH_PUBLIC_CPP
        ERRORHERE;
        #endif
        #ifndef GUCEF_MATH_PRIVATE_CPP
        ERRORHERE;
        #endif
        ASSERT_TRUE( 5 == ForceInlinedSum( 2, 3 ) );
        ASSERT_TRUE( 8 == RealtimeContextFunction( 4 ) );
        GUCEF_MATH_STATIC_ASSERT( 1, GucefMathTestStaticAssertInFunction );
        GUCEF_MATH_ASSERT( true );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

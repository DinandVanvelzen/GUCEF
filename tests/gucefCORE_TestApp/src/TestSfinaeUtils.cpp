/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_SFINAE_UTILS_H
#include "gucefCORE_SFINAE_utils.h"
#define GUCEF_CORE_SFINAE_UTILS_H
#endif /* GUCEF_CORE_SFINAE_UTILS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSfinaeUtils.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SPlain
{
    int value;
};

struct SOwnAssignment
{
    SOwnAssignment& operator=( const SOwnAssignment& src ) { value = src.value; return *this; }
    int value;
};

class CNotAssignable
{
    public:
    CNotAssignable( void ) {}

    private:
    CNotAssignable& operator=( const CNotAssignable& src );    /**< not implemented */
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSfinaeUtilsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.SfinaeUtils" );

    GUCEF_TESTFW_TESTCASE( "SfinaeUtils: TypeHasAssignmentOperator finds an accessible copy assignment, also the implicit one" )
    try
    {
        ASSERT_TRUE( 0 != CORE::TypeHasAssignmentOperator< SPlain >::value );
        ASSERT_TRUE( 0 != CORE::TypeHasAssignmentOperator< SOwnAssignment >::value );
        ASSERT_TRUE( 0 != CORE::TypeHasAssignmentOperator< CORE::CString >::value );
        ASSERT_FALSE( 0 != CORE::TypeHasAssignmentOperator< CNotAssignable >::value );
        ASSERT_FALSE( 0 != CORE::TypeHasAssignmentOperator< int >::value );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

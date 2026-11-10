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

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#include "gucefCORE_CTScopedValue.h"
#define GUCEF_CORE_CTSCOPEDVALUE_H
#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestScopedValue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** Guards a block against coming back into it: the flag is the guard */
bool
EnterGuardedBlock( bool& busy, bool returnEarly, bool throwInside, bool* reenteredWhileBusy )
{
    if ( busy )
    {
        *reenteredWhileBusy = true;
        return false;
    }

    CORE::CTScopedValue< bool > scope( busy, true );
    if ( returnEarly )
        return false;
    if ( throwInside )
        throw 42;

    // a call from inside the block finds the flag set
    EnterGuardedBlock( busy, false, false, reenteredWhileBusy );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformScopedValueTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ScopedValue" );

    GUCEF_TESTFW_TESTCASE( "ScopedValue: The variable has the value of the scope and gets back what it had when the block ends" )
    try
    {
        bool flag = false;
        {
            CORE::CTScopedValue< bool > scope( flag, true );
            ASSERT_TRUE( flag );
            ASSERT_FALSE( scope.GetPreviousValue() );
        }
        ASSERT_FALSE( flag );

        // what was there is what comes back, not a default
        Int32 number = 7;
        {
            CORE::CTScopedValue< Int32 > scope( number, 9 );
            ASSERT_TRUE( 9 == number );
            number = 11;
        }
        ASSERT_TRUE( 7 == number );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScopedValue: The value is put back on a return from the block and when an exception unwinds it" )
    try
    {
        bool busy = false;
        bool reentered = false;
        ASSERT_FALSE( EnterGuardedBlock( busy, true, false, &reentered ) );
        ASSERT_FALSE( busy );

        bool caught = false;
        try
        {
            EnterGuardedBlock( busy, false, true, &reentered );
        }
        catch ( int )
        {
            caught = true;
        }
        ASSERT_TRUE( caught );
        ASSERT_FALSE( busy );

        // the block can be entered again, and a call from inside it is turned away
        ASSERT_FALSE( reentered );
        ASSERT_TRUE( EnterGuardedBlock( busy, false, false, &reentered ) );
        ASSERT_TRUE( reentered );
        ASSERT_FALSE( busy );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScopedValue: Scopes of the same variable nest" )
    try
    {
        Int32 level = 0;
        {
            CORE::CTScopedValue< Int32 > outer( level, 1 );
            {
                CORE::CTScopedValue< Int32 > inner( level, 2 );
                ASSERT_TRUE( 2 == level );
                ASSERT_TRUE( 1 == inner.GetPreviousValue() );
            }
            ASSERT_TRUE( 1 == level );
        }
        ASSERT_TRUE( 0 == level );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

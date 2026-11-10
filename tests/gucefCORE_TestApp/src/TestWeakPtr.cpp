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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestWeakPtr.h"

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

/** Counts how many of its kind are alive */
class CCounted
{
    public:

    static Int32 alive;
    Int32 value;

    CCounted( Int32 v ) : value( v ) { ++alive; }
    virtual ~CCounted() { --alive; }
};

Int32 CCounted::alive = 0;

class CDerivedCounted : public CCounted
{
    public:

    CDerivedCounted( Int32 v ) : CCounted( v ) {}
    virtual ~CDerivedCounted() {}
};

typedef CORE::CTSharedPtr< CCounted, MT::CMutex > TCountedPtr;
typedef CORE::CTWeakPtr< CCounted, MT::CMutex > TCountedWeakPtr;
typedef CORE::CTSharedPtr< CDerivedCounted, MT::CMutex > TDerivedCountedPtr;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWeakPtrTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.WeakPtr" );

    GUCEF_TESTFW_TESTCASE( "WeakPtr: A weak pointer does not keep the object alive and expires when the last shared pointer goes" )
    try
    {
        CCounted::alive = 0;
        TCountedWeakPtr weak;
        ASSERT_TRUE( weak.IsExpired() && weak.GetSharedPtr().IsNULL() );
        {
            TCountedPtr shared( GUCEF_NEW CCounted( 7 ) );
            weak = shared;
            ASSERT_FALSE( weak.IsExpired() );
            ASSERT_TRUE( 1 == shared.GetReferenceCount() );

            TCountedPtr again = weak.GetSharedPtr();
            ASSERT_TRUE( !again.IsNULL() && 7 == again->value && again == shared );
            ASSERT_TRUE( 2 == shared.GetReferenceCount() );
        }
        ASSERT_TRUE( 0 == CCounted::alive );
        ASSERT_TRUE( weak.IsExpired() );
        ASSERT_TRUE( weak.GetSharedPtr().IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WeakPtr: A shared pointer from a weak one keeps the object alive after the others went" )
    try
    {
        CCounted::alive = 0;
        TCountedPtr kept;
        TCountedWeakPtr weak;
        {
            TCountedPtr shared( GUCEF_NEW CCounted( 3 ) );
            weak = shared;
            kept = weak.GetSharedPtr();
        }
        ASSERT_TRUE( 1 == CCounted::alive && !weak.IsExpired() && 3 == kept->value );
        kept.Unlink();
        ASSERT_TRUE( 0 == CCounted::alive && weak.IsExpired() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WeakPtr: Copies, moves and weak pointers to a base class refer to the same object, in sets too" )
    try
    {
        CCounted::alive = 0;
        TDerivedCountedPtr derived( GUCEF_NEW CDerivedCounted( 11 ) );
        TCountedWeakPtr weak( derived );
        TCountedWeakPtr copy( weak );
        TCountedWeakPtr assigned;
        assigned = copy;
        ASSERT_TRUE( weak == copy && copy == assigned );
        ASSERT_TRUE( 11 == assigned.GetSharedPtr()->value );

        #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
        TCountedWeakPtr moved( GUCEF_MOVE( copy ) );
        ASSERT_TRUE( copy.IsExpired() && moved == weak );
        #endif

        TCountedPtr other( GUCEF_NEW CCounted( 12 ) );
        GUCEF::set< TCountedWeakPtr > weakSet;
        weakSet.insert( weak );
        weakSet.insert( assigned );
        weakSet.insert( TCountedWeakPtr( other ) );
        ASSERT_TRUE( 2 == weakSet.size() );

        derived.Unlink();
        ASSERT_TRUE( 1 == CCounted::alive );
        ASSERT_TRUE( weak.IsExpired() && assigned.IsExpired() );
        UInt32 alive = 0;
        for ( GUCEF::set< TCountedWeakPtr >::iterator i=weakSet.begin(); i!=weakSet.end(); ++i )
            if ( !(*i).IsExpired() )
                ++alive;
        ASSERT_TRUE( 1 == alive );
        weakSet.clear();
        other.Unlink();
        ASSERT_TRUE( 0 == CCounted::alive );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WeakPtr: Giving up ownership of the last shared pointer leaves the object alive and the weak pointer expired" )
    try
    {
        CCounted::alive = 0;
        TCountedPtr shared( GUCEF_NEW CCounted( 5 ) );
        TCountedWeakPtr weak( shared );
        CCounted* raw = GUCEF_NULL;
        ASSERT_TRUE( shared.RelinquishOwnership( raw ) );
        ASSERT_TRUE( GUCEF_NULL != raw && 1 == CCounted::alive );
        ASSERT_TRUE( weak.IsExpired() );
        delete raw;
        ASSERT_TRUE( 0 == CCounted::alive );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

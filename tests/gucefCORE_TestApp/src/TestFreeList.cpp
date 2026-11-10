/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTFREELIST_H
#include "gucefCORE_CTFreeList.h"
#define GUCEF_CORE_CTFREELIST_H
#endif /* GUCEF_CORE_CTFREELIST_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFreeList.h"

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */


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
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// Lock type: use noLock for single-thread basic tests
typedef MT::CNoLock TNoLock;

// A simple test type with optional Clear()
struct TestObj
{
    Int32 value;
    TestObj() : value( 0 ) {}
    TestObj( const TestObj& src ) : value( src.value ) {}
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObj, TNoLock >::TSharedPtr TestObjPtr;

// A test type with optional Clear() and its own shared pointer creator
// having said creator should force a different code path in the free list wrt which objects to maintain internally
struct TestObjWithPtrCreator : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >
{
    Int32 value;
    TestObjWithPtrCreator( void ) : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >( this ), value( 321 ) {}
    TestObjWithPtrCreator( const TestObjWithPtrCreator& src ) : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >( this ), value( src.value ) {}
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr TestObjWithPtrCreatorPtr;

// A type without Clear()
struct TestObjNoClear
{
    Int32 value;
    TestObjNoClear( void ) : value( 123 ) {}
    TestObjNoClear( const TestObjNoClear& src ) : value( src.value ) {}
};

typedef CTFreeList< TestObjNoClear, TNoLock >::TSharedPtr TestObjNoClearPtr;

// A type which tracks the lifetime of every instance by address, catching double construction and double destruction
struct TestObjLifetime
{
    static GUCEF::set< const void* > liveInstances;
    static UInt32 doubleConstructions;
    static UInt32 doubleDestructions;

    Int32 value;
    TestObjLifetime( void ) : value( 7 ) { if ( !liveInstances.insert( this ).second ) ++doubleConstructions; }
    TestObjLifetime( const TestObjLifetime& src ) : value( src.value ) { if ( !liveInstances.insert( this ).second ) ++doubleConstructions; }
    ~TestObjLifetime() { if ( 0 == liveInstances.erase( this ) ) ++doubleDestructions; }

    static void Reset( void ) { liveInstances.clear(); doubleConstructions = 0; doubleDestructions = 0; }
};

GUCEF::set< const void* > TestObjLifetime::liveInstances;
UInt32 TestObjLifetime::doubleConstructions = 0;
UInt32 TestObjLifetime::doubleDestructions = 0;

typedef CTFreeList< TestObjLifetime, TNoLock > TLifetimeFreeList;

// A type which is reset by Clear() and counts constructions and clears
struct TestObjCountedClear
{
    static UInt32 constructions;
    static UInt32 clears;

    Int32 value;
    TestObjCountedClear( void ) : value( 0 ) { ++constructions; }
    TestObjCountedClear( const TestObjCountedClear& src ) : value( src.value ) { ++constructions; }
    void Clear( void ) { value = 0; ++clears; }
};

UInt32 TestObjCountedClear::constructions = 0;
UInt32 TestObjCountedClear::clears = 0;

// A type which only inherits its Clear()
struct TestObjInheritedClear : public TestObjCountedClear
{
    Int32 extra;
    TestObjInheritedClear( void ) : TestObjCountedClear(), extra( 0 ) {}
};

/** Tries to lock a shared pointer from a thread of its own, the shared lock must not be held by anyone */
class CSharedPtrLockProbe : public MT::CActiveObject
{
    public:

    typedef CTFreeList< TestObj, MT::CMutex >::TSharedPtr TPtr;

    explicit CSharedPtrLockProbe( TPtr& ptr ) : MT::CActiveObject(), m_ptr( ptr ), result( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::TLockStatus lockStatus = MT::LOCKSTATUS_UNDEFINED;
        {
            MT::CObjectScopeLock lock( m_ptr, 500, lockStatus );
        }
        result.Store( MT::LOCKSTATUS_OPERATION_SUCCESS == lockStatus ? 1 : 2 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    TPtr& m_ptr;
    MT::CTAtomic< Int32 > result;
};

static void TestReserveAndAcquire_Reconstruct()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( sizeof( CTFreeList< TestObj, TNoLock >::TSharedPtrCreator ) > sizeof( TestObj ) );

    pool.Reserve( 10 );
    UInt32 freed = pool.GetFreedCount();
    ASSERT_TRUE( freed >= 10 );

    TestObjPtr a = pool.Acquire();
    ASSERT_TRUE( !a.IsNULL() );
    ASSERT_TRUE( pool.GetActiveCount() == 1 );
    ASSERT_TRUE( pool.GetFreedCount() == freed-1 );

    a->value = 42;

    TestObj* rawPtr = static_cast< TestObj* >( a.GetPointerAlways() );
    CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* sharedPtrCreator = static_cast< CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* >( rawPtr );
    ASSERT_TRUE( sharedPtrCreator->value == 42 );

    TestObjPtr b = sharedPtrCreator->CreateBasicSharedPtr();
    ASSERT_TRUE( !b.IsNULL() );
    ASSERT_TRUE( b.GetPointerAlways() == a.GetPointerAlways() );
    ASSERT_TRUE( b->value == 42 );
    ASSERT_TRUE( b.GetReferenceCount() == 2 );
    a.Unlink();
    ASSERT_TRUE( b.GetReferenceCount() == 1 );

    // Dropping reference returns memory to pool
    b.Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freed );
}

static void TestReserveBlocksAndAcquire()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );
    ASSERT_TRUE( pool.Reserve( 7 ) );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    GUCEF::vector< CTFreeList< TestObj, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        ASSERT_TRUE( !objs.back().IsNULL() );
        objs.back()->value = i;
    }
    ASSERT_TRUE( pool.GetActiveCount() == 7 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize-7 );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );
}

static void TestReserveBlocksAndAcquireWithPtrCreator()
{
    CTFreeList< TestObjWithPtrCreator, TNoLock > pool( CTFreeList< TestObjWithPtrCreator, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    ASSERT_TRUE( sizeof( CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtrCreator ) == sizeof( TestObjWithPtrCreator ) );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );
    ASSERT_TRUE( pool.Reserve( 7 ) );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    GUCEF::vector< CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        ASSERT_TRUE( !objs.back().IsNULL() );
        objs.back()->value = i;
    }
    ASSERT_TRUE( pool.GetActiveCount() == 7 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize-7 );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );
}

static void TestClearPolicy()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    ASSERT_TRUE( pool.GetFreedCount() == 0 );
    pool.Reserve( 3 );
    UInt32 blockSize = pool.GetBlockSize();
    ASSERT_TRUE( blockSize > 0 );    
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    TestObjPtr a = pool.Acquire();
    a->value = 99;
    a.Unlink();
    ASSERT_TRUE( pool.GetFreedCount() == blockSize );

    TestObjPtr b = pool.Acquire();
    ASSERT_TRUE( b->value == 0 );
}

static void TestNoClearPolicyGracefulFallback()
{
    // test to check if the container gracefully falls back to reconstruct when Clear() is not available
    // even when the Clear()-based-policy is requested. It says 'if available' after all.

    CTFreeList< TestObjNoClear, TNoLock > pool( CTFreeList< TestObjNoClear, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    pool.Reserve( 2 );
    UInt32 freeItems = pool.GetFreedCount();
    ASSERT_TRUE( freeItems >= 2 );

    TestObjNoClearPtr a = pool.Acquire();
    ASSERT_TRUE( a->value == 123 );
    a.Unlink();
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );
}

static void TestObjectLifetimeAcrossReuse( TLifetimeFreeList::TReusePolicy policy )
{
    TestObjLifetime::Reset();
    {
        TLifetimeFreeList pool( policy );
        ASSERT_TRUE( pool.Reserve( 3 ) );

        TLifetimeFreeList::TSharedPtr a = pool.Acquire();
        ASSERT_TRUE( !a.IsNULL() && 7 == a->value );
        a->value = 1;
        a.Unlink();                                         // back to the free list

        TLifetimeFreeList::TSharedPtr b = pool.Acquire();   // reuses the slot a released
        ASSERT_TRUE( !b.IsNULL() && 7 == b->value );
        TLifetimeFreeList::TSharedPtr c = pool.Acquire();   // a slot which was never used before
        ASSERT_TRUE( !c.IsNULL() && 7 == c->value );
        b.Unlink();
        c.Unlink();
    }
    // Destroying the free list must leave no object alive, and no object may have been constructed or destroyed twice
    ASSERT_TRUE( 0 == TestObjLifetime::doubleConstructions );
    ASSERT_TRUE( 0 == TestObjLifetime::doubleDestructions );
    ASSERT_TRUE( TestObjLifetime::liveInstances.empty() );
}

static void TestClearPolicyReusesObjectsThroughClear( void )
{
    TestObjCountedClear::constructions = 0;
    TestObjCountedClear::clears = 0;
    {
        CTFreeList< TestObjCountedClear, TNoLock > pool( CTFreeList< TestObjCountedClear, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );
        ASSERT_TRUE( pool.Reserve( 2 ) );
        const UInt32 constructedByReserve = TestObjCountedClear::constructions;

        CTFreeList< TestObjCountedClear, TNoLock >::TSharedPtr a = pool.Acquire();
        const void* address = a.GetPointerAlways();
        a->value = 99;
        a.Unlink();
        ASSERT_TRUE( 1 == TestObjCountedClear::clears );

        CTFreeList< TestObjCountedClear, TNoLock >::TSharedPtr b = pool.Acquire();     // the object a released
        ASSERT_TRUE( address == b.GetPointerAlways() );
        ASSERT_TRUE( 0 == b->value );
        ASSERT_TRUE( constructedByReserve == TestObjCountedClear::constructions );    // reused, not reconstructed

        CTFreeList< TestObjCountedClear, TNoLock >::TSharedPtr c = b;                   // the reused object hands out shared pointers as before
        ASSERT_TRUE( 2 == c.GetReferenceCount() );
        b.Unlink();
        c.Unlink();
        ASSERT_TRUE( 2 == TestObjCountedClear::clears );
        ASSERT_TRUE( 0 == pool.GetActiveCount() );
    }
}

static void TestClearPolicyFindsAnInheritedClear( void )
{
    TestObjCountedClear::clears = 0;
    CTFreeList< TestObjInheritedClear, TNoLock > pool( CTFreeList< TestObjInheritedClear, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );
    CTFreeList< TestObjInheritedClear, TNoLock >::TSharedPtr a = pool.Acquire();
    a->value = 5;
    a.Unlink();
    ASSERT_TRUE( 1 == TestObjCountedClear::clears );
}

static void TestReusedObjectSharedLockIsFree( TLifetimeFreeList::TReusePolicy policy )
{
    CTFreeList< TestObj, MT::CMutex > pool( (CTFreeList< TestObj, MT::CMutex >::TReusePolicy) (int) policy );
    CSharedPtrLockProbe::TPtr a = pool.Acquire();
    a.Unlink();
    CSharedPtrLockProbe::TPtr b = pool.Acquire();       // reuses the object a released, on this thread

    CSharedPtrLockProbe probe( b );
    ASSERT_TRUE( probe.Activate() );
    UInt32 waited = 0;
    while ( 0 == probe.result.Load() && waited < 5000 )
    {
        MT::PrecisionDelay( 5 );
        waited += 5;
    }
    probe.Deactivate( false, true );
    ASSERT_TRUE( 1 == probe.result.Load() );
}

static void TestDormantTransitions()
{
    CTFreeList< TestObj, TNoLock > pool;
    CTFreeList< TestObj, TNoLock >::TSharedPtr a = pool.Acquire();
    UInt32 freeItems = pool.GetFreedCount();
    ASSERT_TRUE( pool.GetActiveCount() == 1 );

    bool moved = pool.MarkDormant( a );
    ASSERT_TRUE( moved );
    ASSERT_TRUE( pool.GetDormantCount() == 1 );
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );    

    bool back = pool.MarkActive( a );
    ASSERT_TRUE( back );
    ASSERT_TRUE( pool.GetActiveCount() == 1 );
    ASSERT_TRUE( pool.GetDormantCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() == freeItems );

    a.Unlink();
    ASSERT_TRUE( pool.GetActiveCount() == 0 );
    ASSERT_TRUE( pool.GetFreedCount() > freeItems );
}

}; /* namespace CORE */
}; /* namespace GUCEF */

void PerformFreeListTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "COMMENCING CTFreeList TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CTFreeList" );

    GUCEF_TESTFW_TESTCASE( "Test 1: Reserve and Acquire with Reconstruct" )
        try
        {
            GUCEF::CORE::TestReserveAndAcquire_Reconstruct();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 2: Reserve Blocks and Acquire" )
        try
        {
            GUCEF::CORE::TestReserveBlocksAndAcquire();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 3: Reserve Blocks and Acquire With PtrCreator" )
        try
        {
            GUCEF::CORE::TestReserveBlocksAndAcquireWithPtrCreator();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 4: Clear Policy" )
        try
        {
            GUCEF::CORE::TestClearPolicy();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 5: No Clear Policy Graceful Fallback" )
        try
        {
            GUCEF::CORE::TestNoClearPolicyGracefulFallback();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 6: Dormant Transitions" )
        try
        {
            GUCEF::CORE::TestDormantTransitions();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 7: Objects are constructed and destroyed once per use with Reconstruct" )
        try
        {
            GUCEF::CORE::TestObjectLifetimeAcrossReuse( GUCEF::CORE::TLifetimeFreeList::REUSE_RECONSTRUCT_OBJECT );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 8: Objects are constructed and destroyed once per use with the Clear Policy fallback" )
        try
        {
            GUCEF::CORE::TestObjectLifetimeAcrossReuse( GUCEF::CORE::TLifetimeFreeList::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 9: The Clear Policy reuses objects through Clear() without reconstructing them" )
        try
        {
            GUCEF::CORE::TestClearPolicyReusesObjectsThroughClear();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 10: The Clear Policy finds a Clear() that the type inherits" )
        try
        {
            GUCEF::CORE::TestClearPolicyFindsAnInheritedClear();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test 11: A reused object's shared pointer lock is free for other threads" )
        try
        {
            GUCEF::CORE::TestReusedObjectSharedLockIsFree( GUCEF::CORE::TLifetimeFreeList::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );
            GUCEF::CORE::TestReusedObjectSharedLockIsFree( GUCEF::CORE::TLifetimeFreeList::REUSE_RECONSTRUCT_OBJECT );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF::CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "ALL CTFreeList TESTS COMPLETED" );
}



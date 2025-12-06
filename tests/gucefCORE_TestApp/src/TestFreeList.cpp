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

#include "TestFreeList.h"

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
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObj, TNoLock >::TSharedPtr TestObjPtr;

// A test type with optional Clear() and its own shared pointer creator
// having said creator should force a different code path in the free list wrt which objects to maintain internally
struct TestObjWithPtrCreator : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >
{
    Int32 value;
    TestObjWithPtrCreator() : CORE::CTBasicSharedPtrCreator< TestObjWithPtrCreator, TNoLock >( this ), value( 321 ) {}
    void Clear() { value = 0; }
};

typedef CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr TestObjWithPtrCreatorPtr;

// A type without Clear()
struct TestObjNoClear
{
    Int32 value;
    TestObjNoClear() : value( 123 ) {}
};

typedef CTFreeList< TestObjNoClear, TNoLock >::TSharedPtr TestObjNoClearPtr;

// Helpers for logging assertions
static void AssertTrue( bool cond, const CString& msg )
{
    if ( cond )
    { GUCEF_LOG( LOGLEVEL_NORMAL, "CTFreeList Test PASS: " + msg ); }
    else
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "CTFreeList Test FAIL: " + msg );
}

static void TestReserveAndAcquire_Reconstruct()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    AssertTrue( sizeof( CTFreeList< TestObj, TNoLock >::TSharedPtrCreator ) > sizeof( TestObj ), "SharedPtrCreator wrapper is larger than TestObj due to added shared state" );

    pool.Reserve( 10 );
    UInt32 freed = pool.GetFreedCount();
    AssertTrue( freed >= 10, "Reserve(10) seeds freed count (reconstruct)" );

    TestObjPtr a = pool.Acquire();
    AssertTrue( !a.IsNULL(), "Acquire returns non-null shared ptr" );
    AssertTrue( pool.GetActiveCount() == 1, "Active count increments after Acquire" );
    AssertTrue( pool.GetFreedCount() == freed-1, "Freed count decremented after Acquire" );

    a->value = 42;

    TestObj* rawPtr = static_cast< TestObj* >( a.GetPointerAlways() );
    CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* sharedPtrCreator = static_cast< CTFreeList< TestObj, TNoLock >::TSharedPtrCreator* >( rawPtr );
    AssertTrue( sharedPtrCreator->value == 42, "State set on acquired casted object" );

    TestObjPtr b = sharedPtrCreator->CreateBasicSharedPtr();
    AssertTrue( !b.IsNULL(), "Create BasicSharedPtr returns non-null shared ptr" );
    AssertTrue( b.GetPointerAlways() == a.GetPointerAlways(), "New shared ptr references same underlying object" );
    AssertTrue( b->value == 42, "State preserved on new shared ptr reference" );
    AssertTrue( b.GetReferenceCount() == 2, "Reference count is 2 after copy" );
    a.Unlink();
    AssertTrue( b.GetReferenceCount() == 1, "Reference count is 1 after unlinking original reference" );

    // Dropping reference returns memory to pool
    b.Unlink();
    AssertTrue( pool.GetActiveCount() == 0, "Active count back to 0 after release" );
    AssertTrue( pool.GetFreedCount() == freed, "Freed count incremented after release (reconstruct policy)" );
}

static void TestReserveBlocksAndAcquire()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    AssertTrue( pool.GetFreedCount() == 0, "Initial freed count is 0" );
    UInt32 blockSize = pool.GetBlockSize();
    AssertTrue( blockSize > 0, "Block size is non-zero" );
    AssertTrue( pool.Reserve( 7 ), "Reserve(7) successful" );
    AssertTrue( pool.GetFreedCount() == blockSize, "First reserve matched blocksize for freed count" );

    GUCEF::vector< CTFreeList< TestObj, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        AssertTrue( !objs.back().IsNULL(), "Acquire block object non-null" );
        objs.back()->value = i;
    }
    AssertTrue( pool.GetActiveCount() == 7, "All acquired objects active" );
    AssertTrue( pool.GetFreedCount() == blockSize-7, "Freed reduced by objects created" );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    AssertTrue( pool.GetActiveCount() == 0, "All released" );
    AssertTrue( pool.GetFreedCount() == blockSize, "Freed replenished" );

    //pool.PurgeBlocks();
    //AssertTrue( pool.GetFreedCount() <= 7, "After PurgeBlocks freed entries adjusted" );
}

static void TestReserveBlocksAndAcquireWithPtrCreator()
{
    CTFreeList< TestObjWithPtrCreator, TNoLock > pool( CTFreeList< TestObjWithPtrCreator, TNoLock >::REUSE_RECONSTRUCT_OBJECT );

    AssertTrue( sizeof( CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtrCreator ) == sizeof( TestObjWithPtrCreator ), "SharedPtrCreator wrapper is the same size as TestObj due to already having shared state" );

    AssertTrue( pool.GetFreedCount() == 0, "Initial freed count is 0" );
    UInt32 blockSize = pool.GetBlockSize();
    AssertTrue( blockSize > 0, "Block size is non-zero" );
    AssertTrue( pool.Reserve( 7 ), "Reserve(7) successful" );
    AssertTrue( pool.GetFreedCount() == blockSize, "First reserve matched blocksize for freed count" );

    GUCEF::vector< CTFreeList< TestObjWithPtrCreator, TNoLock >::TSharedPtr > objs;
    for ( Int32 i=0; i<7; ++i )
    {
        objs.push_back( pool.Acquire() );
        AssertTrue( !objs.back().IsNULL(), "Acquire block object non-null" );
        objs.back()->value = i;
    }
    AssertTrue( pool.GetActiveCount() == 7, "All acquired objects active" );
    AssertTrue( pool.GetFreedCount() == blockSize-7, "Freed reduced by objects created" );

    // Release
    for ( Int32 i=0; i<7; ++i )
        objs[ i ].Unlink();
    AssertTrue( pool.GetActiveCount() == 0, "All released" );
    AssertTrue( pool.GetFreedCount() == blockSize, "Freed replenished" );
}

static void TestClearPolicy()
{
    CTFreeList< TestObj, TNoLock > pool( CTFreeList< TestObj, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    AssertTrue( pool.GetFreedCount() == 0, "Initial freed count is 0" );
    pool.Reserve( 3 );
    UInt32 blockSize = pool.GetBlockSize();
    AssertTrue( blockSize > 0, "Block size is non-zero" );    
    AssertTrue( pool.GetFreedCount() == blockSize, "Reserve(3) with Clear-policy" );

    TestObjPtr a = pool.Acquire();
    a->value = 99;
    a.Unlink();
    AssertTrue( pool.GetFreedCount() == blockSize, "Release keeps constructed entries (clear-policy)" );

    TestObjPtr b = pool.Acquire();
    AssertTrue( b->value == 0, "Acquire after Clear-policy ensures cleared state" );
}

static void TestNoClearPolicyGraceful()
{
    CTFreeList< TestObjNoClear, TNoLock > pool( CTFreeList< TestObjNoClear, TNoLock >::REUSE_OBJECT_VIA_CLEAR_METHOD_IF_AVAILABLE );

    pool.Reserve( 2 );
    AssertTrue( pool.GetFreedCount() == 2, "Reserve without Clear() still works" );

    TestObjNoClearPtr a = pool.Acquire();
    AssertTrue( a->value == 123, "State preserved for type without Clear()" );
    a = CTFreeList< TestObjNoClear, TNoLock >::TSharedPtr();
    AssertTrue( pool.GetFreedCount() == 2, "Release back to freed list" );
}

static void TestDormantTransitions()
{
    CTFreeList< TestObj, TNoLock > pool;
    CTFreeList< TestObj, TNoLock >::TSharedPtr a = pool.Acquire();

    bool moved = pool.MarkDormant( a );
    AssertTrue( moved, "MarkDormant returns true for active object" );
    AssertTrue( pool.GetDormantCount() == 1, "Dormant count increments" );

    bool back = pool.MarkActive( a );
    AssertTrue( back, "MarkActive returns true for dormant object" );
    AssertTrue( pool.GetActiveCount() == 1, "Active count restored" );

    a = CTFreeList< TestObj, TNoLock >::TSharedPtr();
    AssertTrue( pool.GetActiveCount() == 0, "Released active to freed" );
}

}; /* namespace CORE */
}; /* namespace GUCEF */

void PerformFreeListTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Running CTFreeList tests" );

    GUCEF::CORE::TestReserveAndAcquire_Reconstruct();
    GUCEF::CORE::TestReserveBlocksAndAcquire();
    GUCEF::CORE::TestReserveBlocksAndAcquireWithPtrCreator();
    GUCEF::CORE::TestClearPolicy();
    GUCEF::CORE::TestNoClearPolicyGraceful();
    GUCEF::CORE::TestDormantTransitions();

    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "CTFreeList tests completed" );
}



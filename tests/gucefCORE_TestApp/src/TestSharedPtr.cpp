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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#include "gucefCORE_CTMailboxForSharedCloneables.h"
#define GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#endif /* GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSharedPtr.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define TESTASSERT(t)   GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void* lastDeleted = GUCEF_NULL;


class IBase
{
    public:
    
    IBase() {}

    virtual ~IBase() { lastDeleted = this; }

    virtual int GetNr( void ) const = 0;
        
};

class Base : public IBase
{
    public:
    
    int y;
    
    Base() : y( 1111 ) {}

    virtual ~Base() {}

    virtual int GetNr( void ) const { return y; }
        
};

class AltBase
{
    public:
    
    int q;
    
    AltBase() : q( 3333 ) {}

    virtual ~AltBase() {}

    virtual int GetAltNr( void ) const { return q; }
        
};

class Derived : public Base    ,
                public AltBase
{
    public:
    
    int x;
    
    Derived() : Base(), AltBase(), x( 2222 ) {}

    virtual ~Derived() {}

    virtual int GetNr( void ) const GUCEF_VIRTUAL_OVERRIDE { return x; }        
};

class Unrelated
{
    public:
    
    Unrelated() : z( 9999 ) {}
    
    virtual ~Unrelated() {}
    
    int z;
};

/*-------------------------------------------------------------------------*/

void
PerformBasicSharedPtrSanityTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Basic SharedPtr sanity tests" )
    try
    {
        typedef GUCEF::CORE::CTSharedPtr< IBase, GUCEF::MT::CMutex > TIBasePtr;
        typedef GUCEF::CORE::CTSharedPtr< Base, GUCEF::MT::CMutex > TBasePtr;
        typedef GUCEF::CORE::CTSharedPtr< AltBase, GUCEF::MT::CMutex > TAltBasePtr;
        typedef GUCEF::CORE::CTSharedPtr< Derived, GUCEF::MT::CMutex > TDerivedPtr;
        typedef GUCEF::CORE::CTSharedPtr< Unrelated, GUCEF::MT::CMutex > TUnrelatedPtr;

        typedef GUCEF::CORE::CTBasicSharedPtr< IBase, GUCEF::MT::CMutex > TIBasicBasePtr;
        typedef GUCEF::CORE::CTBasicSharedPtr< Base, GUCEF::MT::CMutex > TBasicBasePtr;
        typedef GUCEF::CORE::CTBasicSharedPtr< AltBase, GUCEF::MT::CMutex > TBasicAltBasePtr;
        typedef GUCEF::CORE::CTBasicSharedPtr< Derived, GUCEF::MT::CMutex > TBasicDerivedPtr;
        typedef GUCEF::CORE::CTBasicSharedPtr< Unrelated, GUCEF::MT::CMutex > TBasicUnrelatedPtr;
        //--- compile time tests, uncomment to test
        
        TUnrelatedPtr uptr;
        TDerivedPtr dptr;
        TBasePtr bptr;
        
        //a = b;                        //<- should not compile
        //b = a;                        //<- should not compile
        //a.StaticCast< Derived >();    //<- should not compile
        
        //c = a;                        //<- should not compile
        //a = c;                        //<- should not compile
        //a.StaticCast< Base >();       //<- should not compile
        
        
        //---- runtime tests
        
        Derived* d = new Derived();
        Derived* d2 = GUCEF_NULL;
        
        bool bool1 = false;
        TBasePtr bptr2;

        TESTASSERT( bptr2.GetReferenceCount() == 0 );
        TESTASSERT( bptr2.IsNULL() );

        bool1 = bptr2;

        TESTASSERT( !bool1 );

        TDerivedPtr dptr2( d );
        bool1 = dptr2;

        TESTASSERT( dptr2.GetReferenceCount() == 1 );
        TESTASSERT( !dptr2.IsNULL() );
        TESTASSERT( bool1 );
        
        bptr2 = dptr2;
        
        TESTASSERT( bptr2.GetReferenceCount() == 2 );
        TESTASSERT( dptr2.GetReferenceCount() == 2 );
        TESTASSERT( d == bptr2.GetPointerAlways() );
        TESTASSERT( d == dptr2.GetPointerAlways() );
        TESTASSERT( d->GetNr() == dptr2->GetNr() );
        TESTASSERT( !dptr2.IsNULL() );
        TESTASSERT( !bptr2.IsNULL() );
        TESTASSERT( GUCEF_NULL != dptr2.GetSharedData() );
        TESTASSERT( GUCEF_NULL != bptr2.GetSharedData() );
        TESTASSERT( dptr2.GetSharedData() == bptr2.GetSharedData() );
        TESTASSERT( bptr2 == dptr2.StaticCast< Base >() );
        TESTASSERT( !dptr2.GetSharedData()->m_lock.IsLocked() );
        TESTASSERT( !bptr2.GetSharedData()->m_lock.IsLocked() );
        
        d2 = new Derived();
        Base* d2b = static_cast< Base* >( d2 ); 

        bptr2 = TBasePtr( d2 );
        
        TESTASSERT( (bptr2.GetReferenceCount() == 1) );
        TESTASSERT( (dptr2.GetReferenceCount() == 1) );
        TESTASSERT( d2b == bptr2.GetPointerAlways() );
        TESTASSERT( d == dptr2.GetPointerAlways() );
        TESTASSERT( d2->GetNr() == bptr2->GetNr() );
        
        dptr2 = bptr2;

        TESTASSERT( bptr2.GetPointerAlways() == dptr2.GetPointerAlways() );
        TESTASSERT( bptr2.GetReferenceCount() == 2 );
        TESTASSERT( dptr2.GetReferenceCount() == 2 );
        TESTASSERT( d2b == bptr2.GetPointerAlways() );
        TESTASSERT( d2 == dptr2.GetPointerAlways() );

        dptr = dptr2; 

        TESTASSERT( dptr.GetPointerAlways() == dptr2.GetPointerAlways() );
        TESTASSERT( dptr.GetReferenceCount() == 3 );
        TESTASSERT( dptr2.GetReferenceCount() == 3 );
        TESTASSERT( d2 == dptr.GetPointerAlways() );
        TESTASSERT( d2 == dptr2.GetPointerAlways() );
        TESTASSERT( GUCEF_NULL != dptr.GetSharedData() );
        TESTASSERT( GUCEF_NULL != dptr2.GetSharedData() );
        TESTASSERT( dptr.GetSharedData() == dptr2.GetSharedData() );
        TESTASSERT( dptr == dptr2 );
        TESTASSERT( !dptr.GetSharedData()->m_lock.IsLocked() );
        TESTASSERT( !dptr2.GetSharedData()->m_lock.IsLocked() );

        Derived* d3 = new Derived();
        Base* d3b = static_cast< Base* >( d3 );
        Derived* d4 = new Derived();
        Base* d4b = static_cast< Base* >( d4 );

        TDerivedPtr dptr3( d3 );
        TDerivedPtr dptr4( d4 );

        TESTASSERT( dptr3.GetPointerAlways() != dptr4.GetPointerAlways() );
        TESTASSERT( dptr3.GetReferenceCount() == 1 );
        TESTASSERT( dptr4.GetReferenceCount() == 1 );
        TESTASSERT( d3 == dptr3.GetPointerAlways() );
        TESTASSERT( d4 == dptr4.GetPointerAlways() );
        TESTASSERT( GUCEF_NULL != dptr3.GetSharedData() );
        TESTASSERT( GUCEF_NULL != dptr4.GetSharedData() );
        TESTASSERT( dptr3.GetSharedData() != dptr4.GetSharedData() );
        TESTASSERT( dptr3 != dptr4 );
        TESTASSERT( !dptr3.GetSharedData()->m_lock.IsLocked() );
        TESTASSERT( !dptr4.GetSharedData()->m_lock.IsLocked() );

        TBasePtr bptr3 = dptr3;
        TBasePtr bptr4( dptr4 );

        TESTASSERT( dptr3.GetPointerAlways() != dptr4.GetPointerAlways() );
        TESTASSERT( dptr3.GetPointerAlways() == bptr3.GetPointerAlways() );
        TESTASSERT( dptr4.GetPointerAlways() == bptr4.GetPointerAlways() );
        TESTASSERT( bptr3.GetPointerAlways() != bptr4.GetPointerAlways() );
        TESTASSERT( dptr3.GetReferenceCount() == 2 );
        TESTASSERT( bptr3.GetReferenceCount() == 2 );
        TESTASSERT( dptr4.GetReferenceCount() == 2 );
        TESTASSERT( bptr4.GetReferenceCount() == 2 );
        TESTASSERT( d3b == bptr3.GetPointerAlways() );
        TESTASSERT( d3 == bptr3.StaticCast< Derived >().GetPointerAlways() );
        TESTASSERT( d4b == bptr4.GetPointerAlways() );
        TESTASSERT( d4 == bptr4.StaticCast< Derived >().GetPointerAlways() );
        TESTASSERT( GUCEF_NULL != bptr3.GetSharedData() );
        TESTASSERT( GUCEF_NULL != bptr4.GetSharedData() );
        TESTASSERT( bptr3.GetSharedData() != bptr4.GetSharedData() );
        TESTASSERT( bptr3 != bptr4 );
        TESTASSERT( !bptr3.GetSharedData()->m_lock.IsLocked() );
        TESTASSERT( !bptr4.GetSharedData()->m_lock.IsLocked() );

        TBasePtr ibptr3( bptr3 );
        IBase* ib3 = static_cast< IBase* >( d3b );

        TESTASSERT( bptr3.GetReferenceCount() == 3 );
        TESTASSERT( ibptr3.GetReferenceCount() == 3 );
        TESTASSERT( ib3 == ibptr3.GetPointerAlways() );
        TESTASSERT( d3 == ibptr3.StaticCast< Derived >().GetPointerAlways() );
        TESTASSERT( GUCEF_NULL != ibptr3.GetSharedData() );
        TESTASSERT( !ibptr3.GetSharedData()->m_lock.IsLocked() );

        bptr4 = bptr3;
        TESTASSERT( bptr3.GetReferenceCount() == 4 );
        TESTASSERT( ibptr3.GetReferenceCount() == 4 );
        TESTASSERT( dptr4.GetReferenceCount() == 1 );

        TDerivedPtr* dynDptr4 = new TDerivedPtr( dptr4 );
        TESTASSERT( dptr4.GetReferenceCount() == 2 );
        TESTASSERT( GUCEF_NULL != dptr4.GetPointerAlways() );
        TESTASSERT( dynDptr4->GetReferenceCount() == 2 );
        TESTASSERT( GUCEF_NULL != dynDptr4->GetPointerAlways() );

        dptr4 = dptr3;
        
        TESTASSERT( dptr3.GetReferenceCount() == 5 );
        TESTASSERT( dptr4.GetReferenceCount() == 5 );
        TESTASSERT( dynDptr4->GetReferenceCount() == 1 );
        
        IBase* ib4 = static_cast< IBase* >( d4 );
        delete dynDptr4;
        
        TESTASSERT( lastDeleted == ib4 );

        TIBasicBasePtr bb4ptr;
        bb4ptr = dptr4;
        TESTASSERT( bb4ptr.GetReferenceCount() == 6 );
        TESTASSERT( dptr4.GetReferenceCount() == 6 );
        bb4ptr = dptr4;
        TESTASSERT( bb4ptr.GetReferenceCount() == 6 );
        TESTASSERT( dptr4.GetReferenceCount() == 6 );
        TBasicDerivedPtr bd4ptr = bb4ptr.StaticCast< Derived >();
        TESTASSERT( bb4ptr.GetReferenceCount() == 7 );
        TESTASSERT( dptr4.GetReferenceCount() == 7 );
        TESTASSERT( bd4ptr.GetReferenceCount() == 7 );
        TESTASSERT( (void*) bd4ptr.GetDestructor() == (void*) bb4ptr.GetDestructor() );
        TESTASSERT( (void*) bd4ptr.GetDestructor() == (void*) dptr4.GetDestructor() );

    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Counts its destructions, safe to destroy on any thread */
class CReleaseCounted
{
    public:

    static GUCEF::MT::CTAtomic< GUCEF::Int32 > destroyed;

    CReleaseCounted( void ) {}
    virtual ~CReleaseCounted() { destroyed.FetchAdd( 1 ); }
};

GUCEF::MT::CTAtomic< GUCEF::Int32 > CReleaseCounted::destroyed( 0 );

typedef GUCEF::CORE::CTSharedPtr< CReleaseCounted, GUCEF::MT::CMutex > TReleaseCountedPtr;
typedef GUCEF::CORE::CTWeakPtr< CReleaseCounted, GUCEF::MT::CMutex > TReleaseCountedWeakPtr;

/** Shared data (lock included) embedded in the object itself */
class CEmbeddedReleaseCounted : public GUCEF::CORE::CTSharedObjCreator< CEmbeddedReleaseCounted, GUCEF::MT::CMutex >
{
    public:

    static GUCEF::MT::CTAtomic< GUCEF::Int32 > destroyed;

    CEmbeddedReleaseCounted( void ) : GUCEF::CORE::CTSharedObjCreator< CEmbeddedReleaseCounted, GUCEF::MT::CMutex >( this ) {}
    virtual ~CEmbeddedReleaseCounted() { destroyed.FetchAdd( 1 ); }
};

GUCEF::MT::CTAtomic< GUCEF::Int32 > CEmbeddedReleaseCounted::destroyed( 0 );

typedef GUCEF::CORE::CTSharedPtr< CEmbeddedReleaseCounted, GUCEF::MT::CMutex > TEmbeddedReleaseCountedPtr;

/** A reference handed to a thread, released there once told to */
struct SReleasingThread
{
    TReleaseCountedPtr ptr;
    GUCEF::MT::CTAtomic< bool >* go;
};

GUCEF::UInt32 GUCEF_CALLSPEC_PREFIX
ReleaseWhenToldThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SReleasingThread* threadData = static_cast< SReleasingThread* >( data );
    while ( !threadData->go->Load( GUCEF::MT::MEMORDER_ACQUIRE ) )
        GUCEF::MT::ThreadDelay( 1 );
    threadData->ptr.Unlink();
    return 0;
}

GUCEF::UInt32 GUCEF_CALLSPEC_PREFIX
UnlinkUnlessLastWhenToldThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SReleasingThread* threadData = static_cast< SReleasingThread* >( data );
    while ( !threadData->go->Load( GUCEF::MT::MEMORDER_ACQUIRE ) )
        GUCEF::MT::ThreadDelay( 1 );
    threadData->ptr.UnlinkUnlessLast();
    return 0;
}

}

/*-------------------------------------------------------------------------*/

/**
 *  The lock of a shared pointer lives in the shared data, which goes away with the last reference.
 *  None of these releases may destroy that lock while it is still locked: a thread sanitizer reports
 *  "destroy of a locked mutex" for that, and with recursive pthread mutexes on musl (Emscripten) the
 *  freed mutex stays linked in the thread's list of held mutexes, which later lock operations write to.
 */
void
PerformSharedPtrReleaseTests( void )
{
    GUCEF_TESTFW_TESTCASE( "SharedPtr: The last reference destroys the object once and releases its lock before the lock goes" )
    try
    {
        CReleaseCounted::destroyed.Store( 0 );
        {
            TReleaseCountedPtr first( GUCEF_NEW CReleaseCounted() );
            TReleaseCountedPtr second( first );
            TESTASSERT( 2 == first.GetReferenceCount() );
            first.Unlink();
            TESTASSERT( 0 == CReleaseCounted::destroyed.Load() );
        }
        TESTASSERT( 1 == CReleaseCounted::destroyed.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SharedPtr: Shared data kept by a weak pointer is destroyed by the weak pointer, unlocked" )
    try
    {
        CReleaseCounted::destroyed.Store( 0 );
        TReleaseCountedWeakPtr weak;
        {
            TReleaseCountedPtr shared( GUCEF_NEW CReleaseCounted() );
            weak = shared;
        }
        TESTASSERT( 1 == CReleaseCounted::destroyed.Load() );
        TESTASSERT( weak.IsExpired() );
        weak.Unlink();
        TESTASSERT( 1 == CReleaseCounted::destroyed.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SharedPtr: A lock embedded in the object (CTSharedObjCreator) is released before the object goes" )
    try
    {
        CEmbeddedReleaseCounted::destroyed.Store( 0 );
        {
            TEmbeddedReleaseCountedPtr first = CEmbeddedReleaseCounted::CreateSharedObj();
            TEmbeddedReleaseCountedPtr second( first );
            TESTASSERT( 2 == second.GetReferenceCount() );
        }
        TESTASSERT( 1 == CEmbeddedReleaseCounted::destroyed.Load() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SharedPtr: References released at the same time on several threads destroy the object exactly once" )
    try
    {
        static const GUCEF::UInt32 threadCount = 4;
        static const GUCEF::UInt32 rounds = 25;

        CReleaseCounted::destroyed.Store( 0 );
        for ( GUCEF::UInt32 round=0; round<rounds; ++round )
        {
            GUCEF::MT::CTAtomic< bool > go( false );
            SReleasingThread threadData[ threadCount ];
            GUCEF::MT::SThreadData* threads[ threadCount ];

            {
                TReleaseCountedPtr shared( GUCEF_NEW CReleaseCounted() );
                for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
                {
                    threadData[ i ].ptr = shared;
                    threadData[ i ].go = &go;
                    threads[ i ] = GUCEF::MT::ThreadDataReserve();
                    TESTASSERT( GUCEF_NULL != threads[ i ] );
                    if ( GUCEF_NULL != threads[ i ] )
                        TESTASSERT( 0 != GUCEF::MT::ThreadCreate( threads[ i ], &ReleaseWhenToldThreadMain, &threadData[ i ] ) );
                }
                go.Store( true, GUCEF::MT::MEMORDER_RELEASE );
            }

            for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
            {
                if ( GUCEF_NULL != threads[ i ] )
                {
                    GUCEF::MT::ThreadWait( threads[ i ], 60000 );
                    GUCEF::MT::ThreadDataCleanup( threads[ i ] );
                }
            }
            TESTASSERT( round+1 == (GUCEF::UInt32) CReleaseCounted::destroyed.Load() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SharedPtr: UnlinkUnlessLast lets go of a reference others share and keeps the last one" )
    try
    {
        CReleaseCounted::destroyed.Store( 0 );
        {
            TReleaseCountedPtr first( GUCEF_NEW CReleaseCounted() );
            TReleaseCountedPtr second( first );
            TESTASSERT( second.UnlinkUnlessLast() );
            TESTASSERT( second.IsNULL() && 1 == first.GetReferenceCount() );
            TESTASSERT( !first.UnlinkUnlessLast() );
            TESTASSERT( !first.IsNULL() && 1 == first.GetReferenceCount() );
            TESTASSERT( 0 == CReleaseCounted::destroyed.Load() );
            TESTASSERT( second.UnlinkUnlessLast() );
        }
        TESTASSERT( 1 == CReleaseCounted::destroyed.Load() );

        // shared data embedded in the object
        CEmbeddedReleaseCounted::destroyed.Store( 0 );
        {
            TEmbeddedReleaseCountedPtr first = CEmbeddedReleaseCounted::CreateSharedObj();
            TEmbeddedReleaseCountedPtr second( first );
            TESTASSERT( first.UnlinkUnlessLast() && first.IsNULL() );
            TESTASSERT( !second.UnlinkUnlessLast() && !second.IsNULL() );
            TESTASSERT( 0 == CEmbeddedReleaseCounted::destroyed.Load() );
        }
        TESTASSERT( 1 == CEmbeddedReleaseCounted::destroyed.Load() );

        // a weak pointer is no reference that keeps the object
        CReleaseCounted::destroyed.Store( 0 );
        TReleaseCountedWeakPtr weak;
        {
            TReleaseCountedPtr shared( GUCEF_NEW CReleaseCounted() );
            weak = shared;
            TESTASSERT( !shared.UnlinkUnlessLast() );
        }
        TESTASSERT( 1 == CReleaseCounted::destroyed.Load() && weak.IsExpired() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SharedPtr: UnlinkUnlessLast on several threads at the same time: exactly one of them keeps the last reference" )
    try
    {
        static const GUCEF::UInt32 threadCount = 4;
        static const GUCEF::UInt32 rounds = 25;

        CReleaseCounted::destroyed.Store( 0 );
        for ( GUCEF::UInt32 round=0; round<rounds; ++round )
        {
            GUCEF::MT::CTAtomic< bool > go( false );
            SReleasingThread threadData[ threadCount ];
            GUCEF::MT::SThreadData* threads[ threadCount ];

            {
                TReleaseCountedPtr shared( GUCEF_NEW CReleaseCounted() );
                for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
                {
                    threadData[ i ].ptr = shared;
                    threadData[ i ].go = &go;
                    threads[ i ] = GUCEF::MT::ThreadDataReserve();
                    TESTASSERT( GUCEF_NULL != threads[ i ] );
                    if ( GUCEF_NULL != threads[ i ] )
                        TESTASSERT( 0 != GUCEF::MT::ThreadCreate( threads[ i ], &UnlinkUnlessLastWhenToldThreadMain, &threadData[ i ] ) );
                }
                go.Store( true, GUCEF::MT::MEMORDER_RELEASE );
                TESTASSERT( shared.UnlinkUnlessLast() || 1 == shared.GetReferenceCount() );
            }

            GUCEF::UInt32 kept = 0;
            for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
            {
                if ( GUCEF_NULL != threads[ i ] )
                {
                    GUCEF::MT::ThreadWait( threads[ i ], 60000 );
                    GUCEF::MT::ThreadDataCleanup( threads[ i ] );
                }
                if ( !threadData[ i ].ptr.IsNULL() )
                    ++kept;
            }

            // the test thread's reference went with its scope: it was the last one or one of the threads kept it
            TESTASSERT( kept <= 1 );
            TESTASSERT( (GUCEF::UInt32) CReleaseCounted::destroyed.Load() == round + ( 0 == kept ? 1 : 0 ) );
            for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
                threadData[ i ].ptr.Unlink();
            TESTASSERT( round+1 == (GUCEF::UInt32) CReleaseCounted::destroyed.Load() );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

namespace {

/** One shared pointer object that a thread reassigns while other threads copy it */
struct SCopyRace
{
    TReleaseCountedPtr target;
    TReleaseCountedPtr first;
    TReleaseCountedPtr second;
    GUCEF::MT::CTAtomic< bool > stop;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > assignments;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > copies;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > wrongCopies;
    GUCEF::MT::CTAtomic< GUCEF::UInt32 > timeouts;

    SCopyRace( void )
        : target()
        , first()
        , second()
        , stop( false )
        , assignments( 0 )
        , copies( 0 )
        , wrongCopies( 0 )
        , timeouts( 0 )
    {}

    bool IsOneOfTheValues( const TReleaseCountedPtr& copy ) const
    {
        return copy.IsNULL() || copy == first || copy == second;
    }
};

GUCEF::UInt32 GUCEF_CALLSPEC_PREFIX
ReassignUntilStoppedThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SCopyRace* race = static_cast< SCopyRace* >( data );
    while ( !race->stop.Load( GUCEF::MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            race->target = race->first;
            race->target.Unlink();
            race->target = race->second;
            race->target = TReleaseCountedPtr();
            race->assignments.FetchAdd( 1 );
        }
        catch ( const GUCEF::timeout_exception& )
        {
            race->timeouts.FetchAdd( 1 );
        }
    }
    return 0;
}

GUCEF::UInt32 GUCEF_CALLSPEC_PREFIX
CopyUntilStoppedThreadMain( void* data ) GUCEF_CALLSPEC_SUFFIX
{
    SCopyRace* race = static_cast< SCopyRace* >( data );
    while ( !race->stop.Load( GUCEF::MT::MEMORDER_ACQUIRE ) )
    {
        try
        {
            TReleaseCountedPtr copy( race->target );
            TReleaseCountedPtr assigned;
            assigned = race->target;
            if ( !race->IsOneOfTheValues( copy ) || !race->IsOneOfTheValues( assigned ) )
                race->wrongCopies.FetchAdd( 1 );
            race->copies.FetchAdd( 1 );
        }
        catch ( const GUCEF::timeout_exception& )
        {
            race->timeouts.FetchAdd( 1 );
        }
    }
    return 0;
}

}

/*-------------------------------------------------------------------------*/

/**
 *  Code shares a shared pointer object between threads and relies on a copy of it being safe while another thread
 *  assigns to it (a task's consumer, a delegator's task). A copy that locks the shared data the object points at and
 *  unlocks through the object again unlocks whatever the object points at by then: the lock it took stays locked, and
 *  the next thread that takes that lock waits for good (a recursive mutex: the thread that left it locked does not
 *  notice; on Linux the lock waits of CMutex do not time out). The test never takes such a lock itself: when the
 *  reassigning thread stops getting anywhere the test fails and leaves the threads behind.
 */
void
PerformSharedPtrConcurrentCopyTests( void )
{
    GUCEF_TESTFW_TESTCASE( "SharedPtr: Copies of a shared pointer another thread reassigns at the same time hold a reference or none, and leave no lock locked" )
    try
    {
        static const GUCEF::UInt32 copierCount = 2;
        static const GUCEF::UInt32 threadCount = copierCount + 1;
        static const GUCEF::UInt32 wantedAssignments = 200000;

        // on the heap: left behind with the threads when a lock was left locked
        SCopyRace* race = GUCEF_NEW SCopyRace();
        race->first = TReleaseCountedPtr( GUCEF_NEW CReleaseCounted() );
        race->second = TReleaseCountedPtr( GUCEF_NEW CReleaseCounted() );

        GUCEF::MT::SThreadData* threads[ threadCount ];
        for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
        {
            threads[ i ] = GUCEF::MT::ThreadDataReserve();
            TESTASSERT( GUCEF_NULL != threads[ i ] );
            if ( GUCEF_NULL != threads[ i ] )
                TESTASSERT( 0 != GUCEF::MT::ThreadCreate( threads[ i ], 0 == i ? &ReassignUntilStoppedThreadMain : &CopyUntilStoppedThreadMain, race ) );
        }

        // until enough rounds were done, at most 6 seconds, or until the reassigning thread does not get anywhere for
        // 2 seconds: it waits for a lock that was left locked
        bool stalled = false;
        GUCEF::UInt32 lastAssignments = 0;
        GUCEF::UInt32 unchanged = 0;
        for ( GUCEF::UInt32 i=0; i<600 && race->assignments.Load() < wantedAssignments; ++i )
        {
            GUCEF::MT::ThreadDelay( 10 );
            const GUCEF::UInt32 assignments = race->assignments.Load();
            if ( assignments == lastAssignments )
            {
                if ( ++unchanged >= 200 )
                {
                    stalled = true;
                    break;
                }
            }
            else
            {
                unchanged = 0;
                lastAssignments = assignments;
            }
        }

        race->stop.Store( true, GUCEF::MT::MEMORDER_RELEASE );
        bool allEnded = true;
        for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
        {
            if ( GUCEF_NULL != threads[ i ] && GUCEF_THREAD_WAIT_OK != GUCEF::MT::ThreadWait( threads[ i ], 5000 ) )
                allEnded = false;
        }

        GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "SharedPtr concurrent copies: " << race->assignments.Load() << " rounds of assignments, "
                   << race->copies.Load() << " rounds of copies, " << race->wrongCopies.Load() << " wrong copies, " << race->timeouts.Load()
                   << " lock timeouts, stalled: " << ( stalled ? "yes" : "no" ) << ", all threads ended: " << ( allEnded ? "yes" : "no" ) );

        TESTASSERT( !stalled );
        TESTASSERT( allEnded );
        TESTASSERT( 0 == race->timeouts.Load() );
        TESTASSERT( 0 == race->wrongCopies.Load() );
        TESTASSERT( race->copies.Load() > 0 );

        if ( allEnded )
        {
            for ( GUCEF::UInt32 i=0; i<threadCount; ++i )
                if ( GUCEF_NULL != threads[ i ] )
                    GUCEF::MT::ThreadDataCleanup( threads[ i ] );

            // every reference a copy took was given back, and no copy took one it did not count
            race->target.Unlink();
            TESTASSERT( 1 == race->first.GetReferenceCount() && 1 == race->second.GetReferenceCount() );
            GUCEF_DELETE race;
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

void
PerformSharedPtrTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "COMMENCING SHARED PTR TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "SharedPtr" );

    PerformBasicSharedPtrSanityTests();
    PerformSharedPtrReleaseTests();
    PerformSharedPtrConcurrentCopyTests();

    GUCEF::CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "ALL SHARED PTR TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/

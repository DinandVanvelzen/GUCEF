/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#include "CTAbstractFactoryWithParam.h"
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORYWITHPARAM_H
#include "CTFactoryWithParam.h"
#define GUCEF_CORE_CTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#include "TestAbstractFactory.h"
#define GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Base interface for testing CTAbstractFactory.
 *  Represents a generic shape with a type name query.
 */
class ITestShape
{
    public:
    virtual ~ITestShape() {}
    virtual CString GetShapeType( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: circle (no-param construction)
 */
class CTestCircle : public ITestShape
{
    public:
    CTestCircle() {}
    virtual ~CTestCircle() {}
    virtual CString GetShapeType( void ) const { return "Circle"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: square (no-param construction)
 */
class CTestSquare : public ITestShape
{
    public:
    CTestSquare() {}
    virtual ~CTestSquare() {}
    virtual CString GetShapeType( void ) const { return "Square"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: triangle (no-param construction)
 */
class CTestTriangle : public ITestShape
{
    public:
    CTestTriangle() {}
    virtual ~CTestTriangle() {}
    virtual CString GetShapeType( void ) const { return "Triangle"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Base interface for testing CTAbstractFactoryWithParam.
 *  Represents a colored shape where the color is passed at construction time.
 */
class ITestColoredShape
{
    public:
    virtual ~ITestColoredShape() {}
    virtual CString GetShapeType( void ) const = 0;
    virtual CString GetColor( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored circle (param = color string)
 */
class CTestColoredCircle : public ITestColoredShape
{
    public:
    explicit CTestColoredCircle( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredCircle() {}
    virtual CString GetShapeType( void ) const { return "ColoredCircle"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored triangle (param = color string)
 */
class CTestColoredTriangle : public ITestColoredShape
{
    public:
    explicit CTestColoredTriangle( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredTriangle() {}
    virtual CString GetShapeType( void ) const { return "ColoredTriangle"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored square (param = color string)
 */
class CTestColoredSquare : public ITestColoredShape
{
    public:
    explicit CTestColoredSquare( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredSquare() {}
    virtual CString GetShapeType( void ) const { return "ColoredSquare"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  The abstract factories that the products and concrete factories below call into
 */
typedef CTAbstractFactory< CString, ITestShape, MT::CMutex >                            TTestShapeAbstractFactory;
typedef CTAbstractFactoryWithParam< CString, ITestColoredShape, CString, MT::CMutex >    TTestColoredShapeAbstractFactory;

/*-------------------------------------------------------------------------*/

/*
 *  A gate that a product, or a concrete factory, waits at while the test keeps it shut. A thread waits at most 10 s, so
 *  a test that fails leaves no thread behind.
 */
class CTestGate
{
    public:

    CTestGate( void ) : m_open( 1 ), m_waiting( 0 ) {}

    void Close( void ) { m_open.Store( 0 ); }
    void Open( void ) { m_open.Store( 1 ); }

    /** Waits while the gate is shut; first takes and lets go of the given lock, when there is one, as one that waits */
    void Pass( MT::CMutex* lockToTakeFirst = GUCEF_NULL )
    {
        m_waiting.FetchAdd( 1 );
        if ( GUCEF_NULL != lockToTakeFirst )
        {
            MT::CScopeMutex lock( *lockToTakeFirst );
        }
        for ( UInt32 i=0; i<10000 && 0 == m_open.Load(); ++i )
            MT::ThreadDelay( 1 );
        m_waiting.FetchSub( 1 );
    }

    /** Waits until the given number of threads wait at the gate; false after the time */
    bool WaitForWaiting( UInt32 count, UInt32 timeoutInMs = 10000 ) const
    {
        for ( UInt32 i=0; i<timeoutInMs && m_waiting.Load() < count; ++i )
            MT::ThreadDelay( 1 );
        return m_waiting.Load() >= count;
    }

    private:

    MT::CTAtomic< UInt32 > m_open;
    MT::CTAtomic< UInt32 > m_waiting;
};

/*
 *  What the gated shapes wait for while they are being constructed: a lock another thread holds (as the notification
 *  lock of a pulse generator that a product subscribes to in its constructor), or a gate that the test opens
 */
static MT::CMutex g_gatedShapeLock;
static MT::CTAtomic< UInt32 > g_gatedShapeTakesLock( 0 );
static CTestGate g_shapeGate;                               /**< the gated shapes wait here */
static CTestGate g_secondShapeGate;                         /**< the second gated shapes wait here */
static CTestGate g_factoryNameGate;                         /**< the slow name factories wait here when it is armed */
static MT::CTAtomic< UInt32 > g_factoryNameGateArmed( 0 );

/** Opens every gate, after a test or a test that failed */
static void
ResetTestGates( void )
{
    g_gatedShapeTakesLock.Store( 0 );
    g_factoryNameGateArmed.Store( 0 );
    g_shapeGate.Open();
    g_secondShapeGate.Open();
    g_factoryNameGate.Open();
}

static void
WaitAsGatedShape( void )
{
    g_shapeGate.Pass( 0 != g_gatedShapeTakesLock.Load() ? &g_gatedShapeLock : GUCEF_NULL );
}

/*
 *  Concrete shape whose construction waits at the shape gate, see WaitAsGatedShape()
 */
class CTestGatedShape : public ITestShape
{
    public:
    CTestGatedShape() { WaitAsGatedShape(); }
    virtual ~CTestGatedShape() {}
    virtual CString GetShapeType( void ) const { return "Gated"; }
};

/*
 *  Concrete colored shape whose construction waits at the shape gate, see WaitAsGatedShape()
 */
class CTestGatedColoredShape : public ITestColoredShape
{
    public:
    explicit CTestGatedColoredShape( const CString& color ) : m_color( color ) { WaitAsGatedShape(); }
    virtual ~CTestGatedColoredShape() {}
    virtual CString GetShapeType( void ) const { return "Gated"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*
 *  Concrete shape whose construction waits at the second shape gate
 */
class CTestSecondGatedShape : public ITestShape
{
    public:
    CTestSecondGatedShape() { g_secondShapeGate.Pass(); }
    virtual ~CTestSecondGatedShape() {}
    virtual CString GetShapeType( void ) const { return "SecondGated"; }
};

/*
 *  Concrete colored shape whose construction waits at the second shape gate
 */
class CTestSecondGatedColoredShape : public ITestColoredShape
{
    public:
    explicit CTestSecondGatedColoredShape( const CString& color ) : m_color( color ) { g_secondShapeGate.Pass(); }
    virtual ~CTestSecondGatedColoredShape() {}
    virtual CString GetShapeType( void ) const { return "SecondGated"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  What the constructor of a throwing shape throws: not a std::exception, which the concrete factories catch, so it
 *  leaves the create of the abstract factory
 */
struct STestShapeConstructionFailure {};

/*
 *  Concrete shape whose construction fails with an exception
 */
class CTestThrowingShape : public ITestShape
{
    public:
    CTestThrowingShape() { throw STestShapeConstructionFailure(); }
    virtual ~CTestThrowingShape() {}
    virtual CString GetShapeType( void ) const { return "Throwing"; }
};

/*
 *  Concrete colored shape whose construction fails with an exception
 */
class CTestThrowingColoredShape : public ITestColoredShape
{
    public:
    explicit CTestThrowingColoredShape( const CString& ) { throw STestShapeConstructionFailure(); }
    virtual ~CTestThrowingColoredShape() {}
    virtual CString GetShapeType( void ) const { return "Throwing"; }
    virtual CString GetColor( void ) const { return CString::Empty; }
};

/*-------------------------------------------------------------------------*/

/*
 *  The abstract factories that the self unregistering shapes remove their concrete factory from
 */
static TTestShapeAbstractFactory* g_selfUnregisteringShapeFactory = GUCEF_NULL;
static TTestColoredShapeAbstractFactory* g_selfUnregisteringColoredShapeFactory = GUCEF_NULL;

/*
 *  Concrete shape that removes its own concrete factory while it is constructed
 */
class CTestSelfUnregisteringShape : public ITestShape
{
    public:
    CTestSelfUnregisteringShape()
    {
        if ( GUCEF_NULL != g_selfUnregisteringShapeFactory )
            g_selfUnregisteringShapeFactory->UnregisterConcreteFactory( "SelfUnregistering" );
    }
    virtual ~CTestSelfUnregisteringShape() {}
    virtual CString GetShapeType( void ) const { return "SelfUnregistering"; }
};

/*
 *  Concrete colored shape that removes its own concrete factory while it is constructed
 */
class CTestSelfUnregisteringColoredShape : public ITestColoredShape
{
    public:
    explicit CTestSelfUnregisteringColoredShape( const CString& color ) : m_color( color )
    {
        if ( GUCEF_NULL != g_selfUnregisteringColoredShapeFactory )
            g_selfUnregisteringColoredShapeFactory->UnregisterConcreteFactory( "SelfUnregistering" );
    }
    virtual ~CTestSelfUnregisteringColoredShape() {}
    virtual CString GetShapeType( void ) const { return "SelfUnregistering"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  The abstract factories that the self replacing shapes replace their concrete factory in, and the concrete factories
 *  they register in its place
 */
static TTestShapeAbstractFactory* g_selfReplacingShapeFactory = GUCEF_NULL;
static TTestColoredShapeAbstractFactory* g_selfReplacingColoredShapeFactory = GUCEF_NULL;
static TTestShapeAbstractFactory::TFactory* g_shapeReplacement = GUCEF_NULL;
static TTestColoredShapeAbstractFactory::TFactory* g_coloredShapeReplacement = GUCEF_NULL;

/*
 *  Concrete shape that replaces its own concrete factory while it is constructed
 */
class CTestSelfReplacingShape : public ITestShape
{
    public:
    CTestSelfReplacingShape()
    {
        if ( GUCEF_NULL != g_selfReplacingShapeFactory && GUCEF_NULL != g_shapeReplacement )
            g_selfReplacingShapeFactory->RegisterConcreteFactory( "SelfReplacing", g_shapeReplacement );
    }
    virtual ~CTestSelfReplacingShape() {}
    virtual CString GetShapeType( void ) const { return "SelfReplacing"; }
};

/*
 *  Concrete colored shape that replaces its own concrete factory while it is constructed
 */
class CTestSelfReplacingColoredShape : public ITestColoredShape
{
    public:
    explicit CTestSelfReplacingColoredShape( const CString& color ) : m_color( color )
    {
        if ( GUCEF_NULL != g_selfReplacingColoredShapeFactory && GUCEF_NULL != g_coloredShapeReplacement )
            g_selfReplacingColoredShapeFactory->RegisterConcreteFactory( "SelfReplacing", g_coloredShapeReplacement );
    }
    virtual ~CTestSelfReplacingColoredShape() {}
    virtual CString GetShapeType( void ) const { return "SelfReplacing"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  The abstract factories of the cycle shapes: shape A unregisters the type of shape B in its constructor and the other
 *  way around, once both are being constructed (each waits at most 10 s for the other)
 */
static TTestShapeAbstractFactory* g_cycleShapeFactory = GUCEF_NULL;
static TTestColoredShapeAbstractFactory* g_cycleColoredShapeFactory = GUCEF_NULL;
static MT::CTAtomic< UInt32 > g_cycleShapesInConstruction( 0 );

static void
EnterCycleShapeConstruction( void )
{
    g_cycleShapesInConstruction.FetchAdd( 1 );
    for ( UInt32 i=0; i<10000 && g_cycleShapesInConstruction.Load() < 2; ++i )
        MT::ThreadDelay( 1 );
}

class CTestCycleShapeA : public ITestShape
{
    public:
    CTestCycleShapeA()
    {
        EnterCycleShapeConstruction();
        if ( GUCEF_NULL != g_cycleShapeFactory )
            g_cycleShapeFactory->UnregisterConcreteFactory( "CycleB" );
    }
    virtual ~CTestCycleShapeA() {}
    virtual CString GetShapeType( void ) const { return "CycleA"; }
};

class CTestCycleShapeB : public ITestShape
{
    public:
    CTestCycleShapeB()
    {
        EnterCycleShapeConstruction();
        if ( GUCEF_NULL != g_cycleShapeFactory )
            g_cycleShapeFactory->UnregisterConcreteFactory( "CycleA" );
    }
    virtual ~CTestCycleShapeB() {}
    virtual CString GetShapeType( void ) const { return "CycleB"; }
};

class CTestCycleColoredShapeA : public ITestColoredShape
{
    public:
    explicit CTestCycleColoredShapeA( const CString& color ) : m_color( color )
    {
        EnterCycleShapeConstruction();
        if ( GUCEF_NULL != g_cycleColoredShapeFactory )
            g_cycleColoredShapeFactory->UnregisterConcreteFactory( "CycleB" );
    }
    virtual ~CTestCycleColoredShapeA() {}
    virtual CString GetShapeType( void ) const { return "CycleA"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

class CTestCycleColoredShapeB : public ITestColoredShape
{
    public:
    explicit CTestCycleColoredShapeB( const CString& color ) : m_color( color )
    {
        EnterCycleShapeConstruction();
        if ( GUCEF_NULL != g_cycleColoredShapeFactory )
            g_cycleColoredShapeFactory->UnregisterConcreteFactory( "CycleA" );
    }
    virtual ~CTestCycleColoredShapeB() {}
    virtual CString GetShapeType( void ) const { return "CycleB"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete factory that gives its class type name slowly while the name gate is armed: the abstract factory asks for
 *  it under its lock when the concrete factory is registered, so that registration holds the lock until the test opens
 *  the gate
 */
template< class TBaseFactory >
class CTSlowNameFactory : public TBaseFactory
{
    public:
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != g_factoryNameGateArmed.Load() )
            g_factoryNameGate.Pass();
        return TBaseFactory::GetConcreteClassTypeName();
    }
};

/*
 *  Concrete factories that count their destruction
 */
static MT::CTAtomic< UInt32 > g_countedFactoriesDestroyed( 0 );

template< class TBaseFactory >
class CTCountedFactory : public TBaseFactory
{
    public:
    virtual ~CTCountedFactory() { g_countedFactoriesDestroyed.FetchAdd( 1 ); }
};

/*
 *  The number of counted factories destroyed when a self unregistering factory's removal returned within its create
 */
static MT::CTAtomic< UInt32 > g_destroyedDuringOwnCreate( 0 );

/*
 *  Concrete factory, owned by the abstract factory, that removes itself in its own create and makes no product
 */
class CTestSelfUnregisteringShapeFactory : public CTFactoryBase< ITestShape, MT::CMutex >
{
    public:
    explicit CTestSelfUnregisteringShapeFactory( TTestShapeAbstractFactory& abstractFactory ) : m_abstractFactory( abstractFactory ) {}
    virtual ~CTestSelfUnregisteringShapeFactory() { g_countedFactoriesDestroyed.FetchAdd( 1 ); }
    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_abstractFactory.UnregisterConcreteFactory( "SelfUnregisteringFactory" );
        g_destroyedDuringOwnCreate.Store( g_countedFactoriesDestroyed.Load() );
        return TProductPtr();
    }
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "CTestSelfUnregisteringShapeFactory"; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestSelfUnregisteringShapeFactory( m_abstractFactory ); }
    private:
    TTestShapeAbstractFactory& m_abstractFactory;
};

/*
 *  The parameter form of CTestSelfUnregisteringShapeFactory
 */
class CTestSelfUnregisteringColoredShapeFactory : public CTFactoryBaseWithParam< ITestColoredShape, CString, MT::CMutex >
{
    public:
    explicit CTestSelfUnregisteringColoredShapeFactory( TTestColoredShapeAbstractFactory& abstractFactory ) : m_abstractFactory( abstractFactory ) {}
    virtual ~CTestSelfUnregisteringColoredShapeFactory() { g_countedFactoriesDestroyed.FetchAdd( 1 ); }
    virtual TProductPtr Create( const CString& ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_abstractFactory.UnregisterConcreteFactory( "SelfUnregisteringFactory" );
        g_destroyedDuringOwnCreate.Store( g_countedFactoriesDestroyed.Load() );
        return TProductPtr();
    }
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "CTestSelfUnregisteringColoredShapeFactory"; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestSelfUnregisteringColoredShapeFactory( m_abstractFactory ); }
    private:
    TTestColoredShapeAbstractFactory& m_abstractFactory;
};

/*-------------------------------------------------------------------------*/

/*
 *  Keeps the warnings that reach it, for the test of the warnings of a removal that waits long
 */
class CTestWarningCaptureLogger : public CILogger
{
    public:

    CTestWarningCaptureLogger( void ) : CILogger(), m_lock(), m_warnings() {}

    virtual void Log( const TLogMsgType logMsgType, const Int32, const CString& logMessage, const TSystemThreadId, const CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    { Keep( logMsgType, logMessage ); }

    virtual void Log( const TLogMsgType logMsgType, const Int32, const CVariantStream& logMessage, const TSystemThreadId, const CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    { Keep( logMsgType, logMessage.ToString() ); }

    virtual void LogWithoutFormatting( const TLogMsgType logMsgType, const Int32, const CString& logMessage, const TSystemThreadId, const CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    { Keep( logMsgType, logMessage ); }

    virtual void LogWithoutFormatting( const TLogMsgType logMsgType, const Int32, const CVariantStream& logMessage, const TSystemThreadId, const CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    { Keep( logMsgType, logMessage.ToString() ); }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return LOGLEVEL_EVERYTHING; }

    /** Whether a warning holds both texts */
    bool HasWarningWith( const CString& text, const CString& otherText )
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_warnings.size(); ++i )
        {
            if ( 0 <= m_warnings[ i ].HasSubstr( text ) && 0 <= m_warnings[ i ].HasSubstr( otherText ) )
                return true;
        }
        return false;
    }

    private:

    void Keep( const TLogMsgType logMsgType, const CString& logMessage )
    {
        if ( LOG_WARNING == logMsgType )
        {
            MT::CScopeMutex lock( m_lock );
            m_warnings.push_back( logMessage );
        }
    }

    MT::CMutex m_lock;
    GUCEF::vector< CString > m_warnings;
};

/*-------------------------------------------------------------------------*/

/*
 *  Records the identity of its thread, twice, and stays alive until it is told to end
 */
class CTestThreadIdentityThread : public MT::CActiveObject
{
    public:

    CTestThreadIdentityThread( void ) : MT::CActiveObject(), m_identity(), m_secondIdentity(), m_recorded( 0 ), m_end( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_identity = GetAbstractFactoryThreadId();
        m_secondIdentity = GetAbstractFactoryThreadId();
        m_recorded.Store( 1 );
        for ( UInt32 i=0; i<10000 && 0 == m_end.Load(); ++i )
            MT::ThreadDelay( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilRecorded( void ) const
    {
        for ( UInt32 i=0; i<10000 && 0 == m_recorded.Load(); ++i )
            MT::ThreadDelay( 1 );
        return 0 != m_recorded.Load();
    }

    void End( void ) { m_end.Store( 1 ); }

    TAbstractFactoryThreadId m_identity;         /**< written before m_recorded is set, read after it is */
    TAbstractFactoryThreadId m_secondIdentity;

    private:

    MT::CTAtomic< UInt32 > m_recorded;
    MT::CTAtomic< UInt32 > m_end;
};

/*-------------------------------------------------------------------------*/

/*
 *  Creates that used a stress factory after its removal returned
 */
static MT::CTAtomic< UInt32 > g_stressUsedAfterRemoval( 0 );

/*
 *  Concrete factory of the race between creates and removals: it makes no product, so no product outlives it, and it
 *  counts every create that still uses it once its removal returned
 */
class CTestStressShapeFactory : public CTFactoryBase< ITestShape, MT::CMutex >
{
    public:
    CTestStressShapeFactory( void ) : m_removed( 0 ) {}
    void MarkRemoved( void ) { m_removed.Store( 1 ); }
    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != m_removed.Load() )
            g_stressUsedAfterRemoval.FetchAdd( 1 );
        return TProductPtr();
    }
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "CTestStressShapeFactory"; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestStressShapeFactory(); }
    private:
    MT::CTAtomic< UInt32 > m_removed;
};

/*
 *  The parameter form of CTestStressShapeFactory
 */
class CTestStressColoredShapeFactory : public CTFactoryBaseWithParam< ITestColoredShape, CString, MT::CMutex >
{
    public:
    CTestStressColoredShapeFactory( void ) : m_removed( 0 ) {}
    void MarkRemoved( void ) { m_removed.Store( 1 ); }
    virtual TProductPtr Create( const CString& ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != m_removed.Load() )
            g_stressUsedAfterRemoval.FetchAdd( 1 );
        return TProductPtr();
    }
    virtual CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "CTestStressColoredShapeFactory"; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestStressColoredShapeFactory(); }
    private:
    MT::CTAtomic< UInt32 > m_removed;
};

/*-------------------------------------------------------------------------*/

/*
 *  A create through either form of the abstract factory; the parameter form makes "Gray" products
 */
template< class TAbstractFactory >
struct TTestCreateThrough
{
    static typename TAbstractFactory::TProductPtr Create( TAbstractFactory& factory, const CString& type ) { return factory.Create( type ); }
};

template< typename SelectionCriteriaType, class BaseClassType, typename ConstructionParamType, class LockType >
struct TTestCreateThrough< CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType > >
{
    typedef CTAbstractFactoryWithParam< SelectionCriteriaType, BaseClassType, ConstructionParamType, LockType > TAbstractFactory;
    static typename TAbstractFactory::TProductPtr Create( TAbstractFactory& factory, const CString& type ) { return factory.Create( type, "Gray" ); }
};

/*
 *  What a factory call thread does
 */
enum ETestFactoryCall
{
    TESTFACTORYCALL_CREATE ,            /**< creates a product of the type */
    TESTFACTORYCALL_REGISTER ,          /**< registers the given concrete factory for the type */
    TESTFACTORYCALL_UNREGISTER ,        /**< unregisters the concrete factory of the type */
    TESTFACTORYCALL_UNREGISTER_ALL ,    /**< unregisters every concrete factory */
    TESTFACTORYCALL_DESTROY             /**< deletes the abstract factory, which was made with GUCEF_NEW */
};

/*
 *  Calls an abstract factory once, on a thread of its own
 */
template< class TAbstractFactory >
class CTFactoryCallThread : public MT::CActiveObject
{
    public:

    typedef typename TAbstractFactory::TProductPtr TProductPtr;
    typedef typename TAbstractFactory::TFactory TFactory;

    CTFactoryCallThread( TAbstractFactory& factory, const CString& type, ETestFactoryCall call, TFactory* concreteFactory = GUCEF_NULL )
        : MT::CActiveObject(), product(), m_factory( factory ), m_type( type ), m_call( call ), m_concreteFactory( concreteFactory ), m_done( 0 ), m_failed( 0 )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            switch ( m_call )
            {
                case TESTFACTORYCALL_CREATE: { product = TTestCreateThrough< TAbstractFactory >::Create( m_factory, m_type ); break; }
                case TESTFACTORYCALL_REGISTER: { m_factory.RegisterConcreteFactory( m_type, m_concreteFactory ); break; }
                case TESTFACTORYCALL_UNREGISTER: { m_factory.UnregisterConcreteFactory( m_type ); break; }
                case TESTFACTORYCALL_UNREGISTER_ALL: { m_factory.UnregisterAllConcreteFactories(); break; }
                case TESTFACTORYCALL_DESTROY: { GUCEF_DELETE &m_factory; break; }
                default: { m_failed.Store( 1 ); break; }
            }
        }
        catch ( ... )
        {
            m_failed.Store( 1 );
        }
        m_done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return 0 != m_done.Load(); }
    bool HasFailed( void ) const { return 0 != m_failed.Load(); }

    /** Waits for the call to return; false after the time */
    bool WaitUntilDone( UInt32 timeoutInMs ) const
    {
        for ( UInt32 i=0; i<timeoutInMs && !IsDone(); ++i )
            MT::ThreadDelay( 1 );
        return IsDone();
    }

    TProductPtr product;

    private:

    TAbstractFactory& m_factory;
    CString m_type;
    ETestFactoryCall m_call;
    TFactory* m_concreteFactory;
    MT::CTAtomic< UInt32 > m_done;
    MT::CTAtomic< UInt32 > m_failed;
};

/*
 *  Creates products of the type "Stress" through an abstract factory, over and over, until it is told to stop
 */
template< class TAbstractFactory >
class CTStressCreatorThread : public MT::CActiveObject
{
    public:

    explicit CTStressCreatorThread( TAbstractFactory& factory )
        : MT::CActiveObject(), m_factory( factory ), m_stop( 0 ), m_creates( 0 ), m_failed( 0 )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        try
        {
            while ( 0 == m_stop.Load() )
            {
                TTestCreateThrough< TAbstractFactory >::Create( m_factory, "Stress" );
                m_creates.FetchAdd( 1 );
            }
        }
        catch ( ... )
        {
            m_failed.Store( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    void Stop( void ) { m_stop.Store( 1 ); }
    UInt32 GetCreates( void ) const { return m_creates.Load(); }
    bool HasFailed( void ) const { return 0 != m_failed.Load(); }

    private:

    TAbstractFactory& m_factory;
    MT::CTAtomic< UInt32 > m_stop;
    MT::CTAtomic< UInt32 > m_creates;
    MT::CTAtomic< UInt32 > m_failed;
};

/*-------------------------------------------------------------------------*/

/*
 *  The types of the threading tests of the plain form of the abstract factory
 */
struct STestPlainForm
{
    typedef TTestShapeAbstractFactory                                                       TAbstractFactory;
    typedef CTFactory< ITestShape, CTestGatedShape, MT::CMutex >                            TGatedFactory;
    typedef CTFactory< ITestShape, CTestSecondGatedShape, MT::CMutex >                      TSecondGatedFactory;
    typedef CTFactory< ITestShape, CTestCircle, MT::CMutex >                                TCircleFactory;
    typedef CTSlowNameFactory< CTFactory< ITestShape, CTestSquare, MT::CMutex > >           TSlowNameFactory;
    typedef CTCountedFactory< CTFactory< ITestShape, CTestThrowingShape, MT::CMutex > >     TThrowingFactory;
    typedef CTFactory< ITestShape, CTestSelfUnregisteringShape, MT::CMutex >                TSelfUnregisteringProductFactory;
    typedef CTestSelfUnregisteringShapeFactory                                              TSelfUnregisteringFactory;
    typedef CTestStressShapeFactory                                                         TStressFactory;
    typedef CTCountedFactory< CTFactory< ITestShape, CTestSelfUnregisteringShape, MT::CMutex > > TCountedSelfUnregisteringProductFactory;
    typedef CTFactory< ITestShape, CTestSelfReplacingShape, MT::CMutex >                    TSelfReplacingProductFactory;
    typedef CTCountedFactory< CTFactory< ITestShape, CTestCircle, MT::CMutex > >            TCountedCircleFactory;
    typedef CTCountedFactory< CTFactory< ITestShape, CTestCycleShapeA, MT::CMutex > >       TCycleAFactory;
    typedef CTCountedFactory< CTFactory< ITestShape, CTestCycleShapeB, MT::CMutex > >       TCycleBFactory;

    static TAbstractFactory*& SelfUnregisteringTarget( void ) { return g_selfUnregisteringShapeFactory; }
    static TAbstractFactory*& SelfReplacingTarget( void ) { return g_selfReplacingShapeFactory; }
    static TAbstractFactory::TFactory*& SelfReplacement( void ) { return g_shapeReplacement; }
    static TAbstractFactory*& CycleTarget( void ) { return g_cycleShapeFactory; }
};

/*
 *  The types of the threading tests of the parameter form of the abstract factory
 */
struct STestParamForm
{
    typedef TTestColoredShapeAbstractFactory                                                                    TAbstractFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestGatedColoredShape, CString, MT::CMutex >                TGatedFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestSecondGatedColoredShape, CString, MT::CMutex >          TSecondGatedFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex >                    TCircleFactory;
    typedef CTSlowNameFactory< CTFactoryWithParam< ITestColoredShape, CTestColoredSquare, CString, MT::CMutex > > TSlowNameFactory;
    typedef CTCountedFactory< CTFactoryWithParam< ITestColoredShape, CTestThrowingColoredShape, CString, MT::CMutex > > TThrowingFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestSelfUnregisteringColoredShape, CString, MT::CMutex >    TSelfUnregisteringProductFactory;
    typedef CTestSelfUnregisteringColoredShapeFactory                                                          TSelfUnregisteringFactory;
    typedef CTestStressColoredShapeFactory                                                                     TStressFactory;
    typedef CTCountedFactory< CTFactoryWithParam< ITestColoredShape, CTestSelfUnregisteringColoredShape, CString, MT::CMutex > > TCountedSelfUnregisteringProductFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestSelfReplacingColoredShape, CString, MT::CMutex >        TSelfReplacingProductFactory;
    typedef CTCountedFactory< CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex > > TCountedCircleFactory;
    typedef CTCountedFactory< CTFactoryWithParam< ITestColoredShape, CTestCycleColoredShapeA, CString, MT::CMutex > > TCycleAFactory;
    typedef CTCountedFactory< CTFactoryWithParam< ITestColoredShape, CTestCycleColoredShapeB, CString, MT::CMutex > > TCycleBFactory;

    static TAbstractFactory*& SelfUnregisteringTarget( void ) { return g_selfUnregisteringColoredShapeFactory; }
    static TAbstractFactory*& SelfReplacingTarget( void ) { return g_selfReplacingColoredShapeFactory; }
    static TAbstractFactory::TFactory*& SelfReplacement( void ) { return g_coloredShapeReplacement; }
    static TAbstractFactory*& CycleTarget( void ) { return g_cycleColoredShapeFactory; }
};

/*-------------------------------------------------------------------------*/

/*
 *  The registry of the abstract factory is guarded by its lock, and a product is made without that lock: a product whose
 *  construction waits for a lock that another thread holds while it creates through the same factory does not hold up
 *  that create
 */
template< class TForm >
static void
TestProductConstructedWithoutFactoryLock( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory factory;
    typename TForm::TGatedFactory gatedFactory;
    typename TForm::TCircleFactory circleFactory;
    typename TForm::TSlowNameFactory slowNameFactory;
    factory.RegisterConcreteFactory( "Gated", &gatedFactory );
    factory.RegisterConcreteFactory( "Circle", &circleFactory );

    // A registration holds the lock while it asks the concrete factory for its class type name: a create waits for it
    g_factoryNameGate.Close();
    g_factoryNameGateArmed.Store( 1 );
    TCallThread registerer( factory, "SlowName", TESTFACTORYCALL_REGISTER, &slowNameFactory );
    TCallThread lockedOutCreator( factory, "Circle", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( registerer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_factoryNameGate.WaitForWaiting( 1 ) );
    GUCEF_TESTFW_ASSERT_TRUE( lockedOutCreator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( lockedOutCreator.WaitUntilDone( 300 ) );
    g_factoryNameGateArmed.Store( 0 );
    g_factoryNameGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( registerer.WaitUntilDone( 10000 ) && lockedOutCreator.WaitUntilDone( 10000 ) );
    registerer.Deactivate( false, true );
    lockedOutCreator.Deactivate( false, true );
    GUCEF_TESTFW_ASSERT_FALSE( registerer.HasFailed() || lockedOutCreator.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !lockedOutCreator.product.IsNULL() && factory.IsConstructible( "SlowName" ) );
    lockedOutCreator.product.Unlink();

    // This thread holds the lock (as a thread that pulses a pulse generator holds its notification lock), the gated
    // product waits for it in its constructor (as a product subscribes to that pulse generator); the holder of that lock
    // creates through the same factory, which is done without the gated product
    g_gatedShapeTakesLock.Store( 1 );
    TCallThread gatedCreator( factory, "Gated", TESTFACTORYCALL_CREATE );
    TCallThread circleCreator( factory, "Circle", TESTFACTORYCALL_CREATE );
    {
        MT::CScopeMutex lock( g_gatedShapeLock );
        GUCEF_TESTFW_ASSERT_TRUE( gatedCreator.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );
        GUCEF_TESTFW_ASSERT_TRUE( circleCreator.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF_TESTFW_ASSERT_TRUE( circleCreator.WaitUntilDone( 500 ) );
        GUCEF_TESTFW_ASSERT_FALSE( gatedCreator.IsDone() );
    }
    GUCEF_TESTFW_ASSERT_TRUE( gatedCreator.WaitUntilDone( 10000 ) && circleCreator.WaitUntilDone( 10000 ) );
    gatedCreator.Deactivate( false, true );
    circleCreator.Deactivate( false, true );
    g_gatedShapeTakesLock.Store( 0 );

    GUCEF_TESTFW_ASSERT_FALSE( circleCreator.HasFailed() || gatedCreator.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !circleCreator.product.IsNULL() );
    GUCEF_TESTFW_ASSERT_TRUE( !gatedCreator.product.IsNULL() && gatedCreator.product->GetShapeType() == "Gated" );
    gatedCreator.product.Unlink();
    circleCreator.product.Unlink();

    factory.UnregisterAllConcreteFactories();
}

/*
 *  Unregistering a concrete factory returns only once a create through it is done; the concrete factory may be
 *  destroyed then, once its products are gone
 */
template< class TForm >
static void
TestUnregisterWaitsForCreateUnderWay( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory factory;
    typename TForm::TGatedFactory* gatedFactory = GUCEF_NEW typename TForm::TGatedFactory();
    factory.RegisterConcreteFactory( "Gated", gatedFactory );

    g_shapeGate.Close();
    TCallThread creator( factory, "Gated", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

    TCallThread unregisterer( factory, "Gated", TESTFACTORYCALL_UNREGISTER );
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( unregisterer.WaitUntilDone( 300 ) );
    GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Gated" ) );    // no new create finds it meanwhile

    g_shapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.WaitUntilDone( 10000 ) && creator.WaitUntilDone( 10000 ) );
    unregisterer.Deactivate( false, true );
    creator.Deactivate( false, true );

    GUCEF_TESTFW_ASSERT_FALSE( creator.HasFailed() || unregisterer.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !creator.product.IsNULL() && creator.product->GetShapeType() == "Gated" );

    // a product goes through the concrete factory that made it: the factory goes after its products
    creator.product.Unlink();
    GUCEF_DELETE gatedFactory;
}

/*
 *  Unregistering a concrete factory waits for the creates through that concrete factory only: not for a create through
 *  another one, also not for one that starts while the unregistration waits
 */
template< class TForm >
static void
TestUnregisterWaitsOnlyForCreatesOfItsFactory( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory factory;
    typename TForm::TGatedFactory gatedFactory;
    typename TForm::TSecondGatedFactory secondGatedFactory;
    typename TForm::TCircleFactory circleFactory;
    factory.RegisterConcreteFactory( "Gated", &gatedFactory );
    factory.RegisterConcreteFactory( "SecondGated", &secondGatedFactory );
    factory.RegisterConcreteFactory( "Circle", &circleFactory );

    g_shapeGate.Close();
    g_secondShapeGate.Close();
    TCallThread gatedCreator( factory, "Gated", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( gatedCreator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

    // a create through another concrete factory does not hold up the removal of this one
    TCallThread circleUnregisterer( factory, "Circle", TESTFACTORYCALL_UNREGISTER );
    GUCEF_TESTFW_ASSERT_TRUE( circleUnregisterer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( circleUnregisterer.WaitUntilDone( 2000 ) );
    GUCEF_TESTFW_ASSERT_FALSE( gatedCreator.IsDone() );

    // the removal of the gated factory waits for its create, and not for a create through the second gated factory that
    // starts while it waits
    TCallThread gatedUnregisterer( factory, "Gated", TESTFACTORYCALL_UNREGISTER );
    GUCEF_TESTFW_ASSERT_TRUE( gatedUnregisterer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( gatedUnregisterer.WaitUntilDone( 300 ) );
    TCallThread secondCreator( factory, "SecondGated", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( secondCreator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_secondShapeGate.WaitForWaiting( 1, 2000 ) );
    g_shapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( gatedCreator.WaitUntilDone( 10000 ) );
    GUCEF_TESTFW_ASSERT_TRUE( gatedUnregisterer.WaitUntilDone( 2000 ) );
    GUCEF_TESTFW_ASSERT_FALSE( secondCreator.IsDone() );

    g_secondShapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( secondCreator.WaitUntilDone( 10000 ) && gatedUnregisterer.WaitUntilDone( 10000 ) && circleUnregisterer.WaitUntilDone( 10000 ) );
    gatedCreator.Deactivate( false, true );
    circleUnregisterer.Deactivate( false, true );
    gatedUnregisterer.Deactivate( false, true );
    secondCreator.Deactivate( false, true );

    GUCEF_TESTFW_ASSERT_FALSE( gatedCreator.HasFailed() || circleUnregisterer.HasFailed() || gatedUnregisterer.HasFailed() || secondCreator.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !gatedCreator.product.IsNULL() && !secondCreator.product.IsNULL() );
    GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "SecondGated" ) && !factory.IsConstructible( "Gated" ) && !factory.IsConstructible( "Circle" ) );
    gatedCreator.product.Unlink();
    secondCreator.product.Unlink();

    factory.UnregisterAllConcreteFactories();
}

/*
 *  Unregistering every concrete factory returns only once the creates under way are done
 */
template< class TForm >
static void
TestUnregisterAllWaitsForCreateUnderWay( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory factory;
    typename TForm::TGatedFactory gatedFactory;
    typename TForm::TCircleFactory circleFactory;
    factory.RegisterConcreteFactory( "Gated", &gatedFactory );
    factory.RegisterConcreteFactory( "Circle", &circleFactory );

    g_shapeGate.Close();
    TCallThread creator( factory, "Gated", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

    TCallThread unregisterer( factory, CString::Empty, TESTFACTORYCALL_UNREGISTER_ALL );
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( unregisterer.WaitUntilDone( 300 ) );

    g_shapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.WaitUntilDone( 10000 ) && creator.WaitUntilDone( 10000 ) );
    unregisterer.Deactivate( false, true );
    creator.Deactivate( false, true );

    GUCEF_TESTFW_ASSERT_FALSE( creator.HasFailed() || unregisterer.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !creator.product.IsNULL() );
    GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Gated" ) || factory.IsConstructible( "Circle" ) );
    creator.product.Unlink();
}

/*
 *  The destruction of the abstract factory waits for the creates under way
 */
template< class TForm >
static void
TestDestructorWaitsForCreateUnderWay( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    typename TForm::TGatedFactory gatedFactory;
    TAbstractFactory* factory = GUCEF_NEW TAbstractFactory();
    factory->RegisterConcreteFactory( "Gated", &gatedFactory );

    g_shapeGate.Close();
    TCallThread creator( *factory, "Gated", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

    TCallThread destroyer( *factory, CString::Empty, TESTFACTORYCALL_DESTROY );
    GUCEF_TESTFW_ASSERT_TRUE( destroyer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( destroyer.WaitUntilDone( 300 ) );

    g_shapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( destroyer.WaitUntilDone( 10000 ) && creator.WaitUntilDone( 10000 ) );
    destroyer.Deactivate( false, true );
    creator.Deactivate( false, true );

    GUCEF_TESTFW_ASSERT_FALSE( creator.HasFailed() || destroyer.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( !creator.product.IsNULL() );
    creator.product.Unlink();
}

/*
 *  A concrete factory unregistered from within a create through it, on the thread of that create: the unregistration
 *  does not wait for that create, which would never end, and a concrete factory that the abstract factory owns is kept
 *  until the abstract factory is destroyed, as the product of that create goes through it
 */
template< class TForm >
static void
TestUnregisterFromWithinOwnCreate( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    // From within the constructor of a product. The thread and the abstract factory are made on the heap and left alone
    // when the create does not return, as the thread that hangs in it still uses them.
    {
        typename TForm::TSelfUnregisteringProductFactory productFactory;
        TAbstractFactory* factory = GUCEF_NEW TAbstractFactory();
        factory->RegisterConcreteFactory( "SelfUnregistering", &productFactory );
        TForm::SelfUnregisteringTarget() = factory;
        TCallThread* creator = GUCEF_NEW TCallThread( *factory, "SelfUnregistering", TESTFACTORYCALL_CREATE );
        GUCEF_TESTFW_ASSERT_TRUE( creator->Activate( GUCEF_NULL, 0, 0 ) );
        const bool done = creator->WaitUntilDone( 5000 );
        GUCEF_TESTFW_ASSERT_TRUE( done );
        if ( done )
        {
            creator->Deactivate( false, true );
            GUCEF_TESTFW_ASSERT_FALSE( creator->HasFailed() );
            GUCEF_TESTFW_ASSERT_TRUE( !creator->product.IsNULL() && creator->product->GetShapeType() == "SelfUnregistering" );
            GUCEF_TESTFW_ASSERT_FALSE( factory->IsConstructible( "SelfUnregistering" ) );
            creator->product.Unlink();
            GUCEF_DELETE creator;
            TForm::SelfUnregisteringTarget() = GUCEF_NULL;
            GUCEF_DELETE factory;
        }
    }

    // From within the create of a concrete factory that the abstract factory owns
    {
        TAbstractFactory* factory = GUCEF_NEW TAbstractFactory( true );
        g_countedFactoriesDestroyed.Store( 0 );
        g_destroyedDuringOwnCreate.Store( 99 );
        factory->RegisterConcreteFactory( "SelfUnregisteringFactory", GUCEF_NEW typename TForm::TSelfUnregisteringFactory( *factory ) );
        TCallThread* creator = GUCEF_NEW TCallThread( *factory, "SelfUnregisteringFactory", TESTFACTORYCALL_CREATE );
        GUCEF_TESTFW_ASSERT_TRUE( creator->Activate( GUCEF_NULL, 0, 0 ) );
        const bool done = creator->WaitUntilDone( 5000 );
        GUCEF_TESTFW_ASSERT_TRUE( done );
        if ( done )
        {
            creator->Deactivate( false, true );
            GUCEF_TESTFW_ASSERT_FALSE( creator->HasFailed() );
            GUCEF_TESTFW_ASSERT_TRUE( 0 == g_destroyedDuringOwnCreate.Load() );     // not while its create was under way
            GUCEF_TESTFW_ASSERT_TRUE( 0 == g_countedFactoriesDestroyed.Load() );    // nor once it ended: kept
            GUCEF_TESTFW_ASSERT_FALSE( factory->IsConstructible( "SelfUnregisteringFactory" ) );
            GUCEF_DELETE creator;
            GUCEF_DELETE factory;
            GUCEF_TESTFW_ASSERT_TRUE( 1 == g_countedFactoriesDestroyed.Load() );    // with the abstract factory
        }
    }

    // From within the constructor of a product of a concrete factory that the abstract factory owns: the product goes
    // through that concrete factory when it is released after the create, so the concrete factory stays until the
    // abstract factory is destroyed (under the address sanitizer a release through a deleted factory is an error)
    {
        TAbstractFactory* factory = GUCEF_NEW TAbstractFactory( true );
        g_countedFactoriesDestroyed.Store( 0 );
        factory->RegisterConcreteFactory( "SelfUnregistering", GUCEF_NEW typename TForm::TCountedSelfUnregisteringProductFactory() );
        TForm::SelfUnregisteringTarget() = factory;
        TCallThread* creator = GUCEF_NEW TCallThread( *factory, "SelfUnregistering", TESTFACTORYCALL_CREATE );
        GUCEF_TESTFW_ASSERT_TRUE( creator->Activate( GUCEF_NULL, 0, 0 ) );
        const bool done = creator->WaitUntilDone( 5000 );
        GUCEF_TESTFW_ASSERT_TRUE( done );
        if ( done )
        {
            creator->Deactivate( false, true );
            GUCEF_TESTFW_ASSERT_FALSE( creator->HasFailed() );
            GUCEF_TESTFW_ASSERT_TRUE( !creator->product.IsNULL() && creator->product->GetShapeType() == "SelfUnregistering" );
            GUCEF_TESTFW_ASSERT_FALSE( factory->IsConstructible( "SelfUnregistering" ) );
            GUCEF_TESTFW_ASSERT_TRUE( 0 == g_countedFactoriesDestroyed.Load() );
            creator->product.Unlink();                                              // through the kept concrete factory
            GUCEF_TESTFW_ASSERT_TRUE( 0 == g_countedFactoriesDestroyed.Load() );
            GUCEF_DELETE creator;
            TForm::SelfUnregisteringTarget() = GUCEF_NULL;
            GUCEF_DELETE factory;
            GUCEF_TESTFW_ASSERT_TRUE( 1 == g_countedFactoriesDestroyed.Load() );
        }
    }
}

/*
 *  Registering a concrete factory for a type that has one registered replaces it: the registration returns only once a
 *  create through the replaced concrete factory is done, the creates that start meanwhile use the new one, and the
 *  replaced one is not deleted, also not by an abstract factory that owns its concrete factories
 */
template< class TForm >
static void
TestReplaceWaitsForCreateUnderWay( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    // Not owned: the replaced concrete factory may be destroyed once the registration returned and its products are gone
    {
        TAbstractFactory factory;
        typename TForm::TGatedFactory* gatedFactory = GUCEF_NEW typename TForm::TGatedFactory();
        typename TForm::TCircleFactory circleFactory;
        factory.RegisterConcreteFactory( "Shape", gatedFactory );

        g_shapeGate.Close();
        TCallThread creator( factory, "Shape", TESTFACTORYCALL_CREATE );
        GUCEF_TESTFW_ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

        TCallThread registerer( factory, "Shape", TESTFACTORYCALL_REGISTER, &circleFactory );
        GUCEF_TESTFW_ASSERT_TRUE( registerer.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF_TESTFW_ASSERT_FALSE( registerer.WaitUntilDone( 300 ) );

        // a create that starts meanwhile uses the new concrete factory
        TCallThread newCreator( factory, "Shape", TESTFACTORYCALL_CREATE );
        GUCEF_TESTFW_ASSERT_TRUE( newCreator.Activate( GUCEF_NULL, 0, 0 ) );
        GUCEF_TESTFW_ASSERT_TRUE( newCreator.WaitUntilDone( 2000 ) );
        GUCEF_TESTFW_ASSERT_FALSE( registerer.IsDone() );

        g_shapeGate.Open();
        GUCEF_TESTFW_ASSERT_TRUE( registerer.WaitUntilDone( 10000 ) && creator.WaitUntilDone( 10000 ) );
        registerer.Deactivate( false, true );
        creator.Deactivate( false, true );
        newCreator.Deactivate( false, true );

        GUCEF_TESTFW_ASSERT_FALSE( creator.HasFailed() || registerer.HasFailed() || newCreator.HasFailed() );
        GUCEF_TESTFW_ASSERT_TRUE( !creator.product.IsNULL() && creator.product->GetShapeType() == "Gated" );
        GUCEF_TESTFW_ASSERT_TRUE( !newCreator.product.IsNULL() && newCreator.product->GetShapeType() != "Gated" );
        creator.product.Unlink();
        newCreator.product.Unlink();
        GUCEF_DELETE gatedFactory;
        factory.UnregisterAllConcreteFactories();
    }

    // Owned: the replaced concrete factory goes back to whoever registered it
    {
        TAbstractFactory* factory = GUCEF_NEW TAbstractFactory( true );
        g_countedFactoriesDestroyed.Store( 0 );
        typename TForm::TCountedCircleFactory* first = GUCEF_NEW typename TForm::TCountedCircleFactory();
        factory->RegisterConcreteFactory( "Circle", first );
        factory->RegisterConcreteFactory( "Circle", first );                       // the same one again: no change
        GUCEF_TESTFW_ASSERT_TRUE( factory->IsConstructible( "Circle" ) && 0 == g_countedFactoriesDestroyed.Load() );
        factory->RegisterConcreteFactory( "Circle", GUCEF_NEW typename TForm::TCountedCircleFactory() );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == g_countedFactoriesDestroyed.Load() );
        GUCEF_TESTFW_ASSERT_FALSE( TTestCreateThrough< TAbstractFactory >::Create( *factory, "Circle" ).IsNULL() );
        GUCEF_DELETE first;
        GUCEF_TESTFW_ASSERT_TRUE( 1 == g_countedFactoriesDestroyed.Load() );
        GUCEF_DELETE factory;
        GUCEF_TESTFW_ASSERT_TRUE( 2 == g_countedFactoriesDestroyed.Load() );
    }
}

/*
 *  A concrete factory replaced from within a create through it, on the thread of that create: the registration does
 *  not wait for that create, which would never end; the creates after it use the new concrete factory
 */
template< class TForm >
static void
TestReplaceFromWithinOwnCreate( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    // The thread and the abstract factory are made on the heap and left alone when the create does not return
    typename TForm::TSelfReplacingProductFactory* productFactory = GUCEF_NEW typename TForm::TSelfReplacingProductFactory();
    typename TForm::TCircleFactory* replacement = GUCEF_NEW typename TForm::TCircleFactory();
    TAbstractFactory* factory = GUCEF_NEW TAbstractFactory();
    factory->RegisterConcreteFactory( "SelfReplacing", productFactory );
    TForm::SelfReplacingTarget() = factory;
    TForm::SelfReplacement() = replacement;
    TCallThread* creator = GUCEF_NEW TCallThread( *factory, "SelfReplacing", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creator->Activate( GUCEF_NULL, 0, 0 ) );
    const bool done = creator->WaitUntilDone( 5000 );
    GUCEF_TESTFW_ASSERT_TRUE( done );
    if ( done )
    {
        creator->Deactivate( false, true );
        GUCEF_TESTFW_ASSERT_FALSE( creator->HasFailed() );
        GUCEF_TESTFW_ASSERT_TRUE( !creator->product.IsNULL() && creator->product->GetShapeType() == "SelfReplacing" );
        typename TAbstractFactory::TProductPtr next = TTestCreateThrough< TAbstractFactory >::Create( *factory, "SelfReplacing" );
        GUCEF_TESTFW_ASSERT_TRUE( !next.IsNULL() && next->GetShapeType() != "SelfReplacing" );
        next.Unlink();
        creator->product.Unlink();
        GUCEF_DELETE creator;
        TForm::SelfReplacingTarget() = GUCEF_NULL;
        TForm::SelfReplacement() = GUCEF_NULL;
        GUCEF_DELETE factory;
        GUCEF_DELETE productFactory;
        GUCEF_DELETE replacement;
    }
}

/*
 *  Two creates whose product constructors unregister each other's type: each removal waits for the other create, a
 *  cycle. An abstract factory that owns its concrete factories does not let such a removal wait: both creates end, and
 *  the concrete factories, which their products go through, stay until the abstract factory is destroyed. (With
 *  concrete factories that are not owned the removals wait for each other and do not end; that is logged as an error
 *  and is not run here, as it would leave two threads behind.)
 */
template< class TForm >
static void
TestRemovalCycleWithOwnedFactories( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory* factory = GUCEF_NEW TAbstractFactory( true );
    g_countedFactoriesDestroyed.Store( 0 );
    g_cycleShapesInConstruction.Store( 0 );
    factory->RegisterConcreteFactory( "CycleA", GUCEF_NEW typename TForm::TCycleAFactory() );
    factory->RegisterConcreteFactory( "CycleB", GUCEF_NEW typename TForm::TCycleBFactory() );
    TForm::CycleTarget() = factory;

    // The threads and the abstract factory are left alone when the creates do not return
    TCallThread* creatorA = GUCEF_NEW TCallThread( *factory, "CycleA", TESTFACTORYCALL_CREATE );
    TCallThread* creatorB = GUCEF_NEW TCallThread( *factory, "CycleB", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creatorA->Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( creatorB->Activate( GUCEF_NULL, 0, 0 ) );
    const bool done = creatorA->WaitUntilDone( 15000 ) && creatorB->WaitUntilDone( 15000 );
    GUCEF_TESTFW_ASSERT_TRUE( done );
    if ( done )
    {
        creatorA->Deactivate( false, true );
        creatorB->Deactivate( false, true );
        GUCEF_TESTFW_ASSERT_FALSE( creatorA->HasFailed() || creatorB->HasFailed() );
        GUCEF_TESTFW_ASSERT_TRUE( !creatorA->product.IsNULL() && creatorA->product->GetShapeType() == "CycleA" );
        GUCEF_TESTFW_ASSERT_TRUE( !creatorB->product.IsNULL() && creatorB->product->GetShapeType() == "CycleB" );
        GUCEF_TESTFW_ASSERT_FALSE( factory->IsConstructible( "CycleA" ) || factory->IsConstructible( "CycleB" ) );
        GUCEF_TESTFW_ASSERT_TRUE( 0 == g_countedFactoriesDestroyed.Load() );
        creatorA->product.Unlink();
        creatorB->product.Unlink();
        GUCEF_DELETE creatorA;
        GUCEF_DELETE creatorB;
        TForm::CycleTarget() = GUCEF_NULL;
        GUCEF_DELETE factory;
        GUCEF_TESTFW_ASSERT_TRUE( 2 == g_countedFactoriesDestroyed.Load() );
    }
}

/*
 *  A removal that waits long for a create through the concrete factory logs a warning that names the type; it still
 *  returns once that create ended. The time after which it warns (5 s by default) is shortened to 300 ms for the test.
 */
template< class TForm >
static void
TestLongRemovalWaitIsLogged( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    UInt32 firstWarningAfterInMs = 0;
    UInt32 warningIntervalInMs = 0;
    CAbstractFactoryEntryTracker::GetLongRemovalWaitWarningTimes( firstWarningAfterInMs, warningIntervalInMs );
    CAbstractFactoryEntryTracker::SetLongRemovalWaitWarningTimes( 300, warningIntervalInMs );

    CLogManager& logManager = CCoreGlobal::Instance()->GetLogManager();
    CTestWarningCaptureLogger captureLogger;
    logManager.AddLogger( &captureLogger );

    TAbstractFactory factory;
    typename TForm::TGatedFactory gatedFactory;
    factory.RegisterConcreteFactory( "SlowToRemove", &gatedFactory );

    g_shapeGate.Close();
    TCallThread creator( factory, "SlowToRemove", TESTFACTORYCALL_CREATE );
    GUCEF_TESTFW_ASSERT_TRUE( creator.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( g_shapeGate.WaitForWaiting( 1 ) );

    TCallThread unregisterer( factory, "SlowToRemove", TESTFACTORYCALL_UNREGISTER );
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_FALSE( unregisterer.WaitUntilDone( 600 ) );

    g_shapeGate.Open();
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer.WaitUntilDone( 10000 ) && creator.WaitUntilDone( 10000 ) );
    unregisterer.Deactivate( false, true );
    creator.Deactivate( false, true );
    GUCEF_TESTFW_ASSERT_FALSE( creator.HasFailed() || unregisterer.HasFailed() );
    creator.product.Unlink();

    logManager.FlushLogs();
    logManager.RemoveLogger( &captureLogger );
    CAbstractFactoryEntryTracker::SetLongRemovalWaitWarningTimes( firstWarningAfterInMs, warningIntervalInMs );
    GUCEF_TESTFW_ASSERT_TRUE( captureLogger.HasWarningWith( "\"SlowToRemove\"", "has waited" ) );
}

/*
 *  The identity of a thread stays the same within the thread and differs between threads that are alive at the same
 *  time (the abstract factories tell the creates of the removing thread apart by it)
 */
static void
TestThreadIdentity( void )
{
    CTestThreadIdentityThread first;
    CTestThreadIdentityThread second;
    GUCEF_TESTFW_ASSERT_TRUE( first.Activate( GUCEF_NULL, 0, 0 ) && second.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( first.WaitUntilRecorded() && second.WaitUntilRecorded() );
    const TAbstractFactoryThreadId thisThread = GetAbstractFactoryThreadId();
    GUCEF_TESTFW_ASSERT_TRUE( thisThread == GetAbstractFactoryThreadId() );
    GUCEF_TESTFW_ASSERT_TRUE( first.m_identity == first.m_secondIdentity && second.m_identity == second.m_secondIdentity );
    GUCEF_TESTFW_ASSERT_TRUE( first.m_identity != second.m_identity );
    GUCEF_TESTFW_ASSERT_TRUE( first.m_identity != thisThread && second.m_identity != thisThread );
    first.End();
    second.End();
    first.Deactivate( false, true );
    second.Deactivate( false, true );
}

/*
 *  A create whose product constructor throws is no longer under way: the removal of its concrete factory neither waits
 *  for it on another thread nor takes itself for a removal from within that create on its own thread. The thread and
 *  the abstract factory are left alone when the removal does not return.
 */
template< class TForm >
static void
TestCreateThatThrowsLeavesNoCreateUnderWay( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef CTFactoryCallThread< TAbstractFactory > TCallThread;

    TAbstractFactory* factory = GUCEF_NEW TAbstractFactory( true );
    g_countedFactoriesDestroyed.Store( 0 );
    factory->RegisterConcreteFactory( "Throwing", GUCEF_NEW typename TForm::TThrowingFactory() );
    factory->RegisterConcreteFactory( "Throwing2", GUCEF_NEW typename TForm::TThrowingFactory() );

    UInt32 thrown = 0;
    try { TTestCreateThrough< TAbstractFactory >::Create( *factory, "Throwing" ); }
    catch ( const STestShapeConstructionFailure& ) { ++thrown; }
    try { TTestCreateThrough< TAbstractFactory >::Create( *factory, "Throwing2" ); }
    catch ( const STestShapeConstructionFailure& ) { ++thrown; }
    GUCEF_TESTFW_ASSERT_TRUE( 2 == thrown );

    TCallThread* unregisterer = GUCEF_NEW TCallThread( *factory, "Throwing", TESTFACTORYCALL_UNREGISTER );
    GUCEF_TESTFW_ASSERT_TRUE( unregisterer->Activate( GUCEF_NULL, 0, 0 ) );
    const bool done = unregisterer->WaitUntilDone( 5000 );
    GUCEF_TESTFW_ASSERT_TRUE( done );
    if ( done )
    {
        unregisterer->Deactivate( false, true );
        GUCEF_TESTFW_ASSERT_FALSE( unregisterer->HasFailed() );
        GUCEF_TESTFW_ASSERT_TRUE( 1 == g_countedFactoriesDestroyed.Load() );
        GUCEF_DELETE unregisterer;

        factory->UnregisterConcreteFactory( "Throwing2" );
        GUCEF_TESTFW_ASSERT_TRUE( 2 == g_countedFactoriesDestroyed.Load() );
        GUCEF_DELETE factory;
    }
}

/*
 *  Creates on two threads while this thread registers a concrete factory, unregisters it and takes it as gone once the
 *  unregistration returned, over and over (at most 50000 rounds, at most 1 second): no create uses a concrete factory
 *  after its unregistration returned, and the registry is not changed under a create (under the address sanitizer a
 *  use of a freed map node is an error of its own)
 */
template< class TForm >
static void
TestCreatesRacingUnregistration( void )
{
    typedef typename TForm::TAbstractFactory TAbstractFactory;
    typedef typename TForm::TStressFactory TStressFactory;

    TAbstractFactory factory( false, false );
    g_stressUsedAfterRemoval.Store( 0 );
    GUCEF::vector< TStressFactory* > removed;
    removed.reserve( 50000 );

    CTStressCreatorThread< TAbstractFactory > creator1( factory );
    CTStressCreatorThread< TAbstractFactory > creator2( factory );
    GUCEF_TESTFW_ASSERT_TRUE( creator1.Activate( GUCEF_NULL, 0, 0 ) );
    GUCEF_TESTFW_ASSERT_TRUE( creator2.Activate( GUCEF_NULL, 0, 0 ) );

    const UInt64 start = MT::PrecisionTickCount();
    const UInt64 resolution = MT::PrecisionTimerResolution();
    UInt32 rounds = 0;
    while ( rounds < 50000 && ( MT::PrecisionTickCount() - start ) < resolution )
    {
        TStressFactory* concreteFactory = GUCEF_NEW TStressFactory();
        factory.RegisterConcreteFactory( "Stress", concreteFactory );
        factory.UnregisterConcreteFactory( "Stress" );
        concreteFactory->MarkRemoved();
        removed.push_back( concreteFactory );
        ++rounds;
    }

    creator1.Stop();
    creator2.Stop();
    creator1.Deactivate( false, true );
    creator2.Deactivate( false, true );

    GUCEF_LOG( LOGLEVEL_NORMAL, "AbstractFactory: " << rounds << " registrations and removals, " << creator1.GetCreates() + creator2.GetCreates()
        << " creates, " << g_stressUsedAfterRemoval.Load() << " uses after a removal" );
    GUCEF_TESTFW_ASSERT_FALSE( creator1.HasFailed() || creator2.HasFailed() );
    GUCEF_TESTFW_ASSERT_TRUE( 0 == g_stressUsedAfterRemoval.Load() );

    for ( size_t i=0; i<removed.size(); ++i )
        GUCEF_DELETE removed[ i ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAbstractFactoryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AbstractFactory" );

    /*
     *  Type aliases for readability
     */
    typedef CTAbstractFactory< CString, ITestShape, MT::CMutex >            TShapeFactory;
    typedef CTFactory< ITestShape, CTestCircle, MT::CMutex >                TCircleFactory;
    typedef CTFactory< ITestShape, CTestSquare, MT::CMutex >                TSquareFactory;
    typedef CTFactory< ITestShape, CTestTriangle, MT::CMutex >              TTriangleFactory;
    typedef TShapeFactory::TProductPtr                                       TShapePtr;

    typedef CTAbstractFactoryWithParam< CString, ITestColoredShape, CString, MT::CMutex >            TColoredShapeFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex >        TColoredCircleFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredSquare, CString, MT::CMutex >        TColoredSquareFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredTriangle, CString, MT::CMutex >      TColoredTriangleFactory;
    typedef TColoredShapeFactory::TProductPtr                                                        TColoredShapePtr;

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_DefaultConstruct" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_TRUE( true ); // construction succeeded
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUnregisteredType_ReturnsNull" )
        try
        {
            TShapeFactory factory;
            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsConstructible_UnregisteredType_ReturnsFalse" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_RegisterAndCreate" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Circle" );
            }

            factory.UnregisterConcreteFactory( "Circle" ); 
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsConstructible_RegisteredType_ReturnsTrue" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_MultipleTypes_CreateCorrectConcrete" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );

            TShapePtr circle = factory.Create( "Circle" );
            TShapePtr square = factory.Create( "Square" );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "Circle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUsingClassTypeName" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            // CreateUsingClassTypeName uses the concrete class's type name,
            // not the selection key. For CTFactory this is ToString<CTestCircle>()
            // which maps via the reverse lookup maintained by CTAbstractFactory.
            CString className = circleFactory.GetConcreteClassTypeName();
            TShapePtr ptr = factory.CreateUsingClassTypeName( className );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Circle" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsClassConstructible_RegisteredClass_ReturnsTrue" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsClassConstructible( className ) );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsClassConstructible_UnregisteredClass_ReturnsFalse" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsClassConstructible( "CTestCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetConcreteClassTypeNameForSelectionCriterea" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( className.IsNULLOrEmpty() );
            GUCEF_TESTFW_ASSERT_TRUE( className == circleFactory.GetConcreteClassTypeName() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetConcreteClassTypeNameForSelectionCriterea_Unregistered" )
        try
        {
            TShapeFactory factory;
            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( className.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetSelectionCritereaForConcreteClassTypeName" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            CString selectionKey = factory.GetSelectionCritereaForConcreteClassTypeName( className );
            GUCEF_TESTFW_ASSERT_TRUE( selectionKey == "Circle" );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_Empty" )
        try
        {
            TShapeFactory factory;
            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_WithRegistrations" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 2 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Circle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Square" ) != keys.end() );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterConcreteFactory" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterAllConcreteFactories" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            TTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );
            factory.RegisterConcreteFactory( "Triangle", &triangleFactory );

            CStringSet keysBefore;
            factory.ObtainKeySet( keysBefore );
            GUCEF_TESTFW_ASSERT_TRUE( keysBefore.size() == 3 );

            factory.UnregisterAllConcreteFactories();

            CStringSet keysAfter;
            factory.ObtainKeySet( keysAfter );
            GUCEF_TESTFW_ASSERT_TRUE( keysAfter.empty() );

            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Square" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Triangle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
            factory.UnregisterConcreteFactory( "Triangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_AfterUnregisterAll_IsEmpty" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.UnregisterAllConcreteFactories();

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ReRegisterAfterUnregister" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;

            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.UnregisterConcreteFactory( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );

            // Re-register with a different concrete type under the same key
            factory.RegisterConcreteFactory( "Circle", &squareFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            // Now "Circle" key maps to CTestSquare
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUnknownClassTypeName_ReturnsNull" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            TShapePtr ptr = factory.CreateUsingClassTypeName( "NonExistentClass" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ThreeTypes_ObtainKeySet" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            TTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );
            factory.RegisterConcreteFactory( "Triangle", &triangleFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 3 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Circle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Square" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Triangle" ) != keys.end() );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Square" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Triangle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Hexagon" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
            factory.UnregisterConcreteFactory( "Triangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IntKeySelector_RegisterAndCreate" )
        try
        {
            // Use integer keys instead of strings
            typedef CTAbstractFactory< Int32, ITestShape, MT::CMutex >  TIntKeyShapeFactory;
            typedef TIntKeyShapeFactory::TProductPtr                     TIntShapePtr;

            TIntKeyShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( 1, &circleFactory );
            factory.RegisterConcreteFactory( 2, &squareFactory );

            TIntShapePtr circle = factory.Create( 1 );
            TIntShapePtr square = factory.Create( 2 );
            TIntShapePtr unknown = factory.Create( 99 );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            GUCEF_TESTFW_ASSERT_TRUE( unknown.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "Circle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( 1 );
            factory.UnregisterConcreteFactory( 2 );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_DefaultConstruct" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_TRUE( true ); // construction succeeded
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUnregisteredType_ReturnsNull" )
        try
        {
            TColoredShapeFactory factory;
            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsConstructible_UnregisteredType_ReturnsFalse" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_RegisterAndCreate" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Blue" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Blue" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsConstructible_RegisteredType_ReturnsTrue" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsClassConstructible_RegisteredClass_ReturnsTrue" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsClassConstructible( className ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsClassConstructible_UnregisteredClass_ReturnsFalse" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsClassConstructible( "CTestColoredCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_MultipleTypes_CreateCorrectConcrete" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            TColoredShapePtr circle = factory.Create( "ColoredCircle", "Red" );
            TColoredShapePtr square = factory.Create( "ColoredSquare", "Green" );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "ColoredSquare" );
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetColor() == "Red" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetColor() == "Green" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_DifferentParamValues_CreateIndependentObjects" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr red    = factory.Create( "ColoredCircle", "Red" );
            TColoredShapePtr yellow = factory.Create( "ColoredCircle", "Yellow" );
            TColoredShapePtr blue   = factory.Create( "ColoredCircle", "Blue" );

            GUCEF_TESTFW_ASSERT_FALSE( red.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( yellow.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( blue.IsNULL() );
            if ( !red.IsNULL() && !yellow.IsNULL() && !blue.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( red->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( yellow->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( blue->GetShapeType() == "ColoredCircle" );
            }
            if ( !red.IsNULL() && !yellow.IsNULL() && !blue.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( red->GetColor() == "Red" );
                GUCEF_TESTFW_ASSERT_TRUE( yellow->GetColor() == "Yellow" );
                GUCEF_TESTFW_ASSERT_TRUE( blue->GetColor() == "Blue" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUsingClassTypeName" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            TColoredShapePtr ptr = factory.CreateUsingClassTypeName( className, "Purple" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Purple" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_WithRegistrations" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 2 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredCircle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredSquare" ) != keys.end() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterConcreteFactory" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );

            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterAllConcreteFactories" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            factory.UnregisterAllConcreteFactories();

            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredSquare" ) );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUnknownClassTypeName_ReturnsNull" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr ptr = factory.CreateUsingClassTypeName( "NonExistentClass", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetConcreteClassTypeNameForSelectionCriterea" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_FALSE( className.IsNULLOrEmpty() );
            GUCEF_TESTFW_ASSERT_TRUE( className == circleFactory.GetConcreteClassTypeName() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetSelectionCritereaForConcreteClassTypeName" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            CString selectionKey = factory.GetSelectionCritereaForConcreteClassTypeName( className );
            GUCEF_TESTFW_ASSERT_TRUE( selectionKey == "ColoredCircle" );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetConcreteClassTypeNameForSelectionCriterea_Unregistered" )
        try
        {
            TColoredShapeFactory factory;
            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_TRUE( className.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_Empty" )
        try
        {
            TColoredShapeFactory factory;
            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_AfterUnregisterAll_IsEmpty" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.UnregisterAllConcreteFactories();

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ReRegisterAfterUnregister" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;

            factory.RegisterConcreteFactory( "MyShape", &circleFactory );
            factory.UnregisterConcreteFactory( "MyShape" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "MyShape" ) );

            // Re-register with a different concrete type under the same key
            factory.RegisterConcreteFactory( "MyShape", &squareFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "MyShape" ) );

            TColoredShapePtr ptr = factory.Create( "MyShape", "Teal" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            // Now "MyShape" key maps to CTestColoredSquare
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredSquare" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Teal" );
            }

            factory.UnregisterConcreteFactory( "MyShape" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ThreeTypes_ObtainKeySet" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            TColoredTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );
            factory.RegisterConcreteFactory( "ColoredTriangle", &triangleFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 3 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredCircle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredSquare" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredTriangle" ) != keys.end() );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredSquare" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredTriangle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredHexagon" ) );

            TColoredShapePtr tri = factory.Create( "ColoredTriangle", "Gold" );
            GUCEF_TESTFW_ASSERT_FALSE( tri.IsNULL() );
            if ( !tri.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( tri->GetShapeType() == "ColoredTriangle" );
                GUCEF_TESTFW_ASSERT_TRUE( tri->GetColor() == "Gold" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
            factory.UnregisterConcreteFactory( "ColoredTriangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IntKeySelector" )
        try
        {
            typedef CTAbstractFactoryWithParam< Int32, ITestColoredShape, CString, MT::CMutex >        TIntColoredShapeFactory;
            typedef CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex >   TIntColoredCircleFactory;
            typedef TIntColoredShapeFactory::TProductPtr                                               TIntColoredShapePtr;

            TIntColoredShapeFactory factory;
            TIntColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( 10, &circleFactory );

            TIntColoredShapePtr ptr = factory.Create( 10, "Orange" );
            TIntColoredShapePtr missing = factory.Create( 99, "Orange" );

            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            GUCEF_TESTFW_ASSERT_TRUE( missing.IsNULL() );
            if ( !ptr.IsNULL() )
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Orange" );

            factory.UnregisterConcreteFactory( 10 );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ProductConstructedWithoutFactoryLock: a create waits for a registration that holds the lock of the factory; a product whose construction waits for a lock that another thread holds while it creates through the same factory: that create is not held up" )
        try
        {
            ResetTestGates();
            TestProductConstructedWithoutFactoryLock< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterWaitsForCreateUnderWay: unregistering a concrete factory returns only once a create through it is done, the concrete factory may be destroyed then once its products are gone" )
        try
        {
            ResetTestGates();
            TestUnregisterWaitsForCreateUnderWay< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterWaitsOnlyForCreatesOfItsFactory: unregistering a concrete factory does not wait for a create through another one, also not for one that starts while it waits" )
        try
        {
            ResetTestGates();
            TestUnregisterWaitsOnlyForCreatesOfItsFactory< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterAllWaitsForCreateUnderWay: unregistering every concrete factory returns only once a create under way is done" )
        try
        {
            ResetTestGates();
            TestUnregisterAllWaitsForCreateUnderWay< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_DestructorWaitsForCreateUnderWay: the destruction of the abstract factory waits for a create under way" )
        try
        {
            ResetTestGates();
            TestDestructorWaitsForCreateUnderWay< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterFromWithinOwnCreate: a concrete factory unregistered from within a create through it does not hang that create, an owned one is kept until the abstract factory is destroyed as the product goes through it" )
        try
        {
            ResetTestGates();
            TestUnregisterFromWithinOwnCreate< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateThatThrowsLeavesNoCreateUnderWay: after a product constructor threw, unregistering its concrete factory neither waits nor is put off, on this thread and on another one" )
        try
        {
            TestCreateThatThrowsLeavesNoCreateUnderWay< STestPlainForm >();
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreatesRacingUnregistration: creates on two threads while a concrete factory is registered, unregistered and taken as gone over and over: no create uses it after its unregistration returned" )
        try
        {
            TestCreatesRacingUnregistration< STestPlainForm >();
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ReplaceWaitsForCreateUnderWay: registering a concrete factory for a type that has one replaces it and returns only once a create through the replaced one is done; the replaced one is not deleted, also not when owned" )
        try
        {
            ResetTestGates();
            TestReplaceWaitsForCreateUnderWay< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ReplaceFromWithinOwnCreate: a concrete factory replaced from within a create through it does not hang that create" )
        try
        {
            ResetTestGates();
            TestReplaceFromWithinOwnCreate< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_RemovalCycleWithOwnedFactories: two creates whose products unregister each other's type end when the abstract factory owns the concrete factories, which stay until it is destroyed" )
        try
        {
            ResetTestGates();
            TestRemovalCycleWithOwnedFactories< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_LongRemovalWaitIsLogged: a removal that waits long for a create logs a warning naming the type" )
        try
        {
            ResetTestGates();
            TestLongRemovalWaitIsLogged< STestPlainForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ThreadIdentity: the thread identity the abstract factories count creates by stays the same within a thread and differs between live threads" )
        try
        {
            TestThreadIdentity();
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ProductConstructedWithoutFactoryLock: a create waits for a registration that holds the lock of the factory; a product whose construction waits for a lock that another thread holds while it creates through the same factory: that create is not held up" )
        try
        {
            ResetTestGates();
            TestProductConstructedWithoutFactoryLock< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterWaitsForCreateUnderWay: unregistering a concrete factory returns only once a create through it is done, the concrete factory may be destroyed then once its products are gone" )
        try
        {
            ResetTestGates();
            TestUnregisterWaitsForCreateUnderWay< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterWaitsOnlyForCreatesOfItsFactory: unregistering a concrete factory does not wait for a create through another one, also not for one that starts while it waits" )
        try
        {
            ResetTestGates();
            TestUnregisterWaitsOnlyForCreatesOfItsFactory< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterAllWaitsForCreateUnderWay: unregistering every concrete factory returns only once a create under way is done" )
        try
        {
            ResetTestGates();
            TestUnregisterAllWaitsForCreateUnderWay< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_DestructorWaitsForCreateUnderWay: the destruction of the abstract factory waits for a create under way" )
        try
        {
            ResetTestGates();
            TestDestructorWaitsForCreateUnderWay< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterFromWithinOwnCreate: a concrete factory unregistered from within a create through it does not hang that create, an owned one is kept until the abstract factory is destroyed as the product goes through it" )
        try
        {
            ResetTestGates();
            TestUnregisterFromWithinOwnCreate< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateThatThrowsLeavesNoCreateUnderWay: after a product constructor threw, unregistering its concrete factory neither waits nor is put off, on this thread and on another one" )
        try
        {
            TestCreateThatThrowsLeavesNoCreateUnderWay< STestParamForm >();
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreatesRacingUnregistration: creates on two threads while a concrete factory is registered, unregistered and taken as gone over and over: no create uses it after its unregistration returned" )
        try
        {
            TestCreatesRacingUnregistration< STestParamForm >();
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ReplaceWaitsForCreateUnderWay: registering a concrete factory for a type that has one replaces it and returns only once a create through the replaced one is done; the replaced one is not deleted, also not when owned" )
        try
        {
            ResetTestGates();
            TestReplaceWaitsForCreateUnderWay< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ReplaceFromWithinOwnCreate: a concrete factory replaced from within a create through it does not hang that create" )
        try
        {
            ResetTestGates();
            TestReplaceFromWithinOwnCreate< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_RemovalCycleWithOwnedFactories: two creates whose products unregister each other's type end when the abstract factory owns the concrete factories, which stay until it is destroyed" )
        try
        {
            ResetTestGates();
            TestRemovalCycleWithOwnedFactories< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_LongRemovalWaitIsLogged: a removal that waits long for a create logs a warning naming the type" )
        try
        {
            ResetTestGates();
            TestLongRemovalWaitIsLogged< STestParamForm >();
        }
        catch( ... )
        {
            ResetTestGates();
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/

/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#include "gucefWEB_IWebViewEngine.h"
#define GUCEF_WEB_IWEBVIEWENGINE_H
#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMAND_H
#include "gucefWEB_CWebViewCommand.h"
#define GUCEF_WEB_CWEBVIEWCOMMAND_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMAND_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEHOST_H
#include "gucefWEB_CWebViewEngineHost.h"
#define GUCEF_WEB_CWEBVIEWENGINEHOST_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEHOST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebViewBase;
class CIWebViewEngineLoop;

/**
 *  The part of an engine every engine shares: the engine thread (CWebViewEngineHost, started at the first view on the
 *  pool "WebView.<engine>"), the views of the engine by id, view creation, command dispatch, ticks, the watchdog and
 *  shutdown. An engine implements CreateViewOnEngineThread() and, when it has a foreign loop, CreateEngineLoop(),
 *  OnEngineThreadStart() (gtk_init() and the like) and OnEngineThreadStop().
 *
 *  A derived engine calls ShutdownEngine() in its own destructor: the engine thread calls its virtual functions, which
 *  are gone once the derived destructor has run.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewEngineBase : public IWebViewEngine
{
    public:

    explicit CWebViewEngineBase( const CString& engineName );

    virtual ~CWebViewEngineBase();

    virtual const CString& GetEngineName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CFutureResult CreateViewAsync( const CWebViewSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Shutdown( UInt32 timeoutMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanUnload( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CheckWatchdog( UInt32 watchdogMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsStalled( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsEngineThread( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetViewCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** True once Shutdown() began */
    bool IsShuttingDown( void ) const;

    /** Whether the engine thread runs */
    bool IsRunning( void ) const;

    /** The host of the engine thread, NULL before the first view; for tests and diagnostics */
    TWebViewEngineHostPtr GetHost( void ) const;

    /*
     *  For views and the host
     */

    /** Any thread: posts a command of a view with a result; a future that failed at once when the engine cannot take it */
    CORE::CFutureResult PostCommand( CWebViewCommand& command );

    /** Any thread: posts a fire and forget command; false when the engine cannot take it */
    bool PostCommandWithoutResult( const CWebViewCommand& command );

    /** Engine thread: carries out a command */
    void ExecuteCommand( CWebViewCommand& command );

    /** Engine thread: deadlines of the views */
    void OnEngineTick( UInt64 nowMs );

    /** Engine thread: how long the thread may sleep before a deadline */
    UInt32 GetTickWaitMs( UInt64 nowMs );

    /** Engine thread: the thread started; calls OnEngineThreadStart() */
    bool OnEngineThreadStarted( void );

    /** Engine thread: the thread ends; closes what is left and calls OnEngineThreadStop() */
    void OnEngineThreadStopping( void );

    /** A future that has ended already, with a result of the given error and text */
    static CORE::CFutureResult MakeEndedFuture( TWebViewError error, const CString& text );

    protected:

    /** The loop of the engine thread; the default sleeps on a semaphore (CWebViewSemaphoreEngineLoop). Owned by this object */
    virtual CIWebViewEngineLoop* CreateEngineLoop( void );

    /** Engine thread: initialise the engine; false when it cannot run */
    virtual bool OnEngineThreadStart( void );

    /** Engine thread: release the engine; every view is closed already */
    virtual void OnEngineThreadStop( void );

    /** Engine thread: makes a view; NULL with 'problem' when it cannot */
    virtual CWebViewBase* CreateViewOnEngineThread( const CWebViewSettings& settings, UInt32 viewId, CString& problem ) = 0;

    /** For derived destructors: Shutdown() with the time out of the global settings */
    void ShutdownEngine( void );

    private:

    bool EnsureHost( CString& problem );

    void CloseAllViews( TWebViewError reason );

    CWebViewEngineBase( const CWebViewEngineBase& src );                /**< not implemented */
    CWebViewEngineBase& operator=( const CWebViewEngineBase& src );     /**< not implemented */

    typedef GUCEF::map< UInt32, TWebViewPtr > TViewMap;
    typedef GUCEF::map< UInt32, CWebViewSettings > TSettingsMap;

    private:

    const CString m_engineName;
    mutable MT::CMutex m_lock;
    TWebViewEngineHostPtr m_host;
    CIWebViewEngineLoop* m_loop;
    TViewMap m_views;
    TSettingsMap m_pendingCreations;
    UInt32 m_nextViewId;
    UInt32 m_nextRequestId;
    MT::CTAtomic< UInt32 > m_shuttingDown;
    bool m_abandoned;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

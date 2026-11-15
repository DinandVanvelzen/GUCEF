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

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#define GUCEF_WEB_CWEBVIEWGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H
#include "gucefWEB_CWebViewGlobalSettings.h"
#define GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINESELECTION_H
#include "gucefWEB_CWebViewEngineSelection.h"
#define GUCEF_WEB_CWEBVIEWENGINESELECTION_H
#endif /* GUCEF_WEB_CWEBVIEWENGINESELECTION_H ? */

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

class CWebGlobal;
class CWebViewEngineRegistry;

/**
 *  The globals of the web views of gucefWEB: the registry of engines, the web view settings, and the threads that must
 *  never wait for a web view (the engine threads, the GUI thread). Owned by CWebGlobal, the global of the module: made when
 *  the module is first used (CWebGlobal::Instance() or Instance() here) and released with it when the module unloads.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewGlobal
{
    public:

    static CWebViewGlobal* Instance( void );

    CWebViewEngineRegistry& GetEngineRegistry( void );

    CWebViewGlobalSettings GetSettings( void ) const;

    void SetSettings( const CWebViewGlobalSettings& settings );

    /** The name of the application, for the default profile folders ($APPDATADIR$/<application>/WebView/...) */
    void SetApplicationName( const CString& applicationName );

    CString GetApplicationName( void ) const;

    /**
     *  The first engine, in the order of preference ('preferredOrder', else the engineOrder setting, else registration
     *  order), that is available and has all of 'requiredCapabilities'. 'selection' says why each other engine did not
     *  qualify. Last resort engines (ChromeDevTools) only when the settings allow them. False when none qualifies.
     */
    bool FindEngine( UInt32 requiredCapabilities, const CString::StringVector& preferredOrder, CWebViewEngineSelection& selection );

    /** Runs the watchdog of every engine (any thread, cheap); the number of engines that are stalled */
    UInt32 CheckEngineWatchdogs( void );

    /** Closes the views and stops the threads of every engine, for the stop of the application before plugins unload */
    void Shutdown( void );

    /** Marks a thread as one that must never wait for a web view (CWebViewCommandResult::Await() refuses there) */
    void SetThreadMustNotWait( TThreadId threadId, bool mustNotWait );

    bool MayWaitOnCurrentThread( void ) const;

    /** Whether an engine name is one of a last resort engine */
    static bool IsLastResortEngine( const CString& engineName );

    private:

    friend class CWebGlobal;

    CWebViewGlobal( void );

    ~CWebViewGlobal();

    void Initialize( void );

    CWebViewGlobal( const CWebViewGlobal& src );            /**< not implemented */
    CWebViewGlobal& operator=( const CWebViewGlobal& src ); /**< not implemented */

    typedef GUCEF::set< TThreadId > TThreadIdSet;

    private:

    mutable MT::CMutex m_lock;
    CWebViewEngineRegistry* m_registry;
    CWebViewGlobalSettings m_settings;
    CString m_applicationName;
    TThreadIdSet m_mustNotWait;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

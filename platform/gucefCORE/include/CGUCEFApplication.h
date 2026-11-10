/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#define GUCEF_CORE_CGUCEFAPPLICATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#include "CISysConsoleCmdHandler.h"
#define GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#endif /* GUCEF_CORE_CISYSCONSOLECMDHANDLER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#include "gucefCORE_CBusyWaitPulseGeneratorDriver.h"
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

#ifndef GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H
#include "gucefCORE_CEmscriptenPulseGeneratorDriver.h"
#define GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CEMSCRIPTENPULSEGENERATORDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *
 *      Dispatches the following events:
 *       - AppInitEvent:
 *              Indicates that the application is now initializing
 *       - AppShutdownEvent
 *              Signals that the application is shutting down at the
 *              GUCEF level. All higher level code should shutdown as well
 */
class GUCEF_CORE_PUBLIC_CPP CGUCEFApplication : public CTSGNotifier                   ,
                                                public CGloballyConfigurable          ,
                                                public virtual CISysConsoleCmdHandler
{
    public:

    static const CEvent AppInitEvent;
    static const CEvent AppShutdownEvent;
    static const CEvent AppShutdownAbortedEvent;
    static const CEvent AppShutdownCompleteEvent;
    static const CEvent FirstCycleEvent;

    static void RegisterEvents( void );

    struct SAppInitEventData
    {
        #ifdef GUCEF_MSWIN_BUILD
        HINSTANCE hinstance;
        LPSTR lpcmdline;
        int ncmdshow;
        #endif

        int argc;
        char** argv;
    };
    typedef CTCloneableObj< struct SAppInitEventData > TAppInitEventData;

    public:

    #ifdef GUCEF_MSWIN_BUILD
    int Main( HINSTANCE hinstance     ,
              LPSTR lpcmdline         ,
              int ncmdshow            ,
              bool run                );
    #endif

    int main( int argc    ,
              char** argv ,
              bool run    );


    void Update( void );

    bool Stop( bool wait = false );

    /**
     *  Asks the application to stop, from a signal handler (SIGINT, SIGTERM) or a console control handler.
     *  Async-signal-safe: it only stores the signal number in a sig_atomic_t, nothing else. The application acts on
     *  it on its pulse thread: at the next pulse of its pulse generator Stop() is called there, and main() returns at
     *  once without running when the request came before it. Calling it again changes nothing.
     *  @param signal the signal number, logged when the stop is made; 0 is taken as SIGTERM
     */
    static void RequestStopFromSignal( int signal );

    /** Whether RequestStopFromSignal() was called in this process */
    static bool IsStopRequestedFromSignal( void );

    /** The signal given to RequestStopFromSignal(), 0 when it was not called */
    static int GetStopRequestSignal( void );

    CString GetApplicationDir( void ) const;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @return whether storing the tree was successful
     */
    virtual bool SaveConfig( CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successful
     */
    virtual bool LoadConfig( const CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetForcedMinimalCycleDeltaInMilliSecs( UInt32 cycleDelta );

    UInt32 GetForcedMinimalCycleDeltaInMilliSecs( void ) const;

    void SetDesiredMaximumCycleDeltaInMilliSecs( UInt32 cycleDelta );

    UInt32 GetDesiredMaximumCycleDeltaInMilliSecs( void ) const;

    virtual void SetPulseGenerator( PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnSysConsoleCommand( const CString& path               ,
                                      const CString& command            ,
                                      const CString::StringVector& args ,
                                      CString::StringVector& resultdata ) GUCEF_VIRTUAL_OVERRIDE;

    /** Acts on a stop requested from a signal at every pulse, passes everything else on */
    virtual void OnPumpedNotify( CNotifier* notifier                 ,
                                 const CEvent& eventid               ,
                                 CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CGUCEFApplication( void );

    virtual ~CGUCEFApplication();

    bool LinkPulseGeneratorDriver( void );
    
    private:

    CGUCEFApplication( const CGUCEFApplication& src );            /**< not implemented */
    
    CGUCEFApplication& operator=( const CGUCEFApplication& src ); /**< not implemented */

    int MainLoop( void );

    /** Stops the application when a stop was requested from a signal and that was not acted on yet; on the pulse thread */
    bool StopIfRequestedFromSignal( void );

    private:

    bool _initialized;
    bool m_shutdownRequested;
    bool m_signalStopHandled;
    CString _appdir;
    CBusyWaitPulseGeneratorDriver m_busyWaitPulseDriver;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    CEmscriptenPulseGeneratorDriver m_emscriptenPulseDriver;
    #endif
    UInt32 m_forcedMinimalCycleDeltaInMilliSecs; 
    UInt32 m_desiredMaximumCycleDeltaInMilliSecs;
};


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* CGUCEFAPPLICATION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-08-2005 :
        - Added GetCycleDelay()
- 12-11-2004 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/

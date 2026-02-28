/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <assert.h>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"   /* Additional utilities for manipulating string class objects */
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_TSPRINTING_H
#include "tsprinting.h"         /* centralized console output */
#define GUCEF_CORE_TSPRINTING_H
#endif /* GUCEF_CORE_TSPRINTING_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"     /* centralized plugin control */
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"          /* node for data storage */
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLE_H
#include "CSysConsole.h"
#define GUCEF_CORE_CSYSCONSOLE_H
#endif /* GUCEF_CORE_CSYSCONSOLE_H ? */

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "CGUCEFApplication.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CGUCEFApplication::AppInitEvent = "GUCEF::CORE::CGUCEFApplication::AppInitEvent";
const CEvent CGUCEFApplication::AppShutdownEvent = "GUCEF::CORE::CGUCEFApplication::AppShutdownEvent";
const CEvent CGUCEFApplication::AppShutdownAbortedEvent = "GUCEF::CORE::CGUCEFApplication::AppShutdownAbortedEvent";
const CEvent CGUCEFApplication::AppShutdownCompleteEvent = "GUCEF::CORE::CGUCEFApplication::AppShutdownCompleteEvent";
const CEvent CGUCEFApplication::FirstCycleEvent = "GUCEF::CORE::CGUCEFApplication::FirstCycleEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUCEFApplication::CGUCEFApplication( void )
    : CGloballyConfigurable( false )
    , CTSGNotifier()
    , _initialized( false )
    , m_shutdownRequested( false )
    , m_busyWaitPulseDriver()
    , m_forcedMinimalCycleDeltaInMilliSecs( 25 )
    , m_desiredMaximumCycleDeltaInMilliSecs( 100 )
{GUCEF_TRACE;

    RegisterEvents();

    /*
     *      Set an initial app dir just in case we have trouble getting one
     *      at a later stage
     */
    _appdir = RelativePath( "$MODULEDIR$" );

    /*
     *      Register some functionality at the system console
     */
    CSysConsole* sysconsole = &CORE::CCoreGlobal::Instance()->GetSysConsole();
    if ( GUCEF_NULL != sysconsole )
    {
        CString::StringVector args;
        sysconsole->RegisterCmd( "GUCEF\\CORE\\CGUCEFApplication" ,
                                 "Stop"                           ,
                                 args                             ,
                                 this                             );
        sysconsole->RegisterCmd( "GUCEF\\CORE\\CGUCEFApplication" ,
                                 "GetApplicationDir"              ,
                                 args                             ,
                                 this                             );
        sysconsole->RegisterAlias( "exit"                           ,
                                   "GUCEF\\CORE\\CGUCEFApplication" ,
                                   "Stop"                           );
    }
}

/*-------------------------------------------------------------------------*/

CGUCEFApplication::~CGUCEFApplication()
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
    {
        CIPulseGeneratorDriver* pulseDriver = pulseGenerator->GetPulseGeneratorDriver();
        if ( &m_busyWaitPulseDriver == pulseDriver )
        {
            // decouple the pulse driver since we dont want to get tripped up with
            // order of destruction
            pulseGenerator->SetPulseGeneratorDriver( GUCEF_NULL );
        }
    }

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFApplication::LinkPulseGeneratorDriver( void )
{GUCEF_TRACE;

    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
    {
        CIPulseGeneratorDriver* pulseDriver = pulseGenerator->GetPulseGeneratorDriver();
        if ( GUCEF_NULL == pulseDriver )
        {
            // Set the busy wait pulse driver as the default driver for the
            // pulse generator
            pulseGenerator->SetPulseGeneratorDriver( &m_busyWaitPulseDriver );
        }
        pulseGenerator->AllowPeriodicPulses();
        pulseGenerator->RequestPeriodicPulses( this );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

int
CGUCEFApplication::MainLoop( void )
{GUCEF_TRACE;

    Lock();
    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
    {
        LinkPulseGeneratorDriver();

        // Unless the pulse drive is overridden we will now start the loop
        if ( pulseGenerator->GetPulseGeneratorDriver() == &m_busyWaitPulseDriver )
        {
            Unlock();

            // Notify that the first app cycle is starting
            if ( !NotifyObservers( FirstCycleEvent ) ) return 0;

            // Now keep looping until we are externally triggered to break out of the loop
            m_busyWaitPulseDriver.Run( *pulseGenerator.GetPointerAlways(), m_forcedMinimalCycleDeltaInMilliSecs, m_desiredMaximumCycleDeltaInMilliSecs );
            return 0;
        }
        else
        {
            Unlock();
        }
    }
    else
    {
        Unlock();
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFApplication::SetForcedMinimalCycleDeltaInMilliSecs( UInt32 cycleDelta )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_forcedMinimalCycleDeltaInMilliSecs = cycleDelta;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUCEFApplication::GetForcedMinimalCycleDeltaInMilliSecs( void ) const
{GUCEF_TRACE;

    return m_forcedMinimalCycleDeltaInMilliSecs;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFApplication::SetDesiredMaximumCycleDeltaInMilliSecs( UInt32 cycleDelta )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_desiredMaximumCycleDeltaInMilliSecs = cycleDelta;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUCEFApplication::GetDesiredMaximumCycleDeltaInMilliSecs( void ) const
{GUCEF_TRACE;

    return m_desiredMaximumCycleDeltaInMilliSecs;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFApplication::SetPulseGenerator( PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    LinkPulseGeneratorDriver();
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD
int
CGUCEFApplication::Main( HINSTANCE hinstance     ,
                         LPSTR lpcmdline         ,
                         int ncmdshow            ,
                         bool run                )
{GUCEF_TRACE;

    /*
     *      Set the application dir
     *      This is pure MSWIN code and is not portable !!!
     */
    {
        /*
         *      Set the given values as environment vars
         */
        SetEnv( "HINSTANCE", PointerToString( hinstance ) );

        char apppath[ MAX_PATH ];

        if ( GetModuleFileNameA( hinstance, apppath, MAX_PATH ) )
        {
                _appdir = apppath;
        }
    }

    struct SAppInitEventData data;

    // Copy the MSWIN params
    data.hinstance = hinstance;
    data.lpcmdline = lpcmdline;
    data.ncmdshow = ncmdshow;

    // Parse the MSWIN param into the old style param list
    CString::StringVector argList;
    if ( GUCEF_NULL != lpcmdline )
    {
        argList = CString( lpcmdline ).ParseElements( ' ' );
    }

    data.argc = (int)argList.size();
    SetEnv( "argc", Int32ToString( data.argc ) );
    if ( data.argc > 0 )
    {
        data.argv = GUCEF_NEW char*[ data.argc ];
        for ( Int32 i=0; i<data.argc; ++i )
        {
            data.argv[ i ] = const_cast< char* >( argList[ i ].C_String() );
        }
        SetEnv( "argv", lpcmdline );
    }
    else
    {
        data.argv = NULL;
    }

    // Set shutdown to false before we emit the init event so that observers can
    // request a shutdown if something went wrong
    m_shutdownRequested = false;

    // Dispatch the initialization event
    TAppInitEventData cloneableData( data );
    if ( !NotifyObservers( AppInitEvent, &cloneableData ) ) return 0;

    // Check if we need to abort because a shutdown was requested
    int appReturnCode = 0;
    if ( !m_shutdownRequested )
    {
        _initialized = true;
        if ( run )
            appReturnCode = MainLoop();
        else
        {
            // Notify that the first app cycle is starting
            if ( !NotifyObservers( FirstCycleEvent ) ) return 0;
        }
    }

    GUCEF_DELETE []data.argv;
    return appReturnCode;
}
#endif

/*-------------------------------------------------------------------------*/

int
CGUCEFApplication::main( int argc    ,
                         char** argv ,
                         bool run    )
{GUCEF_TRACE;

    /*
     *      Set the given values as environment vars
     */
    SetEnv( "argc", Int32ToString( argc ) );
    if ( argc > 0 )
    {
        CString argvStr;
        for ( int i=0; i<argc; ++i )
        {
            argvStr += argv[ i ];
            if ( i+1<argc )
                argvStr += ' ';
        }
        SetEnv( "argv", argvStr );
    }

    _initialized = false;

    // Set shutdown to false before we emit the init event so that observers can
    // request a shutdown if something went wrong
    m_shutdownRequested = false;

    /*
     *      We now know we have an instance of this singleton and can begin
     *      our main() code. We will send the application initialization
     *      event to all event clients. The following code segment is a
     *      special case since it may be followed by the main application
     *      loop which would keep anything statically allocated here in memory
     *      Thus we turn the following into a compound statement.
     */
    {
        struct SAppInitEventData data;

        data.argc = argc;
        data.argv = argv;

        TAppInitEventData cloneableData( data );
        if ( !NotifyObservers( AppInitEvent, &cloneableData ) ) return 0;
    }

    // Check if we need to abort because a shutdown was requested
    int appReturnCode = 0;
    if ( !m_shutdownRequested )
    {
        _initialized = true;
        if ( run )
            appReturnCode = MainLoop();
        else
        {
            // Notify that the first app cycle is starting
            if ( !NotifyObservers( FirstCycleEvent ) ) return 0;
        }
    }
    return appReturnCode;
}

/*-------------------------------------------------------------------------*/

void
CGUCEFApplication::RegisterEvents( void )
{GUCEF_TRACE;

    AppInitEvent.Initialize();
    AppShutdownEvent.Initialize();
    AppShutdownAbortedEvent.Initialize();
    AppShutdownCompleteEvent.Initialize();
    FirstCycleEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CGUCEFApplication::Update( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
    if ( !pulseGenerator.IsNULL() )
        pulseGenerator->RequestImmediatePulse();
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFApplication::Stop( bool wait )
{GUCEF_TRACE;

    {
        MT::CObjectScopeLock lock( this );

        if ( !m_shutdownRequested )
        {
            if ( !NotifyObservers( AppShutdownEvent ) ) return false;

            PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
            if ( !pulseGenerator.IsNULL() )
            {
                pulseGenerator->ForceStopOfPeriodicPulses();

                // Once last round for pumped observers
                pulseGenerator->ForceDirectPulse();
            }
        }
    }

    if ( CCoreGlobal::Instance()->GetTaskManager().RequestAllThreadsToStop( wait, false, GUCEF_MT_SUPER_LONG_LOCK_TIMEOUT ) )
    {
        if ( !m_shutdownRequested )
        {
            m_shutdownRequested = true;
            if ( !NotifyObservers( AppShutdownCompleteEvent ) ) return false;

            PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
            if ( !pulseGenerator.IsNULL() )
            {
                // Once last round for pumped observers
                pulseGenerator->ForceDirectPulse();
            }
        }

        CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        
        return true;
    }
    else
    {
        NotifyObservers( AppShutdownAbortedEvent );
        return false;
    }    
}

/*-------------------------------------------------------------------------*/

CString
CGUCEFApplication::GetApplicationDir( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return _appdir;
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFApplication::SaveConfig( CDataNode& tree )  const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    CDataNode* n = tree.Structure( "GUCEF%CORE%CGUCEFApplication" ,
                                   '%'                            );
    n->SetAttribute( "appdir" ,
                     _appdir  );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFApplication::LoadConfig( const CDataNode& tree )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    GUCEF_SYSTEM_LOG( LOGLEVEL_BELOW_NORMAL, "CGUCEFApplication: Loading config" );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGUCEFApplication::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CGUCEFApplication";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFApplication::OnSysConsoleCommand( const CString& path               ,
                                        const CString& command            ,
                                        const CString::StringVector& args ,
                                        CString::StringVector& resultdata )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( command == "Stop" )
    {
        Stop();
        return true;
    }
    else
    if ( command == "GetApplicationDir" )
    {
        resultdata.push_back( GetApplicationDir() );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

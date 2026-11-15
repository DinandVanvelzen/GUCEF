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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <signal.h>
#include <string.h>

#include "gucefCORE_CServiceStopSignals.h"

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CServiceStopSignals::StopRequestedEvent = "GUCEF::CORE::CServiceStopSignals::StopRequestedEvent";

/*-------------------------------------------------------------------------*/

namespace {

/** The signal noted last: written by the handler, read by the service's thread */
volatile sig_atomic_t g_pendingStopSignal = 0;

bool g_handlersInstalled = false;

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

struct sigaction g_previousTermAction;
struct sigaction g_previousIntAction;

/** The handler: one store, nothing else is safe in a signal handler */
void
StopSignalHandler( int signalNumber )
{
    CServiceStopSignals::NoteSignal( signalNumber );
}

#else

BOOL WINAPI
StopConsoleControlHandler( DWORD controlType )
{
    switch ( controlType )
    {
        case CTRL_C_EVENT:
        {
            CServiceStopSignals::NoteSignal( SIGINT );
            return TRUE;
        }
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            CServiceStopSignals::NoteSignal( SIGTERM );
            return TRUE;
        }
        default:
            return FALSE;
    }
}

#endif

}

/*-------------------------------------------------------------------------*/

void
CServiceStopSignals::RegisterEvents( void )
{GUCEF_TRACE;

    StopRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CServiceStopSignals::NoteSignal( int signalNumber )
{
    // signal context: no trace, no lock, no allocation
    g_pendingStopSignal = (sig_atomic_t) ( 0 != signalNumber ? signalNumber : SIGTERM );
}

/*-------------------------------------------------------------------------*/

int
CServiceStopSignals::GetPendingSignal( void )
{GUCEF_TRACE;

    return (int) g_pendingStopSignal;
}

/*-------------------------------------------------------------------------*/

void
CServiceStopSignals::Reset( void )
{GUCEF_TRACE;

    g_pendingStopSignal = 0;
}

/*-------------------------------------------------------------------------*/

bool
CServiceStopSignals::InstallHandlers( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )

    struct sigaction action;
    memset( &action, 0, sizeof( action ) );
    action.sa_handler = &StopSignalHandler;
    sigemptyset( &action.sa_mask );
    action.sa_flags = 0;

    struct sigaction previousTerm;
    struct sigaction previousInt;
    if ( 0 != sigaction( SIGTERM, &action, &previousTerm ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ServiceStopSignals: the handler of SIGTERM cannot be installed" );
        return false;
    }
    if ( 0 != sigaction( SIGINT, &action, &previousInt ) )
    {
        sigaction( SIGTERM, &previousTerm, GUCEF_NULL );
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ServiceStopSignals: the handler of SIGINT cannot be installed" );
        return false;
    }
    if ( !g_handlersInstalled )
    {
        // what was there before the first install is what RestoreHandlers() puts back
        g_previousTermAction = previousTerm;
        g_previousIntAction = previousInt;
    }
    g_handlersInstalled = true;
    GUCEF_LOG( LOGLEVEL_NORMAL, "ServiceStopSignals: SIGTERM and SIGINT are noted for an orderly stop" );
    return true;

    #else

    if ( FALSE == ::SetConsoleCtrlHandler( &StopConsoleControlHandler, TRUE ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "ServiceStopSignals: the console control handler cannot be installed" );
        return false;
    }
    g_handlersInstalled = true;
    GUCEF_LOG( LOGLEVEL_NORMAL, "ServiceStopSignals: the console control events are noted for an orderly stop" );
    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
CServiceStopSignals::RestoreHandlers( void )
{GUCEF_TRACE;

    if ( !g_handlersInstalled )
        return false;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    bool ok = 0 == sigaction( SIGTERM, &g_previousTermAction, GUCEF_NULL );
    ok = 0 == sigaction( SIGINT, &g_previousIntAction, GUCEF_NULL ) && ok;
    #else
    bool ok = FALSE != ::SetConsoleCtrlHandler( &StopConsoleControlHandler, FALSE );
    #endif

    g_handlersInstalled = false;
    return ok;
}

/*-------------------------------------------------------------------------*/

CServiceStopSignals::CServiceStopSignals( PulseGeneratorPtr pulseGenerator )
    : CTSGNotifier( pulseGenerator.IsNULL() ? CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, true, false )
    , m_raised( false )
{GUCEF_TRACE;

    RegisterEvents();

    // every pulse comes to OnPumpedNotify(); periodic pulses so that a signal is seen without other activity
    SetPropagatePulseEvent( true );
    PulseGeneratorPtr pulse = GetPulseGenerator();
    if ( !pulse.IsNULL() )
        pulse->RequestPeriodicPulses( this );
}

/*-------------------------------------------------------------------------*/

CServiceStopSignals::~CServiceStopSignals()
{GUCEF_TRACE;

    PulseGeneratorPtr pulse = GetPulseGenerator();
    if ( !pulse.IsNULL() )
        pulse->RequestStopOfPeriodicUpdates( this );
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool
CServiceStopSignals::IsStopRequested( void ) const
{GUCEF_TRACE;

    return m_raised;
}

/*-------------------------------------------------------------------------*/

const CString&
CServiceStopSignals::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CServiceStopSignals";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CServiceStopSignals::OnPumpedNotify( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData )
{GUCEF_TRACE;

    if ( CPulseGenerator::PulseEvent == eventId )
    {
        const int signalNumber = (int) g_pendingStopSignal;
        if ( 0 != signalNumber && !m_raised )
        {
            m_raised = true;
            GUCEF_LOG( LOGLEVEL_IMPORTANT, "ServiceStopSignals: stop signal " << signalNumber << " received" );
            TCloneableInt32 signalData( (Int32) signalNumber );
            NotifyObservers( StopRequestedEvent, &signalData );
        }
        return;
    }
    CTSGNotifier::OnPumpedNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

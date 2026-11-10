/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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
#include <unistd.h>

#if defined( __GLIBC__ )
  #include <execinfo.h>
  #define GUCEF_PLUGINGLUE_WAYLAND_HAS_BACKTRACE
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pluginglueWAYLAND_CEventGuard.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Plain data only: read by a signal handler */
const char* volatile g_currentEvent = GUCEF_NULL;
const char* volatile g_lastEvent = GUCEF_NULL;
volatile sig_atomic_t g_dispatchDepth = 0;
UInt32 g_exceptionCount = 0;

const UInt32 MaxLoggedHandlers = 64;
const char* g_loggedHandlers[ MaxLoggedHandlers ];
UInt32 g_loggedHandlerCount = 0;

const int g_reportedSignals[] = { SIGABRT, SIGSEGV, SIGBUS, SIGFPE, SIGILL };
const UInt32 g_reportedSignalCount = sizeof( g_reportedSignals ) / sizeof( g_reportedSignals[ 0 ] );
struct sigaction g_previousActions[ sizeof( g_reportedSignals ) / sizeof( g_reportedSignals[ 0 ] ) ];
volatile sig_atomic_t g_reporterInstalled = 0;

/** The line of the fatal signal, made in memory that exists from the start */
char g_signalMessage[ 384 ];

/** Appends text to a buffer that has room for 'size' bytes, always leaving room for the end; returns the new length */
UInt32
Append( char* buffer, UInt32 size, UInt32 length, const char* text )
{
    if ( GUCEF_NULL == text )
        return length;
    while ( '\0' != *text && length + 1 < size )
        buffer[ length++ ] = *text++;
    return length;
}

UInt32
AppendNumber( char* buffer, UInt32 size, UInt32 length, UInt32 value )
{
    char digits[ 12 ];
    UInt32 count = 0;
    do
    {
        digits[ count++ ] = (char) ( '0' + value % 10 );
        value /= 10;
    }
    while ( 0 != value && count < sizeof( digits ) );
    while ( count > 0 && length + 1 < size )
        buffer[ length++ ] = digits[ --count ];
    return length;
}

void
FatalSignalHandler( int signalNumber, siginfo_t* info, void* )
{
    const UInt32 length = CEventGuard::FormatFatalSignalMessage( signalNumber, g_signalMessage, sizeof( g_signalMessage ) );
    const ssize_t written = ::write( STDERR_FILENO, g_signalMessage, length );
    (void) written;

    // where the process was: the address that was accessed and the functions that were running
    if ( GUCEF_NULL != info && ( SIGSEGV == signalNumber || SIGBUS == signalNumber ) )
    {
        static const char hex[] = "0123456789abcdef";
        char line[ 64 ];
        UInt32 n = 0;
        n = Append( line, sizeof( line ), n, "Address accessed: 0x" );
        const unsigned long address = (unsigned long) info->si_addr;
        bool started = false;
        for ( int shift = (int) ( sizeof( address ) * 8 ) - 4; shift >= 0; shift -= 4 )
        {
            const unsigned digit = (unsigned) ( ( address >> shift ) & 0xF );
            if ( started || 0 != digit || 0 == shift )
            {
                started = true;
                if ( n + 2 < sizeof( line ) )
                    line[ n++ ] = hex[ digit ];
            }
        }
        line[ n++ ] = '\n';
        const ssize_t addressWritten = ::write( STDERR_FILENO, line, n );
        (void) addressWritten;
    }
    #ifdef GUCEF_PLUGINGLUE_WAYLAND_HAS_BACKTRACE
    {
        void* frames[ 48 ];
        const int count = ::backtrace( frames, 48 );
        if ( count > 0 )
            ::backtrace_symbols_fd( frames, count, STDERR_FILENO );
    }
    #endif

    // the process goes on as it would have without us
    for ( UInt32 i=0; i<g_reportedSignalCount; ++i )
        if ( g_reportedSignals[ i ] == signalNumber )
            ::sigaction( signalNumber, &g_previousActions[ i ], GUCEF_NULL );
    ::raise( signalNumber );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEventGuard::CEventGuard( const char* eventName )
    : m_name( eventName )
    , m_previous( g_currentEvent )
{
    g_currentEvent = eventName;
    g_lastEvent = eventName;
}

/*-------------------------------------------------------------------------*/

CEventGuard::~CEventGuard()
{
    g_currentEvent = m_previous;
}

/*-------------------------------------------------------------------------*/

void
CEventGuard::OnException( const char* what )
{
    ++g_exceptionCount;
    for ( UInt32 i=0; i<g_loggedHandlerCount; ++i )
        if ( g_loggedHandlers[ i ] == m_name || ( GUCEF_NULL != m_name && GUCEF_NULL != g_loggedHandlers[ i ] && 0 == strcmp( g_loggedHandlers[ i ], m_name ) ) )
            return;
    if ( g_loggedHandlerCount < MaxLoggedHandlers )
        g_loggedHandlers[ g_loggedHandlerCount++ ] = m_name;
    const char* handlerName = GUCEF_NULL != m_name ? m_name : "?";
    const char* reason = GUCEF_NULL != what ? what : "unknown exception";
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CEventGuard: An exception was thrown in the handler of a Wayland event (" << handlerName
                     << "): " << reason << ". It is dropped; this is logged once for each handler" );
}

/*-------------------------------------------------------------------------*/

const char* CEventGuard::GetCurrentEvent( void ) { return g_currentEvent; }
const char* CEventGuard::GetLastEvent( void ) { return g_lastEvent; }
UInt32 CEventGuard::GetExceptionCount( void ) { return g_exceptionCount; }
bool CEventGuard::IsDispatching( void ) { return g_dispatchDepth > 0; }

/*-------------------------------------------------------------------------*/

void
CEventGuard::ResetExceptionLog( void )
{
    g_loggedHandlerCount = 0;
    g_exceptionCount = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CEventGuard::FormatFatalSignalMessage( int signalNumber, char* buffer, UInt32 bufferSize )
{
    if ( GUCEF_NULL == buffer || bufferSize < 2 )
        return 0;
    const char* current = g_currentEvent;
    const char* last = g_lastEvent;
    UInt32 length = 0;
    length = Append( buffer, bufferSize, length, "Wayland driver: fatal signal " );
    length = AppendNumber( buffer, bufferSize, length, signalNumber > 0 ? (UInt32) signalNumber : 0 );
    if ( GUCEF_NULL != current )
    {
        length = Append( buffer, bufferSize, length, " in " );
        length = Append( buffer, bufferSize, length, current );
    }
    else
    {
        if ( g_dispatchDepth > 0 )
            length = Append( buffer, bufferSize, length, " while libwayland dispatched events, outside of a handler of ours (the failing event is the one after the last one handled)" );
        else
            length = Append( buffer, bufferSize, length, " outside of the handling of Wayland events" );
        length = Append( buffer, bufferSize, length, ", last event handled: " );
        length = Append( buffer, bufferSize, length, GUCEF_NULL != last ? last : "none" );
    }
    length = Append( buffer, bufferSize, length, "\n" );
    buffer[ length ] = '\0';
    return length;
}

/*-------------------------------------------------------------------------*/

bool
CEventGuard::InstallFatalSignalReporter( void )
{
    if ( 0 != g_reporterInstalled )
        return true;
    g_reporterInstalled = 1;
    bool ok = true;

    #ifdef GUCEF_PLUGINGLUE_WAYLAND_HAS_BACKTRACE
    {
        // the first backtrace() loads the unwinder, which is not something to do in a signal handler
        void* frames[ 4 ];
        (void) ::backtrace( frames, 4 );
    }
    #endif
    for ( UInt32 i=0; i<g_reportedSignalCount; ++i )
    {
        struct sigaction action;
        memset( &action, 0, sizeof( action ) );
        action.sa_sigaction = &FatalSignalHandler;
        action.sa_flags = SA_SIGINFO | SA_NODEFER;
        ::sigemptyset( &action.sa_mask );
        memset( &g_previousActions[ i ], 0, sizeof( g_previousActions[ i ] ) );
        g_previousActions[ i ].sa_handler = SIG_DFL;
        if ( 0 != ::sigaction( g_reportedSignals[ i ], &action, &g_previousActions[ i ] ) )
            ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CEventGuard::RemoveFatalSignalReporter( void )
{
    if ( 0 == g_reporterInstalled )
        return false;
    for ( UInt32 i=0; i<g_reportedSignalCount; ++i )
    {
        // only what is ours is taken away: a handler that somebody installed after us stays
        struct sigaction current;
        memset( &current, 0, sizeof( current ) );
        if ( 0 == ::sigaction( g_reportedSignals[ i ], GUCEF_NULL, &current ) && current.sa_sigaction == &FatalSignalHandler )
            ::sigaction( g_reportedSignals[ i ], &g_previousActions[ i ], GUCEF_NULL );
    }
    g_reporterInstalled = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEventGuard::IsFatalSignalReporterInstalled( void )
{
    return 0 != g_reporterInstalled;
}

/*-------------------------------------------------------------------------*/

CDispatchPhase::CDispatchPhase( void )
{
    ++g_dispatchDepth;
}

/*-------------------------------------------------------------------------*/

CDispatchPhase::~CDispatchPhase()
{
    --g_dispatchDepth;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

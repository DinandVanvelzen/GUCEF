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

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H
#define GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <exception>

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The boundary between libwayland and the code of the drivers. The functions the compositor's events call are C callbacks
 *  that libwayland-client invokes from its own C frames: an exception that leaves one of them ends the process (there is no
 *  handler to unwind to), so every callback holds a guard, catches whatever the code under it throws, logs it once and goes
 *  on with the next event.
 *
 *  The guard also remembers which callback runs and which ran last. A fatal signal (see InstallFatalSignalReporter) names
 *  that callback on stderr, so a crash in an event is recognised as such without a debugger.
 *
 *  Everything is meant for the one thread that pumps the connection.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CEventGuard
{
    public:

    /** The name must outlive the guard (a string literal or __FUNCTION__) */
    explicit CEventGuard( const char* eventName );

    ~CEventGuard();

    /** An exception that reached the end of a handler is let go of here: logged the first time for each handler, counted always. 'what' may be NULL */
    void OnException( const char* what );

    /** The handler that runs now, NULL when none does */
    static const char* GetCurrentEvent( void );

    /** The handler that ran last or runs now, NULL when none ran */
    static const char* GetLastEvent( void );

    /** How many exceptions were dropped at the boundary */
    static UInt32 GetExceptionCount( void );

    /** Forgets what was logged, so the next exception of each handler is logged again (for tests) */
    static void ResetExceptionLog( void );

    /** True while libwayland is dispatching events (see CDispatchPhase) */
    static bool IsDispatching( void );

    /**
     *  Makes SIGABRT, SIGSEGV, SIGBUS, SIGFPE and SIGILL print one line to stderr before the process goes on as it would
     *  have without: the previous handler (or the default action) is restored and the signal raised again. Done once; what
     *  was installed before is chained to. The line says which event handler was running, or that libwayland was
     *  dispatching and no handler of ours was (the failing event is then the one after 'last event handled').
     *  The line is followed by the address that was accessed (for a segmentation fault) and a list of the functions that were
     *  running, as the C library gives it (backtrace()), for every fatal signal of the process: the reporter does not know whether
     *  the signal came from this library, a graphics driver or another module, the list says. Nothing but write() and backtrace_symbols_fd() are used inside the handler.
     */
    static bool InstallFatalSignalReporter( void );

    /**
     *  Puts back what the signals did before InstallFatalSignalReporter(), so that no handler of this library is left in a process
     *  that unloaded it (a signal after that would call code that is not there any more). False when nothing was installed.
     */
    static bool RemoveFatalSignalReporter( void );

    /** Whether the reporter is installed */
    static bool IsFatalSignalReporterInstalled( void );

    /** The line the reporter writes for a signal now, with a newline, in a buffer the caller gives; returns its length. Safe to call from a signal handler */
    static UInt32 FormatFatalSignalMessage( int signalNumber, char* buffer, UInt32 bufferSize );

    private:

    CEventGuard( void );
    CEventGuard( const CEventGuard& src );
    CEventGuard& operator=( const CEventGuard& src );

    private:

    const char* m_name;
    const char* m_previous;
};

/*-------------------------------------------------------------------------*/

/** Marks the time libwayland is dispatching events (it calls the handlers from there, and aborts from there on a listener it cannot call) */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CDispatchPhase
{
    public:

    CDispatchPhase( void );

    ~CDispatchPhase();

    private:

    CDispatchPhase( const CDispatchPhase& src );
    CDispatchPhase& operator=( const CDispatchPhase& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Around the body of a callback that libwayland calls:
 *
 *    static void PointerFrame( void* data, struct wl_pointer* )
 *    {
 *        GUCEF_WL_GUARD_BEGIN
 *        ...
 *        GUCEF_WL_GUARD_END
 *    }
 */
#undef GUCEF_WL_GUARD_BEGIN
#define GUCEF_WL_GUARD_BEGIN                                                                  \
    ::GUCEF::PLUGINGLUE::WAYLAND::CEventGuard eventGuard_( __FUNCTION__ );                    \
    try                                                                                       \
    {

#undef GUCEF_WL_GUARD_END
#define GUCEF_WL_GUARD_END                                                                    \
    }                                                                                         \
    catch ( const std::exception& guardedException_ )                                         \
    {                                                                                         \
        eventGuard_.OnException( guardedException_.what() );                                  \
    }                                                                                         \
    catch ( ... )                                                                             \
    {                                                                                         \
        eventGuard_.OnException( GUCEF_NULL );                                                \
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

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CEVENTGUARD_H ? */

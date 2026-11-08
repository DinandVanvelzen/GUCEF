/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H
#define GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* simple types */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

#ifndef GUCEF_MT_GUCEFMT_MACROS_H
#include "gucefMT_macros.h"             /* often used gucef macros */
#define GUCEF_MT_GUCEFMT_MACROS_H
#endif /* GUCEF_MT_GUCEFMT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Raises the calling thread to real-time scheduling for the life of the scope, for threads that must meet a
 *  deadline every few milliseconds (audio device threads). The destructor puts the scheduling of the thread back
 *  to what it was, so a pooled thread goes back to its pool as it came.
 *
 *  Linux and Android: SCHED_FIFO at the given priority, limited to the range of the system (1..99 on Linux).
 *                     Needs the right to do so: a RLIMIT_RTPRIO at or above the priority (limits.d "rtprio", or
 *                     LimitRTPRIO in a systemd unit) or CAP_SYS_NICE.
 *  Windows:           the multimedia class scheduler task "Pro Audio" (MMCSS, avrt.dll bound at run time); when
 *                     that is not available the thread priority THREAD_PRIORITY_TIME_CRITICAL. The priority
 *                     argument is not used.
 *  Elsewhere:         nothing happens and the scope is not active.
 *
 *  Not getting real-time scheduling is not an error of the program: the thread then simply keeps the scheduling it
 *  had and nothing changes. IsActive() and GetErrorCode() tell what happened, so that the owner of the thread can
 *  report it once (the scope itself never logs: it is made on threads that must not).
 *
 *  A scope belongs to the thread that made it and must be destroyed on that thread. Scopes may be nested, each
 *  restores what was there when it was made.
 */
class GUCEF_MT_PUBLIC_CPP CRealtimeSchedulingScope
{
    public:

    /** The priority used when none is given (70), high enough to be above ordinary real-time helpers of the system */
    static const UInt32 DefaultPriority;

    /** Asks for real-time scheduling of the calling thread at the given priority. Never fails: see IsActive() */
    explicit CRealtimeSchedulingScope( UInt32 priority = DefaultPriority );

    /** Restores the scheduling policy and priority the thread had when the scope was made */
    ~CRealtimeSchedulingScope();

    /** True when the thread got real-time scheduling, false when the system refused it or does not support it */
    bool IsActive( void ) const;

    /**
     *  The error of the operating system when the scope is not active though the platform supports it
     *  (EPERM on Linux when the user has no right to real-time scheduling, the GetLastError() value on Windows),
     *  0 when the scope is active or the platform has no real-time scheduling
     */
    Int32 GetErrorCode( void ) const;

    /** True on the platforms where a scope can become active: Linux, Android and Windows */
    static bool IsSupported( void );

    private:

    CRealtimeSchedulingScope( const CRealtimeSchedulingScope& src );            /**< not implemented, a scope belongs to its thread */
    CRealtimeSchedulingScope& operator=( const CRealtimeSchedulingScope& src ); /**< not implemented, a scope belongs to its thread */

    private:

    Int32 m_mechanism;          /**< how the scope made the thread real-time, a value of the implementation; 0 when not active */
    Int32 m_errorCode;
    Int32 m_previousPolicy;     /**< POSIX: the scheduling policy before the scope */
    Int32 m_previousPriority;   /**< POSIX: the static priority before the scope; Windows: the thread priority before the scope */
    void* m_handle;             /**< Windows: the MMCSS task handle */
    void* m_module;             /**< Windows: avrt.dll while the MMCSS task is held */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CREALTIMESCHEDULINGSCOPE_H ? */

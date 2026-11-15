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

#ifndef GUCEF_CORE_CSERVICERESOURCEWATCH_H
#define GUCEF_CORE_CSERVICERESOURCEWATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CISERVICERESOURCESOURCE_H
#include "gucefCORE_CIServiceResourceSource.h"
#define GUCEF_CORE_CISERVICERESOURCESOURCE_H
#endif /* GUCEF_CORE_CISERVICERESOURCESOURCE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/** What the process looked like at a sample */
struct GUCEF_CORE_PUBLIC_CPP SServiceProcessResources
{
    UInt64 sampledAtMs;         /**< monotonic; 0: never sampled */
    UInt64 rssBytes;            /**< resident memory; 0 where the platform does not say */
    UInt64 hwmBytes;            /**< peak resident memory since the start */
    UInt32 threadCount;         /**< threads of the process; 0 where unknown */
    UInt32 openDescriptors;     /**< open file descriptors (Windows: handles); 0 where unknown */

    SServiceProcessResources( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The resource watch of an unattended service: from the owner's tick it samples the process (resident memory and its
 *  peak, threads, descriptors) and the counters of the registered sources every sample interval, logs one line at
 *  LOGLEVEL_NORMAL every log interval (no secrets: counters only), and tells whether the resident memory is above a
 *  soft limit (with hysteresis: it counts as above until it is below releasePercent of the limit), so that the service
 *  can hold back optional work. Thread safe; Tick() and SampleNow() run on the owner's thread.
 */
class GUCEF_CORE_PUBLIC_CPP CServiceResourceWatch
{
    public:

    static const UInt32 DefaultSampleMs = 5000;
    static const UInt32 DefaultLogSeconds = 300;
    static const UInt32 ReleasePercent = 90;

    CServiceResourceWatch( void );
    virtual ~CServiceResourceWatch();

    /** 'sampleMs' 0: DefaultSampleMs; 'logSeconds' 0: no periodic line; 'memorySoftLimitBytes' 0: no limit */
    void Configure( UInt32 sampleMs, UInt32 logSeconds, UInt64 memorySoftLimitBytes );

    /** Not owned; removed before it goes */
    void AddSource( CIServiceResourceSource* source );
    void RemoveSource( CIServiceResourceSource* source );

    /** From the owner's tick ('nowMs' monotonic): samples when it is time and logs when it is time */
    void Tick( UInt64 nowMs );

    /** Samples now ('log': the line too) */
    void SampleNow( UInt64 nowMs, bool log );

    SServiceProcessResources GetLastProcessSample( void ) const;

    /** The counters of the sources at the last sample, "<source>.<counter>" */
    CValueList GetLastCounters( void ) const;

    /** Whether the resident memory is above the soft limit (see the class description) */
    bool IsAboveSoftLimit( void ) const;

    /** Values for an info page: rssMB, hwmMB, threads, descriptors, aboveSoftLimit, and the counters */
    void GetInfo( CValueList& info ) const;

    /** The log line of the last sample, "ResourceWatch: rssMB=... hwmMB=... threads=... fds=... <counters>" */
    CString FormatLine( void ) const;

    /** Reads the resources of this process now; false where the platform says nothing */
    static bool ReadProcessResources( SServiceProcessResources& resources );

    protected:

    /** How a sample reads the process: ReadProcessResources(); a test overrides it to give chosen values */
    virtual bool SampleProcess( SServiceProcessResources& resources );

    private:

    CServiceResourceWatch( const CServiceResourceWatch& src );              /**< not implemented */
    CServiceResourceWatch& operator=( const CServiceResourceWatch& src );   /**< not implemented */

    typedef GUCEF::vector< CIServiceResourceSource* > TSourceVector;

    mutable MT::CMutex m_lock;
    TSourceVector m_sources;
    UInt32 m_sampleMs;
    UInt32 m_logSeconds;
    UInt64 m_softLimitBytes;
    bool m_aboveSoftLimit;
    UInt64 m_lastSampleMs;
    UInt64 m_lastLogMs;
    SServiceProcessResources m_lastProcess;
    CValueList m_lastCounters;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSERVICERESOURCEWATCH_H ? */

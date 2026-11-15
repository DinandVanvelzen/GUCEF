/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H
#define GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Keeps a discovery responder from being used as a reflector or an amplifier: a query is a datagram whose source address
 *  anyone on the network can forge, so the answer can be aimed at a victim. The limiter decides per query:
 *
 *  - whether the source can be a real asker at all (IsAcceptableSource(): no port 0, no 0.0.0.0, no broadcast or
 *    multicast source),
 *  - whether the source still has answers left (a token bucket per source address: answerBurstPerSource answers at
 *    once, refilled at answersPerSourcePerMinute) and the process as a whole (answersPerSecond), and
 *  - how many bytes the answers to it may hold together (GetAnswerBudget(): amplificationFactor times the size of the
 *    query, unless the source is verified by the caller).
 *
 *  The table of sources holds at most maxTrackedSources addresses; when it is full the source heard longest ago makes
 *  room, so a flood of forged addresses costs memory within that bound only. Thread safe.
 *
 *  Settings (LoadConfig, also part of the settings of every discovery client): answersPerSourcePerMinute (default 60),
 *  answerBurstPerSource (default 8), answersPerSecond (default 50), amplificationFactor (default 1) and
 *  maxTrackedSources (default 256). 0 means no maximum for each of them, which switches that protection off.
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryAnswerLimiter : public CORE::CIConfigurable
{
    public:

    /** What the limiter decided since it was made or reset */
    struct GUCEF_COMCORE_EXPORT_CPP SStats
    {
        UInt64 admitted;                /**< queries that may be answered */
        UInt64 refusedSource;           /**< queries from a source that cannot be a real asker */
        UInt64 refusedSourceRate;       /**< queries from a source without answers left */
        UInt64 refusedGlobalRate;       /**< queries while the process had no answers left */
        UInt64 answersWithheld;         /**< answers not sent because they did not fit the budget of their query */

        SStats( void );
    };
    typedef struct SStats TStats;

    static const UInt32 DefaultAnswersPerSourcePerMinute;   /**< 60 */
    static const UInt32 DefaultAnswerBurstPerSource;        /**< 8 */
    static const UInt32 DefaultAnswersPerSecond;            /**< 50 */
    static const UInt32 DefaultAmplificationFactor;         /**< 1 */
    static const UInt32 DefaultMaxTrackedSources;           /**< 256 */

    CDiscoveryAnswerLimiter( void );
    CDiscoveryAnswerLimiter( const CDiscoveryAnswerLimiter& src );
    virtual ~CDiscoveryAnswerLimiter();
    CDiscoveryAnswerLimiter& operator=( const CDiscoveryAnswerLimiter& src );

    void SetAnswersPerSourcePerMinute( UInt32 answersPerMinute );
    UInt32 GetAnswersPerSourcePerMinute( void ) const;

    void SetAnswerBurstPerSource( UInt32 burst );
    UInt32 GetAnswerBurstPerSource( void ) const;

    void SetAnswersPerSecond( UInt32 answersPerSecond );
    UInt32 GetAnswersPerSecond( void ) const;

    void SetAmplificationFactor( UInt32 factor );
    UInt32 GetAmplificationFactor( void ) const;

    void SetMaxTrackedSources( UInt32 maxSources );
    UInt32 GetMaxTrackedSources( void ) const;

    /** Whether a datagram with this source address can come from a real asker that waits for an answer */
    static bool IsAcceptableSource( const CIPv4Address& source );

    /**
     *  Decides whether a query from the source is answered now, and takes one answer from its bucket and from the
     *  process bucket when it is. The time is a monotonic clock in milliseconds.
     */
    bool AdmitQuery( const CIPv4Address& source, UInt64 nowInMs );

    /**
     *  The bytes the answers to a query of the given size may hold together: amplificationFactor times the query size,
     *  or no bound (0xFFFFFFFF) when the factor is 0 or the source is verified by the caller
     */
    UInt32 GetAnswerBudget( UInt32 querySizeInBytes, bool sourceIsVerified ) const;

    /** Counts an answer that was not sent because it did not fit the budget */
    void CountAnswerWithheld( void );

    TStats GetStats( void ) const;
    UInt32 GetTrackedSourceCount( void ) const;

    /** Forgets the sources and the counters, the settings stay */
    void Reset( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SBucket
    {
        UInt64 milliTokens;     /**< answers left times 1000 */
        UInt64 lastRefillInMs;
        UInt64 lastSeenInMs;

        SBucket( void );
    };
    typedef struct SBucket TBucket;
    typedef GUCEF::map< UInt32, TBucket > TBucketMap;

    static void Refill( TBucket& bucket, UInt64 nowInMs, UInt64 tokensPerMinute, UInt64 capacityInTokens );
    void MakeRoom( void );

    UInt32 m_answersPerSourcePerMinute;
    UInt32 m_answerBurstPerSource;
    UInt32 m_answersPerSecond;
    UInt32 m_amplificationFactor;
    UInt32 m_maxTrackedSources;
    TBucketMap m_sources;
    TBucket m_global;
    bool m_globalInitialized;
    TStats m_stats;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H ? */

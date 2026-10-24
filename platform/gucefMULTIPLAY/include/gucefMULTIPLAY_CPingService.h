/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CPINGSERVICE_H
#define GUCEF_MULTIPLAY_CPINGSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIRTTPROBE_H
#include "gucefCOMCORE_CIRttProbe.h"
#define GUCEF_COMCORE_CIRTTPROBE_H
#endif /* GUCEF_COMCORE_CIRTTPROBE_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of PingUpdatedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CPingUpdate : public CORE::CICloneable
{
    public:

    CORE::CString target;
    Int32 rttMs;                /**< -1 when it is unknown */
    CORE::CString method;       /**< the probe method the figure comes from, empty when unknown */

    CPingUpdate( void );

    CPingUpdate( const CORE::CString& target, Int32 rttMs, const CORE::CString& method );

    virtual ~CPingUpdate();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Keeps the round trip time to targets current with the COMCORE RTT probes.
 *
 *  Per target the methods are tried in the configured order: the first that is available and answers is kept,
 *  one that fails or times out maxFailuresPerMethod times in a row makes way for the next. When none works the
 *  target is "unknown" (-1), which is shown as nothing rather than as an error, and the methods are tried again
 *  after retryAfterInMs. The RTT is smoothed: rtt += smoothing * ( sample - rtt ).
 *
 *  A target is named by "host:port" or by an endpoint URI such as "udp://host:port". The host is resolved when the
 *  target is added.
 *
 *  Config: methods (comma separated, default "udp-echo,tcp-connect,icmp"), intervalInMs (2000), timeoutInMs (1000),
 *  smoothing (0.25), maxFailuresPerMethod (3), retryAfterInMs (30000)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CPingService : public CORE::CObservingNotifier ,
                                                public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent PingUpdatedEvent;     /**< CPingUpdate */

    static void RegisterEvents( void );

    CPingService( void );

    explicit CPingService( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CPingService();

    /** Starts measuring the target, true when it already was */
    bool AddTarget( const CORE::CString& target );

    /** Starts measuring the address under the given name */
    bool AddTarget( const CORE::CString& name, const COMCORE::CHostAddress& address );

    bool RemoveTarget( const CORE::CString& target );

    void RemoveAllTargets( void );

    bool HasTarget( const CORE::CString& target ) const;

    void GetTargets( CORE::CString::StringVector& targets ) const;

    /** The smoothed RTT in milliseconds, -1 when it is unknown */
    Int32 GetRttMs( const CORE::CString& target ) const;

    /** The method that measures the target now, empty when none does */
    CORE::CString GetMethod( const CORE::CString& target ) const;

    void SetMethods( const CORE::CString::StringVector& methods );

    const CORE::CString::StringVector& GetMethods( void ) const;

    void SetIntervalInMs( UInt32 intervalInMs );

    void SetTimeoutInMs( UInt32 timeoutInMs );

    void SetSmoothing( Float32 smoothing );

    void SetMaxFailuresPerMethod( UInt32 maxFailures );

    void SetRetryAfterInMs( UInt32 retryAfterInMs );

    /** Parses "host:port" or an endpoint URI */
    static bool ParseTarget( const CORE::CString& target, COMCORE::CHostAddress& address );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct STarget
    {
        CORE::CString name;
        COMCORE::CHostAddress address;
        COMCORE::CIRttProbePtr probe;
        UInt32 methodIndex;             /**< into m_methods, m_methods.size() when none works */
        UInt32 failures;                /**< in a row, for the current method */
        Float64 rttMs;                  /**< -1 when unknown */
        bool switchPending;             /**< the current method gave up, the next one starts on the timer */
        UInt64 retryAtInMs;             /**< when no method works: when to start again */
    };
    typedef struct STarget TTarget;
    typedef GUCEF::map< CORE::CString, TTarget > TTargetMap;
    typedef GUCEF::vector< COMCORE::CIRttProbePtr > TProbeVector;

    CPingService( const CPingService& src );
    CPingService& operator=( const CPingService& src );

    void Initialize( void );
    bool StartNextMethod( TTarget& target );
    void RetireProbe( TTarget& target );
    TTarget* FindTargetOfProbe( CORE::CNotifier* probe );
    void OnProbeResult( TTarget& target, UInt64 rttInMicroSecs );
    void OnProbeFailure( TTarget& target );
    void OnTimer( void );
    void SetUnknown( TTarget& target );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CTimer m_timer;
    TTargetMap m_targets;
    TProbeVector m_retiredProbes;       /**< stopped probes, destroyed on the next timer tick and not inside their own events */
    CORE::CString::StringVector m_methods;
    UInt32 m_intervalInMs;
    UInt32 m_timeoutInMs;
    Float32 m_smoothing;
    UInt32 m_maxFailuresPerMethod;
    UInt32 m_retryAfterInMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CPINGSERVICE_H ? */

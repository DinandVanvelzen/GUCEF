/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CSTATSDCLIENT_H
#define GUCEF_COM_CSTATSDCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H
#include "gucefCORE_CIMetricsSystemClient.h"
#define GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H
#endif /* GUCEF_CORE_CIMETRICSSYSTEMCLIENT_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Status codes for DogStatsD service checks.
 *  Values map directly to the DogStatsD wire protocol status integers.
 */
enum EDataDogServiceCheckStatus
{
    DDSC_OK       = 0,
    DDSC_WARNING  = 1,
    DDSC_CRITICAL = 2,
    DDSC_UNKNOWN  = 3
};

/*-------------------------------------------------------------------------*/

class GUCEF_COM_PUBLIC_CPP CStatsDClient : public CORE::CTSGNotifier          ,
                                           public CORE::CGloballyConfigurable ,
                                           public CORE::CIMetricsSystemClient
{
    public:

    static const CORE::CString Type;

    CStatsDClient( void );

    CStatsDClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CStatsDClient();

    virtual bool Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Increments the key, at a given frequency rate
     */
    virtual void Increment( const CString& key, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Decrements the key, at a given frequency rate
     */
    virtual void Decrement( const CString& key, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Adjusts the specified key by a given delta, at a given frequency rate
     */
    virtual void Count( const CString& key, const Int32 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const Int64 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const UInt32 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const UInt64 delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Count( const CString& key, const CORE::CVariant& delta, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Records a gauge for the key, with a given value, at a given frequency rate
     */
    virtual void Gauge( const CString& key, const Int32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const UInt32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Int64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Gauge( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Records a timing for a key, at a given frequency
     */
    virtual void Timing( const CString& key, const UInt32 ms, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Records a unique occurrence of value within the flush interval (standard StatsD Set type)
     */
    void Set( const CString& key, const CString& value, const Float32 frequency = 1.0f ) const;

    /**
     *  DogStatsD: records a histogram sample (type "h").
     *  Client-side aggregated; the agent computes min/max/avg/percentiles per flush.
     *  When DogStatsD extensions are disabled, falls back to standard StatsD timer (type "ms").
     */
    void Histogram( const CString& key, const Int32   value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Histogram( const CString& key, const UInt32  value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Histogram( const CString& key, const Int64   value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Histogram( const CString& key, const UInt64  value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Histogram( const CString& key, const Float32 value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Histogram( const CString& key, const Float64 value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;

    /**
     *  DogStatsD: records a distribution sample (type "d").
     *  Server-side globally aggregated across all agents; enables globally accurate percentiles.
     *  When DogStatsD extensions are disabled, falls back to standard StatsD timer (type "ms").
     */
    void Distribution( const CString& key, const Int32   value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Distribution( const CString& key, const UInt32  value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Distribution( const CString& key, const Int64   value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Distribution( const CString& key, const UInt64  value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Distribution( const CString& key, const Float32 value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;
    void Distribution( const CString& key, const Float64 value, const Float32 frequency = 1.0f, const CString& tags = CString::Empty ) const;

    /**
     *  DogStatsD: sends an event to the Datadog event stream.
     *  alertType: "error", "warning", "info", or "success" (empty = omitted from packet)
     *  priority:  "normal" or "low" (empty = omitted from packet)
     *  timestamp: Unix epoch seconds; 0 = omit from packet (agent uses current time)
     *  No-op when DogStatsD extensions are disabled.
     */
    void SendDogStatsDEvent( const CString& title                        ,
                             const CString& text                         ,
                             const CString& tags           = CString::Empty ,
                             const CString& hostname       = CString::Empty ,
                             const CString& alertType      = CString::Empty ,
                             const CString& priority       = CString::Empty ,
                             const CString& aggregationKey = CString::Empty ,
                             Int64          timestamp      = 0            ) const;

    /**
     *  DogStatsD: sends a service check.
     *  timestamp: Unix epoch seconds; 0 = omit from packet (agent uses current time)
     *  No-op when DogStatsD extensions are disabled.
     */
    void SendDogStatsDServiceCheck( const CString& name                        ,
                                    EDataDogServiceCheckStatus status           ,
                                    const CString& tags      = CString::Empty  ,
                                    const CString& hostname  = CString::Empty  ,
                                    const CString& message   = CString::Empty  ,
                                    Int64          timestamp = 0               ) const;

    /**
     *  Enables or disables DogStatsD protocol extensions (histogram, distribution,
     *  events, service checks, and tag support).
     */
    void SetDogStatsDEnabled( bool enabled );
    bool GetDogStatsDEnabled( void ) const;

    /**
     *  Global tags appended to every metric when DogStatsD extensions are enabled.
     *  Format: comma-separated "key:value" pairs, e.g. "env:production,service:api"
     */
    void SetGlobalTags( const CString& tags );
    const CString& GetGlobalTags( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    bool SetStatsDestination( const COMCORE::CHostAddress& dest );

    const COMCORE::CHostAddress& GetStatsDestination( void ) const;

    void SetStatsInterface( const COMCORE::CHostAddress& interface );

    const COMCORE::CIPv4Address& GetStatsInterface( void ) const;

    void SetStatNamePrefix( const CString& prefix );

    const CString& GetStatNamePrefix( void ) const;

    virtual const CString& GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CStatsDClient > TEventCallback;

    void OnStatsDTargetDnsChange( CORE::CNotifier* notifier                 ,
                                  const CORE::CEvent& eventId               ,
                                  CORE::CICloneable* eventdata = GUCEF_NULL );

    CStatsDClient( const CStatsDClient& src );             /** not implemented */
    CStatsDClient& operator=( const CStatsDClient& src );  /** not implemented */

    template < typename valueType >
    void
    Transmit( const CString& key      ,
              const valueType value   ,
              const CString& type     ,
              const Float32 frequency ,
              const CString& tags     ) const;

    /**
     *  Merges per-call tags with the configured global tags.
     *  Returns a combined "tag1:v1,tag2:v2" string, or empty if both are empty.
     */
    CString MergeTags( const CString& callTags ) const;

    /**
     *  Sends the current content of m_transmitBuffer via UDP.
     *  Caller must hold m_dataAccessMutex and have populated m_transmitBuffer.
     */
    void SendTransmitBuffer( void ) const;

    private:

    mutable MT::CMutex m_dataAccessMutex;
    mutable CORE::CDynamicBuffer m_transmitBuffer;
    COMCORE::CUDPSocket m_udpSender;
    COMCORE::CHostAddress m_statsDestination;
    COMCORE::CIPv4Address m_statsInterface;
    CString m_statNamePrefix;
    CString m_globalTags;
    bool m_transmit;
    bool m_logStats;
    bool m_dogStatsDEnabled;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSTATSDCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 03-03-2007 :
        - Dinand: Added this section

---------------------------------------------------------------------------*/

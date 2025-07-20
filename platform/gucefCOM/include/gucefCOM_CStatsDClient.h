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
     *  Increments the key, at a given frequency rate
     */
    virtual void Decrement( const CString& key, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *   Adjusts the specified key by a given delta, at a given frequency rate
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
    virtual void Timing( const CString& key, const UInt32 mvalues, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const UInt64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float32 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void Timing( const CString& key, const Float64 value, const Float32 frequency = 1.0f ) const GUCEF_VIRTUAL_OVERRIDE;

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
              const Float32 frequency ) const;

    private:

    COMCORE::CUDPSocket m_udpSender;
    COMCORE::CHostAddress m_statsDestination;
    COMCORE::CIPv4Address m_statsInterface;
    CString m_statNamePrefix;
    bool m_transmit;
    bool m_logStats;
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

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

#ifndef GUCEF_COM_CNATPMPPORTMAPPER_H
#define GUCEF_COM_CNATPMPPORTMAPPER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#include "gucefCOMCORE_CIPortMapper.h"
#define GUCEF_COMCORE_CIPORTMAPPER_H
#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */

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
 *  Port mapping with NAT-PMP (RFC 6886) and PCP (RFC 6887) at the default gateway on UDP port 5351.
 *
 *  Mappings are asked for with the PCP MAP opcode first. A gateway that answers PCP with version 0 only speaks
 *  NAT-PMP, and so does one that does not answer PCP at all: the request goes again as a NAT-PMP map request.
 *  Requests are sent again with a doubling interval until an answer comes or the attempts run out.
 *  The mappings that were made are renewed when half of their lease has passed.
 *
 *  The gateway is the gateway of the default route: from /proc/net/route on Linux and GetBestRoute on Windows,
 *  unless one is configured.
 *
 *  Settings (LoadConfig): gateway (address or address:port), usePcp, initialRetransmitIntervalInMs, maxAttempts
 */
class GUCEF_COM_PUBLIC_CPP CNatPmpPortMapper : public COMCORE::CIPortMapper
{
    public:

    static const CORE::CString MapperName;      /**< "natpmp" */
    static const UInt16 GatewayPort;            /**< 5351 */

    CNatPmpPortMapper( void );

    explicit CNatPmpPortMapper( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CNatPmpPortMapper();

    virtual bool RequestMapping( TProtocol protocol            ,
                                 UInt16 internalPort           ,
                                 UInt16 externalPort           ,
                                 UInt32 leaseInSecs            ,
                                 CORE::CFutureResult& future   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveMapping( TProtocol protocol            ,
                                UInt16 internalPort           ,
                                CORE::CFutureResult& future   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetExternalAddress( CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetMapperName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Talks to the given gateway instead of the default one, address and port */
    void SetGateway( const COMCORE::CIPv4Address& gateway );

    /** The gateway in use, detected when none was set */
    bool GetGateway( COMCORE::CIPv4Address& gateway ) const;

    void SetUsePcp( bool usePcp );

    void SetRetransmission( UInt32 initialIntervalInMs, UInt32 maxAttempts );

    /** The number of mappings that are kept alive */
    UInt32 GetMappingCount( void ) const;

    /** Finds the gateway of the default route of the system */
    static bool GetDefaultGateway( COMCORE::CIPv4Address& gateway );

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum ERequestKind
    {
        REQUESTKIND_EXTERNAL_ADDRESS ,
        REQUESTKIND_MAP              ,
        REQUESTKIND_UNMAP
    };

    enum EProtocolMode
    {
        PROTOCOLMODE_UNKNOWN ,
        PROTOCOLMODE_PCP     ,
        PROTOCOLMODE_NATPMP
    };

    struct SRequest
    {
        ERequestKind kind;
        TProtocol protocol;
        UInt16 internalPort;
        UInt16 externalPort;
        UInt32 leaseInSecs;
        bool usesPcp;
        UInt8 nonce[ 12 ];
        UInt32 attempts;
        UInt32 intervalInMs;
        UInt64 nextSendInMs;
        bool isRenewal;
        bool hasFuture;
        bool awaitsExternalAddress;
        COMCORE::CPortMappingResult result;
        CORE::CFutureResultSource source;
    };
    typedef struct SRequest TRequest;
    typedef GUCEF::vector< TRequest > TRequestVector;

    struct SMapping
    {
        TProtocol protocol;
        UInt16 internalPort;
        UInt16 externalPort;
        UInt32 leaseInSecs;
        UInt64 renewAtInMs;
        UInt64 expiresAtInMs;
        bool renewing;
    };
    typedef struct SMapping TMapping;
    typedef GUCEF::vector< TMapping > TMappingVector;

    CNatPmpPortMapper( const CNatPmpPortMapper& src );
    CNatPmpPortMapper& operator=( const CNatPmpPortMapper& src );

    bool EnsureReady( void );
    bool QueueRequest( TRequest& request, CORE::CFutureResult* future );
    void EnsureExternalAddressRequest( void );
    bool SendRequest( TRequest& request );
    void FillPcpClientAddress( UInt8* address ) const;

    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet );
    bool OnPcpResponse( const UInt8* data, UInt32 size );
    bool OnNatPmpResponse( const UInt8* data, UInt32 size );
    bool OnRequestSucceeded( size_t index );
    bool OnRequestFailed( size_t index, const CORE::CString& reason );
    bool CompleteAddressWaiters( bool addressKnown );
    bool OnTimer( void );

    TMapping* FindMapping( TProtocol protocol, UInt16 internalPort );
    static UInt64 GetTimeInMs( void );

    private:

    COMCORE::CUDPSocket m_socket;
    CORE::CTimer m_timer;
    COMCORE::CIPv4Address m_gateway;
    bool m_gatewayConfigured;
    bool m_usePcp;
    EProtocolMode m_protocolMode;
    UInt32 m_initialIntervalInMs;
    UInt32 m_maxAttempts;
    COMCORE::CIPv4Address m_externalAddress;
    bool m_externalAddressKnown;
    TRequestVector m_requests;
    TMappingVector m_mappings;
    UInt32 m_nonceCounter;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIPortMapper, CNatPmpPortMapper, MT::CMutex > TNatPmpPortMapperFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CNATPMPPORTMAPPER_H ? */

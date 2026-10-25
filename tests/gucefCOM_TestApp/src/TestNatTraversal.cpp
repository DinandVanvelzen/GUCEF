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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COM_CNATPMPPORTMAPPER_H
#include "gucefCOM_CNatPmpPortMapper.h"
#define GUCEF_COM_CNATPMPPORTMAPPER_H
#endif /* GUCEF_COM_CNATPMPPORTMAPPER_H ? */

#ifndef GUCEF_COM_CSTUNCLIENT_H
#include "gucefCOM_CStunClient.h"
#define GUCEF_COM_CSTUNCLIENT_H
#endif /* GUCEF_COM_CSTUNCLIENT_H ? */

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestNatTraversal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt16 FakeGatewayPort = 47841;
const UInt16 FakeStunPort = 47842;
const UInt16 StunClientPort = 47843;
const char* FakeExternalAddress = "203.0.113.7";

/*-------------------------------------------------------------------------*/

bool
PumpUntilReady( const CORE::CFutureResult& future, UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<timeoutInMs && !future.IsReady(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return future.IsReady();
}

/*-------------------------------------------------------------------------*/

/**
 *  A gateway on loopback that speaks PCP, only NAT-PMP, only PCP or nothing at all
 */
class CFakeGateway : public CORE::CObserver
{
    public:

    enum EMode
    {
        MODE_PCP_AND_NATPMP ,
        MODE_NATPMP_ONLY    ,
        MODE_PCP_ONLY       ,
        MODE_SILENT
    };

    EMode mode;
    UInt32 pcpMapRequests;
    UInt32 natPmpMapRequests;
    UInt32 natPmpAddressRequests;
    UInt32 lastLifetime;
    UInt16 lastInternalPort;

    CFakeGateway( EMode gatewayMode )
        : CORE::CObserver()
        , mode( gatewayMode )
        , pcpMapRequests( 0 )
        , natPmpMapRequests( 0 )
        , natPmpAddressRequests( 0 )
        , lastLifetime( 0 )
        , lastInternalPort( 0 )
        , m_socket( false )
    {GUCEF_TRACE;

        SubscribeTo( &m_socket );
    }

    virtual ~CFakeGateway()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        m_socket.Close();
    }

    bool Start( void )
    {GUCEF_TRACE;

        return m_socket.Open( "127.0.0.1", FakeGatewayPort );
    }

    COMCORE::CIPv4Address GetAddress( void ) const
    {GUCEF_TRACE;

        return COMCORE::CIPv4Address( "127.0.0.1", FakeGatewayPort );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CFakeGateway";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;

        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
            OnRequest( data.packets[ i ].sourceAddress, data.packets[ i ].dataBuffer.GetData() );
    }

    private:

    void OnRequest( const COMCORE::CIPv4Address& client, const CORE::CDynamicBuffer& request )
    {GUCEF_TRACE;

        const UInt8* data = static_cast< const UInt8* >( request.GetConstBufferPtr() );
        UInt32 size = request.GetDataSize();
        if ( size < 2 || MODE_SILENT == mode )
            return;

        COMCORE::CIPv4Address external( FakeExternalAddress, 0 );
        UInt32 externalAddress = external.GetAddress();
        CORE::CDynamicBuffer response;
        COM::CNetworkByteWriter writer( response );

        if ( 2 == data[ 0 ] && size >= 60 )
        {
            ++pcpMapRequests;
            COM::CNetworkByteReader reader( data, size );
            reader.Skip( 4 );
            reader.ReadUInt32( lastLifetime );
            reader.Seek( 40 );
            reader.ReadUInt16( lastInternalPort );

            if ( MODE_NATPMP_ONLY == mode )
            {
                // RFC 6887 section 9: a NAT-PMP server answers a PCP request with version 0 and 'unsupported version'
                writer.WriteUInt8( 0 );
                writer.WriteUInt8( 0x80 | data[ 1 ] );
                writer.WriteUInt16( 1 );
                writer.WriteUInt32( 1234 );
            }
            else
            {
                writer.WriteUInt8( 2 );
                writer.WriteUInt8( 0x81 );
                writer.WriteUInt8( 0 );
                writer.WriteUInt8( 0 );                     // success
                writer.WriteUInt32( lastLifetime );
                writer.WriteUInt32( 1234 );                 // epoch
                UInt8 zeros[ 12 ];
                memset( zeros, 0, sizeof( zeros ) );
                writer.WriteBytes( zeros, 12 );
                writer.WriteBytes( data + 24, 12 );         // the nonce
                writer.WriteUInt8( data[ 36 ] );            // the protocol
                writer.WriteBytes( zeros, 3 );
                writer.WriteUInt16( lastInternalPort );
                writer.WriteUInt16( (UInt16)( 0 == lastLifetime ? 0 : lastInternalPort + 10000 ) );
                writer.WriteBytes( zeros, 10 );
                writer.WriteUInt8( 0xFF );
                writer.WriteUInt8( 0xFF );
                writer.WriteBytes( &externalAddress, 4 );
            }
        }
        else
        if ( 0 == data[ 0 ] )
        {
            if ( MODE_PCP_ONLY == mode )
            {
                writer.WriteUInt8( 2 );
                writer.WriteUInt8( 0x80 | data[ 1 ] );
                writer.WriteUInt8( 0 );
                writer.WriteUInt8( 1 );                     // unsupported version
                UInt8 zeros[ 20 ];
                memset( zeros, 0, sizeof( zeros ) );
                writer.WriteBytes( zeros, 20 );
            }
            else
            if ( 0 == data[ 1 ] )
            {
                ++natPmpAddressRequests;
                writer.WriteUInt8( 0 );
                writer.WriteUInt8( 128 );
                writer.WriteUInt16( 0 );
                writer.WriteUInt32( 1234 );
                writer.WriteBytes( &externalAddress, 4 );
            }
            else
            if ( ( 1 == data[ 1 ] || 2 == data[ 1 ] ) && size >= 12 )
            {
                ++natPmpMapRequests;
                COM::CNetworkByteReader reader( data, size );
                reader.Skip( 4 );
                reader.ReadUInt16( lastInternalPort );
                reader.Skip( 2 );
                reader.ReadUInt32( lastLifetime );
                writer.WriteUInt8( 0 );
                writer.WriteUInt8( (UInt8)( 128 + data[ 1 ] ) );
                writer.WriteUInt16( 0 );
                writer.WriteUInt32( 1234 );
                writer.WriteUInt16( lastInternalPort );
                writer.WriteUInt16( (UInt16)( 0 == lastLifetime ? 0 : lastInternalPort + 10000 ) );
                writer.WriteUInt32( lastLifetime );
            }
        }

        if ( response.GetDataSize() > 0 )
            m_socket.SendPacketTo( client, response.GetConstBufferPtr(), (UInt16) response.GetDataSize() );
    }

    COMCORE::CUDPSocket m_socket;
};

/*-------------------------------------------------------------------------*/

/**
 *  Counts the events of a port mapper
 */
class CMapperRecorder : public CORE::CObserver
{
    public:

    UInt32 created;
    UInt32 lost;
    UInt32 errors;

    CMapperRecorder( void )
        : CORE::CObserver()
        , created( 0 )
        , lost( 0 )
        , errors( 0 )
    {GUCEF_TRACE;

    }

    virtual ~CMapperRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CMapperRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CIPortMapper::MappingCreatedEvent == eventId )
            ++created;
        else if ( COMCORE::CIPortMapper::MappingLostEvent == eventId )
            ++lost;
        else if ( COMCORE::CIPortMapper::PortMapperErrorEvent == eventId )
            ++errors;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A STUN server on loopback: answers Binding requests with the address they came from
 */
class CFakeStunServer : public CORE::CObserver
{
    public:

    UInt32 requests;
    UInt32 requestsToIgnore;
    bool classicAnswer;         /**< MAPPED-ADDRESS instead of XOR-MAPPED-ADDRESS */
    bool errorAnswer;

    CFakeStunServer( void )
        : CORE::CObserver()
        , requests( 0 )
        , requestsToIgnore( 0 )
        , classicAnswer( false )
        , errorAnswer( false )
        , m_socket( false )
    {GUCEF_TRACE;

        SubscribeTo( &m_socket );
    }

    virtual ~CFakeStunServer()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        m_socket.Close();
    }

    bool Start( void )
    {GUCEF_TRACE;

        return m_socket.Open( "127.0.0.1", FakeStunPort );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CFakeStunServer";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;

        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
        {
            const CORE::CDynamicBuffer& request = data.packets[ i ].dataBuffer.GetData();
            if ( !COM::CStunClient::IsStunMessage( request.GetConstBufferPtr(), request.GetDataSize() ) )
                continue;
            ++requests;
            if ( requests <= requestsToIgnore )
                continue;

            const UInt8* bytes = static_cast< const UInt8* >( request.GetConstBufferPtr() );
            const COMCORE::CIPv4Address& client = data.packets[ i ].sourceAddress;
            CORE::CDynamicBuffer response;
            COM::CNetworkByteWriter writer( response );
            if ( errorAnswer )
            {
                writer.WriteUInt16( 0x0111 );
                writer.WriteUInt16( 8 );
                writer.WriteUInt32( COM::CStunClient::MagicCookie );
                writer.WriteBytes( bytes + 8, 12 );
                writer.WriteUInt16( 0x0009 );
                writer.WriteUInt16( 4 );
                writer.WriteUInt16( 0 );
                writer.WriteUInt8( 4 );
                writer.WriteUInt8( 0 );         // 400 Bad Request
            }
            else
            {
                writer.WriteUInt16( 0x0101 );
                writer.WriteUInt16( 12 );
                writer.WriteUInt32( COM::CStunClient::MagicCookie );
                writer.WriteBytes( bytes + 8, 12 );
                UInt16 port = client.GetPortInHostByteOrder();
                UInt32 address = client.GetAddressInHostByteOrder();
                if ( classicAnswer )
                {
                    writer.WriteUInt16( 0x0001 );
                    writer.WriteUInt16( 8 );
                    writer.WriteUInt8( 0 );
                    writer.WriteUInt8( 1 );
                    writer.WriteUInt16( port );
                    writer.WriteUInt32( address );
                }
                else
                {
                    writer.WriteUInt16( 0x0020 );
                    writer.WriteUInt16( 8 );
                    writer.WriteUInt8( 0 );
                    writer.WriteUInt8( 1 );
                    writer.WriteUInt16( (UInt16)( port ^ ( COM::CStunClient::MagicCookie >> 16 ) ) );
                    writer.WriteUInt32( address ^ COM::CStunClient::MagicCookie );
                }
            }
            m_socket.SendPacketTo( client, response.GetConstBufferPtr(), (UInt16) response.GetDataSize() );
        }
    }

    private:

    COMCORE::CUDPSocket m_socket;
};

/*-------------------------------------------------------------------------*/

bool
LoadPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNatPmpTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "NatPmp" );

    GUCEF_TESTFW_TESTCASE( "Test: A mapping is made with PCP when the gateway speaks it" )
        try
        {
            CFakeGateway gateway( CFakeGateway::MODE_PCP_AND_NATPMP );
            ASSERT_TRUE( gateway.Start() );

            COM::CNatPmpPortMapper mapper;
            mapper.SetGateway( gateway.GetAddress() );
            CMapperRecorder recorder;
            recorder.SubscribeTo( &mapper );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7777, 0, 3600, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );

            COMCORE::CPortMappingResult result;
            ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( 7777 == result.internalPort );
            ASSERT_TRUE( 17777 == result.externalPort );
            ASSERT_TRUE( 3600 == result.leaseInSecs );
            ASSERT_TRUE( result.externalAddress.AddressAsString() == FakeExternalAddress );
            ASSERT_TRUE( result.mapperName == "natpmp" );
            ASSERT_TRUE( 1 == recorder.created );
            ASSERT_TRUE( 1 == mapper.GetMappingCount() );
            ASSERT_TRUE( 1 == gateway.pcpMapRequests );
            ASSERT_TRUE( 0 == gateway.natPmpMapRequests );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A gateway that only speaks NAT-PMP gets the request again as NAT-PMP" )
        try
        {
            CFakeGateway gateway( CFakeGateway::MODE_NATPMP_ONLY );
            ASSERT_TRUE( gateway.Start() );

            COM::CNatPmpPortMapper mapper;
            mapper.SetGateway( gateway.GetAddress() );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_TCP, 8888, 0, 3600, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );

            COMCORE::CPortMappingResult result;
            ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( COMCORE::CPortMappingResult::PROTOCOL_TCP == result.protocol );
            ASSERT_TRUE( 18888 == result.externalPort );
            ASSERT_TRUE( result.externalAddress.AddressAsString() == FakeExternalAddress );
            ASSERT_TRUE( 18888 == result.externalAddress.GetPortInHostByteOrder() );
            ASSERT_TRUE( 1 == gateway.pcpMapRequests );
            ASSERT_TRUE( 1 == gateway.natPmpMapRequests );
            ASSERT_TRUE( 1 == gateway.natPmpAddressRequests );

            // From now on it goes NAT-PMP right away
            CORE::CFutureResult second( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 9999, 0, 3600, second ) );
            ASSERT_TRUE( PumpUntilReady( second, 5000 ) );
            ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( second, result ) );
            ASSERT_TRUE( 1 == gateway.pcpMapRequests );
            ASSERT_TRUE( 2 == gateway.natPmpMapRequests );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The external address comes from NAT-PMP, or from the mappings on a PCP only gateway" )
        try
        {
            {
                CFakeGateway gateway( CFakeGateway::MODE_PCP_AND_NATPMP );
                ASSERT_TRUE( gateway.Start() );
                COM::CNatPmpPortMapper mapper;
                mapper.SetGateway( gateway.GetAddress() );

                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( mapper.GetExternalAddress( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 5000 ) );
                COMCORE::CPortMappingResult result;
                ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( future, result ) );
                ASSERT_TRUE( result.externalAddress.AddressAsString() == FakeExternalAddress );
                ASSERT_TRUE( 0 == result.externalPort );
            }
            {
                CFakeGateway gateway( CFakeGateway::MODE_PCP_ONLY );
                ASSERT_TRUE( gateway.Start() );
                COM::CNatPmpPortMapper mapper;
                mapper.SetGateway( gateway.GetAddress() );

                CORE::CFutureResult unknown( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( mapper.GetExternalAddress( unknown ) );
                ASSERT_TRUE( PumpUntilReady( unknown, 5000 ) );
                COMCORE::CPortMappingResult result;
                ASSERT_FALSE( COMCORE::CPortMappingResult::TryGetFromFuture( unknown, result ) );

                CORE::CFutureResult mapping( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7777, 0, 3600, mapping ) );
                ASSERT_TRUE( PumpUntilReady( mapping, 5000 ) );
                ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( mapping, result ) );

                CORE::CFutureResult known( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( mapper.GetExternalAddress( known ) );
                ASSERT_TRUE( PumpUntilReady( known, 5000 ) );
                ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( known, result ) );
                ASSERT_TRUE( result.externalAddress.AddressAsString() == FakeExternalAddress );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A mapping is renewed at half its lease and removed on request" )
        try
        {
            CFakeGateway gateway( CFakeGateway::MODE_PCP_AND_NATPMP );
            ASSERT_TRUE( gateway.Start() );
            COM::CNatPmpPortMapper mapper;
            mapper.SetGateway( gateway.GetAddress() );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7777, 0, 2, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );
            ASSERT_TRUE( 1 == gateway.pcpMapRequests );

            ASSERT_TRUE( PumpUntilAtLeast( gateway.pcpMapRequests, 2, 3000 ) );
            ASSERT_TRUE( 2 == gateway.lastLifetime );
            ASSERT_TRUE( 1 == mapper.GetMappingCount() );

            CORE::CFutureResult removal( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RemoveMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7777, removal ) );
            ASSERT_TRUE( PumpUntilReady( removal, 5000 ) );
            COMCORE::CPortMappingResult result;
            ASSERT_TRUE( COMCORE::CPortMappingResult::TryGetFromFuture( removal, result ) );
            ASSERT_TRUE( 0 == gateway.lastLifetime );
            ASSERT_TRUE( 0 == mapper.GetMappingCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A mapping that cannot be renewed is reported lost, a silent gateway fails the request" )
        try
        {
            CFakeGateway gateway( CFakeGateway::MODE_PCP_AND_NATPMP );
            ASSERT_TRUE( gateway.Start() );
            COM::CNatPmpPortMapper mapper;
            mapper.SetGateway( gateway.GetAddress() );
            mapper.SetRetransmission( 20, 3 );
            CMapperRecorder recorder;
            recorder.SubscribeTo( &mapper );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7777, 0, 1, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );
            ASSERT_TRUE( 1 == recorder.created );

            gateway.mode = CFakeGateway::MODE_SILENT;
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 5000 ) );
            ASSERT_TRUE( 0 == mapper.GetMappingCount() );

            CORE::CFutureResult failing( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( mapper.RequestMapping( COMCORE::CPortMappingResult::PROTOCOL_UDP, 7778, 0, 60, failing ) );
            ASSERT_TRUE( PumpUntilReady( failing, 5000 ) );
            COMCORE::CPortMappingResult result;
            ASSERT_FALSE( COMCORE::CPortMappingResult::TryGetFromFuture( failing, result ) );
            ASSERT_TRUE( recorder.errors >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The port mapper is created by name and finds the default gateway" )
        try
        {
            COMCORE::CPortMapperFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetPortMapperFactory();
            COMCORE::CIPortMapperPtr mapper = factory.Create( "natpmp" );
            ASSERT_FALSE( mapper.IsNULL() );
            if ( !mapper.IsNULL() )
                ASSERT_TRUE( mapper->GetMapperName() == "natpmp" );

            COMCORE::CIPv4Address gateway;
            if ( COM::CNatPmpPortMapper::GetDefaultGateway( gateway ) )
            {
                ASSERT_TRUE( 0 != gateway.GetAddress() );
                ASSERT_TRUE( COM::CNatPmpPortMapper::GatewayPort == gateway.GetPortInHostByteOrder() );
                ASSERT_TRUE( mapper.IsNULL() || mapper->IsAvailable() );
            }

            CORE::CDataNode config;
            config.SetAttribute( "gateway", "127.0.0.1:47841" );
            config.SetAttribute( "usePcp", false );
            COM::CNatPmpPortMapper configured;
            ASSERT_TRUE( configured.LoadConfig( config ) );
            COMCORE::CIPv4Address configuredGateway;
            ASSERT_TRUE( configured.GetGateway( configuredGateway ) );
            ASSERT_TRUE( configuredGateway.AddressAndPortAsString() == "127.0.0.1:47841" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformStunTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Stun" );

    GUCEF_TESTFW_TESTCASE( "Test: The reflexive address of a socket comes back from the server" )
        try
        {
            CFakeStunServer server;
            ASSERT_TRUE( server.Start() );

            COMCORE::CUDPSocket socket( false );
            ASSERT_TRUE( socket.Open( "127.0.0.1", StunClientPort ) );
            COM::CStunClient stun( socket );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( stun.QueryReflexiveAddress( COMCORE::CHostAddress( "127.0.0.1", FakeStunPort ), future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );

            COM::CStunClient::CStunResult result;
            ASSERT_TRUE( COM::CStunClient::TryGetResultFromFuture( future, result ) );
            ASSERT_TRUE( result.reflexiveAddress.AddressAndPortAsString() == "127.0.0.1:47843" );
            ASSERT_TRUE( result.server.GetPortInHostByteOrder() == FakeStunPort );
            ASSERT_TRUE( 1 == server.requests );
            ASSERT_TRUE( 0 == stun.GetPendingQueryCount() );

            // The old style answer works too
            server.classicAnswer = true;
            CORE::CFutureResult classic( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( stun.QueryReflexiveAddress( COMCORE::CHostAddress( "127.0.0.1", FakeStunPort ), classic ) );
            ASSERT_TRUE( PumpUntilReady( classic, 5000 ) );
            ASSERT_TRUE( COM::CStunClient::TryGetResultFromFuture( classic, result ) );
            ASSERT_TRUE( result.reflexiveAddress.AddressAndPortAsString() == "127.0.0.1:47843" );
            socket.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lost request is sent again, a silent server or an error answer fails the query" )
        try
        {
            CFakeStunServer server;
            ASSERT_TRUE( server.Start() );
            server.requestsToIgnore = 1;

            COMCORE::CUDPSocket socket( false );
            ASSERT_TRUE( socket.Open( "127.0.0.1", StunClientPort ) );
            COM::CStunClient stun( socket );
            stun.SetRetransmission( 50, 3 );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( stun.QueryReflexiveAddress( COMCORE::CHostAddress( "127.0.0.1", FakeStunPort ), future ) );
            ASSERT_TRUE( PumpUntilReady( future, 5000 ) );
            COM::CStunClient::CStunResult result;
            ASSERT_TRUE( COM::CStunClient::TryGetResultFromFuture( future, result ) );
            ASSERT_TRUE( 2 == server.requests );

            server.requestsToIgnore = 100;
            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( stun.QueryReflexiveAddress( COMCORE::CHostAddress( "127.0.0.1", FakeStunPort ), silent ) );
            ASSERT_TRUE( PumpUntilReady( silent, 5000 ) );
            ASSERT_FALSE( COM::CStunClient::TryGetResultFromFuture( silent, result ) );
            ASSERT_TRUE( 5 == server.requests );

            server.requestsToIgnore = 0;
            server.errorAnswer = true;
            CORE::CFutureResult refused( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( stun.QueryReflexiveAddress( COMCORE::CHostAddress( "127.0.0.1", FakeStunPort ), refused ) );
            ASSERT_TRUE( PumpUntilReady( refused, 5000 ) );
            ASSERT_FALSE( COM::CStunClient::TryGetResultFromFuture( refused, result ) );
            socket.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: STUN messages are recognized and damaged ones refused" )
        try
        {
            UInt8 transactionId[ 12 ] = { 1,2,3,4,5,6,7,8,9,10,11,12 };
            CORE::CDynamicBuffer request;
            COM::CStunClient::EncodeBindingRequest( transactionId, request );
            ASSERT_TRUE( 20 == request.GetDataSize() );
            ASSERT_TRUE( COM::CStunClient::IsStunMessage( request.GetConstBufferPtr(), request.GetDataSize() ) );

            // A request is not a response
            UInt8 readId[ 12 ];
            bool isSuccess = false;
            COMCORE::CIPv4Address mapped;
            UInt16 errorCode = 0;
            ASSERT_FALSE( COM::CStunClient::DecodeBindingResponse( request.GetConstBufferPtr(), request.GetDataSize(), readId, isSuccess, mapped, errorCode ) );

            const char junk[] = "not a stun message at all";
            ASSERT_FALSE( COM::CStunClient::IsStunMessage( junk, sizeof( junk ) ) );

            // A success without an address is no use
            CORE::CDynamicBuffer empty;
            COM::CNetworkByteWriter writer( empty );
            writer.WriteUInt16( 0x0101 );
            writer.WriteUInt16( 0 );
            writer.WriteUInt32( COM::CStunClient::MagicCookie );
            writer.WriteBytes( transactionId, 12 );
            ASSERT_FALSE( COM::CStunClient::DecodeBindingResponse( empty.GetConstBufferPtr(), empty.GetDataSize(), readId, isSuccess, mapped, errorCode ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A STUN result is only taken from a future that carries one" )
        try
        {
            COM::CStunClient::CStunResult made;
            made.reflexiveAddress = COMCORE::CIPv4Address( "203.0.113.9", 4000 );

            CORE::CFutureResultSource good;
            CORE::CFutureResult goodFuture = good.GetFuture();
            COM::CStunClient::CStunResult result;
            ASSERT_FALSE( COM::CStunClient::TryGetResultFromFuture( goodFuture, result ) );    // not ready
            ASSERT_TRUE( good.Complete( made.Clone(), true ) );
            ASSERT_TRUE( COM::CStunClient::TryGetResultFromFuture( goodFuture, result ) );
            ASSERT_TRUE( result.reflexiveAddress.AddressAndPortAsString() == "203.0.113.9:4000" );

            // finished well, but with the data of another class
            CORE::CFutureResultSource foreign;
            CORE::CFutureResult foreignFuture = foreign.GetFuture();
            COM::CStunClient::CStunResult untouched;
            ASSERT_TRUE( foreign.Complete( GUCEF_NEW CORE::TCloneableString( "not a stun result" ), true ) );
            ASSERT_FALSE( COM::CStunClient::TryGetResultFromFuture( foreignFuture, untouched ) );
            ASSERT_TRUE( untouched.reflexiveAddress.AddressAndPortAsString() != "203.0.113.9:4000" );

            // and the one of a port mapper is no STUN result either
            COMCORE::CPortMappingResult mapping;
            CORE::CFutureResultSource mapper;
            CORE::CFutureResult mapperFuture = mapper.GetFuture();
            ASSERT_TRUE( mapper.Complete( mapping.Clone(), true ) );
            ASSERT_FALSE( COM::CStunClient::TryGetResultFromFuture( mapperFuture, untouched ) );
            ASSERT_FALSE( COMCORE::CPortMappingResult::TryGetFromFuture( goodFuture, mapping ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformUpnpPluginTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "UpnpPlugin" );

    GUCEF_TESTFW_TESTCASE( "Test: The UPnP plugin registers its port mapper, which answers asking for the external address" )
        COMCORE::CPortMapperFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetPortMapperFactory();
        if ( !factory.IsConstructible( "upnp" ) )
            LoadPlugin( "comcorepluginUPNP" );
        GUCEF_TESTFW_SKIP_IF( !factory.IsConstructible( "upnp" ), "The comcorepluginUPNP plugin could not be loaded" );
        try
        {
            COMCORE::CIPortMapperPtr mapper = factory.Create( "upnp" );
            ASSERT_FALSE( mapper.IsNULL() );
            if ( !mapper.IsNULL() )
            {
                ASSERT_TRUE( mapper->GetMapperName() == "upnp" );
                ASSERT_TRUE( mapper->IsAvailable() );

                CMapperRecorder recorder;
                recorder.SubscribeTo( mapper.GetPointerAlways() );

                // Harmless on any network: it only asks, there may or may not be a gateway to answer
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( mapper->GetExternalAddress( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 20000 ) );

                COMCORE::CPortMappingResult result;
                if ( COMCORE::CPortMappingResult::TryGetFromFuture( future, result ) )
                {
                    ASSERT_TRUE( 0 != result.externalAddress.GetAddress() );
                }
                else
                {
                    // Without a gateway the mapper says so and reports the problem
                    ASSERT_TRUE( PumpUntilAtLeast( recorder.errors, 1, 2000 ) );
                }
            }
            mapper.Unlink();

            // The plugin goes before the globals it registered with
            // The group forgets the plugin too, a later load of the application config would load it again
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "test" ) );
            CORE::CCoreGlobal::Instance()->GetPluginControl().ClearMetaDataFromGroup( "test" );
            ASSERT_FALSE( factory.IsConstructible( "upnp" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

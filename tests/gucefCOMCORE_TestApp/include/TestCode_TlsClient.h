/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTCODE_TLSCLIENT_H
#define GUCEF_COMCORE_TESTAPP_TESTCODE_TLSCLIENT_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/** a client of the connection, recording what it sees */
class CTlsTestClient : public CORE::CObserver
{
    public:

    CORE::CString m_events;             /**< names of the events in order, comma separated */
    CORE::CString m_plain;              /**< plaintext received */
    UInt32 m_sentEvents;
    UInt32 m_sentBytes;
    ETlsErrorClass m_tlsErrorClass;
    Int32 m_socketErrorCode;
    bool m_gotTlsError;
    bool m_gotSocketError;
    bool m_disconnected;
    bool m_connected;
    CORE::CString m_sendOnConnected;
    CORE::CString m_deleteOn;           /**< name of the event on which the observer deletes the connection */
    bool m_deleted;
    CORE::CString m_plainReceivedBeforeUpgrade;
    const CTlsSettings* m_upgradeSettings;      /**< when set, the plain connection is upgraded once "OK" was read */
    CORE::CString m_upgradeName;
    bool m_upgradeStarted;
    bool m_upgradeResult;

    CTlsTestClient( void )
        : CORE::CObserver(), m_events(), m_plain(), m_sentEvents( 0 ), m_sentBytes( 0 ), m_tlsErrorClass( TLSERR_NONE ), m_socketErrorCode( 0 )
        , m_gotTlsError( false ), m_gotSocketError( false ), m_disconnected( false ), m_connected( false ), m_sendOnConnected(), m_deleteOn()
        , m_deleted( false ), m_plainReceivedBeforeUpgrade(), m_upgradeSettings( GUCEF_NULL ), m_upgradeName(), m_upgradeStarted( false ), m_upgradeResult( false ), m_conn( new CTlsClientConnection( false ) )
    {
        SubscribeTo( m_conn );
    }

    virtual ~CTlsTestClient()
    {
        UnsubscribeAllFromObserver();
        delete m_conn;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTlsTestClient";
        return classTypeName;
    }

    CTlsClientConnection* Connection( void ) { return m_conn; }

    bool HasEvent( const char* name ) const { return m_events.HasSubstr( CORE::CString( name ) + "," ) >= 0; }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( notifier != m_conn || m_deleted )
            return;

        const char* name = GUCEF_NULL;
        if ( CTCPClientSocket::ConnectingEvent == eventId ) name = "Connecting";
        else if ( CTlsClientConnection::HandshakeCompletedEvent == eventId ) name = "HandshakeCompleted";
        else if ( CTCPConnection::ConnectedEvent == eventId ) name = "Connected";
        else if ( CTCPConnection::DataRecievedEvent == eventId ) name = "Data";
        else if ( CTCPConnection::DataSentEvent == eventId ) name = "Sent";
        else if ( CTCPConnection::DisconnectedEvent == eventId ) name = "Disconnected";
        else if ( CTCPConnection::SocketErrorEvent == eventId ) name = "SocketError";
        else if ( CTlsClientConnection::TlsErrorEvent == eventId ) name = "TlsError";
        if ( GUCEF_NULL == name )
            return;

        // consecutive data and sent events are noted once
        CORE::CString entry = CORE::CString( name ) + ",";
        if ( !( ( CTCPConnection::DataRecievedEvent == eventId || CTCPConnection::DataSentEvent == eventId ) && m_events.EndsWith( entry ) ) )
            m_events += entry;

        if ( CTCPConnection::ConnectedEvent == eventId )
        {
            m_connected = true;
            if ( !m_sendOnConnected.IsNULLOrEmpty() )
                m_conn->Send( m_sendOnConnected.C_String(), m_sendOnConnected.Length() );
        }
        else
        if ( CTCPConnection::DataRecievedEvent == eventId )
        {
            CTCPConnection::TDataRecievedEventData* data = static_cast< CTCPConnection::TDataRecievedEventData* >( eventData );
            const char* bytes = data->GetData().AsConstTypePtr< char >();
            UInt32 size = data->GetData().GetDataSize();
            if ( GUCEF_NULL != m_upgradeSettings && !m_upgradeStarted && !m_conn->IsTlsRequested() )
            {
                // the line that says the other side is ready: what comes behind it is already ciphertext
                m_plainReceivedBeforeUpgrade.Append( bytes, size );
                if ( m_plainReceivedBeforeUpgrade.HasSubstr( "OK\n" ) >= 0 )
                {
                    m_upgradeStarted = true;
                    UInt32 consumed = 3;
                    m_upgradeResult = m_conn->StartTls( *m_upgradeSettings, m_upgradeName, bytes + consumed, size > consumed ? size - consumed : 0 );
                }
            }
            else
                m_plain.Append( bytes, size );
        }
        else
        if ( CTCPConnection::DataSentEvent == eventId )
        {
            CTCPConnection::TDataSentEventData* data = static_cast< CTCPConnection::TDataSentEventData* >( eventData );
            ++m_sentEvents;
            m_sentBytes += data->GetData().GetDataSize();
        }
        else
        if ( CTCPConnection::SocketErrorEvent == eventId )
        {
            m_gotSocketError = true;
            m_socketErrorCode = static_cast< CTCPConnection::TSocketErrorEventData* >( eventData )->GetData();
        }
        else
        if ( CTlsClientConnection::TlsErrorEvent == eventId )
        {
            m_gotTlsError = true;
            m_tlsErrorClass = static_cast< TTlsErrorEventData* >( eventData )->GetData().errorClass;
        }
        else
        if ( CTCPConnection::DisconnectedEvent == eventId )
            m_disconnected = true;

        if ( m_deleteOn == name )
        {
            // from inside an event of the notifier this is how a notifier is destroyed
            m_deleted = true;
            UnsubscribeFrom( *m_conn );
            m_conn->ScheduleForDestruction();
            m_conn = GUCEF_NULL;
        }
    }

    private:

    CTlsClientConnection* m_conn;
};

/*-------------------------------------------------------------------------*/

/** condition: the client has at least this much plaintext */
struct PlainReached
{
    PlainReached( const CTlsTestClient& client, UInt32 size ) : m_client( client ), m_size( size ) {}
    bool operator()( void ) const { return m_client.m_plain.Length() >= m_size; }
    const CTlsTestClient& m_client;
    UInt32 m_size;
};

/** condition: the handshake of the client is done */
struct HandshakeDone
{
    HandshakeDone( const CTlsTestClient& client ) : m_client( client ) {}
    bool operator()( void ) const { return m_client.HasEvent( "HandshakeCompleted" ); }
    const CTlsTestClient& m_client;
};


}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTCODE_TLSCLIENT_H ? */

/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_COM_TESTMAILSHARED_H
#define GUCEF_COM_TESTMAILSHARED_H

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

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

#ifndef GUCEF_COM_CPOP3CLIENT_H
#include "gucefCOM_CPop3Client.h"
#define GUCEF_COM_CPOP3CLIENT_H
#endif /* GUCEF_COM_CPOP3CLIENT_H ? */

#ifndef GUCEF_COM_CIMAPCLIENT_H
#include "gucefCOM_CImapClient.h"
#define GUCEF_COM_CIMAPCLIENT_H
#endif /* GUCEF_COM_CIMAPCLIENT_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "TestCode_Pump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MailTest {

using namespace GUCEF;

/** Watches the connection level events of a mail client */
class CClientWatch : public CORE::CObserver
{
    public:

    bool ready;
    UInt32 readyCount;
    bool failed;                        /**< ErrorEvent was raised */
    bool disconnected;
    bool settled;                       /**< ready, or failed, or disconnected: the end of waiting for a connect */
    COM::CMailErrorEventData error;

    CClientWatch( void )
        : CORE::CObserver(), ready( false ), readyCount( 0 ), failed( false ), disconnected( false ), settled( false ), error()
    {
    }

    virtual ~CClientWatch()
    {
        UnsubscribeAllFromObserver();
    }

    void Attach( COM::CMailClientBase& client )
    {
        typedef CORE::CTEventHandlerFunctor< CClientWatch > TCallback;
        TCallback readyCallback( this, &CClientWatch::OnReady );
        SubscribeTo( &client, COM::CMailClientBase::ReadyEvent, readyCallback );
        TCallback errorCallback( this, &CClientWatch::OnError );
        SubscribeTo( &client, COM::CMailClientBase::ErrorEvent, errorCallback );
        TCallback disconnectedCallback( this, &CClientWatch::OnDisconnected );
        SubscribeTo( &client, COM::CMailClientBase::DisconnectedEvent, disconnectedCallback );
    }

    void Reset( void )
    {
        ready = false; failed = false; disconnected = false; settled = false;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "MailTest::CClientWatch";
        return classTypeName;
    }

    private:

    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ready = true; settled = true; ++readyCount; }

    void OnError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const COM::CMailErrorEventData* data = CORE::CICloneable::CloneableAs< COM::CMailErrorEventData >( eventData );
        if ( GUCEF_NULL != data )
            error = *data;
        failed = true;
        settled = true;
    }

    void OnDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { disconnected = true; settled = true; }
};

/*-------------------------------------------------------------------------*/

class CSmtpWatch : public CClientWatch
{
    public:

    GUCEF::vector< COM::CSmtpSendResult > results;
    UInt32 resultCount;

    CSmtpWatch( void ) : CClientWatch(), results(), resultCount( 0 ) {}

    void Attach( COM::CSmtpClient& client )
    {
        CClientWatch::Attach( client );
        typedef CORE::CTEventHandlerFunctor< CSmtpWatch > TCallback;
        TCallback callback( this, &CSmtpWatch::OnSendCompleted );
        SubscribeTo( &client, COM::CSmtpClient::SendCompletedEvent, callback );
    }

    const COM::CSmtpSendResult* Find( UInt32 requestId ) const
    {
        for ( size_t i=0; i<results.size(); ++i )
            if ( results[ i ].requestId == requestId )
                return &results[ i ];
        return GUCEF_NULL;
    }

    private:

    void OnSendCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const COM::CSmtpSendResult* data = CORE::CICloneable::CloneableAs< COM::CSmtpSendResult >( eventData );
        if ( GUCEF_NULL != data )
        {
            results.push_back( *data );
            ++resultCount;
        }
    }
};

/*-------------------------------------------------------------------------*/

class CPop3Watch : public CClientWatch
{
    public:

    GUCEF::vector< COM::CPop3CommandResult > results;
    UInt32 resultCount;

    CPop3Watch( void ) : CClientWatch(), results(), resultCount( 0 ) {}

    void Attach( COM::CPop3Client& client )
    {
        CClientWatch::Attach( client );
        typedef CORE::CTEventHandlerFunctor< CPop3Watch > TCallback;
        TCallback callback( this, &CPop3Watch::OnCompleted );
        SubscribeTo( &client, COM::CPop3Client::CommandCompletedEvent, callback );
    }

    const COM::CPop3CommandResult* Find( UInt32 requestId ) const
    {
        for ( size_t i=0; i<results.size(); ++i )
            if ( results[ i ].requestId == requestId )
                return &results[ i ];
        return GUCEF_NULL;
    }

    private:

    void OnCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const COM::CPop3CommandResult* data = CORE::CICloneable::CloneableAs< COM::CPop3CommandResult >( eventData );
        if ( GUCEF_NULL != data )
        {
            results.push_back( *data );
            ++resultCount;
        }
    }
};

/*-------------------------------------------------------------------------*/

class CImapWatch : public CClientWatch
{
    public:

    GUCEF::vector< COM::CImapCommandResult > results;
    GUCEF::vector< COM::CImapIdleUpdate > updates;
    UInt32 resultCount;
    UInt32 updateCount;
    UInt32 idleStartedCount;

    CImapWatch( void ) : CClientWatch(), results(), updates(), resultCount( 0 ), updateCount( 0 ), idleStartedCount( 0 ) {}

    void Attach( COM::CImapClient& client )
    {
        CClientWatch::Attach( client );
        typedef CORE::CTEventHandlerFunctor< CImapWatch > TCallback;
        TCallback completed( this, &CImapWatch::OnCompleted );
        SubscribeTo( &client, COM::CImapClient::CommandCompletedEvent, completed );
        TCallback update( this, &CImapWatch::OnUpdate );
        SubscribeTo( &client, COM::CImapClient::IdleUpdateEvent, update );
        TCallback started( this, &CImapWatch::OnIdleStarted );
        SubscribeTo( &client, COM::CImapClient::IdleStartedEvent, started );
    }

    const COM::CImapCommandResult* Find( UInt32 requestId ) const
    {
        for ( size_t i=0; i<results.size(); ++i )
            if ( results[ i ].requestId == requestId )
                return &results[ i ];
        return GUCEF_NULL;
    }

    private:

    void OnCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const COM::CImapCommandResult* data = CORE::CICloneable::CloneableAs< COM::CImapCommandResult >( eventData );
        if ( GUCEF_NULL != data )
        {
            results.push_back( *data );
            ++resultCount;
        }
    }

    void OnUpdate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const COM::CImapIdleUpdate* data = CORE::CICloneable::CloneableAs< COM::CImapIdleUpdate >( eventData );
        if ( GUCEF_NULL != data )
        {
            updates.push_back( *data );
            ++updateCount;
        }
    }

    void OnIdleStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++idleStartedCount; }
};

/*-------------------------------------------------------------------------*/

/** Pulses the global pulse generator until the condition is true or the time is up */
template< class TCondition >
bool
PumpUntilCondition( const TCondition& condition, UInt32 timeoutInMs )
{
    for ( UInt32 waited=0; waited<timeoutInMs && !condition(); ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return condition();
}

/** condition: the counter reached the target */
struct CounterAtLeast
{
    CounterAtLeast( const UInt32& counter, UInt32 target ) : m_counter( counter ), m_target( target ) {}
    bool operator()( void ) const { return m_counter >= m_target; }
    const UInt32& m_counter;
    UInt32 m_target;
};

/** condition: the client watch settled */
struct Settled
{
    Settled( const CClientWatch& watch ) : m_watch( watch ) {}
    bool operator()( void ) const { return m_watch.settled; }
    const CClientWatch& m_watch;
};

/*-------------------------------------------------------------------------*/

/** A message with unique texts in it, so that a test can look for them in a log */
inline COM::CMailMessage
MakeMessage( const char* subject, const char* body )
{
    COM::CMailMessage message;
    message.from.push_back( COM::CMailAddress( "Alice Sender", "alice@example.org" ) );
    message.to.push_back( COM::CMailAddress( "Bob Receiver", "bob@example.com" ) );
    message.subject = subject;
    message.textBody = body;
    return message;
}

/** Whether TLS can be tested here, with the reason when not */
inline bool
TlsTestingPossible( CORE::CString& why )
{
    if ( !TESTFW::CTlsTestCerts::IsToolAvailable() )
    {
        why = "the openssl command line tool is not available";
        return false;
    }
    if ( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( why ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

template< class TSettings >
void
MakePlain( TSettings& settings, UInt16 port )
{
    settings.host = "127.0.0.1";
    settings.port = port;
    settings.tlsMode = COM::MAILTLS_NONE;
    settings.connectTimeoutInMs = 5000;
    settings.commandTimeoutInMs = 5000;
}

template< class TSettings >
void
MakeSecure( TSettings& settings, UInt16 port, const TESTFW::CTlsTestCerts& certs, COM::EMailTlsMode mode )
{
    settings.host = "127.0.0.1";
    settings.port = port;
    settings.tlsMode = mode;
    settings.connectTimeoutInMs = 8000;
    settings.commandTimeoutInMs = 5000;
    settings.tls.SetCaBundlePath( certs.GetPath( "ca.crt" ) );
}

} /* namespace MailTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTMAILSHARED_H ? */

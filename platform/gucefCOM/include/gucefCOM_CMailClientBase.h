/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMAILCLIENTBASE_H
#define GUCEF_COM_CMAILCLIENTBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COM_CIMAILTRANSPORT_H
#include "gucefCOM_CIMailTransport.h"
#define GUCEF_COM_CIMAILTRANSPORT_H
#endif /* GUCEF_COM_CIMAILTRANSPORT_H ? */

#ifndef GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#include "gucefCOM_CIMailCredentialProvider.h"
#define GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#endif /* GUCEF_COM_CIMAILCREDENTIALPROVIDER_H ? */

#ifndef GUCEF_COM_CMAILSASL_H
#include "gucefCOM_CMailSasl.h"
#define GUCEF_COM_CMAILSASL_H
#endif /* GUCEF_COM_CMAILSASL_H ? */

#ifndef GUCEF_COM_CMAILCLIENTSETTINGS_H
#include "gucefCOM_CMailClientSettings.h"
#define GUCEF_COM_CMAILCLIENTSETTINGS_H
#endif /* GUCEF_COM_CMAILCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CMAILPROTOCOLLOG_H
#include "gucefCOM_CMailProtocolLog.h"
#define GUCEF_COM_CMAILPROTOCOLLOG_H
#endif /* GUCEF_COM_CMAILPROTOCOLLOG_H ? */

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

/** What went wrong with a connection or a command. The description never has credentials or message content in it */
class GUCEF_COM_PUBLIC_CPP CMailErrorEventData : public CORE::CICloneable
{
    public:

    EMailErrorClass errorClass;
    CORE::CString description;
    UInt32 serverCode;                  /**< the numeric reply of the server (SMTP, 0 when there was none) */
    CORE::CString enhancedCode;         /**< the enhanced status code of an SMTP reply, for example 5.1.1 */

    CMailErrorEventData( void );
    CMailErrorEventData( EMailErrorClass errorClass, const CORE::CString& description );
    CMailErrorEventData( const CMailErrorEventData& src );
    virtual ~CMailErrorEventData() GUCEF_VIRTUAL_OVERRIDE;
    CMailErrorEventData& operator=( const CMailErrorEventData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  What the SMTP, POP3 and IMAP clients have in common: the transport and its events, the TLS policy, timeouts,
 *  authentication with SASL and the credential provider, and the failure handling.
 *
 *  A client is driven by the pulses of the pulse generator it was made with; it never blocks and never creates a
 *  thread. A failure of the connection (the server is gone, a timeout, a protocol error, a refused login) is
 *  reported once: the pending requests are completed with the error, ErrorEvent and then DisconnectedEvent are
 *  raised and the client is disconnected. The failure of one command is not a failure of the connection and is
 *  reported in the result of that command.
 */
class GUCEF_COM_PUBLIC_CPP CMailClientBase : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ReadyEvent;           /**< the server accepted the login (or none was needed): commands can be given */
    static const CORE::CEvent ErrorEvent;           /**< TErrorEventData: the connection failed, DisconnectedEvent follows */
    static const CORE::CEvent DisconnectedEvent;    /**< the connection is gone */

    typedef CMailErrorEventData TErrorEventData;

    enum EClientState
    {
        CLIENTSTATE_DISCONNECTED = 0,
        CLIENTSTATE_CONNECTING,         /**< connecting, TLS, waiting for the greeting and the capabilities */
        CLIENTSTATE_AUTHENTICATING,
        CLIENTSTATE_READY
    };

    static void RegisterEvents( void );

    virtual ~CMailClientBase() GUCEF_VIRTUAL_OVERRIDE;

    /** Replaces the transport (before Connect). The client takes ownership. Without it a CMailTcpTransport on the pulse generator of the client is made */
    void SetTransport( CIMailTransport* transport );

    /** Where the credentials come from, instead of the user name and password of the settings. Not owned, has to outlive the client */
    void SetCredentialProvider( CIMailCredentialProvider* provider );

    EClientState GetState( void ) const;

    bool IsReady( void ) const;

    /** The last failure of the connection */
    const CMailErrorEventData& GetLastError( void ) const;

    /** The name of the protocol, SMTP, POP3 or IMAP */
    virtual const CORE::CString& GetProtocolName( void ) const = 0;

    /** Ends the session politely when it can (QUIT, LOGOUT) and closes the connection */
    void Disconnect( void );

    /** Drives the pulse generator, from the calling thread, until the flag is true or the time is up. @return the flag */
    static bool RunPulseUntil( CORE::PulseGeneratorPtr pulseGenerator, const bool& flag, UInt32 timeoutInMs );

    /** A pulse generator for a thread that wants to run a client by itself: the blocking use of the clients */
    static CORE::PulseGeneratorPtr CreateLocalPulseGenerator( void );

    static UInt64 GetTimeInMs( void );

    protected:

    explicit CMailClientBase( const CORE::PulseGeneratorPtr& pulseGenerator );

    /** Validates the settings, makes the transport and starts connecting. 'settings' has to stay valid as long as the client lives */
    bool BeginConnect( const CMailClientSettings& settings );

    /** The line, written to the log (redacted) and sent. 'secret': it is part of an authentication exchange */
    bool SendCommandLine( const CORE::CString& line, bool secret = false );

    bool SendDataBlock( const void* data, UInt32 size, const char* what );

    /** The wait for the server: when it passes, the client fails with MAILERR_TIMEOUT and says 'what' it was waiting for */
    void SetDeadline( UInt32 timeoutInMs, const char* what );

    void ClearDeadline( void );

    /** Starts the wait for the server over, what the server sent is no silence. Called for every line and block that arrives */
    void RearmDeadline( void );

    /** Whether the line is written to the log; a protocol returns false for lines that carry the content of messages */
    virtual bool IsServerLineLoggable( const CORE::CString& line ) const;

    /** Fails the connection. The description must not have secrets or content of messages */
    void Fail( EMailErrorClass errorClass, const CORE::CString& description, UInt32 serverCode = 0, const CORE::CString& enhancedCode = CORE::CString::Empty );

    void SetState( EClientState state );

    /** Called once the client is ready, raises ReadyEvent */
    void SetReady( void );

    UInt32 NextRequestId( void );

    CIMailTransport& GetTransport( void );

    const CMailProtocolLog& GetLog( void ) const;

    const CMailClientSettings& GetBaseSettings( void ) const;

    /** What a server that offers STARTTLS (or not) means for the settings */
    enum ETlsAction
    {
        TLSACTION_NONE = 0,     /**< stay as it is: already protected, or plain text is what the settings accept */
        TLSACTION_STARTTLS,     /**< upgrade now */
        TLSACTION_FAILED        /**< STARTTLS is required and not offered: the client failed */
    };

    ETlsAction DecideTls( bool serverOffersStartTls );

    /** Starts the handshake. The server has agreed already. @return false (and the client failed) when it cannot */
    bool BeginTlsUpgrade( void );

    /** Whether credentials may be sent on this connection: with TLS, or when the settings allow it */
    bool IsCredentialExposureAllowed( void ) const;

    /** Whether a login is wanted: the settings have a user name or there is a provider */
    bool WantsLogin( void ) const;

    /** A credential of the provider, for logins that do not use SASL (LOGIN, USER and PASS). False when there is none ready; the client has not failed then */
    bool GetCredentialNow( CMailCredential& credential, bool previousAttemptFailed );

    /**
     *  Picks a mechanism from what the server offers and what the settings prefer, gets the credential and starts
     *  the exchange with SendSaslStart(). The credential may need time: the exchange then starts when the provider
     *  says it is ready. Fails the client (and returns false) when no mechanism can be used.
     *  @param allowInitialResponse the command that starts the exchange can carry the first response
     */
    bool BeginSasl( const CORE::CString::StringVector& serverMechanisms, bool allowInitialResponse );

    /** The base64 text of a challenge of the server. Answers it with SendSaslResponse(), or fails the client */
    void OnSaslChallenge( const CORE::CString& base64Challenge );

    /** The server accepted the login */
    void OnSaslSucceeded( void );

    /** The server refused the login. A token is renewed through the provider and the exchange is tried once more. @return true when it was started again */
    bool OnSaslRejected( const CORE::CString& serverText, UInt32 serverCode );

    bool IsSaslActive( void ) const;

    /** Starts the exchange: the command with the name of the mechanism and the first response when there is one. Returns false when the line cannot be sent */
    virtual bool SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse ) = 0;

    /** Answers a challenge, an empty text is an empty response */
    virtual bool SendSaslResponse( const CORE::CString& base64Response ) = 0;

    /** Gives the exchange up ("*") */
    virtual void SendSaslCancel( void ) = 0;

    /** The protocol sends its farewell (QUIT, LOGOUT) */
    virtual void SendQuitCommand( void ) = 0;

    /** The line of the server, after it was logged */
    virtual void OnServerLine( const CORE::CString& line ) = 0;

    /** A counted block the client asked the transport for */
    virtual void OnServerBytes( const CORE::CDynamicBuffer& bytes );

    /** The connection is there; TLS from the start included. The greeting is yet to come */
    virtual void OnConnectionEstablished( void );

    /** The handshake after STARTTLS is done */
    virtual void OnTlsEstablished( void ) = 0;

    /** The connection failed: requests that are waiting get their error */
    virtual void OnClientFailed( const CMailErrorEventData& error ) = 0;

    /** The connection was closed because Disconnect() was called */
    virtual void OnClientClosed( void );

    /** A deadline passed: the default fails the client with MAILERR_TIMEOUT. IMAP uses it to renew IDLE */
    virtual void OnTimerTick( UInt64 nowInMs );

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CMailClientBase > TEventCallback;

    CMailClientBase( const CMailClientBase& src );
    CMailClientBase& operator=( const CMailClientBase& src );

    void SubscribeToTransport( void );
    void OnTransportConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportTlsEstablished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportLine( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportBytes( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTransportError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCredentialReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void MakeCredentialRequest( CMailCredentialRequest& request, bool previousAttemptFailed ) const;
    bool StartSaslWithCredential( void );
    void ResetSasl( void );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CIMailTransport* m_transport;
    CORE::CTimer m_timer;
    const CMailClientSettings* m_settings;
    CMailProtocolLog m_log;
    CIMailCredentialProvider* m_externalProvider;
    CStaticMailCredentialProvider* m_builtInProvider;
    EClientState m_state;
    CMailErrorEventData m_lastError;
    UInt32 m_nextRequestId;
    UInt64 m_deadlineAt;                    /**< 0: none */
    UInt32 m_deadlineWindow;
    CORE::CString m_deadlineWhat;
    bool m_closing;
    bool m_failureReported;

    CIMailSaslMechanism* m_sasl;
    CORE::CString::StringVector m_saslServerMechanisms;
    bool m_saslAllowInitialResponse;
    bool m_saslAwaitingCredential;
    bool m_saslFailedLocally;
    bool m_saslRetried;
    CORE::CDynamicBuffer m_saslDeferredInitial;
    bool m_saslHasDeferredInitial;
    CMailCredentialRequest m_credentialRequest;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILCLIENTBASE_H ? */

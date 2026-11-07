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

#ifndef GUCEF_COM_CSMTPCLIENT_H
#define GUCEF_COM_CSMTPCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

#ifndef GUCEF_COM_CSMTPCLIENTSETTINGS_H
#include "gucefCOM_CSmtpClientSettings.h"
#define GUCEF_COM_CSMTPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CSMTPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

#ifndef GUCEF_COM_CSMTPREPLY_H
#include "gucefCOM_CSmtpReply.h"
#define GUCEF_COM_CSMTPREPLY_H
#endif /* GUCEF_COM_CSMTPREPLY_H ? */

#ifndef GUCEF_COM_CSMTPSENDRESULT_H
#include "gucefCOM_CSmtpSendResult.h"
#define GUCEF_COM_CSMTPSENDRESULT_H
#endif /* GUCEF_COM_CSMTPSENDRESULT_H ? */

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
 *  An SMTP client (RFC 5321) for submission and relay: EHLO with HELO as the fallback, STARTTLS or TLS from the
 *  start, AUTH with the mechanisms of CEMailSaslFactory, SIZE, PIPELINING (MAIL and RCPT in one write), 8BITMIME and
 *  SMTPUTF8, transparency (dot stuffing) and the results of every recipient.
 *
 *      CSmtpClient client;
 *      SubscribeTo( &client, CSmtpClient::SendCompletedEvent, callback );
 *      client.Connect( settings );
 *      client.SendMessage( message );
 *
 *  Messages are sent one after another over the connection, in the order they were given. A message can be given
 *  before the connection is ready. The client keeps the connection until Disconnect(); a connection that fails
 *  completes the messages that were waiting with the error.
 *
 *  For one message from code that does not run a pulse generator there are SendMessageSync() (blocks the calling
 *  thread) and CEMailOperations::SendMessageAsync() (a task of a thread pool, a CFutureResult).
 */
class GUCEF_COM_PUBLIC_CPP CSmtpClient : public CEMailClientBase
{
    public:

    static const CORE::CEvent SendCompletedEvent;       /**< TSendCompletedEventData: a message is sent or has failed */

    typedef CSmtpSendResult TSendCompletedEventData;

    static void RegisterEvents( void );

    CSmtpClient( void );

    explicit CSmtpClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CSmtpClient() GUCEF_VIRTUAL_OVERRIDE;

    /** Connects with a copy of the settings. Ready is signalled by ReadyEvent, the connection is logged in then */
    bool Connect( const CSmtpClientSettings& settings );

    /**
     *  Queues a message. The recipients are to, cc and bcc, the sender of the envelope is Sender, else the first From.
     *  @return the id that SendCompletedEvent carries, 0 when the message is refused right away (invalid, too large, not connected); 'error' has the reason
     */
    UInt32 SendMessage( const CEMailMessage& message, CORE::CString* error = GUCEF_NULL );

    /** Queues message data that was composed elsewhere: bytes of a message in the format of RFC 5322 */
    UInt32 SendRaw( const CEMailAddress& envelopeFrom, const CEMailAddress::TEMailAddressVector& recipients, const void* data, UInt32 size, CORE::CString* error = GUCEF_NULL );

    /** Messages queued or being sent */
    UInt32 GetPendingCount( void ) const;

    /** Whether the server offered the extension (SIZE, PIPELINING, 8BITMIME, SMTPUTF8, STARTTLS...) in its last EHLO */
    bool HasExtension( const CORE::CString& name ) const;

    /** The SIZE the server announced, 0 for none */
    UInt32 GetServerMaxSize( void ) const;

    const CSmtpClientSettings& GetSettings( void ) const;

    virtual const CORE::CString& GetProtocolName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Message data as it goes over the wire: every line end is CR LF (a bare CR or LF is a way to smuggle
     *  commands past a server), a line that starts with a dot gets a second one, the data ends with CR LF and the
     *  terminating ".", CR LF.
     */
    static void PrepareData( const void* data, UInt32 size, CORE::CDynamicBuffer& stuffed );

    /**
     *  Sends one message from the calling thread: makes a pulse generator of its own, connects, sends, says QUIT
     *  and returns. @return whether the message was accepted by the server
     */
    static bool SendMessageSync( const CSmtpClientSettings& settings, const CEMailMessage& message, CSmtpSendResult& result, CIEMailCredentialProvider* credentialProvider = GUCEF_NULL );

    protected:

    virtual bool SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendSaslResponse( const CORE::CString& base64Response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendSaslCancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendQuitCommand( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnConnectionEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTlsEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientFailed( const CEMailErrorEventData& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientClosed( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EStep
    {
        STEP_GREETING = 0,
        STEP_EHLO,
        STEP_HELO,
        STEP_STARTTLS,
        STEP_AUTH,
        STEP_MAIL,
        STEP_RCPT,
        STEP_DATA,
        STEP_DATA_END,
        STEP_RSET
    };

    struct SJob
    {
        UInt32 requestId;
        CORE::CString envelopeFrom;
        GUCEF::vector< CORE::CString > recipients;
        CORE::CDynamicBuffer data;              /**< as given */
        bool needsUtf8;
        bool isEightBit;
        bool mailFailed;
        GUCEF::vector< CSmtpRecipientResult > results;
        UInt32 rcptSent;
        UInt32 rcptReplies;
        CSmtpSendResult failure;                /**< what decides when the job fails */
        bool hasFailure;
    };

    CSmtpClient( const CSmtpClient& src );
    CSmtpClient& operator=( const CSmtpClient& src );

    void Initialize( void );
    UInt32 QueueJob( const CORE::CString& envelopeFrom, const CEMailAddress::TEMailAddressVector& recipients, const void* data, UInt32 size, bool needsUtf8, CORE::CString* error );
    void Expect( EStep step );
    void SendEhlo( void );
    void OnReply( const CSmtpReply& reply );
    void OnEhloReply( const CSmtpReply& reply, bool isHelo );
    void ParseExtensions( const CSmtpReply& reply );
    void ProceedToLogin( void );
    void ProcessQueue( void );
    void StartJob( void );
    void SendNextRecipient( void );
    void OnRecipientsDone( void );
    void SetJobFailure( EMailErrorClass errorClass, const CSmtpReply* reply, const char* description );
    void CompleteCurrentJob( bool success, const CSmtpReply* finalReply );
    void CompleteJob( SJob* job, bool success, const CSmtpReply* finalReply );
    void FailAllJobs( EMailErrorClass errorClass, const CORE::CString& description );
    void UpdateDeadline( void );
    static CORE::CString SanitizeHeloName( const CORE::CString& name );

    private:

    CSmtpClientSettings m_settings;
    CORE::CString m_receivedText;
    CSmtpReply m_reply;
    GUCEF::vector< EStep > m_expected;
    GUCEF::map< CORE::CString, CORE::CString > m_extensions;
    bool m_triedHelo;
    bool m_authenticated;
    bool m_isHelo;
    UInt32 m_replyBytes;
    GUCEF::vector< SJob* > m_jobs;
    SJob* m_current;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

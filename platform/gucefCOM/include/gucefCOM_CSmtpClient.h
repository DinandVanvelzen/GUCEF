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

#ifndef GUCEF_COM_CMAILCLIENTBASE_H
#include "gucefCOM_CMailClientBase.h"
#define GUCEF_COM_CMAILCLIENTBASE_H
#endif /* GUCEF_COM_CMAILCLIENTBASE_H ? */

#ifndef GUCEF_COM_CSMTPCLIENTSETTINGS_H
#include "gucefCOM_CSmtpClientSettings.h"
#define GUCEF_COM_CSMTPCLIENTSETTINGS_H
#endif /* GUCEF_COM_CSMTPCLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CMAILMESSAGE_H
#include "gucefCOM_CMailMessage.h"
#define GUCEF_COM_CMAILMESSAGE_H
#endif /* GUCEF_COM_CMAILMESSAGE_H ? */

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

/** One reply of an SMTP server, which may span several lines */
class GUCEF_COM_PUBLIC_CPP CSmtpReply
{
    public:

    UInt32 code;                                /**< the three digits */
    CORE::CString enhancedCode;                 /**< "2.0.0", empty when the server gave none */
    CORE::CString::StringVector lines;          /**< the text of every line, without code and enhanced code */

    CSmtpReply( void );

    /** The text of all lines joined by a space */
    CORE::CString GetText( void ) const;

    /** What kind of failure a reply with this code is: 4xx temporary, 5xx permanent, 2xx and 3xx none */
    static EMailErrorClass GetErrorClass( UInt32 code );

    /**
     *  Takes a line of a reply apart: "250-text" continues the reply, "250 text" ends it. A code without text ("250") ends it.
     *  @return false when the line does not start with three digits followed by a space, a hyphen or the end
     */
    static bool ParseLine( const CORE::CString& line, UInt32& code, bool& isLastLine, CORE::CString& text );

    /** Reads an enhanced status code (RFC 3463) from the start of the text, "5.1.1 No such user" gives "5.1.1" and the rest of the text */
    static bool SplitEnhancedCode( const CORE::CString& text, UInt32 replyCode, CORE::CString& enhancedCode, CORE::CString& rest );
};

/*-------------------------------------------------------------------------*/

/** What the server said about one recipient */
class GUCEF_COM_PUBLIC_CPP CSmtpRecipientResult
{
    public:

    CORE::CString address;
    bool accepted;
    UInt32 replyCode;
    CORE::CString enhancedCode;
    CORE::CString replyText;
    EMailErrorClass errorClass;

    CSmtpRecipientResult( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The outcome of sending one message. 'success' means the server accepted the message for the recipients that it
 *  accepted (see 'recipients'); the other fields say what failed otherwise, 'errorClass' tells a failure
 *  that is worth another try later (MAILERR_TEMPORARY, MAILERR_CONNECTION, MAILERR_TIMEOUT) from one that is not.
 */
class GUCEF_COM_PUBLIC_CPP CSmtpSendResult : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    bool success;
    EMailErrorClass errorClass;
    UInt32 replyCode;                           /**< the reply that decided: to the end of the data when it was accepted */
    CORE::CString enhancedCode;
    CORE::CString replyText;                    /**< what the server said, for example 'OK queued as 4F2A1'. Not for logs without redaction */
    CORE::CString errorDescription;             /**< a phrase of ours, never has addresses or content */
    GUCEF::vector< CSmtpRecipientResult > recipients;

    CSmtpSendResult( void );
    CSmtpSendResult( const CSmtpSendResult& src );
    virtual ~CSmtpSendResult() GUCEF_VIRTUAL_OVERRIDE;
    CSmtpSendResult& operator=( const CSmtpSendResult& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );

    UInt32 GetAcceptedCount( void ) const;
    UInt32 GetRejectedCount( void ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  An SMTP client (RFC 5321) for submission and relay: EHLO with HELO as the fallback, STARTTLS or TLS from the
 *  start, AUTH with the mechanisms of CMailSasl, SIZE, PIPELINING (MAIL and RCPT in one write), 8BITMIME and
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
 *  thread) and CMailOperations::SendMessageAsync() (a task of a thread pool, a CFutureResult).
 */
class GUCEF_COM_PUBLIC_CPP CSmtpClient : public CMailClientBase
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
    UInt32 SendMessage( const CMailMessage& message, CORE::CString* error = GUCEF_NULL );

    /** Queues message data that was composed elsewhere: bytes of a message in the format of RFC 5322 */
    UInt32 SendRaw( const CMailAddress& envelopeFrom, const CMailAddress::TMailAddressVector& recipients, const void* data, UInt32 size, CORE::CString* error = GUCEF_NULL );

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
    static bool SendMessageSync( const CSmtpClientSettings& settings, const CMailMessage& message, CSmtpSendResult& result, CIMailCredentialProvider* credentialProvider = GUCEF_NULL );

    protected:

    virtual bool SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendSaslResponse( const CORE::CString& base64Response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendSaslCancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendQuitCommand( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnConnectionEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTlsEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientFailed( const CMailErrorEventData& error ) GUCEF_VIRTUAL_OVERRIDE;
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
    UInt32 QueueJob( const CORE::CString& envelopeFrom, const CMailAddress::TMailAddressVector& recipients, const void* data, UInt32 size, bool needsUtf8, CORE::CString* error );
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

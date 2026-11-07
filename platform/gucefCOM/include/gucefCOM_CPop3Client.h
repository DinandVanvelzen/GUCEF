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

#ifndef GUCEF_COM_CPOP3CLIENT_H
#define GUCEF_COM_CPOP3CLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

#ifndef GUCEF_COM_CPOP3CLIENTSETTINGS_H
#include "gucefCOM_CPop3ClientSettings.h"
#define GUCEF_COM_CPOP3CLIENTSETTINGS_H
#endif /* GUCEF_COM_CPOP3CLIENTSETTINGS_H ? */

#ifndef GUCEF_COM_CEMAILMESSAGE_H
#include "gucefCOM_CEMailMessage.h"
#define GUCEF_COM_CEMAILMESSAGE_H
#endif /* GUCEF_COM_CEMAILMESSAGE_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_COM_CPOP3COMMANDRESULT_H
#include "gucefCOM_CPop3CommandResult.h"
#define GUCEF_COM_CPOP3COMMANDRESULT_H
#endif /* GUCEF_COM_CPOP3COMMANDRESULT_H ? */

#ifndef GUCEF_COM_CPOP3FETCHOPTIONS_H
#include "gucefCOM_CPop3FetchOptions.h"
#define GUCEF_COM_CPOP3FETCHOPTIONS_H
#endif /* GUCEF_COM_CPOP3FETCHOPTIONS_H ? */

#ifndef GUCEF_COM_CPOP3FETCHRESULT_H
#include "gucefCOM_CPop3FetchResult.h"
#define GUCEF_COM_CPOP3FETCHRESULT_H
#endif /* GUCEF_COM_CPOP3FETCHRESULT_H ? */

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
 *  A POP3 client (RFC 1939) with CAPA (RFC 2449), STLS (RFC 2595) and AUTH (RFC 5034, with PLAIN, LOGIN, SCRAM-SHA-256
 *  and XOAUTH2 through CEMailSaslFactory), USER and PASS, and APOP when the settings ask for it and allow it.
 *
 *  Commands are given when the client is ready and are carried out one after another; every command raises
 *  CommandCompletedEvent with its result. The unique id of a message for de-duplication is the UIDL, the numbers
 *  are only good for the session. Nothing is deleted unless Dele() is called (or deleteAfterFetch asked for).
 *
 *      CPop3Client client;
 *      SubscribeTo( &client, CPop3Client::ReadyEvent, ... );
 *      SubscribeTo( &client, CPop3Client::CommandCompletedEvent, ... );
 *      client.Connect( settings );
 *      // when ready:  client.Uidl();  client.Retr( 3 );
 *
 *  For one shot use FetchNewSync() (blocks the calling thread) and CEMailOperations::FetchNewPop3Async().
 */
class GUCEF_COM_PUBLIC_CPP CPop3Client : public CEMailClientBase
{
    public:

    static const CORE::CEvent CommandCompletedEvent;    /**< TCommandCompletedEventData */

    typedef CPop3CommandResult TCommandCompletedEventData;

    static void RegisterEvents( void );

    CPop3Client( void );

    explicit CPop3Client( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CPop3Client() GUCEF_VIRTUAL_OVERRIDE;

    bool Connect( const CPop3ClientSettings& settings );

    /** The number of messages and their total size */
    UInt32 Stat( void );

    /** The size of every message, or of one */
    UInt32 List( UInt32 messageNumber = 0 );

    /** The unique id of every message, or of one */
    UInt32 Uidl( UInt32 messageNumber = 0 );

    /** The message */
    UInt32 Retr( UInt32 messageNumber );

    /** The headers of the message and the first 'lines' lines of its body */
    UInt32 Top( UInt32 messageNumber, UInt32 lines );

    /** Marks the message for deletion, which the server does when the session ends properly (Disconnect) */
    UInt32 Dele( UInt32 messageNumber );

    UInt32 Rset( void );

    UInt32 Noop( void );

    /** Whether the server listed the capability (CAPA) of the name, "UIDL" or "TOP" for example */
    bool HasCapability( const CORE::CString& name ) const;

    const CPop3ClientSettings& GetSettings( void ) const;

    virtual const CORE::CString& GetProtocolName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Fetches what is new in the mailbox from the calling thread: connects (on a pulse generator that this thread owns),
     *  reads the UIDL of all messages, retrieves the ones not in options.knownUids, parses them, deletes them when asked, and says QUIT.
     */
    static bool FetchNewSync( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, CPop3FetchResult& result, CIEMailCredentialProvider* credentialProvider = GUCEF_NULL );

    protected:

    virtual bool SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendSaslResponse( const CORE::CString& base64Response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendSaslCancel( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SendQuitCommand( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnServerBytes( const CORE::CDynamicBuffer& bytes ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnConnectionEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTlsEstablished( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientFailed( const CEMailErrorEventData& error ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnClientClosed( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EPhase
    {
        PHASE_GREETING = 0,
        PHASE_CAPA,
        PHASE_STLS,
        PHASE_AUTH,
        PHASE_USER,
        PHASE_PASS,
        PHASE_APOP,
        PHASE_READY
    };

    struct SCommand
    {
        UInt32 requestId;
        EPop3Command command;
        UInt32 number;
        UInt32 lines;
    };

    CPop3Client( const CPop3Client& src );
    CPop3Client& operator=( const CPop3Client& src );

    void Initialize( void );
    UInt32 Queue( EPop3Command command, UInt32 number, UInt32 lines );
    void SendCapa( void );
    void SendNextCommand( void );
    void OnStatus( bool ok, const CORE::CString& text );
    void OnContinuation( const CORE::CString& text );
    void OnCapabilities( void );
    void ProceedToLogin( void );
    void OnLoggedIn( void );
    void CompleteCommand( bool ok, const CORE::CString& text );
    void FailAllCommands( EMailErrorClass errorClass, const CORE::CString& description );
    static void Unstuff( const CORE::CDynamicBuffer& stuffed, CORE::CDynamicBuffer& message );

    private:

    CPop3ClientSettings m_settings;
    EPhase m_phase;
    CORE::CString m_timestamp;                  /**< the APOP timestamp of the greeting */
    GUCEF::map< CORE::CString, CORE::CString > m_capabilities;
    GUCEF::vector< SCommand > m_queue;
    bool m_commandSent;
    bool m_collectingLines;
    bool m_awaitingBytes;
    CPop3CommandResult m_result;
    UInt32 m_collectedLines;
    UInt32 m_collectedBytes;
    CORE::CString m_pendingUser;
    bool m_capaKnown;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CPOP3CLIENT_H ? */

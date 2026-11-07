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

#ifndef GUCEF_COM_CIEMAILTRANSPORT_H
#define GUCEF_COM_CIEMAILTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

#ifndef GUCEF_COM_CEMAILTRANSPORTERRORDATA_H
#include "gucefCOM_CEMailTransportErrorData.h"
#define GUCEF_COM_CEMAILTRANSPORTERRORDATA_H
#endif /* GUCEF_COM_CEMAILTRANSPORTERRORDATA_H ? */

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
 *  Carries the lines of a mail protocol to and from a server, and counted blocks of bytes for the protocols that
 *  have them (IMAP literals). The lines are without their CR LF in both directions. The interface exists so that the
 *  clients do not depend on sockets: CEMailTcpTransport is the implementation, a test can bring a transport that
 *  feeds the client any bytes it wants, a tunnel can bring another.
 *
 *  What a transport guarantees: a line that is longer than the limit, a block larger than the limit and a line
 *  to send with CR, LF or NUL in it are errors, never data.
 */
class GUCEF_COM_PUBLIC_CPP CIEMailTransport : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ConnectedEvent;           /**< the connection is there, with TLS when it was asked for from the start */
    static const CORE::CEvent TlsEstablishedEvent;      /**< the handshake of StartTls() is done */
    static const CORE::CEvent LineReceivedEvent;        /**< TLineReceivedEventData: one line */
    static const CORE::CEvent BytesReceivedEvent;       /**< TBytesReceivedEventData: the block that ExpectBytes() asked for */
    static const CORE::CEvent DisconnectedEvent;        /**< the connection is gone, by either side */
    static const CORE::CEvent TransportErrorEvent;      /**< TTransportErrorEventData */

    typedef CORE::TCloneableString TLineReceivedEventData;
    typedef CORE::TLinkedCloneableBuffer TBytesReceivedEventData;
    typedef CEMailTransportErrorData TTransportErrorEventData;

    static void RegisterEvents( void );

    CIEMailTransport( void );

    virtual ~CIEMailTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts connecting. With TLS settings the connection is protected from the first byte and the host is the name the certificate is checked for */
    virtual bool Connect( const CORE::CString& host, UInt16 port, const COMCORE::CTlsSettings* implicitTlsSettings ) = 0;

    virtual void Disconnect( void ) = 0;

    /** Sends the line and CR LF. Refused (false, and an error event) when the line has CR, LF or NUL */
    virtual bool SendLine( const CORE::CString& line ) = 0;

    /** Sends bytes as they are: message data, literals */
    virtual bool SendData( const void* data, UInt32 size ) = 0;

    /**
     *  Upgrades the connection to TLS now. Refused when unread bytes are buffered, which is
     *  what the injection attack on STARTTLS looks like: the plain text of the attacker must never be read as if it
     *  came through TLS. The error event comes with it, and the connection is closed.
     */
    virtual bool StartTls( const COMCORE::CTlsSettings& settings, const CORE::CString& serverName ) = 0;

    /** The next 'count' bytes come as one BytesReceivedEvent, before any line. False when the count is above the limit */
    virtual bool ExpectBytes( UInt32 count ) = 0;

    /** The bytes up to the line with a single dot (a POP3 multi line response) come as one BytesReceivedEvent, as they are. Messages of more than 'maxBytes' are an error */
    virtual bool ExpectDotTerminatedBytes( UInt32 maxBytes ) = 0;

    virtual void SetLimits( UInt32 maxLineLength, UInt32 maxBytes ) = 0;

    virtual bool HasBufferedData( void ) const = 0;

    virtual bool IsConnected( void ) const = 0;

    /** Whether the connection is protected by TLS */
    virtual bool IsSecure( void ) const = 0;

    private:

    CIEMailTransport( const CIEMailTransport& src );
    CIEMailTransport& operator=( const CIEMailTransport& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIEMAILTRANSPORT_H ? */

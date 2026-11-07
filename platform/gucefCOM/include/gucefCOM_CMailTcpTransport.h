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

#ifndef GUCEF_COM_CMAILTCPTRANSPORT_H
#define GUCEF_COM_CMAILTCPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_COM_CIMAILTRANSPORT_H
#include "gucefCOM_CIMailTransport.h"
#define GUCEF_COM_CIMAILTRANSPORT_H
#endif /* GUCEF_COM_CIMAILTRANSPORT_H ? */

#ifndef GUCEF_COM_CMAILSTREAMREADER_H
#include "gucefCOM_CMailStreamReader.h"
#define GUCEF_COM_CMAILSTREAMREADER_H
#endif /* GUCEF_COM_CMAILSTREAMREADER_H ? */

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
 *  The mail transport on a CTlsClientConnection: plain TCP, TLS from the first byte, or plain TCP upgraded to TLS
 *  with StartTls(). Everything happens on the thread of the pulse generator given to the constructor; nothing is
 *  pumped here.
 *
 *  Received bytes go through a CMailStreamReader. The events are raised after each item, and the transport
 *  looks at its own state again after every event: an observer may close the connection, ask for a counted block
 *  or destroy the transport with ScheduleForDestruction().
 */
class GUCEF_COM_PUBLIC_CPP CMailTcpTransport : public CIMailTransport
{
    public:

    CMailTcpTransport( void );

    explicit CMailTcpTransport( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CMailTcpTransport() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Connect( const CORE::CString& host, UInt16 port, const COMCORE::CTlsSettings* implicitTlsSettings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendData( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool StartTls( const COMCORE::CTlsSettings& settings, const CORE::CString& serverName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ExpectBytes( UInt32 count ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ExpectDotTerminatedBytes( UInt32 maxBytes ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetLimits( UInt32 maxLineLength, UInt32 maxBytes ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool HasBufferedData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSecure( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMailTcpTransport > TEventCallback;

    CMailTcpTransport( const CMailTcpTransport& src );
    CMailTcpTransport& operator=( const CMailTcpTransport& src );

    void Initialize( const CORE::PulseGeneratorPtr& pulseGenerator );
    void RegisterSocketEventHandlers( void );
    void Drain( void );
    void ReportError( EMailErrorClass errorClass, const CORE::CString& description );

    void OnSocketConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDataReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketTlsError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketHandshakeCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    COMCORE::CTlsClientConnection* m_connection;    /**< not a member: it has to outlive a notification that was started by it and ended in our destruction */
    CMailStreamReader m_reader;
    CORE::CDynamicBuffer m_sendBuffer;
    CORE::CString m_host;
    CORE::CString m_tlsFailure;
    bool m_isConnected;
    bool m_isSecure;
    bool m_upgrading;
    bool m_closing;
    UInt32 m_generation;                            /**< changes whenever the connection is closed or started again */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILTCPTRANSPORT_H ? */

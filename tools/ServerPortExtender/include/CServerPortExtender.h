/*
 *  ServerPortExtender: Generic networking tool for connecting clients
 *  to a server that is behind a NAT firewall.
 *
 *  Copyright (C) 2002 - 2009.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef CSERVERPORTEXTENDER_H
#define CSERVERPORTEXTENDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#include "CCyclicDynamicBuffer.h"
#define GUCEF_CORE_CCYCLICDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CCYCLICDYNAMICBUFFER_H ? */

#ifndef SERVERPORTEXTENDERPROTOCOL_H
#include "ServerPortExtenderProtocol.h"
#define SERVERPORTEXTENDERPROTOCOL_H
#endif /* SERVERPORTEXTENDERPROTOCOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CServerPortExtender : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent ControlSocketOpenedEvent;
    static const CORE::CEvent ReversedSocketOpenedEvent;
    static const CORE::CEvent ClientSocketOpenedEvent;
    static const CORE::CEvent ControlConnectionEstablishedEvent;
    
    static void RegisterEvents( void );
    
    public:
    
    CServerPortExtender( CORE::PulseGeneratorPtr pulseGenerator );
    
    virtual ~CServerPortExtender();
    
    bool ListenForClientsOnPort( CORE::UInt16 port );
    
    bool ListenForReversedControlClientOnPort( CORE::UInt16 port );
    
    bool ListenForReversedClientsOnPort( CORE::UInt16 port );
    
    bool IsControlConnectionInitialized( void ) const;
    
    void Disconnect( void );

    private:
    
    typedef CORE::CTEventHandlerFunctor< CServerPortExtender > TEventCallback;
    
    COMCORE::CTCPServerConnection* GetLocalConnectionForRemoteConnection( CORE::UInt32 rsSocketId );
    
    COMCORE::CTCPServerConnection* GetRemoteConnectionForLocalConnection( CORE::UInt32 localSocketId );

    void OnControlMsg( TServerPortExtenderProtocolEnum msgType ,
                       const CORE::UInt8* data                 );

    virtual void OnPulse( CORE::CNotifier* notifier           ,
                          const CORE::CEvent& eventid         ,
                          CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL );
    
    virtual void OnServerSocketNotify( CORE::CNotifier* notifier           ,
                                       const CORE::CEvent& eventid         ,
                                       CORE::CICloneable* eventdata = GUCEF_NULL );
                                       
    void OnServerSocketClientConnected( CORE::CNotifier* notifier           ,
                                        const CORE::CEvent& eventid         ,
                                        CORE::CICloneable* eventdata = GUCEF_NULL );
                                                    
    virtual void OnReversedServerSocketNotify( CORE::CNotifier* notifier           ,
                                               const CORE::CEvent& eventid         ,
                                               CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnReversedServerControlSocketNotify( CORE::CNotifier* notifier           ,
                                                      const CORE::CEvent& eventid         ,
                                                      CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSControlClientConnected( CORE::CNotifier* notifier           ,
                                             const CORE::CEvent& eventid         ,
                                             CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSControlClientDisconnected( CORE::CNotifier* notifier           ,
                                                const CORE::CEvent& eventid         ,
                                                CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSControlClientDataSent( CORE::CNotifier* notifier           ,
                                            const CORE::CEvent& eventid         ,
                                            CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSControlClientDataRecieved( CORE::CNotifier* notifier           ,
                                                const CORE::CEvent& eventid         ,
                                                CORE::CICloneable* eventdata = GUCEF_NULL );
                                         
    virtual void OnRSControlClientSocketError( CORE::CNotifier* notifier           ,
                                               const CORE::CEvent& eventid         ,
                                               CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSClientConnected( CORE::CNotifier* notifier           ,
                                      const CORE::CEvent& eventid         ,
                                      CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSClientDisconnected( CORE::CNotifier* notifier           ,
                                         const CORE::CEvent& eventid         ,
                                         CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSClientDataSent( CORE::CNotifier* notifier           ,
                                     const CORE::CEvent& eventid         ,
                                     CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnRSClientDataRecieved( CORE::CNotifier* notifier           ,
                                         const CORE::CEvent& eventid         ,
                                         CORE::CICloneable* eventdata = GUCEF_NULL );
                                         
    virtual void OnRSClientSocketError( CORE::CNotifier* notifier           ,
                                        const CORE::CEvent& eventid         ,
                                        CORE::CICloneable* eventdata = GUCEF_NULL );
    
    virtual void OnClientConnected( CORE::CNotifier* notifier           ,
                                    const CORE::CEvent& eventid         ,
                                    CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnClientDisconnected( CORE::CNotifier* notifier           ,
                                       const CORE::CEvent& eventid         ,
                                       CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnClientDataSent( CORE::CNotifier* notifier           ,
                                   const CORE::CEvent& eventid         ,
                                   CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnClientDataRecieved( CORE::CNotifier* notifier           ,
                                       const CORE::CEvent& eventid         ,
                                       CORE::CICloneable* eventdata = GUCEF_NULL );
                                         
    virtual void OnClientSocketError( CORE::CNotifier* notifier           ,
                                      const CORE::CEvent& eventid         ,
                                      CORE::CICloneable* eventdata = GUCEF_NULL );
                                      
    bool MapRemoteConnectionToLocalConnection( COMCORE::CTCPServerConnection* rsConnection );

    private:
    
    CServerPortExtender( void );
    CServerPortExtender( const CServerPortExtender& src );
    CServerPortExtender& operator=( const CServerPortExtender& src );
    
    private:
    
    typedef std::map< CORE::UInt32, CORE::UInt32 > TConnectionMap;
    typedef std::map< COMCORE::CTCPServerConnection*, CORE::CCyclicDynamicBuffer > TConnectionBuffers;
    typedef std::set< COMCORE::CTCPServerConnection* > TConnectionSet;
     
    COMCORE::CTCPServerSocket m_reversedServerControlSocket;
    COMCORE::CTCPServerSocket m_reversedServerSocket;
    COMCORE::CTCPServerSocket m_serverSocket;
    COMCORE::CTCPServerConnection* m_controlConnection;
    TConnectionBuffers m_cConnectionBuffers;
    TConnectionBuffers m_rsConnectionBuffers;
    TConnectionSet m_unmappedClientConnections;
    CORE::UInt16 m_serverPort;
    CORE::UInt16 m_reversedServerPort;
    CORE::UInt16 m_reversedServerControlPort;
    TConnectionMap m_remoteToLocalConnectionMap;
    bool m_controlConnectionInitialized;
};

/*-------------------------------------------------------------------------*/

#endif /* CSERVERPORTEXTENDER_H ? */
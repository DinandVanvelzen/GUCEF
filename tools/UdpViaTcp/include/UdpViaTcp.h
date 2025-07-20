/*
 *  UdpViaTcp: service to bridge UDP over a TCP connection
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <queue>

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#include "gucefWEB_CDefaultHTTPServerRouter.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#include "gucefWEB_CTaskManagerServerResource.h"
#define GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H ? */

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

class UdpViaTcp;

class RestApiUdpViaTcpInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdpViaTcpInfoResource( UdpViaTcp* app );

    virtual ~RestApiUdpViaTcpInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation );

    private:

    UdpViaTcp* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiUdpViaTcpConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdpViaTcpConfigResource( UdpViaTcp* app, bool appConfig );

    virtual ~RestApiUdpViaTcpConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation );

    private:

    UdpViaTcp* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class UdpViaTcp : public CORE::CObservingNotifier
{
    public:

    UdpViaTcp( void );
    virtual ~UdpViaTcp();

    bool Start( void );

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    void RegisterEventHandlers( void );

    void
    OnUDPReceiveSocketError( CORE::CNotifier* notifier   ,
                             const CORE::CEvent& eventID ,
                             CORE::CICloneable* evenData );

    void
    OnUDPReceiveSocketClosed( CORE::CNotifier* notifier   ,
                              const CORE::CEvent& eventID ,
                              CORE::CICloneable* evenData );

    void
    OnUDPReceiveSocketOpened( CORE::CNotifier* notifier   ,
                              const CORE::CEvent& eventID ,
                              CORE::CICloneable* evenData );

    void
    OnUDPReceiveSocketPacketsRecieved( CORE::CNotifier* notifier   ,
                                       const CORE::CEvent& eventID ,
                                       CORE::CICloneable* evenData );

    void
    OnUDPTransmitSocketError( CORE::CNotifier* notifier   ,
                              const CORE::CEvent& eventID ,
                              CORE::CICloneable* evenData );

    void
    OnUDPTransmitSocketClosed( CORE::CNotifier* notifier   ,
                               const CORE::CEvent& eventID ,
                               CORE::CICloneable* evenData );

    void
    OnUDPTransmitSocketOpened( CORE::CNotifier* notifier   ,
                               const CORE::CEvent& eventID ,
                               CORE::CICloneable* evenData );

    void
    OnUDPTransmitSocketPacketsRecieved( CORE::CNotifier* notifier   ,
                                        const CORE::CEvent& eventID ,
                                        CORE::CICloneable* evenData );

    void
    OnTCPClientConnected( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnTCPClientDisconnected( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );
                       
    void
    OnTCPServerClientConnected( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );
    
    void
    OnTCPServerClientDisconnected( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );
    void
    OnTCPServerClientError( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );
    
    void 
    OnTCPServerSocketOpened( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );
    
    void
    OnTCPServerSocketClosed( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    void
    OnTCPServerSocketError( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void
    OnTCPServerSocketClientError( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void
    OnTCPServerSocketMaxConnectionsChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData );

    void
    OnTCPServerConnectionDataRecieved( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnTcpClientAppLvlKeepAliveTimerCycle( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData );
    
    CORE::UInt32 GetTcpServerCompleteUdpPacketsReceived( bool resetCounter );
    
    private:

    typedef CORE::CTEventHandlerFunctor< UdpViaTcp > TEventCallback;

    enum EUdpViaTcpMode
    {
        UDPVIATCPMODE_UDP_RECEIVER_ONLY    ,
        UDPVIATCPMODE_UDP_TRANSMITTER_ONLY ,
        UDPVIATCPMODE_BIDIRECTIONAL_UDP
    };
    typedef enum EUdpViaTcpMode TUdpViaTcpMode;

    typedef std::vector< COMCORE::CHostAddress > THostAddressVector;
    typedef std::vector< CORE::CDynamicBuffer > TDynamicBufferVector;
    typedef std::queue< CORE::CDynamicBuffer > TDynamicBufferQueue;

    COMCORE::CTCPServerSocket m_tcpServerSocket;
    COMCORE::CTCPClientSocket m_tcpClientSocket;
    COMCORE::CUDPSocket m_udpTransmitSocket;
    COMCORE::CUDPSocket m_udpReceiveSocket;
    TDynamicBufferQueue m_tcpClientSendPacketBuffers;
    TDynamicBufferVector m_tcpServerReceivePacketBuffers;
    CORE::UInt32 m_tcpServerCompleteUdpPacketsReceived;
    CORE::CTimer m_tcpClientAppLvlKeepAliveTimer;
    bool m_useTcpClientAppLvlKeepAlive;
    
    bool m_udpReceiveUnicast;
    bool m_udpReceiveMulticast;
    
    TUdpViaTcpMode m_mode;
    COMCORE::CHostAddress m_tcpDestination;
    COMCORE::CHostAddress m_tcpReceiver;
    THostAddressVector m_udpDestinations;
    COMCORE::CHostAddress m_udpReceiver;
    COMCORE::CHostAddress m_udpTransmitter;
    CORE::CValueList::TVariantVector m_udpReceiverMulticastSources;

    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    WEB::CTaskManagerServerResource m_taskManagementRsc;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;

    CORE::CTimer m_metricsTimer;
    bool m_transmitMetrics;
};

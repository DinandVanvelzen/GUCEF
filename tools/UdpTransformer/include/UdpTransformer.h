/*
 *  UdpTransformer: service to perform basic transformations to in-flight UDP
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class UdpTransformer;

class RestApiUdpTransformerInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdpTransformerInfoResource( UdpTransformer* app );

    virtual ~RestApiUdpTransformerInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation );

    private:

    UdpTransformer* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiUdpTransformerConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdpTransformerConfigResource( UdpTransformer* app, bool appConfig );

    virtual ~RestApiUdpTransformerConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation );

    private:

    UdpTransformer* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class UdpTransformer : public CORE::CObservingNotifier
{
    public:

    UdpTransformer( void );
    virtual ~UdpTransformer();

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
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );    
    
    bool
    TransformUdpPacket( const CORE::CDynamicBuffer& udpPacketBuffer      ,
                        const COMCORE::CIPv4Address& sourceAddress         ,
                        CORE::CDynamicBuffer& transformedUdpOutputBuffer );

    private:

    typedef CORE::CTEventHandlerFunctor< UdpTransformer > TEventCallback;
    typedef std::vector< COMCORE::CHostAddress > THostAddressVector;

    COMCORE::CUDPSocket m_udpTransmitSocket;
    COMCORE::CUDPSocket m_udpReceiveSocket;
    
    CORE::CDynamicBuffer m_packetTransformBuffer;
    
    bool m_udpReceiveUnicast;
    bool m_udpReceiveMulticast;

    THostAddressVector m_udpDestinations;

    COMCORE::CHostAddress m_udpReceiverAddress;
    COMCORE::CHostAddress m_udpTransmitterAddress;
    CORE::CValueList::TVariantVector m_udpReceiverMulticastSources;
    
    CORE::CString m_udpPayloadTransformDescription;
    CORE::CString::StringVector m_transformDescriptionSegments;
    bool m_udpSourceAddrTransformAsString;
    bool m_udpSourceAddrTransformAsStringIncludePort;
    CORE::Int32 m_udpSourceAddrTransformAsStringDotReplacementChar;
    CORE::Int32 m_udpSourceAddrTransformAsStringColonReplacementChar;

    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;

    CORE::CTimer m_metricsTimer;
    bool m_transmitMetrics;
};

/*-------------------------------------------------------------------------*/

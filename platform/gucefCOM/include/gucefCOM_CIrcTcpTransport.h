/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CIRCTCPTRANSPORT_H
#define GUCEF_COM_CIRCTCPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COM_CIIRCTRANSPORT_H
#include "gucefCOM_CIIrcTransport.h"
#define GUCEF_COM_CIIRCTRANSPORT_H
#endif /* GUCEF_COM_CIIRCTRANSPORT_H ? */

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
 *  IRC over plain TCP: the stream is split into lines on LF, a CR before it is dropped
 */
class GUCEF_COM_PUBLIC_CPP CIrcTcpTransport : public CIIrcTransport
{
    public:

    /** A line may be up to 8191 bytes of tags plus 512 of message, anything longer without an end is garbage */
    static const UInt32 MaxLineSizeInBytes;

    CIrcTcpTransport( void );

    explicit CIrcTcpTransport( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CIrcTcpTransport();

    virtual bool Connect( const CIrcClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SendLine( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIrcTcpTransport( const CIrcTcpTransport& src );
    CIrcTcpTransport& operator=( const CIrcTcpTransport& src );

    bool OnData( const CORE::CDynamicBuffer& data );

    private:

    COMCORE::CTCPClientSocket m_socket;
    CORE::CDynamicBuffer m_receiveBuffer;
    CORE::CDynamicBuffer m_sendBuffer;
    bool m_isConnected;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIIrcTransport, CIrcTcpTransport, MT::CMutex > TIrcTcpTransportFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIRCTCPTRANSPORT_H ? */

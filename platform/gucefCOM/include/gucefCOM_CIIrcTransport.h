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

#ifndef GUCEF_COM_CIIRCTRANSPORT_H
#define GUCEF_COM_CIIRCTRANSPORT_H

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

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COM_CIRCCLIENTSETTINGS_H
#include "gucefCOM_CIrcClientSettings.h"
#define GUCEF_COM_CIRCCLIENTSETTINGS_H
#endif /* GUCEF_COM_CIRCCLIENTSETTINGS_H ? */

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
 *  Carries IRC lines to and from a server. The lines are without their CR LF in both directions.
 */
class GUCEF_COM_PUBLIC_CPP CIIrcTransport : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ConnectedEvent;           /**< the connection to the server is there */
    static const CORE::CEvent LineReceivedEvent;        /**< TLineReceivedEventData: one line from the server */
    static const CORE::CEvent DisconnectedEvent;        /**< the connection is gone, by either side */
    static const CORE::CEvent TransportErrorEvent;      /**< TTransportErrorEventData: a description of the problem */

    typedef CORE::TCloneableString TLineReceivedEventData;
    typedef CORE::TCloneableString TTransportErrorEventData;

    static const CORE::CString TcpTransportName;        /**< "tcp" */
    static const CORE::CString WebSocketTransportName;  /**< "websocket" */

    static void RegisterEvents( void );

    CIIrcTransport( void );

    virtual ~CIIrcTransport();

    /** Starts connecting to the server the settings name */
    virtual bool Connect( const CIrcClientSettings& settings ) = 0;

    virtual void Disconnect( void ) = 0;

    /** Sends one line, without CR LF */
    virtual bool SendLine( const CORE::CString& line ) = 0;

    virtual bool IsConnected( void ) const = 0;

    /** The name the transport is registered with in the factory */
    virtual const CORE::CString& GetTransportName( void ) const = 0;

    private:

    CIIrcTransport( const CIIrcTransport& src );
    CIIrcTransport& operator=( const CIIrcTransport& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIIrcTransport, MT::CMutex > CIrcTransportFactory;
typedef CIrcTransportFactory::TProductPtr                                     CIIrcTransportPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIIRCTRANSPORT_H ? */

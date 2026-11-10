/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Accepts transports for incoming sessions (gucefCOMCORE RPC design 10.2). TransportAcceptedEvent is raised on the
 *  listener's pulse generator thread with a CRpcTransportAcceptedEventData; the accepted transport is connected.
 */
class GUCEF_COMCORE_EXPORT_CPP CIRpcTransportListener : public CORE::CObservingNotifier ,
                                                        public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent TransportAcceptedEvent;   /**< CRpcTransportAcceptedEventData */
    static const CORE::CEvent ListenerErrorEvent;       /**< TCloneableString */

    static void RegisterEvents( void );

    CIRpcTransportListener( void );
    virtual ~CIRpcTransportListener() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTransportType( void ) const = 0;

    virtual bool Listen( const CORE::CUri& local ) = 0;
    virtual void StopListening( void ) = 0;
    virtual bool IsListening( void ) const = 0;

    /** Where the listener listens, with an actual port where 0 was asked */
    virtual CORE::CUri GetListenUri( void ) const = 0;

    /**
     *  Before Listen(): see CIRpcTransport::SetPulseGenerator(). The notifier has a lock of its own, as a transport's
     *  (CIRpcTransport): a host subscribes on the thread that adds the listener while the listener's thread raises
     *  TransportAcceptedEvent. An accepted transport delivers no frame before that event announced it
     */
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const = 0;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIRpcTransportListener( const CIRpcTransportListener& src );                    /**< not implemented */
    CIRpcTransportListener& operator=( const CIRpcTransportListener& src );         /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIRpcTransportListener, MT::CMutex >    CRpcTransportListenerFactory;
typedef CRpcTransportListenerFactory::TProductPtr                                        CIRpcTransportListenerPtr;

/*-------------------------------------------------------------------------*/

/** Event data of TransportAcceptedEvent */
class GUCEF_COMCORE_EXPORT_CPP CRpcTransportAcceptedEventData : public CORE::CICloneable
{
    public:

    CIRpcTransportPtr transport;

    explicit CRpcTransportAcceptedEventData( const CIRpcTransportPtr& acceptedTransport );
    CRpcTransportAcceptedEventData( const CRpcTransportAcceptedEventData& src );
    virtual ~CRpcTransportAcceptedEventData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

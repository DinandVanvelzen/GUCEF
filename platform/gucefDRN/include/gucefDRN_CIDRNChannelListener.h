/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNCHANNEL_H
#include "gucefDRN_CIDRNChannel.h"
#define GUCEF_DRN_CIDRNCHANNEL_H
#endif /* GUCEF_DRN_CIDRNCHANNEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSecurityPolicy;

/**
 *  Accepts channels of one type. Every accepted channel is announced with ChannelAcceptedEvent; whoever wants
 *  to keep it holds on to the channel pointer of the event data, otherwise it is closed.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNChannelListener : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ChannelAcceptedEvent;        /**< CDRNChannelEventData */
    static const CORE::CEvent ListenerErrorEvent;          /**< CDRNChannelErrorEventData */

    static void RegisterEvents( void );

    CIDRNChannelListener( void );

    virtual ~CIDRNChannelListener();

    virtual bool Listen( const CDRNEndpoint& local ) = 0;

    virtual void StopListening( void ) = 0;

    virtual bool IsListening( void ) const = 0;

    virtual CDRNEndpoint GetListenEndpoint( void ) const = 0;

    virtual CORE::CString GetChannelType( void ) const = 0;

    /** The pulse generator to run on, to be set before Listen() */
    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator ) = 0;

    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const = 0;

    /** What the listener needs of the node's security policy, to be set before Listen(); the default takes nothing */
    virtual void ApplySecurityPolicy( const CDRNSecurityPolicy& policy );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIDRNChannelListener( const CIDRNChannelListener& src );
    CIDRNChannelListener& operator=( const CIDRNChannelListener& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIDRNChannelListener, MT::CMutex >                       CIDRNChannelListenerPtr;
typedef CORE::CTAbstractFactory< CORE::CString, CIDRNChannelListener, MT::CMutex >  CDRNChannelListenerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

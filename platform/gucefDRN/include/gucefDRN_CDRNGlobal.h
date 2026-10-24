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

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#define GUCEF_DRN_CDRNGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRN_CIDRNCHANNELLISTENER_H
#include "gucefDRN_CIDRNChannelListener.h"
#define GUCEF_DRN_CIDRNCHANNELLISTENER_H
#endif /* GUCEF_DRN_CIDRNCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

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

/**
 *  Global facilities of DRN: the factories of channels, channel listeners and authenticators, keyed by type name.
 *  The built in "tcp", "udp" and "websocket" channels and the "anonymous", "shared-secret" and "token"
 *  authenticators are registered here; plugins register more.
 */
class GUCEF_DRN_EXPORT_CPP CDRNGlobal
{
    public:

    static CDRNGlobal* Instance( void );

    CDRNChannelFactory& GetChannelFactory( void );

    CDRNChannelListenerFactory& GetChannelListenerFactory( void );

    CDRNAuthenticatorFactory& GetAuthenticatorFactory( void );

    private:

    friend class CGUCEFDRNModule;

    static void Deinstance( void );

    void Initialize( void );

    CDRNGlobal( void );
    ~CDRNGlobal();
    CDRNGlobal( const CDRNGlobal& src );            /**< not implemented */
    CDRNGlobal& operator=( const CDRNGlobal& src ); /**< not implemented */

    private:

    static MT::CMutex g_dataLock;
    static CDRNGlobal* g_instance;

    CDRNChannelFactory* m_channelFactory;
    CDRNChannelListenerFactory* m_channelListenerFactory;
    CDRNAuthenticatorFactory* m_authenticatorFactory;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

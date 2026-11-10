/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#include "pluginglueEOS_CEosP2PSocket.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The one place that talks to the P2P interface of the SDK for the local user of a platform: it takes the packets and
 *  the connection events of the SDK on every pump and gives them to the socket they are for, and sends the packets of
 *  the sockets. The DRN channel plugin and the lobby plugin share it, each with sockets of its own.
 *
 *  Pump() runs from the tick of the platform. It takes at most MaxPacketsPerPump packets per call, so that a flood
 *  does not hold up the thread; the rest stays in the queue of the SDK. Nothing in the packet path allocates.
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosP2PRouter
{
    public:

    static const UInt32 MaxPacketsPerPump = 512;

    CEosP2PRouter( const SEosP2PShim& shim );

    ~CEosP2PRouter();

    const SEosP2PShim& GetShim( void ) const;

    /** The Product User Id that packets are sent and received as, empty while nobody is logged in */
    void SetLocalUser( const CORE::CString& localUser );

    const CORE::CString& GetLocalUser( void ) const;

    /** How packets may travel: through the relays of Epic when a direct connection is not possible (default), never, or only through them */
    bool SetRelayControl( EEosRelayControl control );

    EEosRelayControl GetRelayControl( void ) const;

    EEosNatType GetNatType( void ) const;

    /** Opens the socket with the name or gives the one that is open; every Acquire needs a Release. NULL for a name that is not valid */
    CEosP2PSocket* AcquireSocket( const CORE::CString& name );

    void ReleaseSocket( CEosP2PSocket* socket );

    UInt32 GetSocketCount( void ) const;

    CEosP2PSocket* FindSocket( const CORE::CString& name ) const;

    /** Takes the events and packets of the SDK and gives them to the sockets */
    void Pump( void );

    UInt32 GetDroppedPacketCount( void ) const;

    private:

    friend class CEosP2PSocket;

    CEosP2PRouter( const CEosP2PRouter& src );
    CEosP2PRouter& operator=( const CEosP2PRouter& src );

    EEosResult Send( const CORE::CString& remoteUser, const CORE::CString& socketName, UInt8 channel, const void* data, UInt32 size, EEosPacketReliability reliability );
    void Close( const CORE::CString& remoteUser, const CORE::CString& socketName );
    bool Accept( const CORE::CString& remoteUser, const CORE::CString& socketName );
    CEosP2PSocket* FindSocketByText( const char* name ) const;
    void DeleteRetired( void );

    private:

    typedef GUCEF::vector< CEosP2PSocket* > TSocketVector;

    SEosP2PShim m_shim;
    CORE::CString m_localUser;
    EEosRelayControl m_relayControl;
    TSocketVector m_sockets;
    TSocketVector m_retired;
    CEosP2PPacketEventData m_packet;
    SEosP2PEvent m_event;
    UInt32 m_dropped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PROUTER_H ? */

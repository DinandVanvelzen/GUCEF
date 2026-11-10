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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H
#define GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

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

class CEosP2PRouter;

/*-------------------------------------------------------------------------*/

/** The data of CEosP2PSocket::PacketReceivedEvent. The router owns one and reuses it for every packet: nothing is allocated per packet */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosP2PPacketEventData : public CORE::CICloneable
{
    public:

    char remoteUser[ EosP2PUserIdBufferSize ];
    UInt8 channel;
    UInt32 size;
    UInt8 data[ EosP2PMaxPacketSize + 30 ];

    CEosP2PPacketEventData( void );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The data of the connection events of CEosP2PSocket */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosP2PConnectionEventData : public CORE::CICloneable
{
    public:

    char remoteUser[ EosP2PUserIdBufferSize ];
    EEosNetworkType networkType;
    EEosConnectionCloseReason closeReason;
    bool accepted;                          /**< ConnectionRequestEvent: whether the request was accepted */

    CEosP2PConnectionEventData( void );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A named socket of the EOS P2P interface of one local user: the place where the packets of a socket id arrive and
 *  where connection requests are decided. Whoever uses a socket subscribes to it for the events. A socket takes
 *  connections of others only as its accept policy says (nothing by default): a peer that is not on the list is not
 *  accepted, so it cannot send a packet.
 *
 *  Events (all on the thread that pumps the pulse generator, from the tick of the platform):
 *    PacketReceivedEvent, ConnectionRequestEvent, ConnectionEstablishedEvent, ConnectionInterruptedEvent, ConnectionClosedEvent
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosP2PSocket : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent PacketReceivedEvent;              /**< CEosP2PPacketEventData */
    static const CORE::CEvent ConnectionRequestEvent;           /**< CEosP2PConnectionEventData: a peer asked, accepted says what was decided */
    static const CORE::CEvent ConnectionEstablishedEvent;       /**< CEosP2PConnectionEventData */
    static const CORE::CEvent ConnectionInterruptedEvent;       /**< CEosP2PConnectionEventData */
    static const CORE::CEvent ConnectionClosedEvent;            /**< CEosP2PConnectionEventData */

    enum EAcceptPolicy
    {
        ACCEPT_NONE = 0,        /**< no connection request is accepted; connections that this side opens work (default) */
        ACCEPT_LIST,            /**< the requests of the peers on the allow list are accepted */
        ACCEPT_ANY              /**< every request is accepted */
    };
    typedef enum EAcceptPolicy TAcceptPolicy;

    static void RegisterEvents( void );

    /** The name of a socket id: at most 32 characters of letters, digits, '-' and '_' */
    static bool IsValidSocketName( const CORE::CString& name );

    const CORE::CString& GetSocketName( void ) const;

    void SetAcceptPolicy( TAcceptPolicy policy );

    TAcceptPolicy GetAcceptPolicy( void ) const;

    /** Replaces the allow list; a peer that is connected and no longer on the list (with the policy ACCEPT_LIST) is disconnected */
    void SetAllowedRemotes( const CORE::CString::StringSet& remotes );

    void AllowRemote( const CORE::CString& remoteUser );

    /** Takes a peer off the list and closes its connection when the policy is ACCEPT_LIST */
    void DisallowRemote( const CORE::CString& remoteUser );

    bool IsRemoteAllowed( const CORE::CString& remoteUser ) const;

    /** The most connections the socket has open at once, 0 for no limit, default 64 */
    void SetMaxConnections( UInt32 maxConnections );

    UInt32 GetMaxConnections( void ) const;

    UInt32 GetConnectionCount( void ) const;

    bool IsConnectedTo( const CORE::CString& remoteUser ) const;

    /** Sends one packet of at most EosP2PMaxPacketSize bytes. False when it was refused: too big, no local user, the SDK said no */
    bool SendPacket( const CORE::CString& remoteUser, const void* data, UInt32 size, EEosPacketReliability reliability, UInt8 channel = 0 );

    /** Closes the connection to the peer on this socket */
    void CloseConnection( const CORE::CString& remoteUser );

    /** Closes the connections of all peers on this socket */
    void CloseAllConnections( void );

    UInt32 GetRefusedConnectionCount( void ) const;

    UInt32 GetReceivedPacketCount( void ) const;

    UInt32 GetSentPacketCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CEosP2PRouter;

    CEosP2PSocket( CEosP2PRouter& router, const CORE::CString& name );

    virtual ~CEosP2PSocket();

    CEosP2PSocket( const CEosP2PSocket& src );
    CEosP2PSocket& operator=( const CEosP2PSocket& src );

    void OnConnectionRequest( const SEosP2PEvent& event );
    void OnConnectionEstablished( const SEosP2PEvent& event );
    void OnConnectionInterrupted( const SEosP2PEvent& event );
    void OnConnectionClosed( const SEosP2PEvent& event );
    void OnPacket( CEosP2PPacketEventData& packet );
    void AddConnection( const char* remoteUser );
    void RemoveConnection( const char* remoteUser );
    Int32 IndexOfConnection( const char* remoteUser ) const;

    struct SPeerId
    {
        char text[ EosP2PUserIdBufferSize ];
    };
    typedef GUCEF::vector< SPeerId > TPeerVector;

    private:

    CEosP2PRouter& m_router;
    CORE::CString m_name;
    TAcceptPolicy m_policy;
    CORE::CString::StringSet m_allowed;
    TPeerVector m_connections;
    UInt32 m_maxConnections;
    UInt32 m_refCount;
    UInt32 m_refused;
    UInt32 m_received;
    UInt32 m_sent;
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

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSP2PSOCKET_H ? */

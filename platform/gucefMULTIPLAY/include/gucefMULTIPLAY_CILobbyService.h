/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#include "gucefMULTIPLAY_CGameListingFilter.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a lobby service can do beyond the basics */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyServiceCapabilities
{
    public:

    bool voting;
    bool relay;
    bool inProgressSpectating;
    bool matchmaking;
    bool chat;

    CLobbyServiceCapabilities( void );
};

/*-------------------------------------------------------------------------*/

/** A lobby as the result of a CreateLobby() or JoinLobby() future and as the data of LobbyJoinedEvent */
typedef CORE::CTCloneableObj< CILobbyPtr >  CLobbyResultData;

/*-------------------------------------------------------------------------*/

/**
 *  A place where lobbies live: a built-in GUCEF lobby server on the LAN or the internet, or a platform service.
 *
 *  The calls that take a future return whether the request could be made; the future completes with the outcome.
 *  Connect() completes with the CPlayerIdentity the service knows the player by, Browse() with a CGameListingList,
 *  CreateLobby() and JoinLobby() with a CLobbyResultData.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CILobbyService : public CORE::CObservingNotifier ,
                                                  public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ServiceConnectedEvent;        /**< no data */
    static const CORE::CEvent ServiceDisconnectedEvent;     /**< no data */
    static const CORE::CEvent LobbyJoinedEvent;             /**< CLobbyResultData: the lobby */
    static const CORE::CEvent ServiceErrorEvent;            /**< TServiceErrorEventData: a description of the problem */

    typedef CLobbyResultData            TLobbyJoinedEventData;
    typedef CORE::TCloneableString      TServiceErrorEventData;

    static void RegisterEvents( void );

    CILobbyService( void );

    virtual ~CILobbyService();

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) = 0;

    virtual void Disconnect( void ) = 0;

    virtual bool IsConnected( void ) const = 0;

    /** Finds the lobbies and games of the service that pass the filter */
    virtual bool Browse( const CGameListingFilter& filter, CORE::CFutureResult& future ) = 0;

    virtual bool CreateLobby( const CLobbyCreateParams& params, CORE::CFutureResult& future ) = 0;

    virtual bool JoinLobby( const CORE::CString& lobbyId    ,
                            const CORE::CString& password   ,
                            bool asSpectator                ,
                            CORE::CFutureResult& future     ) = 0;

    virtual CLobbyServiceCapabilities GetCapabilities( void ) const = 0;

    /** The name the service is registered with in the factory, for example "gucef" */
    virtual const CORE::CString& GetServiceName( void ) const = 0;

    /**
     *  Lists a lobby or game hosted elsewhere, for a CGameAdvertiser. Services that only list their own lobbies
     *  do not support it, which is the default.
     */
    virtual bool PublishListing( const CGameListingEntry& listing );

    virtual bool WithdrawListing( const CORE::CString& listingId );

    /** Gets the lobby from a finished CreateLobby() or JoinLobby() future */
    static bool TryGetLobbyFromFuture( const CORE::CFutureResult& future, CILobbyPtr& lobby );

    private:

    CILobbyService( const CILobbyService& src );
    CILobbyService& operator=( const CILobbyService& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CILobbyService, MT::CMutex >   CLobbyServiceFactory;
typedef CLobbyServiceFactory::TProductPtr                                       CILobbyServicePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

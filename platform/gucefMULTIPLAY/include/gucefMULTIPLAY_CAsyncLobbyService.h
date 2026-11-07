/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H
#define GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBY_H
#include "gucefMULTIPLAY_CAsyncLobby.h"
#define GUCEF_MULTIPLAY_CASYNCLOBBY_H
#endif /* GUCEF_MULTIPLAY_CASYNCLOBBY_H ? */

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

/**
 *  The lobby service of a CIAsyncLobbyBackend. It is made with the backend by the application, not by the service factory, because a
 *  backend belongs to the application. Connect() completes with the identity it was given when the backend is available and fails
 *  with the reason of the backend when it is not; Browse() completes with the rows of the backend that pass the filter;
 *  CreateLobby() asks the backend to make the lobby and completes with it; JoinLobby() completes with the lobby of the id (a
 *  lobby that exists already, joining is opening it; 'asSpectator' makes the local player a spectator first). A lobby is the
 *  same object for as long as the service holds it (ReleaseLobby()).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CAsyncLobbyService : public CILobbyService
{
    public:

    CAsyncLobbyService( CIAsyncLobbyBackend& backend, const CORE::CString& serviceName );
    virtual ~CAsyncLobbyService();

    virtual bool Connect( const CPlayerIdentity& identity, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Browse( const CGameListingFilter& filter, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CreateLobby( const CLobbyCreateParams& params, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool JoinLobby( const CORE::CString& lobbyId    ,
                            const CORE::CString& password   ,
                            bool asSpectator                ,
                            CORE::CFutureResult& future     ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CLobbyServiceCapabilities GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetServiceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The lobby with the id, made when it is not held yet; a NULL pointer when the backend has no such lobby */
    CILobbyPtr OpenLobby( const CORE::CString& lobbyId );

    /** Lets go of the lobby: the next OpenLobby() makes a new one */
    void ReleaseLobby( const CORE::CString& lobbyId );

    CIAsyncLobbyBackend& GetBackend( void ) const;

    private:

    typedef GUCEF::map< CORE::CString, CILobbyPtr > TLobbyMap;

    CAsyncLobbyService( const CAsyncLobbyService& src );
    CAsyncLobbyService& operator=( const CAsyncLobbyService& src );

    private:

    CIAsyncLobbyBackend* m_backend;
    CORE::CString m_serviceName;
    TLobbyMap m_lobbies;
    bool m_connected;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H ? */

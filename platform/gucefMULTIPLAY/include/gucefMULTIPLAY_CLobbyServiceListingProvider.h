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

#ifndef GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#define GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

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
 *  Lists the lobbies and games of a lobby service through CILobbyService::Browse(), again every refresh interval.
 *  The service is either given with SetLobbyService() (shared with the rest of the application) or made from the
 *  lobby service factory of CMultiplayGlobal by its name.
 *
 *  Config: service (the name in the factory), refreshIntervalInSecs (default 10), a "ServiceConfig" child given to
 *  the service it makes
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyServiceListingProvider : public CIGameListingProvider
{
    public:

    static const CORE::CString ProviderName;        /**< "lobby-service" */

    CLobbyServiceListingProvider( void );

    virtual ~CLobbyServiceListingProvider();

    virtual bool Start( const CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetLobbyService( CILobbyServicePtr service );

    CILobbyServicePtr GetLobbyService( void ) const;

    void SetRefreshIntervalInSecs( UInt32 seconds );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLobbyServiceListingProvider( const CLobbyServiceListingProvider& src );
    CLobbyServiceListingProvider& operator=( const CLobbyServiceListingProvider& src );

    bool EnsureService( void );
    bool StartBrowse( void );
    void OnTimer( void );
    bool OnBrowseDone( void );

    private:

    CILobbyServicePtr m_service;
    CORE::CString m_serviceName;
    CORE::CDataNode m_serviceConfig;
    CORE::CTimer m_timer;
    CORE::CFutureResult m_browseFuture;
    bool m_browsing;
    UInt64 m_nextBrowseInMs;
    UInt32 m_refreshIntervalInSecs;
    bool m_active;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIGameListingProvider, CLobbyServiceListingProvider, MT::CMutex > TLobbyServiceListingProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H ? */

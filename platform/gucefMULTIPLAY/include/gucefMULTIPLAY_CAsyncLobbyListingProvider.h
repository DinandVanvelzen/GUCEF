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

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H
#define GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#include "gucefMULTIPLAY_CIAsyncLobbyBackend.h"
#define GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H
#endif /* GUCEF_MULTIPLAY_CIASYNCLOBBYBACKEND_H ? */

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
 *  Lists the rows of a CIAsyncLobbyBackend in a browser (CGameBrowser): the rows that pass the filter, kept up to date
 *  while the backend says its rows changed. It is made with the backend by the application, like the lobby service.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CAsyncLobbyListingProvider : public CIGameListingProvider
{
    public:

    CAsyncLobbyListingProvider( CIAsyncLobbyBackend& backend, const CORE::CString& providerName );
    virtual ~CAsyncLobbyListingProvider();

    virtual bool Start( const CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Only the standard events (the backend is destroyed) arrive here, the others have their own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAsyncLobbyListingProvider > TEventCallback;

    CAsyncLobbyListingProvider( const CAsyncLobbyListingProvider& src );
    CAsyncLobbyListingProvider& operator=( const CAsyncLobbyListingProvider& src );

    void Synchronize( void );
    void OnBackendListingsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CIAsyncLobbyBackend* m_backend;
    CORE::CString m_providerName;
    bool m_active;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H ? */

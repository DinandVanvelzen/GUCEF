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

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CIIdentityProvider.h"
#define GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H ? */

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
 *  The global systems of gucefMULTIPLAY, set up when the module loads: the factories of the listing providers,
 *  lobby services, chat services and identity providers, with the built-in ones registered. Plugins register
 *  their implementations into these factories.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMultiplayGlobal
{
    public:

    static CMultiplayGlobal* Instance( void );

    /** "lan", "http-list" and "lobby-service" built in */
    CGameListingProviderFactory& GetListingProviderFactory( void );

    CLobbyServiceFactory& GetLobbyServiceFactory( void );

    /** "irc" built in */
    CChatServiceFactory& GetChatServiceFactory( void );

    /** "local" built in */
    CIdentityProviderFactory& GetIdentityProviderFactory( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CMultiplayGlobal( void );
    ~CMultiplayGlobal();
    CMultiplayGlobal( const CMultiplayGlobal& src );            /**< not implemented */
    CMultiplayGlobal& operator=( const CMultiplayGlobal& src ); /**< not implemented */

    private:

    CGameListingProviderFactory* m_listingProviderFactory;
    CLobbyServiceFactory* m_lobbyServiceFactory;
    CChatServiceFactory* m_chatServiceFactory;
    CIdentityProviderFactory* m_identityProviderFactory;

    static MT::CMutex g_dataLock;
    static CMultiplayGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

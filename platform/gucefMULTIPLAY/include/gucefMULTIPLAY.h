/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*
 *  This is the main include header for the gucefMULTIPLAY library.
 *  It is part of a platform called GUCEF.
 *  All classes in this library have to do with enabling multiplayer
 *  functionality
 */

#ifndef GUCEF_MULTIPLAY_H
#define GUCEF_MULTIPLAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_MULTIPLAY_CSERVERLIST_H
#include "gucefMULTIPLAY_CServerList.h"
#define GUCEF_MULTIPLAY_CSERVERLIST_H
#endif /* GUCEF_MULTIPLAY_CSERVERLIST_H ? */

#ifndef GUCE_MULTIPLAY_CSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CServerListProvider.h"
#define GUCE_MULTIPLAY_CSERVERLISTPROVIDER_H
#endif /* GUCE_MULTIPLAY_CSERVERLISTPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#include "gucefMULTIPLAY_CPassiveURLServerListProvider.h"
#define GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CPASSIVEURLSERVERLISTPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#include "gucefMULTIPLAY_CGameListingEntry.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#include "gucefMULTIPLAY_CGameListingFilter.h"
#define GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H
#endif /* GUCEF_MULTIPLAY_CGAMELISTINGFILTER_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLanListingProvider.h"
#define GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CHttpListListingProvider.h"
#define GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"
#define GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#include "gucefMULTIPLAY_CGameAdvertiser.h"
#define GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#endif /* GUCEF_MULTIPLAY_CGAMEADVERTISER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#include "gucefMULTIPLAY_CGameBrowser.h"
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H
#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */

#ifndef GUCEF_MULTIPLAY_CPINGSERVICE_H
#include "gucefMULTIPLAY_CPingService.h"
#define GUCEF_MULTIPLAY_CPINGSERVICE_H
#endif /* GUCEF_MULTIPLAY_CPINGSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CIIdentityProvider.h"
#define GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CLocalIdentityProvider.h"
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CAuthIdentityProvider.h"
#define GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#include "gucefMULTIPLAY_CPlayerIdentitySigner.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H ? */

#ifndef GUCEF_MULTIPLAY_CLINKPROOF_H
#include "gucefMULTIPLAY_CLinkProof.h"
#define GUCEF_MULTIPLAY_CLINKPROOF_H
#endif /* GUCEF_MULTIPLAY_CLINKPROOF_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYRULES_H
#include "gucefMULTIPLAY_CLobbyRules.h"
#define GUCEF_MULTIPLAY_CLOBBYRULES_H
#endif /* GUCEF_MULTIPLAY_CLOBBYRULES_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#include "gucefMULTIPLAY_CLobbyCreateParams.h"
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */

#ifndef GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#include "gucefMULTIPLAY_CLaunchDescription.h"
#define GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H
#endif /* GUCEF_MULTIPLAY_CLAUNCHDESCRIPTION_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CCHATMESSAGE_H
#include "gucefMULTIPLAY_CChatMessage.h"
#define GUCEF_MULTIPLAY_CCHATMESSAGE_H
#endif /* GUCEF_MULTIPLAY_CCHATMESSAGE_H ? */

#ifndef GUCEF_MULTIPLAY_CICHATSERVICE_H
#include "gucefMULTIPLAY_CIChatService.h"
#define GUCEF_MULTIPLAY_CICHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CICHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#include "gucefMULTIPLAY_CIrcChatService.h"
#define GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CIRCCHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyRelayAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#include "gucefMULTIPLAY_CGucefLobbyServer.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H
#include "gucefMULTIPLAY_CGucefLobbyServerApp.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#include "gucefMULTIPLAY_CGucefLobby.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#include "gucefMULTIPLAY_CGucefLobbyService.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H
#include "gucefMULTIPLAY_CLobbyChatService.h"
#define GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CTOKENBUCKET_H
#include "gucefMULTIPLAY_CTokenBucket.h"
#define GUCEF_MULTIPLAY_CTOKENBUCKET_H
#endif /* GUCEF_MULTIPLAY_CTOKENBUCKET_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERREPORT_H
#include "gucefMULTIPLAY_CPlayerReport.h"
#define GUCEF_MULTIPLAY_CPLAYERREPORT_H
#endif /* GUCEF_MULTIPLAY_CPLAYERREPORT_H ? */

#ifndef GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#include "gucefMULTIPLAY_CIClientIntegrity.h"
#define GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#endif /* GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H ? */

#ifndef GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H
#include "gucefMULTIPLAY_CFileHashClientIntegrity.h"
#define GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H
#endif /* GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 13-09-2005 :
       - Initial version.

-----------------------------------------------------------------------------*/

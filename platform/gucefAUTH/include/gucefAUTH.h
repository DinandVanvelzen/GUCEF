/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

/*
 *  This is the main include header for the gucefAUTH library.
 *  It is part of a platform called GUCEF.
 *  All classes in this library have to do with logging in with an external identity provider
 *  and linking the accounts that gives to the profile of the player
 */

#ifndef GUCEF_AUTH_H
#define GUCEF_AUTH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_CAUTHOPTIONS_H
#include "gucefAUTH_CAuthOptions.h"
#define GUCEF_AUTH_CAUTHOPTIONS_H
#endif /* GUCEF_AUTH_CAUTHOPTIONS_H ? */

#ifndef GUCEF_AUTH_CAUTHEVENTDATA_H
#include "gucefAUTH_CAuthEventData.h"
#define GUCEF_AUTH_CAUTHEVENTDATA_H
#endif /* GUCEF_AUTH_CAUTHEVENTDATA_H ? */

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#include "gucefAUTH_CAuthResult.h"
#define GUCEF_AUTH_CAUTHRESULT_H
#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */

#ifndef GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H
#include "gucefAUTH_CAuthUnavailableProvider.h"
#define GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H
#endif /* GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H ? */

#ifndef GUCEF_AUTH_CIAUTHPROVIDER_H
#include "gucefAUTH_CIAuthProvider.h"
#define GUCEF_AUTH_CIAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CIAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_ILINKSIGNER_H
#include "gucefAUTH_ILinkSigner.h"
#define GUCEF_AUTH_ILINKSIGNER_H
#endif /* GUCEF_AUTH_ILINKSIGNER_H ? */

#ifndef GUCEF_AUTH_CLINKRECORD_H
#include "gucefAUTH_CLinkRecord.h"
#define GUCEF_AUTH_CLINKRECORD_H
#endif /* GUCEF_AUTH_CLINKRECORD_H ? */

#ifndef GUCEF_AUTH_IIDENTITYLINKSTORE_H
#include "gucefAUTH_IIdentityLinkStore.h"
#define GUCEF_AUTH_IIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_IIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#include "gucefAUTH_CAuthProviderRegistry.h"
#define GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H ? */

#ifndef GUCEF_AUTH_IAUTHFLOWENGINE_H
#include "gucefAUTH_IAuthFlowEngine.h"
#define GUCEF_AUTH_IAUTHFLOWENGINE_H
#endif /* GUCEF_AUTH_IAUTHFLOWENGINE_H ? */

#ifndef GUCEF_AUTH_CAUTHJSON_H
#include "gucefAUTH_CAuthJson.h"
#define GUCEF_AUTH_CAUTHJSON_H
#endif /* GUCEF_AUTH_CAUTHJSON_H ? */

#ifndef GUCEF_AUTH_CAUTHURL_H
#include "gucefAUTH_CAuthUrl.h"
#define GUCEF_AUTH_CAUTHURL_H
#endif /* GUCEF_AUTH_CAUTHURL_H ? */

#ifndef GUCEF_AUTH_CJWK_H
#include "gucefAUTH_CJwk.h"
#define GUCEF_AUTH_CJWK_H
#endif /* GUCEF_AUTH_CJWK_H ? */

#ifndef GUCEF_AUTH_CJWT_H
#include "gucefAUTH_CJwt.h"
#define GUCEF_AUTH_CJWT_H
#endif /* GUCEF_AUTH_CJWT_H ? */

#ifndef GUCEF_AUTH_CJWKSCACHE_H
#include "gucefAUTH_CJwksCache.h"
#define GUCEF_AUTH_CJWKSCACHE_H
#endif /* GUCEF_AUTH_CJWKSCACHE_H ? */

#ifndef GUCEF_AUTH_COPENID2_H
#include "gucefAUTH_COpenId2.h"
#define GUCEF_AUTH_COPENID2_H
#endif /* GUCEF_AUTH_COPENID2_H ? */

#ifndef GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H
#include "gucefAUTH_CLoopbackRedirectListener.h"
#define GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H
#endif /* GUCEF_AUTH_CLOOPBACKREDIRECTLISTENER_H ? */

#ifndef GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#include "gucefAUTH_COAuth2EngineSettings.h"
#define GUCEF_AUTH_COAUTH2ENGINESETTINGS_H
#endif /* GUCEF_AUTH_COAUTH2ENGINESETTINGS_H ? */

#ifndef GUCEF_AUTH_COAUTH2PROTOCOL_H
#include "gucefAUTH_COAuth2Protocol.h"
#define GUCEF_AUTH_COAUTH2PROTOCOL_H
#endif /* GUCEF_AUTH_COAUTH2PROTOCOL_H ? */

#ifndef GUCEF_AUTH_COAUTH2HTTP_H
#include "gucefAUTH_COAuth2Http.h"
#define GUCEF_AUTH_COAUTH2HTTP_H
#endif /* GUCEF_AUTH_COAUTH2HTTP_H ? */

#ifndef GUCEF_AUTH_COAUTH2FLOW_H
#include "gucefAUTH_COAuth2Flow.h"
#define GUCEF_AUTH_COAUTH2FLOW_H
#endif /* GUCEF_AUTH_COAUTH2FLOW_H ? */

#ifndef GUCEF_AUTH_COAUTH2ENGINE_H
#include "gucefAUTH_COAuth2Engine.h"
#define GUCEF_AUTH_COAUTH2ENGINE_H
#endif /* GUCEF_AUTH_COAUTH2ENGINE_H ? */

#ifndef GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#include "gucefAUTH_IOpenId2ProviderProfile.h"
#define GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#endif /* GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H ? */

#ifndef GUCEF_AUTH_COPENID2PROFILEREGISTRY_H
#include "gucefAUTH_COpenId2ProfileRegistry.h"
#define GUCEF_AUTH_COPENID2PROFILEREGISTRY_H
#endif /* GUCEF_AUTH_COPENID2PROFILEREGISTRY_H ? */

#ifndef GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#include "gucefAUTH_CDescriptorAuthProvider.h"
#define GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_NAMES_H
#include "gucefAUTH_Names.h"
#define GUCEF_AUTH_NAMES_H
#endif /* GUCEF_AUTH_NAMES_H ? */

#ifndef GUCEF_AUTH_IACCOUNTPROFILE_H
#include "gucefAUTH_IAccountProfile.h"
#define GUCEF_AUTH_IACCOUNTPROFILE_H
#endif /* GUCEF_AUTH_IACCOUNTPROFILE_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGSPROFILE_H
#include "gucefAUTH_CAuthSettingsProfile.h"
#define GUCEF_AUTH_CAUTHSETTINGSPROFILE_H
#endif /* GUCEF_AUTH_CAUTHSETTINGSPROFILE_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_H ? */

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

#ifndef GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H
#define GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Why a provider that exists is not offered for a login */
enum EAuthUnavailableReason
{
    AUTHUNAVAILABLE_NONE = 0                    ,
    AUTHUNAVAILABLE_NO_CLIENT_ID = 1            ,   /**< the provider needs a registered app and the configuration has no client id for it */
    AUTHUNAVAILABLE_MISSING_SETTING = 2         ,   /**< a setting besides the client id is missing (a region, a pool, a tenant) */
    AUTHUNAVAILABLE_MISSING_PLUGIN = 3          ,   /**< the provider is made by a plugin that is not loaded */
    AUTHUNAVAILABLE_PLATFORM_NOT_SUPPORTED = 4  ,   /**< this platform, or this build, cannot run the login */
    AUTHUNAVAILABLE_NOT_AVAILABLE = 5               /**< the provider says it cannot be used here and now (a platform session is missing) */
};

typedef enum EAuthUnavailableReason TAuthUnavailableReason;

/*-------------------------------------------------------------------------*/

/**
 *  A provider the manager knows but does not offer, with what a person or a developer has to know about it: a line for
 *  the person ("Not set up in this build: needs a client id") and the key of the configuration that fills the gap
 *  ("Auth/Providers/google/clientId"). Nothing here is secret.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthUnavailableProvider
{
    public:

    CORE::CString providerName;
    TAuthUnavailableReason reason;
    CORE::CString message;              /**< one short line for the person, without a full stop */
    CORE::CString configKey;            /**< the path in the configuration that has to be filled, for the developer; empty when there is none */

    CAuthUnavailableProvider( void );

    CAuthUnavailableProvider( const CORE::CString& provider       ,
                              TAuthUnavailableReason unavailable  ,
                              const CORE::CString& line           ,
                              const CORE::CString& key            );

    void Clear( void );

    /** The text of a tooltip: what is missing and, when there is a key, where it goes ("Fill in Auth/Providers/google/clientId in the configuration") */
    CORE::CString GetTooltipText( void ) const;

    /** The reason as a word, for logs and tests: "no-client-id", "missing-setting", "missing-plugin", "platform-not-supported", "not-available" */
    static const char* GetReasonName( TAuthUnavailableReason reason );
};

typedef GUCEF::vector< CAuthUnavailableProvider > TAuthUnavailableProviderVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHUNAVAILABLEPROVIDER_H ? */

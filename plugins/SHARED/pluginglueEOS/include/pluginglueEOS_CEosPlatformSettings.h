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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

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

/**
 *  What EOS_Platform_Create needs, read from the node that CAuthManager hands the "epic" provider (section Auth, child
 *  Providers/epic): clientId and clientSecret as attributes, the other keys as attributes of the child Parameters or of
 *  the node itself. These are the keys the auth plugin reads, so the lobby and P2P plugins run on the same settings.
 *
 *    clientId, clientSecret         the client of the product (the secret is never logged)
 *    productId, sandboxId, deploymentId
 *    encryptionKey                  64 hex digits, optional, never logged
 *    productName, productVersion    EOS_Initialize, default "GUCEF" and "1.0"
 *    cacheDirectory                 optional cache directory of the SDK (path variables are resolved)
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosPlatformSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString clientId;
    AUTH::CSecretString clientSecret;
    CORE::CString productId;
    CORE::CString sandboxId;
    CORE::CString deploymentId;
    AUTH::CSecretString encryptionKey;
    CORE::CString productName;
    CORE::CString productVersion;
    CORE::CString cacheDirectory;

    CEosPlatformSettings( void );

    CEosPlatformSettings( const CEosPlatformSettings& src );

    virtual ~CEosPlatformSettings();

    CEosPlatformSettings& operator=( const CEosPlatformSettings& src );

    bool operator==( const CEosPlatformSettings& other ) const;

    /** All of the ids and the client credentials that EOS_Platform_Create needs are there */
    bool IsConfigured( void ) const;

    /** The values that have a form are in it: the encryption key is 64 hex digits when it is set */
    bool IsWellFormed( void ) const;

    /** Whether the two are for the same product and client: the ids, the client id and the secret */
    bool IsSameProduct( const CEosPlatformSettings& other ) const;

    /** Fills the config of EOS_Platform_Create with pointers into the settings, valid while the settings are not changed */
    void FillPlatformConfig( SEosPlatformConfig& config ) const;

    /** Reads the section "Auth" of an application config: the node Providers/epic of it. False when it has none */
    bool LoadAuthSection( const CORE::CDataNode& authSection );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
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

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSETTINGS_H ? */

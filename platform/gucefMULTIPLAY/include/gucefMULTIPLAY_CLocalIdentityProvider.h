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

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  The identity kept in a profile file on this machine, $APPDATADIR$/<app>/PlayerProfile.json by default.
 *  The identity has an Ed25519 key pair made the first time and kept in the file from then on (the private key as
 *  the attribute "privateKey"); the user id is the one of its public key (CPlayerIdentity::MakeUserId()). A profile
 *  from before identities had keys gets a key pair when it is loaded, its old user id is kept as previousUserId.
 *  Without an "ed25519" signature codec the user id is a GUID until there is one.
 *  GetIdentity() gives the private key too, for the services that prove the identity; IdentityChangedEvent does not.
 *  The file is read and written with the data tree codec that its extension names, json by default.
 *
 *  Config: appName, profilePath, defaultDisplayName
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLocalIdentityProvider : public CIIdentityProvider
{
    public:

    static const CORE::CString ProviderName;    /**< "local" */

    CLocalIdentityProvider( void );

    virtual ~CLocalIdentityProvider();

    virtual bool GetIdentity( CPlayerIdentity& identity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The account a backend knows the player by */
    bool SetAccount( const CORE::CString& backendName, const CORE::CString& accountId );

    void SetProfilePath( const CORE::CString& profilePath );

    /** The path with its variables resolved */
    CORE::CString GetProfilePath( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLocalIdentityProvider( const CLocalIdentityProvider& src );
    CLocalIdentityProvider& operator=( const CLocalIdentityProvider& src );

    bool EnsureLoaded( void );
    bool Load( void );
    bool Save( void );

    private:

    CORE::CString m_appName;
    CORE::CString m_profilePath;
    CORE::CString m_defaultDisplayName;
    CPlayerIdentity m_identity;
    bool m_loaded;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIIdentityProvider, CLocalIdentityProvider, MT::CMutex > TLocalIdentityProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

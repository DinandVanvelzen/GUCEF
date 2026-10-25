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

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

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
 *  Where the private key lives. By default in the profile file, as above. With a credential store (SetCredentialStore(),
 *  the stores of gucefAUTH) the key can live there instead, in the slot KeySlotName of the user id, and the profile
 *  file then holds everything but the key and says where the key is (keyLocation="credentialStore"). KeyStorage()
 *  says what is wanted:
 *      KEYSTORAGE_AUTO             a new profile gets its key in the store when there is one that keeps it, a profile
 *                                  that exists is left as it is, in whichever place its key is
 *      KEYSTORAGE_PROFILE_FILE     the key is in the profile file; a key in the store is moved there
 *      KEYSTORAGE_CREDENTIAL_STORE the key is in the store; a key in the profile file is moved there
 *  A move is only made when the option says so (never as a side effect of loading a profile) and is safe to stop at any
 *  point: the key is stored and read back, the new profile is written next to the old one and read back, and only then
 *  does it replace the old file; when any step fails the old file and the old place of the key are as they were and
 *  nothing stays behind in the store. A profile whose key cannot be found in the store has no key pair until it is back;
 *  no new identity is made in its place.
 *
 *  A name an earlier version gave every new profile. When the profile that is read still has exactly that name
 *  (legacyDefaultDisplayName) its player never chose a name: the name becomes defaultDisplayName, once. Only the name
 *  in the file changes: the file is read as a data tree, the name is changed in it and the tree is written next to the
 *  profile, read back and moved over it (the old file stays when a step fails), so the key, the user id and every other
 *  value of the profile stay as they were, also those this version does not know. A name that is any other text is
 *  never changed. Nothing is replaced when there is no default name, when it is the legacy name itself, or when the key of
 *  the profile cannot be found.
 *
 *  Config: appName, profilePath, defaultDisplayName, legacyDefaultDisplayName, keyInCredentialStore (true: KEYSTORAGE_CREDENTIAL_STORE, false:
 *  KEYSTORAGE_PROFILE_FILE, not there: KEYSTORAGE_AUTO)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLocalIdentityProvider : public CIIdentityProvider
{
    public:

    static const CORE::CString ProviderName;    /**< "local" */
    static const CORE::CString KeySlotName;     /**< "profile-key": the provider name of the slot of the credential store that holds the private key */

    /** Where the private key of the profile should live */
    enum EKeyStorage
    {
        KEYSTORAGE_AUTO = 0             ,
        KEYSTORAGE_PROFILE_FILE = 1     ,
        KEYSTORAGE_CREDENTIAL_STORE = 2
    };
    typedef enum EKeyStorage TKeyStorage;

    CLocalIdentityProvider( void );

    virtual ~CLocalIdentityProvider();

    virtual bool GetIdentity( CPlayerIdentity& identity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The account a backend knows the player by */
    bool SetAccount( const CORE::CString& backendName, const CORE::CString& accountId );

    void SetProfilePath( const CORE::CString& profilePath );

    /** The store the key can live in, not owned and it has to outlive the provider or be taken away with GUCEF_NULL first */
    void SetCredentialStore( AUTH::ICredentialStore* store );

    void SetKeyStorage( TKeyStorage storage );

    /** The name of a new profile, "Player" at first */
    void SetDefaultDisplayName( const CORE::CString& displayName );

    /** The name an earlier version gave a new profile, which a profile that still has it gets the default name for; empty for none */
    void SetLegacyDefaultDisplayName( const CORE::CString& displayName );

    TKeyStorage GetKeyStorage( void ) const;

    /** Whether the profile has its key in the credential store (not whether the key could be found there, see HasKey()) */
    bool IsKeyInCredentialStore( void );

    /** Whether the profile has a usable key pair now */
    bool HasKey( void );

    /**
     *  Moves the key from the profile file to the credential store, see the class description. True also when it
     *  already is there. False and nothing changed when there is no store, no key, or a step does not verify
     */
    bool MigrateKeyToCredentialStore( void );

    /** The other way: the key goes back to the profile file and is removed from the store afterwards */
    bool MigrateKeyToProfileFile( void );

    /** The path with its variables resolved */
    CORE::CString GetProfilePath( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLocalIdentityProvider( const CLocalIdentityProvider& src );
    CLocalIdentityProvider& operator=( const CLocalIdentityProvider& src );

    bool EnsureLoaded( void );
    bool ReadProfile( const CORE::CString& path, CPlayerIdentity& identity, bool& keyInStore, bool& keyMissing );
    bool WriteProfile( const CORE::CString& path, const CPlayerIdentity& identity, bool keyInStore );
    bool Save( void );
    bool ReplaceLegacyDefaultName( void );
    bool StoreKey( const CPlayerIdentity& identity );
    bool HasStoredKey( const CORE::CString& userId );
    void RemoveStoredKey( const CORE::CString& userId );
    CORE::CString GetTemporaryProfilePath( const CORE::CString& path ) const;

    private:

    CORE::CString m_appName;
    CORE::CString m_profilePath;
    CORE::CString m_defaultDisplayName;
    CORE::CString m_legacyDefaultDisplayName;
    CPlayerIdentity m_identity;
    AUTH::ICredentialStore* m_credentialStore;
    TKeyStorage m_keyStorage;
    bool m_keyInStore;
    bool m_keyMissing;
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

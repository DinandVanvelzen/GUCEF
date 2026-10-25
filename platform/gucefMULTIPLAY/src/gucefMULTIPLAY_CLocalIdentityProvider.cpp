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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#include "gucefMULTIPLAY_CLocalIdentityProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLocalIdentityProvider::ProviderName = "local";
const CORE::CString CLocalIdentityProvider::KeySlotName = "profile-key";

static const CORE::CString KeyLocationAttribute = "keyLocation";
static const CORE::CString KeyLocationCredentialStore = "credentialStore";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLocalIdentityProvider::CLocalIdentityProvider( void )
    : CIIdentityProvider()
    , m_appName( "GUCEF" )
    , m_profilePath()
    , m_defaultDisplayName( "Player" )
    , m_legacyDefaultDisplayName()
    , m_identity()
    , m_credentialStore( GUCEF_NULL )
    , m_keyStorage( KEYSTORAGE_AUTO )
    , m_keyInStore( false )
    , m_keyMissing( false )
    , m_loaded( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CLocalIdentityProvider::~CLocalIdentityProvider()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLocalIdentityProvider::GetProviderName( void ) const
{GUCEF_TRACE;
    return ProviderName;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::SetProfilePath( const CORE::CString& profilePath )
{GUCEF_TRACE;
    m_profilePath = profilePath;
    m_loaded = false;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::SetCredentialStore( AUTH::ICredentialStore* store )
{GUCEF_TRACE;
    m_credentialStore = store;
    m_loaded = false;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::SetKeyStorage( TKeyStorage storage )
{GUCEF_TRACE;
    if ( storage != m_keyStorage )
    {
        m_keyStorage = storage;
        m_loaded = false;
    }
}

/*-------------------------------------------------------------------------*/

CLocalIdentityProvider::TKeyStorage
CLocalIdentityProvider::GetKeyStorage( void ) const
{GUCEF_TRACE;
    return m_keyStorage;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::SetDefaultDisplayName( const CORE::CString& displayName )
{GUCEF_TRACE;
    if ( displayName != m_defaultDisplayName )
    {
        m_defaultDisplayName = displayName;
        m_loaded = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::SetLegacyDefaultDisplayName( const CORE::CString& displayName )
{GUCEF_TRACE;
    if ( displayName != m_legacyDefaultDisplayName )
    {
        m_legacyDefaultDisplayName = displayName;
        m_loaded = false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalIdentityProvider::GetProfilePath( void ) const
{GUCEF_TRACE;
    if ( !m_profilePath.IsNULLOrEmpty() )
        return CORE::RelativePath( m_profilePath );
    CORE::CString path = CORE::RelativePath( "$APPDATADIR$" );
    path = CORE::CombinePath( path, m_appName );
    return CORE::CombinePath( path, "PlayerProfile.json" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalIdentityProvider::GetTemporaryProfilePath( const CORE::CString& path ) const
{GUCEF_TRACE;
    // Same directory and extension, so the same codec reads it and the move over the profile stays on one volume
    return CORE::CombinePath( CORE::StripFilename( path ), "moving-" + CORE::ExtractFilename( path ) );
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::StoreKey( const CPlayerIdentity& identity )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_credentialStore || !identity.HasKeyPair() )
        return false;
    // A key that cannot be read back from the store is as good as not stored
    if ( !m_credentialStore->Store( KeySlotName, identity.userId, AUTH::CSecretString( identity.privateKey ) ) )
        return false;
    AUTH::CSecretString readBack;
    if ( !m_credentialStore->Load( KeySlotName, identity.userId, readBack ) || readBack.Reveal() != identity.privateKey )
    {
        m_credentialStore->Remove( KeySlotName, identity.userId );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::HasStoredKey( const CORE::CString& userId )
{GUCEF_TRACE;

    AUTH::CSecretString existing;
    return GUCEF_NULL != m_credentialStore && m_credentialStore->Load( KeySlotName, userId, existing );
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityProvider::RemoveStoredKey( const CORE::CString& userId )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_credentialStore )
        m_credentialStore->Remove( KeySlotName, userId );
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::ReadProfile( const CORE::CString& path, CPlayerIdentity& identity, bool& keyInStore, bool& keyMissing )
{GUCEF_TRACE;

    keyInStore = false;
    keyMissing = false;
    if ( !CORE::FileExists( path ) )
        return false;

    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"" << codecName << "\" codec to read the profile " << path );
        return false;
    }
    CORE::CDataNode root;
    if ( !codec->BuildDataTree( &root, path ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: Unable to read the profile " << path );
        return false;
    }

    CORE::CDataNodeSerializableSettings settings;
    if ( !identity.Deserialize( root, settings ) )
        return false;

    // The private key is kept here, next to what Serialize() writes, or in the credential store
    CORE::CString privateKey = root.GetAttributeValueOrChildValueByName( "privateKey" ).AsString( CORE::CString::Empty, true );
    if ( !privateKey.IsNULLOrEmpty() )
    {
        if ( !identity.SetPrivateKey( privateKey ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " cannot be used, is there an \"ed25519\" signature codec?" );
        return true;
    }

    CORE::CString location = root.GetAttributeValueOrChildValueByName( KeyLocationAttribute ).AsString( CORE::CString::Empty, true );
    if ( KeyLocationCredentialStore == location )
    {
        keyInStore = true;
        AUTH::CSecretString secret;
        if ( GUCEF_NULL == m_credentialStore )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " is in a credential store and there is none" );
            keyMissing = true;
        }
        else
        if ( !m_credentialStore->Load( KeySlotName, identity.userId, secret ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " is not in the credential store" );
            keyMissing = true;
        }
        else
        {
            CPlayerIdentity probe;
            if ( probe.SetPrivateKey( secret.Reveal() ) && probe.publicKey.Lowercase() == identity.publicKey.Lowercase() )
            {
                identity.SetPrivateKey( secret.Reveal() );
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key in the credential store is not the key of the profile " << path );
                keyMissing = true;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::WriteProfile( const CORE::CString& path, const CPlayerIdentity& identity, bool keyInStore )
{GUCEF_TRACE;

    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"" << codecName << "\" codec to write the profile " << path );
        return false;
    }

    CORE::CDataNode root( "PlayerProfile" );
    CORE::CDataNodeSerializableSettings settings;
    identity.Serialize( root, settings );
    if ( keyInStore )
        root.SetAttribute( KeyLocationAttribute, KeyLocationCredentialStore );
    else
    if ( !identity.privateKey.IsNULLOrEmpty() )
        root.SetAttribute( "privateKey", identity.privateKey );

    CORE::CreateDirs( CORE::StripFilename( path ) );
    if ( !codec->StoreDataTree( &root, path ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: Unable to write the profile " << path );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::Save( void )
{GUCEF_TRACE;

    return WriteProfile( GetProfilePath(), m_identity, m_keyInStore );
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::EnsureLoaded( void )
{GUCEF_TRACE;

    if ( m_loaded )
        return true;

    CORE::CString path = GetProfilePath();
    CPlayerIdentity loaded;
    bool keyInStore = false;
    bool keyMissing = false;
    m_keyInStore = false;
    m_keyMissing = false;
    if ( !ReadProfile( path, loaded, keyInStore, keyMissing ) )
    {
        // The first time: a new identity, kept from now on
        m_identity = CPlayerIdentity();
        m_identity.displayName = m_defaultDisplayName;
        if ( !m_identity.GenerateKeyPair() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"ed25519\" signature codec, the new identity has no key pair until there is one" );
            m_identity.userId = CPlayerIdentity::MakeGuid();
        }
        else
        if ( KEYSTORAGE_CREDENTIAL_STORE == m_keyStorage || ( KEYSTORAGE_AUTO == m_keyStorage && GUCEF_NULL != m_credentialStore ) )
        {
            if ( StoreKey( m_identity ) )
                m_keyInStore = true;
            else
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the new profile cannot be kept in a credential store, it is kept in the profile file" );
        }
        Save();
        m_loaded = true;
        return true;
    }

    m_identity = loaded;
    m_keyInStore = keyInStore;
    m_keyMissing = keyMissing;
    if ( keyMissing )
    {
        // The identity stays what it is: no key pair, and no other identity in its place
        m_loaded = true;
        return true;
    }

    if ( !m_identity.HasKeyPair() || !m_identity.IsUserIdOfKey() )
    {
        // A profile from before identities had a key pair gets one, its user id becomes the one of the key
        CORE::CString oldUserId = m_identity.userId;
        if ( m_identity.GenerateKeyPair() )
        {
            if ( m_identity.previousUserId.IsNULLOrEmpty() && oldUserId != m_identity.userId )
                m_identity.previousUserId = oldUserId;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The profile has a key pair now, the user id " << oldUserId << " became " << m_identity.userId );
            m_keyInStore = false;
            Save();
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"ed25519\" signature codec, the identity has no key pair" );
        }
    }
    m_loaded = true;

    // A profile that still has the name earlier versions gave every profile was never named by its player
    ReplaceLegacyDefaultName();

    // Where the key lives only changes when the option says so
    if ( m_identity.HasKeyPair() )
    {
        if ( KEYSTORAGE_CREDENTIAL_STORE == m_keyStorage && !m_keyInStore )
            MigrateKeyToCredentialStore();
        else
        if ( KEYSTORAGE_PROFILE_FILE == m_keyStorage && m_keyInStore )
            MigrateKeyToProfileFile();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::ReplaceLegacyDefaultName( void )
{GUCEF_TRACE;

    if ( m_legacyDefaultDisplayName.IsNULLOrEmpty() || m_defaultDisplayName.IsNULLOrEmpty() || m_defaultDisplayName == m_legacyDefaultDisplayName )
        return false;
    if ( m_keyMissing || m_identity.displayName != m_legacyDefaultDisplayName )
        return false;

    const CORE::CString path = GetProfilePath();
    const CORE::CString movingPath = GetTemporaryProfilePath( path );

    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
        return false;

    // the file as it is, with only the name changed: what this version does not know of the profile stays
    CORE::CDataNode root;
    if ( !codec->BuildDataTree( &root, path ) ||
         root.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true ) != m_legacyDefaultDisplayName ||
         !root.SetAttribute( "displayName", m_defaultDisplayName ) )
        return false;

    // next to the profile, read back, and only then over it
    CORE::DeleteFile( movingPath );
    bool verified = codec->StoreDataTree( &root, movingPath );
    if ( verified )
    {
        CPlayerIdentity check;
        bool checkInStore = false;
        bool checkMissing = false;
        verified = ReadProfile( movingPath, check, checkInStore, checkMissing ) && checkInStore == m_keyInStore && !checkMissing &&
                   check.userId == m_identity.userId && check.publicKey == m_identity.publicKey && check.privateKey == m_identity.privateKey &&
                   check.displayName == m_defaultDisplayName;
    }
    if ( verified )
        verified = CORE::MoveFile( movingPath, path, true );
    if ( !verified )
    {
        CORE::DeleteFile( movingPath );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The profile " << path << " could not be renamed, it is as it was and is tried again at the next start" );
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The profile " << path << " still had the name earlier versions gave every profile, it has the default name now" );
    }

    // the name is shown either way, the file follows
    m_identity.displayName = m_defaultDisplayName;
    return verified;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::IsKeyInCredentialStore( void )
{GUCEF_TRACE;

    return EnsureLoaded() && m_keyInStore;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::HasKey( void )
{GUCEF_TRACE;

    return EnsureLoaded() && m_identity.HasKeyPair() && !m_keyMissing;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::MigrateKeyToCredentialStore( void )
{GUCEF_TRACE;

    if ( !EnsureLoaded() || m_keyMissing )
        return false;
    if ( m_keyInStore )
        return true;
    if ( !m_identity.HasKeyPair() || GUCEF_NULL == m_credentialStore )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key stays in the profile file, there is no key or no credential store to move it to" );
        return false;
    }

    CORE::CString path = GetProfilePath();
    CORE::CString movingPath = GetTemporaryProfilePath( path );
    bool hadSlot = HasStoredKey( m_identity.userId );

    // 1. The key in the store, read back. The profile file is untouched
    if ( !StoreKey( m_identity ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The credential store does not keep the key, it stays in the profile file" );
        return false;
    }

    // 2. The new profile next to the old one, read back: it has to give the same identity with the key from the store
    CORE::DeleteFile( movingPath );
    bool verified = WriteProfile( movingPath, m_identity, true );
    if ( verified )
    {
        CPlayerIdentity check;
        bool checkInStore = false;
        bool checkMissing = false;
        verified = ReadProfile( movingPath, check, checkInStore, checkMissing ) && checkInStore && !checkMissing &&
                   check.userId == m_identity.userId && check.publicKey == m_identity.publicKey && check.privateKey == m_identity.privateKey;
    }

    // 3. Only now does it replace the old file
    if ( verified )
        verified = CORE::MoveFile( movingPath, path, true );

    if ( !verified )
    {
        CORE::DeleteFile( movingPath );
        if ( !hadSlot )
            RemoveStoredKey( m_identity.userId );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: Moving the key to the credential store did not verify, the profile file " << path << " is as it was" );
        return false;
    }

    m_keyInStore = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " is in the credential store now" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::MigrateKeyToProfileFile( void )
{GUCEF_TRACE;

    if ( !EnsureLoaded() || m_keyMissing )
        return false;
    if ( !m_keyInStore )
        return true;
    if ( !m_identity.HasKeyPair() )
        return false;

    CORE::CString path = GetProfilePath();
    CORE::CString movingPath = GetTemporaryProfilePath( path );

    CORE::DeleteFile( movingPath );
    bool verified = WriteProfile( movingPath, m_identity, false );
    if ( verified )
    {
        CPlayerIdentity check;
        bool checkInStore = false;
        bool checkMissing = false;
        verified = ReadProfile( movingPath, check, checkInStore, checkMissing ) && !checkInStore && !checkMissing &&
                   check.userId == m_identity.userId && check.publicKey == m_identity.publicKey && check.privateKey == m_identity.privateKey;
    }
    if ( verified )
        verified = CORE::MoveFile( movingPath, path, true );
    if ( !verified )
    {
        CORE::DeleteFile( movingPath );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: Moving the key back to the profile file did not verify, the key stays in the credential store" );
        return false;
    }

    // The profile file has the key now, the copy in the store is not needed any more
    RemoveStoredKey( m_identity.userId );
    m_keyInStore = false;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " is in the profile file again" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::GetIdentity( CPlayerIdentity& identity )
{GUCEF_TRACE;

    if ( !EnsureLoaded() )
        return false;
    identity = m_identity;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::SetDisplayName( const CORE::CString& displayName )
{GUCEF_TRACE;

    if ( displayName.IsNULLOrEmpty() || !EnsureLoaded() )
        return false;

    m_identity.displayName = displayName;
    bool saved = Save();
    CPlayerIdentity eData( m_identity.GetPublicIdentity() );
    NotifyObservers( IdentityChangedEvent, &eData );
    return saved;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::SetAccount( const CORE::CString& backendName, const CORE::CString& accountId )
{GUCEF_TRACE;

    if ( backendName.IsNULLOrEmpty() || !EnsureLoaded() )
        return false;

    m_identity.accounts.SetAllowMultipleValues( false );
    m_identity.accounts.Set( backendName, accountId );
    bool saved = Save();
    CPlayerIdentity eData( m_identity.GetPublicIdentity() );
    NotifyObservers( IdentityChangedEvent, &eData );
    return saved;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "appName", m_appName );
    config.SetAttribute( "profilePath", m_profilePath );
    config.SetAttribute( "defaultDisplayName", m_defaultDisplayName );
    if ( !m_legacyDefaultDisplayName.IsNULLOrEmpty() )
        config.SetAttribute( "legacyDefaultDisplayName", m_legacyDefaultDisplayName );
    if ( KEYSTORAGE_AUTO != m_keyStorage )
        config.SetAttribute( "keyInCredentialStore", KEYSTORAGE_CREDENTIAL_STORE == m_keyStorage );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_appName = config.GetAttributeValueOrChildValueByName( "appName" ).AsString( m_appName, true );
    m_defaultDisplayName = config.GetAttributeValueOrChildValueByName( "defaultDisplayName" ).AsString( m_defaultDisplayName, true );
    m_legacyDefaultDisplayName = config.GetAttributeValueOrChildValueByName( "legacyDefaultDisplayName" ).AsString( m_legacyDefaultDisplayName, true );
    CORE::CString profilePath = config.GetAttributeValueOrChildValueByName( "profilePath" ).AsString( m_profilePath, true );
    if ( profilePath != m_profilePath )
        SetProfilePath( profilePath );
    if ( GUCEF_NULL != config.GetAttribute( "keyInCredentialStore" ) )
    {
        bool inStore = config.GetAttributeValueOrChildValueByName( "keyInCredentialStore" ).AsBool( false, true );
        m_keyStorage = inStore ? KEYSTORAGE_CREDENTIAL_STORE : KEYSTORAGE_PROFILE_FILE;
    }
    m_loaded = false;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLocalIdentityProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLocalIdentityProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

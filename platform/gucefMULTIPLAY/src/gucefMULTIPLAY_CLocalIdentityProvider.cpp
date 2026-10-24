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
    , m_identity()
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

bool
CLocalIdentityProvider::Load( void )
{GUCEF_TRACE;

    CORE::CString path = GetProfilePath();
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
    CPlayerIdentity identity;
    if ( !identity.Deserialize( root, settings ) )
        return false;
    // The private key is kept here, next to what Serialize() writes
    CORE::CString privateKey = root.GetAttributeValueOrChildValueByName( "privateKey" ).AsString( CORE::CString::Empty, true );
    if ( !privateKey.IsNULLOrEmpty() && !identity.SetPrivateKey( privateKey ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The key of the profile " << path << " cannot be used, is there an \"ed25519\" signature codec?" );
    m_identity = identity;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::Save( void )
{GUCEF_TRACE;

    CORE::CString path = GetProfilePath();
    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"" << codecName << "\" codec to write the profile " << path );
        return false;
    }

    CORE::CDataNode root( "PlayerProfile" );
    CORE::CDataNodeSerializableSettings settings;
    m_identity.Serialize( root, settings );
    if ( !m_identity.privateKey.IsNULLOrEmpty() )
        root.SetAttribute( "privateKey", m_identity.privateKey );

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
CLocalIdentityProvider::EnsureLoaded( void )
{GUCEF_TRACE;

    if ( m_loaded )
        return true;

    if ( !Load() )
    {
        // The first time: a new identity, kept from now on
        m_identity = CPlayerIdentity();
        m_identity.displayName = m_defaultDisplayName;
        if ( !m_identity.GenerateKeyPair() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"ed25519\" signature codec, the new identity has no key pair until there is one" );
            m_identity.userId = CPlayerIdentity::MakeGuid();
        }
        Save();
    }
    else
    if ( !m_identity.HasKeyPair() || !m_identity.IsUserIdOfKey() )
    {
        // A profile from before identities had a key pair gets one, its user id becomes the one of the key
        CORE::CString oldUserId = m_identity.userId;
        if ( m_identity.GenerateKeyPair() )
        {
            if ( m_identity.previousUserId.IsNULLOrEmpty() && oldUserId != m_identity.userId )
                m_identity.previousUserId = oldUserId;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: The profile has a key pair now, the user id " << oldUserId << " became " << m_identity.userId );
            Save();
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityProvider: There is no \"ed25519\" signature codec, the identity has no key pair" );
        }
    }
    m_loaded = true;
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
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_appName = config.GetAttributeValueOrChildValueByName( "appName" ).AsString( m_appName, true );
    m_defaultDisplayName = config.GetAttributeValueOrChildValueByName( "defaultDisplayName" ).AsString( m_defaultDisplayName, true );
    CORE::CString profilePath = config.GetAttributeValueOrChildValueByName( "profilePath" ).AsString( m_profilePath, true );
    if ( profilePath != m_profilePath )
        SetProfilePath( profilePath );
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

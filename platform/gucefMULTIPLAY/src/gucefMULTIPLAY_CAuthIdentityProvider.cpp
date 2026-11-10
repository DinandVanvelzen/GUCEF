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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#include "gucefCORE_CTScopedValue.h"
#define GUCEF_CORE_CTSCOPEDVALUE_H
#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#include "gucefMULTIPLAY_CAuthIdentityProvider.h"

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

const CORE::CString CAuthIdentityProvider::ProviderName = "sso";
const CORE::CEvent CAuthIdentityProvider::AuthSettingsChangedEvent = "GUCEF::MULTIPLAY::CAuthIdentityProvider::AuthSettingsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profile key as the manager sees it. It asks the provider every time, so it is right from the first use of the
 *  manager, whoever uses the manager first, and follows the profile when that changes.
 */
class CAuthIdentityProvider::CProfileSigner : public AUTH::ILinkSigner
{
    public:

    explicit CProfileSigner( CAuthIdentityProvider& owner ) : AUTH::ILinkSigner(), m_owner( owner ) {}

    virtual ~CProfileSigner() {}

    virtual CORE::CString GetKeyId( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        m_owner.RefreshSigner();
        return m_owner.m_signer.GetKeyId();
    }

    virtual CORE::CString GetPublicKeyHex( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        m_owner.RefreshSigner();
        return m_owner.m_signer.GetPublicKeyHex();
    }

    virtual bool Sign( const void* data, UInt32 dataSize, CORE::CDynamicBuffer& signature ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_owner.RefreshSigner();
        return m_owner.m_signer.Sign( data, dataSize, signature );
    }

    private:

    CAuthIdentityProvider& m_owner;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::RegisterEvents( void )
{GUCEF_TRACE;

    AuthSettingsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAuthIdentityProvider::CAuthIdentityProvider( void )
    : CIIdentityProvider()
    , m_local()
    , m_signer()
    , m_profileSigner( GUCEF_NULL )
    , m_ownedManager( GUCEF_NULL )
    , m_manager( GUCEF_NULL )
    , m_credentialStore( GUCEF_NULL )
    , m_credentialDirectory()
    , m_defaultDisplayName( "Player" )
    , m_wired( false )
    , m_raising( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_profileSigner = GUCEF_NEW CProfileSigner( *this );
    m_ownedManager = GUCEF_NEW AUTH::CAuthManager();
    m_manager = m_ownedManager;
    TEventCallback localCallback( this, &CAuthIdentityProvider::OnLocalIdentityChanged );
    SubscribeTo( &m_local, IdentityChangedEvent, localCallback );
    SubscribeToManager( m_manager );
}

/*-------------------------------------------------------------------------*/

CAuthIdentityProvider::~CAuthIdentityProvider()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    UnsubscribeFrom( &m_local );
    Detach( m_manager );
    m_manager = GUCEF_NULL;
    GUCEF_DELETE m_ownedManager;
    m_ownedManager = GUCEF_NULL;
    m_local.SetCredentialStore( GUCEF_NULL );
    GUCEF_DELETE m_credentialStore;
    m_credentialStore = GUCEF_NULL;
    GUCEF_DELETE m_profileSigner;
    m_profileSigner = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::Detach( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    if ( GUCEF_NULL == manager )
        return;
    UnsubscribeFrom( manager );
    // The manager must not keep what is about to go away
    if ( m_wired )
    {
        manager->SetSigner( GUCEF_NULL );
        manager->SetCredentialStore( GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthIdentityProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::RefreshSigner( void )
{GUCEF_TRACE;

    CPlayerIdentity identity;
    if ( m_local.GetIdentity( identity ) )
        m_signer.SetIdentity( identity );
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::EnsureWired( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager )
        return;

    // The credential store follows the directory of the settings. One store is shared by the key of the profile and
    // the logins of the manager: two stores on one directory would overwrite each other
    CORE::CString directory = m_manager->GetSettings().GetCredentialDirectory();
    if ( m_wired && directory == m_credentialDirectory )
        return;

    if ( m_wired )
        m_manager->SetCredentialStore( GUCEF_NULL );
    m_local.SetCredentialStore( GUCEF_NULL );
    GUCEF_DELETE m_credentialStore;
    m_credentialStore = GUCEF_NULL;
    m_credentialDirectory = directory;

    if ( AUTH::CFileCredentialStore::IsAvailable() && !directory.IsNULLOrEmpty() )
    {
        m_credentialStore = GUCEF_NEW AUTH::CFileCredentialStore( directory );
        m_local.SetCredentialStore( m_credentialStore );
        m_manager->SetCredentialStore( m_credentialStore );
    }
    else
    {
        // Without a store that keeps secrets the key stays in the profile file; the manager makes its own store
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthIdentityProvider: There is no credential store that keeps secrets, the key of the profile stays in the profile file" );
    }
    m_manager->SetSigner( m_profileSigner );
    m_wired = true;
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthManager&
CAuthIdentityProvider::GetAuthManager( void )
{GUCEF_TRACE;

    EnsureWired();
    return *m_manager;
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::SetAuthManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    if ( GUCEF_NULL == manager )
        manager = m_ownedManager;
    if ( manager == m_manager )
        return;

    Detach( m_manager );
    m_wired = false;
    m_manager = manager;
    SubscribeToManager( m_manager );
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::SubscribeToManager( AUTH::CAuthManager* manager )
{GUCEF_TRACE;

    TEventCallback accountsCallback( this, &CAuthIdentityProvider::OnManagerAccountsChanged );
    TEventCallback settingsCallback( this, &CAuthIdentityProvider::OnManagerSettingsChanged );
    SubscribeTo( manager, AUTH::CAuthManager::AccountLinkedEvent, accountsCallback );
    SubscribeTo( manager, AUTH::CAuthManager::AccountUnlinkedEvent, accountsCallback );
    SubscribeTo( manager, AUTH::CAuthManager::LoginSucceededEvent, accountsCallback );
    SubscribeTo( manager, AUTH::CAuthManager::SettingsChangedEvent, settingsCallback );
}

/*-------------------------------------------------------------------------*/

CLocalIdentityProvider&
CAuthIdentityProvider::GetLocalProvider( void )
{GUCEF_TRACE;

    EnsureWired();
    return m_local;
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::BuildIdentity( CPlayerIdentity& identity, bool withProofs )
{GUCEF_TRACE;

    EnsureWired();
    if ( !m_local.GetIdentity( identity ) )
        return false;
    m_signer.SetIdentity( identity );

    AUTH::TLinkRecordVector links;
    m_manager->GetLinkedAccounts( links );
    if ( links.empty() )
        return true;

    GUCEF::vector< AUTH::CAuthResult > sessions;
    m_manager->GetSessionAccounts( sessions );

    identity.accounts.SetAllowMultipleValues( true );
    CORE::CString accountName;
    for ( size_t i=0; i<links.size(); ++i )
    {
        const AUTH::CAccountRef& account = links[ i ].accountRef;
        if ( !account.IsValid() )
            continue;
        identity.accounts.Set( account.GetProvider(), account.GetSubject() );

        for ( size_t s=0; s<sessions.size(); ++s )
        {
            if ( sessions[ s ].accountRef != account )
                continue;
            if ( accountName.IsNULLOrEmpty() )
                accountName = sessions[ s ].displayName;
            if ( withProofs && identity.linkProofs.size() < CLinkProof::MaxCount )
            {
                CLinkProof proof( sessions[ s ].providerName, sessions[ s ].proof );
                if ( proof.IsSendable() )
                    identity.linkProofs.push_back( proof );
            }
            break;
        }
    }

    // The name of the account is a start for a player who did not choose a name yet
    if ( !accountName.IsNULLOrEmpty() && identity.displayName == m_defaultDisplayName )
        identity.displayName = accountName;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::GetIdentity( CPlayerIdentity& identity )
{GUCEF_TRACE;

    return BuildIdentity( identity, true );
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::RaiseIdentityChanged( void )
{GUCEF_TRACE;

    if ( m_raising )
        return;
    CPlayerIdentity identity;
    if ( !BuildIdentity( identity, false ) )
        return;
    // the guard against raising from within an observer; an observer that throws must not leave it set
    CORE::CTScopedValue< bool > raising( m_raising, true );
    CPlayerIdentity eData( identity.GetPublicIdentity() );
    NotifyObservers( IdentityChangedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::SetDisplayName( const CORE::CString& displayName )
{GUCEF_TRACE;

    EnsureWired();
    // The local provider raises its event, which becomes ours
    return m_local.SetDisplayName( displayName );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::SetAccount( const CORE::CString& backendName, const CORE::CString& accountId )
{GUCEF_TRACE;

    EnsureWired();
    return m_local.SetAccount( backendName, accountId );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::Login( const CORE::CString& externalProviderName, CORE::CFutureResult& future )
{GUCEF_TRACE;

    EnsureWired();
    AUTH::CAuthOptions options;
    options.linkToProfile = true;
    return m_manager->Login( externalProviderName, options, future );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::AutoLogin( CORE::CFutureResult& future )
{GUCEF_TRACE;

    EnsureWired();
    if ( !m_manager->GetSettings().autoLogin )
        return false;
    return m_manager->AutoLogin( future );
}

/*-------------------------------------------------------------------------*/

AUTH::TAuthLoginState
CAuthIdentityProvider::GetLoginState( void )
{GUCEF_TRACE;

    EnsureWired();
    return m_manager->GetLoginState();
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::GetLinkedAccounts( AUTH::TAccountRefVector& accounts )
{GUCEF_TRACE;

    EnsureWired();
    accounts.clear();
    AUTH::TLinkRecordVector links;
    if ( !m_manager->GetLinkedAccounts( links ) )
        return false;
    accounts.reserve( links.size() );
    for ( size_t i=0; i<links.size(); ++i )
        accounts.push_back( links[ i ].accountRef );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::GetOfferedProviders( CORE::CString::StringVector& providerNames )
{GUCEF_TRACE;

    EnsureWired();
    m_manager->GetOfferedProviders( providerNames );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::Unlink( const AUTH::CAccountRef& account )
{GUCEF_TRACE;

    EnsureWired();
    return m_manager->Unlink( account );
}

/*-------------------------------------------------------------------------*/

AUTH::CAccountRef
CAuthIdentityProvider::GetPrimaryAccount( void )
{GUCEF_TRACE;

    EnsureWired();
    AUTH::TLinkRecordVector links;
    if ( !m_manager->GetLinkedAccounts( links ) || links.empty() )
        return AUTH::CAccountRef();
    return links[ 0 ].accountRef;
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::LoadAuthConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    bool loaded = m_manager->LoadConfig( config );

    // An application that keeps its settings in the section Auth can say where the key lives there too
    const CORE::CDataNode* section = config.FindChild( "Auth" );
    if ( GUCEF_NULL == section )
        section = &config;
    if ( GUCEF_NULL != section->GetAttribute( "keyInCredentialStore" ) )
    {
        bool inStore = section->GetAttributeValueOrChildValueByName( "keyInCredentialStore" ).AsBool( false, true );
        m_local.SetKeyStorage( inStore ? CLocalIdentityProvider::KEYSTORAGE_CREDENTIAL_STORE : CLocalIdentityProvider::KEYSTORAGE_PROFILE_FILE );
    }

    // The directories may have changed, the stores follow
    m_wired = false;
    return loaded;
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::SaveAuthConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_manager->SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = m_local.SaveConfig( config );
    config.SetAttribute( "defaultDisplayName", m_defaultDisplayName );
    CORE::CDataNode* authNode = config.AddChild( "Auth", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != authNode )
        success = SaveAuthConfig( *authNode ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAuthIdentityProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    bool success = m_local.LoadConfig( config );
    m_defaultDisplayName = config.GetAttributeValueOrChildValueByName( "defaultDisplayName" ).AsString( m_defaultDisplayName, true );
    const CORE::CDataNode* authNode = config.FindChild( "Auth" );
    if ( GUCEF_NULL != authNode )
        success = LoadAuthConfig( *authNode ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::OnLocalIdentityChanged( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RaiseIdentityChanged();
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::OnManagerAccountsChanged( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RaiseIdentityChanged();
}

/*-------------------------------------------------------------------------*/

void
CAuthIdentityProvider::OnManagerSettingsChanged( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( AuthSettingsChangedEvent );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthIdentityProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CAuthIdentityProvider";
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

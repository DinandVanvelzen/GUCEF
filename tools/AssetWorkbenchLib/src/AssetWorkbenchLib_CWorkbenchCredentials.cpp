/*
 *  AssetWorkbenchLib: the sign in of the workbench: the credential stores, the keyring code and the routing of the sign in of an online store
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H
#include "AssetWorkbenchLib_CWorkbenchCredentials.h"
#define ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHCREDENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchCredentials::GetDefaultFolder( void )
{GUCEF_TRACE;

    return CORE::CombinePath( CORE::CombinePath( "$APPCONFIGDIR$", "AssetWorkbench" ), "Auth" );
}

/*-------------------------------------------------------------------------*/

CWorkbenchCredentials::CWorkbenchCredentials( void )
    : m_sessionStore()
    , m_keyringStore()
    , m_session()
    , m_installed( false )
{GUCEF_TRACE;

    // the sessions have a key file of their own and are used again without a code; the saved passwords need the keyring code
    m_keyringStore.SetKeyMode( AUTH::CFileCredentialStore::KEYMODE_KEYRING_CODE );
    m_session.SetStore( &m_keyringStore );
    Open( GetDefaultFolder() );
}

/*-------------------------------------------------------------------------*/

CWorkbenchCredentials::~CWorkbenchCredentials()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchCredentials::Open( const CORE::CString& folder )
{GUCEF_TRACE;

    if ( folder.IsNULLOrEmpty() )
        return false;
    m_sessionStore.SetDirectory( folder );
    m_keyringStore.SetDirectory( folder );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchCredentials::ApplySettings( const CAssetSourcesSettings& settings )
{GUCEF_TRACE;

    if ( !settings.credentialFolder.IsNULLOrEmpty() )
        Open( settings.credentialFolder );
    AUTH::CKeyringSettings keyring = m_session.GetSettings();
    keyring.idleLockInSeconds = settings.keyringIdleLockInSeconds;
    m_session.SetSettings( keyring );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchCredentials::Install( void )
{GUCEF_TRACE;

    ASSETSTORE::CAssetStoreGlobal::Instance()->SetWebSignInStores( &m_sessionStore, &m_keyringStore );
    m_installed = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchCredentials: The stores of sessions and saved passwords are in " << GetFolder() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchCredentials::Shutdown( void )
{GUCEF_TRACE;

    if ( m_installed )
    {
        ASSETSTORE::CAssetStoreGlobal* global = ASSETSTORE::CAssetStoreGlobal::Instance();
        if ( global->GetWebSessionStore() == &m_sessionStore && global->GetWebCredentialStore() == &m_keyringStore )
            global->SetWebSignInStores( GUCEF_NULL, GUCEF_NULL );
        m_installed = false;
    }
    // locks the keyring, whatever runs is let finish first
    m_session.Shutdown();
    m_keyringStore.LockKeyring();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchCredentials::IsInstalled( void ) const
{GUCEF_TRACE;

    return m_installed;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchCredentials::IsKeyringAvailable( void ) const
{GUCEF_TRACE;

    return AUTH::CFileCredentialStore::IsKeyringAvailable() && AUTH::CFileCredentialStore::IsAvailable();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchCredentials::GetFolder( void ) const
{GUCEF_TRACE;

    return m_keyringStore.GetDirectory();
}

/*-------------------------------------------------------------------------*/

AUTH::CFileCredentialStore&
CWorkbenchCredentials::GetSessionStore( void )
{GUCEF_TRACE;

    return m_sessionStore;
}

/*-------------------------------------------------------------------------*/

AUTH::CFileCredentialStore&
CWorkbenchCredentials::GetKeyringStore( void )
{GUCEF_TRACE;

    return m_keyringStore;
}

/*-------------------------------------------------------------------------*/

AUTH::CKeyringSession&
CWorkbenchCredentials::GetKeyringSession( void )
{GUCEF_TRACE;

    return m_session;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

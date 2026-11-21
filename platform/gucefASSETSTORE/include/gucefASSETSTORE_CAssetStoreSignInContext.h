/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H
#define GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {

namespace AUTH {
class CAuthManager;
class CKeyringSession;
}

namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the application that hosts the asset store service gives the stores for signing in on the server: the manager of
 *  the sign in flows of gucefAUTH, the credential store of the sessions (a key file store: a session is used again
 *  without a person), the credential store of the saved passwords (sealed with the keyring code) with the session that
 *  unlocks it, and the settings of the stores themselves (by provider id, paths of files and the like, nothing secret).
 *  Nothing is owned: the application keeps them alive for as long as the service runs.
 *
 *  The service hands it to every provider once (CIAssetStoreProvider::ConfigureSignIn()), when it opens and for providers
 *  that join later.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreSignInContext
{
    public:

    AUTH::CAuthManager* authManager;            /**< runs the sign in flows of the stores that sign in with a provider of gucefAUTH; GUCEF_NULL for none */
    AUTH::ICredentialStore* sessionStore;       /**< tokens and cookies of the sessions; GUCEF_NULL for none */
    AUTH::ICredentialStore* credentialStore;    /**< saved passwords, behind the keyring code; GUCEF_NULL for none */
    AUTH::CKeyringSession* keyringSession;      /**< unlocks and locks credentialStore for UnlockKeyring() and LockKeyring(); GUCEF_NULL for none */
    CORE::CDataNode storeSettings;              /**< one child per provider id with the settings of that store's sign in */
    bool browserAvailable;                      /**< a browser for the web sign in of the stores runs beside the service; false when there is none */

    CAssetStoreSignInContext( void );

    ~CAssetStoreSignInContext();

    /** The settings child of the store, GUCEF_NULL when the application gave none */
    const CORE::CDataNode* GetStoreSettings( const CORE::CString& storeId ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORESIGNINCONTEXT_H ? */

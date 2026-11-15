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

#ifndef GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H
#define GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The saved credentials of a store for its automatic sign in (section 12.2 of the design): named slots ("username",
 *  "password", ...) as declared by the login script, each value a secret (AUTH::CSecretString) that is never logged
 *  and is wiped when it is let go.
 *
 *  In a gucefAUTH credential store every slot is kept under the provider name CredentialProviderName and the account
 *  "<store id>/<slot>", sealed as the store seals everything (CFileCredentialStore: XChaCha20-Poly1305). Which store
 *  is used is the choice of the application: one unlocked with the keyring code for passwords, for example.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CWebSignInCredentials
{
    public:

    static const CORE::CString CredentialProviderName;     /**< "assetstore-web-credentials" */

    CWebSignInCredentials( void );
    CWebSignInCredentials( const CWebSignInCredentials& src );
    ~CWebSignInCredentials();
    CWebSignInCredentials& operator=( const CWebSignInCredentials& src );

    void Set( const CORE::CString& slot, const AUTH::CSecretString& value );

    /** False when the slot is not there or is empty */
    bool Get( const CORE::CString& slot, AUTH::CSecretString& value ) const;

    bool Has( const CORE::CString& slot ) const;

    void Remove( const CORE::CString& slot );

    /** Wipes every value and forgets the slots */
    void Clear( void );

    UInt32 GetCount( void ) const;

    /** Whether every one of the slots has a value */
    bool HasAll( const CORE::CString::StringVector& slots ) const;

    /** Keeps every slot in the credential store; false when one could not be kept */
    bool SaveTo( AUTH::ICredentialStore& store, const CORE::CString& storeId ) const;

    /** Reads the named slots that the credential store has; true when it could be asked, also when some are missing */
    bool LoadFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots );

    /** Removes the named slots of the store from the credential store */
    static bool RemoveFrom( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots );

    /** Whether the credential store has a value for every one of the slots */
    static bool HasSaved( AUTH::ICredentialStore& store, const CORE::CString& storeId, const CORE::CString::StringVector& slots );

    /** The account key of a slot: "<store id>/<slot>" */
    static CORE::CString AccountKeyOf( const CORE::CString& storeId, const CORE::CString& slot );

    private:

    typedef GUCEF::map< CORE::CString, AUTH::CSecretString > TSlotMap;

    TSlotMap m_slots;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSIGNINCREDENTIALS_H ? */

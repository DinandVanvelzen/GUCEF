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

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#define GUCEF_AUTH_ICREDENTIALSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

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

/**
 *  Where refresh tokens and other secrets that have to survive a restart are kept. A secret has a slot per provider
 *  name and account key (the text of the account reference), so several accounts of one provider can be kept apart.
 *  The v1 implementation is an encrypted file (CFileCredentialStore); stores of the operating system (Windows
 *  Credential Manager, macOS Keychain, libsecret, Android Keystore) are implementations of the same interface.
 *  An implementation can be used from several threads.
 */
class GUCEF_AUTH_EXPORT_CPP ICredentialStore
{
    public:

    ICredentialStore( void );

    virtual ~ICredentialStore();

    /** Keeps the secret in the slot, replacing what was there. False when it could not be kept */
    virtual bool Store( const CORE::CString& providerName ,
                        const CORE::CString& accountKey   ,
                        const CSecretString& secret       ) = 0;

    /** Gives the secret of the slot; false when the slot is not there or cannot be read */
    virtual bool Load( const CORE::CString& providerName ,
                       const CORE::CString& accountKey   ,
                       CSecretString& secret             ) = 0;

    /** Removes the slot; true also when it was not there */
    virtual bool Remove( const CORE::CString& providerName ,
                         const CORE::CString& accountKey   ) = 0;

    /** The account keys that have a slot at the provider, the one stored last first */
    virtual bool ListAccounts( const CORE::CString& providerName     ,
                               CORE::CString::StringVector& accountKeys ) = 0;

    /** Removes every slot of the provider */
    virtual bool RemoveProvider( const CORE::CString& providerName ) = 0;

    /** Removes everything, key material included */
    virtual bool Wipe( void ) = 0;

    private:

    ICredentialStore( const ICredentialStore& src );
    ICredentialStore& operator=( const ICredentialStore& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

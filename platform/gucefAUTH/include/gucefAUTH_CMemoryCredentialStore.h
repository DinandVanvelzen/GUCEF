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

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

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

/** Keeps the secrets in memory only: for tests, for applications that do not want tokens on disk and for platforms without an encrypted store */
class GUCEF_AUTH_EXPORT_CPP CMemoryCredentialStore : public ICredentialStore
{
    public:

    CMemoryCredentialStore( void );

    virtual ~CMemoryCredentialStore();

    virtual bool Store( const CORE::CString& providerName ,
                        const CORE::CString& accountKey   ,
                        const CSecretString& secret       ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Load( const CORE::CString& providerName ,
                       const CORE::CString& accountKey   ,
                       CSecretString& secret             ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Remove( const CORE::CString& providerName ,
                         const CORE::CString& accountKey   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ListAccounts( const CORE::CString& providerName          ,
                               CORE::CString::StringVector& accountKeys   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveProvider( const CORE::CString& providerName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Wipe( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SSlot
    {
        CORE::CString providerName;
        CORE::CString accountKey;
        CSecretString secret;
    };

    typedef GUCEF::vector< SSlot > TSlotVector;

    private:

    mutable MT::CMutex m_lock;
    TSlotVector m_slots;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

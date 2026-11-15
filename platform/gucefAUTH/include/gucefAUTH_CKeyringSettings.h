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

#ifndef GUCEF_AUTH_CKEYRINGSETTINGS_H
#define GUCEF_AUTH_CKEYRINGSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

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
 *  The settings of a CKeyringSession: when the keyring locks by itself and how long a person waits after wrong codes.
 *  They belong to the application (its section "Keyring", for example); the Argon2id parameters of the seal are
 *  settings of the store (CFileCredentialStore: keyringMemoryInKiB, keyringPasses, keyringLanes).
 *
 *  Wrong codes: the first freeAttempts wrong codes in a row have no wait; after that every further one waits
 *  retryDelayInMs, doubled per wrong code, at most maxRetryDelayInMs. A right code resets the count. There is no
 *  lockout for good: the sealed file can be tried offline at the cost of Argon2id anyway, so a lockout would only keep
 *  the owner out; the wait slows down someone at the keyboard and a script driving the form.
 */
class GUCEF_AUTH_EXPORT_CPP CKeyringSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 DefaultIdleLockInSeconds = 900;
    static const UInt32 DefaultFreeAttempts = 3;
    static const UInt32 DefaultRetryDelayInMs = 1000;
    static const UInt32 DefaultMaxRetryDelayInMs = 60000;
    static const UInt32 DefaultCheckIntervalInMs = 250;

    UInt32 idleLockInSeconds;       /**< the keyring locks when it was not used for this long; 0 never */
    UInt32 freeAttempts;            /**< wrong codes in a row without a wait */
    UInt32 retryDelayInMs;          /**< the wait after the first wrong code beyond freeAttempts; doubles with every further one */
    UInt32 maxRetryDelayInMs;       /**< the cap of that doubling */
    UInt32 checkIntervalInMs;       /**< how often the session looks at the clock and at an operation in flight */

    CKeyringSettings( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Values that are not there keep what they were; maxRetryDelayInMs is at least retryDelayInMs, checkIntervalInMs at least 1 */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CKEYRINGSETTINGS_H ? */

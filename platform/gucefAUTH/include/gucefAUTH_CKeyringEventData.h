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

#ifndef GUCEF_AUTH_CKEYRINGEVENTDATA_H
#define GUCEF_AUTH_CKEYRINGEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_AUTH_KEYRINGTYPES_H
#include "gucefAUTH_KeyringTypes.h"
#define GUCEF_AUTH_KEYRINGTYPES_H
#endif /* GUCEF_AUTH_KEYRINGTYPES_H ? */

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
 *  Event data of the events of CKeyringSession, also the data of the future of an asynchronous operation that ran;
 *  no code and nothing derived from one is ever part of it
 */
class GUCEF_AUTH_EXPORT_CPP CKeyringEventData : public CORE::CICloneable
{
    public:

    TKeyringResult result;              /**< the outcome of the operation the event reports */
    TKeyringLockReason lockReason;      /**< LockedEvent: why the keyring locked */
    UInt32 failedAttempts;              /**< wrong codes in a row so far */
    UInt32 retryWaitInMs;               /**< WrongCodeEvent: how long the next attempt has to wait, 0 for none */

    CKeyringEventData( void );
    CKeyringEventData( const CKeyringEventData& src );
    virtual ~CKeyringEventData();
    CKeyringEventData& operator=( const CKeyringEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CKEYRINGEVENTDATA_H ? */

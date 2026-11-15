/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H
#define ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H
#include "AssetStoreConverterHook_CPendingChange.h"
#define ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H
#endif /* ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The previews of writes that wait for their apply. A token is 22 characters of base64url over 128 random bits; it
 *  expires after its time to live, and a principal has at most a bounded number of open tokens (the oldest expires
 *  when a new one comes). Take() hands a change over once: for the principal of the preview, the same operation and
 *  the same arguments only; a token of another principal or for other arguments stays for its owner. Thread safe.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPendingChangeStore
{
    public:

    enum ETakeResult
    {
        TAKE_OK         ,
        TAKE_UNKNOWN    ,   /**< never made, used already, or expired */
        TAKE_MISMATCH       /**< another principal, operation or arguments: the token stays */
    };

    CPendingChangeStore( void );

    /** Keeps the change; sets its token and expiry. False when no random bytes could be had */
    bool Add( CPendingChange& change, UInt32 ttlMs, UInt32 maxPerPrincipal );

    ETakeResult Take( const CORE::CString& token, const CORE::CString& principal, const CORE::CString& operation,
                      const CORE::CString& argumentsDigest, CPendingChange& change );

    /** The changes that wait, after the expired ones were removed */
    UInt32 GetCount( void ) const;

    private:

    CPendingChangeStore( const CPendingChangeStore& src );              /**< not implemented */
    CPendingChangeStore& operator=( const CPendingChangeStore& src );   /**< not implemented */

    void ExpireLocked( UInt64 nowMs ) const;

    typedef GUCEF::vector< CPendingChange > TChangeVector;

    mutable MT::CMutex m_lock;
    mutable TChangeVector m_changes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H ? */

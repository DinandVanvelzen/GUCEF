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

#ifndef GUCEF_AUTH_IIDENTITYLINKSTORE_H
#define GUCEF_AUTH_IIDENTITYLINKSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CLINKRECORD_H
#include "gucefAUTH_CLinkRecord.h"
#define GUCEF_AUTH_CLINKRECORD_H
#endif /* GUCEF_AUTH_CLINKRECORD_H ? */

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

/** The outcome of Link() and Merge() */
enum ELinkResult
{
    LINKRESULT_OK = 0,
    LINKRESULT_CONFLICT,            /**< the account is linked to another profile; unlink it there or merge the profiles first */
    LINKRESULT_INVALID_RECORD,      /**< not well formed, of a version that is not known, or the signature does not verify */
    LINKRESULT_STORE_ERROR          /**< the store could not keep the change; nothing changed */
};

typedef enum ELinkResult TLinkResult;

/*-------------------------------------------------------------------------*/

/**
 *  Which accounts belong to which profile. Every account belongs to at most one profile; a profile has any number of
 *  accounts. The v1 implementation is a local file that serves as a cache (CLocalIdentityLinkStore); the brand wide
 *  service that later keeps the links across products is another implementation of this interface, callers do not
 *  change. An implementation can be used from several threads and only keeps records whose signature verifies.
 */
class GUCEF_AUTH_EXPORT_CPP IIdentityLinkStore
{
    public:

    IIdentityLinkStore( void );

    virtual ~IIdentityLinkStore();

    /**
     *  Keeps the signed record. A record for an account that is linked to the same profile replaces the old one (a
     *  new proof), one for an account linked to another profile is a conflict and changes nothing.
     */
    virtual TLinkResult Link( const CLinkRecord& record ) = 0;

    /** Removes the link of the account to the profile; true also when there was none. False when the store could not keep the change */
    virtual bool Unlink( const CAccountRef& account, const CORE::CString& profileKeyId ) = 0;

    /** The links of a profile, the one made first first */
    virtual bool GetLinks( const CORE::CString& profileKeyId, TLinkRecordVector& links ) = 0;

    /** The profile the account is linked to; false when it is not linked */
    virtual bool Resolve( const CAccountRef& account, CORE::CString& profileKeyId ) = 0;

    /**
     *  Moves all accounts of a profile to the profile of the signer: every record is made again, signed by the signer,
     *  with the old profile in mergedFrom. The caller makes sure the user is entitled to both profiles (the user
     *  logged in to the accounts); a remote store checks that itself. mergedCount receives the number of accounts moved.
     */
    virtual TLinkResult Merge( const CORE::CString& fromProfileKeyId ,
                               ILinkSigner& intoSigner               ,
                               UInt32* mergedCount = GUCEF_NULL      ) = 0;

    private:

    IIdentityLinkStore( const IIdentityLinkStore& src );
    IIdentityLinkStore& operator=( const IIdentityLinkStore& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_IIDENTITYLINKSTORE_H ? */

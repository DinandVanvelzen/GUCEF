/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CIDRNBANLIST_H
#define GUCEF_DRN_CIDRNBANLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a ban is keyed on */
enum TDRNBanKind
{
    DRN_BAN_NONE        = 0,
    DRN_BAN_USER_ID     = 1,    /**< the user id an authenticator established */
    DRN_BAN_PUBLIC_KEY  = 2,    /**< a public key, as lowercase hex */
    DRN_BAN_ADDRESS     = 3     /**< the remote address of a channel, the host without the port */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One ban
 */
class GUCEF_DRN_EXPORT_CPP CDRNBanEntry
{
    public:

    TDRNBanKind kind;
    CORE::CString value;
    UInt64 expiryUnixSeconds;   /**< the ban ends at this time, 0 = never */
    CORE::CString reason;

    CDRNBanEntry( void );

    bool IsExpired( UInt64 nowUnixSeconds ) const;

    static CORE::CString BanKindToString( TDRNBanKind kind );
    static TDRNBanKind StringToBanKind( const CORE::CString& kind );

    /** The value of a DRN_BAN_PUBLIC_KEY entry for the key bytes */
    static CORE::CString PublicKeyToString( const UInt8* key, UInt32 keySize );
};

typedef GUCEF::vector< CDRNBanEntry > TDRNBanEntryVector;

/*-------------------------------------------------------------------------*/

/**
 *  Who may not link. The node consults it when it accepts a channel (the address) and when a link authenticates
 *  (the user id). Called on the node's thread; an implementation that is changed from other threads locks.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNBanList
{
    public:

    CIDRNBanList( void );

    virtual ~CIDRNBanList();

    /**
     *  True if there is a ban of the kind for the value that has not expired
     *  @param entry receives the ban when given
     */
    virtual bool IsBanned( TDRNBanKind kind, const CORE::CString& value, CDRNBanEntry* entry = GUCEF_NULL ) const = 0;

    bool IsUserBanned( const CORE::CString& userId, CDRNBanEntry* entry = GUCEF_NULL ) const;

    bool IsAddressBanned( const CORE::CString& address, CDRNBanEntry* entry = GUCEF_NULL ) const;

    bool IsPublicKeyBanned( const UInt8* key, UInt32 keySize, CDRNBanEntry* entry = GUCEF_NULL ) const;

    private:

    CIDRNBanList( const CIDRNBanList& src );
    CIDRNBanList& operator=( const CIDRNBanList& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNBANLIST_H ? */

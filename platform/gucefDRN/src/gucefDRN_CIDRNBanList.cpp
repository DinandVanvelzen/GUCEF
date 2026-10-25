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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CIDRNBanList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNBanEntry::CDRNBanEntry( void )
    : kind( DRN_BAN_NONE )
    , value()
    , expiryUnixSeconds( 0 )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNBanEntry::IsExpired( UInt64 nowUnixSeconds ) const
{GUCEF_TRACE;

    return 0 != expiryUnixSeconds && expiryUnixSeconds <= nowUnixSeconds;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNBanEntry::BanKindToString( TDRNBanKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case DRN_BAN_USER_ID:       return "user";
        case DRN_BAN_PUBLIC_KEY:    return "key";
        case DRN_BAN_ADDRESS:       return "address";
        case DRN_BAN_ACCOUNT:       return "account";
        default:                    return "none";
    }
}

/*-------------------------------------------------------------------------*/

TDRNBanKind
CDRNBanEntry::StringToBanKind( const CORE::CString& kind )
{GUCEF_TRACE;

    CORE::CString lower = kind.Lowercase();
    if ( lower == "user" )
        return DRN_BAN_USER_ID;
    if ( lower == "key" )
        return DRN_BAN_PUBLIC_KEY;
    if ( lower == "address" )
        return DRN_BAN_ADDRESS;
    if ( lower == "account" )
        return DRN_BAN_ACCOUNT;
    return DRN_BAN_NONE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNBanEntry::PublicKeyToString( const UInt8* key, UInt32 keySize )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    CORE::CString result;
    if ( GUCEF_NULL == key || 0 == keySize )
        return result;
    char pair[ 3 ] = { 0, 0, 0 };
    for ( UInt32 i=0; i<keySize; ++i )
    {
        pair[ 0 ] = hexDigits[ key[ i ] >> 4 ];
        pair[ 1 ] = hexDigits[ key[ i ] & 0x0F ];
        result += pair;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CIDRNBanList::CIDRNBanList( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNBanList::~CIDRNBanList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDRNBanList::IsUserBanned( const CORE::CString& userId, CDRNBanEntry* entry ) const
{GUCEF_TRACE;

    return IsBanned( DRN_BAN_USER_ID, userId, entry );
}

/*-------------------------------------------------------------------------*/

bool
CIDRNBanList::IsAddressBanned( const CORE::CString& address, CDRNBanEntry* entry ) const
{GUCEF_TRACE;

    return IsBanned( DRN_BAN_ADDRESS, address, entry );
}

/*-------------------------------------------------------------------------*/

bool
CIDRNBanList::IsPublicKeyBanned( const UInt8* key, UInt32 keySize, CDRNBanEntry* entry ) const
{GUCEF_TRACE;

    return IsBanned( DRN_BAN_PUBLIC_KEY, CDRNBanEntry::PublicKeyToString( key, keySize ), entry );
}

/*-------------------------------------------------------------------------*/

bool
CIDRNBanList::IsAccountBanned( const CORE::CString& account, CDRNBanEntry* entry ) const
{GUCEF_TRACE;

    return IsBanned( DRN_BAN_ACCOUNT, account, entry );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

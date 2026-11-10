/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#include "matchprovidermail_CEMailIntroFingerprint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CEMailIntroFingerprint::Of( const CORE::CString& publicKeyHex )
{GUCEF_TRACE;

    if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( publicKeyHex, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) )
        return CORE::CString::Empty;
    UInt8 key[ MULTIPLAY::CPlayerIdentity::KeySize ];
    for ( UInt32 i=0; i<MULTIPLAY::CPlayerIdentity::KeySize; ++i )
    {
        UInt8 value = 0;
        for ( UInt32 n=0; n<2; ++n )
        {
            const char c = publicKeyHex[ i * 2 + n ];
            const UInt8 digit = (UInt8)( c >= '0' && c <= '9' ? c - '0' : ( c >= 'a' && c <= 'f' ? c - 'a' + 10 : c - 'A' + 10 ) );
            value = (UInt8)( ( value << 4 ) | digit );
        }
        key[ i ] = value;
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( key, sizeof( key ), digest );
    static const char digits[] = "0123456789ABCDEF";
    CORE::CString text;
    for ( UInt32 i=0; i<10; ++i )
    {
        if ( i > 0 && 0 == i % 2 )
            text += '-';
        text += digits[ digest[ i ] >> 4 ];
        text += digits[ digest[ i ] & 0x0F ];
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

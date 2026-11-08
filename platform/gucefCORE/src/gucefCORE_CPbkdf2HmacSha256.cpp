/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <string.h>

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "gucefCORE_CPbkdf2HmacSha256.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** memset that the optimizer may not drop for a buffer it sees no further use of */
static void
WipeBytes( void* data, UInt32 size )
{
    volatile UInt8* bytes = static_cast< volatile UInt8* >( data );
    for ( UInt32 i=0; i<size; ++i )
        bytes[ i ] = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPbkdf2HmacSha256::Derive( const void* password ,
                           UInt32 passwordSize  ,
                           const void* salt     ,
                           UInt32 saltSize      ,
                           UInt32 iterations    ,
                           UInt8* output        ,
                           UInt32 outputSize    )
{GUCEF_TRACE;

    if ( 0 == iterations || 0 == outputSize || GUCEF_NULL == output )
        return false;

    // the key schedule of the password is computed once, Reset() starts every MAC from it
    CHmacSha256 hmac( password, passwordSize );
    UInt8 u[ CSha256::DigestSize ];
    UInt8 t[ CSha256::DigestSize ];

    UInt32 written = 0;
    UInt32 blockIndex = 1;
    while ( written < outputSize )
    {
        // U1 = PRF( P, S || INT( i ) ), T_i = U1 xor U2 xor ... xor Uc
        const UInt8 blockIndexBytes[ 4 ] = { (UInt8) ( blockIndex >> 24 ), (UInt8) ( blockIndex >> 16 ), (UInt8) ( blockIndex >> 8 ), (UInt8) blockIndex };
        hmac.Reset();
        if ( saltSize > 0 )
            hmac.Update( salt, saltSize );
        hmac.Update( blockIndexBytes, 4 );
        hmac.Final( u );
        memcpy( t, u, sizeof( t ) );
        for ( UInt32 i=1; i<iterations; ++i )
        {
            hmac.Reset();
            hmac.Update( u, sizeof( u ) );
            hmac.Final( u );
            for ( UInt32 k=0; k<CSha256::DigestSize; ++k )
                t[ k ] ^= u[ k ];
        }

        const UInt32 remaining = outputSize - written;
        const UInt32 take = remaining < CSha256::DigestSize ? remaining : CSha256::DigestSize;
        memcpy( output + written, t, take );
        written += take;
        ++blockIndex;
    }

    WipeBytes( u, sizeof( u ) );
    WipeBytes( t, sizeof( t ) );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

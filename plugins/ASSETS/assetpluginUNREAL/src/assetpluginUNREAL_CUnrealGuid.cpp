/*
 *  assetpluginUNREAL: A 128 bit globally unique identifier as stored in Unreal packages
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

#include <stdio.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#include "assetpluginUNREAL_CUnrealGuid.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealGuid::CUnrealGuid( void )
    : a( 0 )
    , b( 0 )
    , c( 0 )
    , d( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealGuid::CUnrealGuid( UInt32 wordA, UInt32 wordB, UInt32 wordC, UInt32 wordD )
    : a( wordA )
    , b( wordB )
    , c( wordC )
    , d( wordD )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealGuid::IsZero( void ) const
{GUCEF_TRACE;

    return 0 == a && 0 == b && 0 == c && 0 == d;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealGuid::operator==( const CUnrealGuid& other ) const
{GUCEF_TRACE;

    return a == other.a && b == other.b && c == other.c && d == other.d;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealGuid::operator!=( const CUnrealGuid& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealGuid::operator<( const CUnrealGuid& other ) const
{GUCEF_TRACE;

    if ( a != other.a ) return a < other.a;
    if ( b != other.b ) return b < other.b;
    if ( c != other.c ) return c < other.c;
    return d < other.d;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealGuid::ToString( void ) const
{GUCEF_TRACE;

    char text[ 48 ];
    sprintf( text, "%08X-%04X-%04X-%04X-%04X%08X", (unsigned int) a, (unsigned int) ( b >> 16 ), (unsigned int) ( b & 0xFFFF ), (unsigned int) ( c >> 16 ), (unsigned int) ( c & 0xFFFF ), (unsigned int) d );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

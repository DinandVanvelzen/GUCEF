/*
 *  assetpluginUNREAL: One entry of the payload table of a package trailer
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
#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H
#include "assetpluginUNREAL_CUnrealPayloadTocEntry.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H ? */

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

CUnrealPayloadTocEntry::CUnrealPayloadTocEntry( void )
    : offsetInFile( 0 )
    , compressedSize( 0 )
    , rawSize( 0 )
    , flags( 0 )
    , accessMode( 0 )
{GUCEF_TRACE;

    memset( identifier, 0, sizeof( identifier ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPayloadTocEntry::HasIdentifier( const UInt8* hash ) const
{GUCEF_TRACE;

    return 0 == memcmp( identifier, hash, sizeof( identifier ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPayloadTocEntry::IdentifierToString( void ) const
{GUCEF_TRACE;

    char text[ 41 ];
    for ( UInt32 i=0; i<20; ++i )
        sprintf( text + i*2, "%02x", (unsigned int) identifier[ i ] );
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

/*
 *  assetpluginUNREAL: Where the bytes of a payload are
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

#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#include "assetpluginUNREAL_CUnrealPayloadReference.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */

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

CUnrealPayloadReference::CUnrealPayloadReference( void )
    : kind( KIND_NONE )
    , offset( 0 )
    , storedSize( 0 )
    , rawSize( 0 )
    , hasIdentifier( false )
    , flags( 0 )
    , reason()
{GUCEF_TRACE;

    memset( identifier, 0, sizeof( identifier ) );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPayloadReference::IsReadable( void ) const
{GUCEF_TRACE;

    return KIND_INLINE == kind || KIND_TRAILER == kind;
}

/*-------------------------------------------------------------------------*/

const char*
CUnrealPayloadReference::KindToString( EKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case KIND_NONE: return "none";
        case KIND_EMPTY: return "empty";
        case KIND_INLINE: return "inline";
        case KIND_TRAILER: return "trailer";
        default: return "unavailable";
    }
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

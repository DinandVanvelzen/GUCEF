/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWebSocketHandshake::ProtocolVersion = "13";

static const char* WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
GetSha1RawCodec( CORE::CCodecRegistry::TICodecPtr& codec )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

    CORE::CCodecRegistry::TCodecFamilyRegistryPtr codecFamilyRegistry;
    if ( codecRegistry.TryLookup( CORE::CoreCodecTypes::CryptographicHashCodec, codecFamilyRegistry, false ) && codecFamilyRegistry )
    {
        return codecFamilyRegistry->TryLookup( "sha1-raw", codec, false ) && !codec.IsNULL();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketHandshake::IsAvailable( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec;
    return GetSha1RawCodec( codec );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketHandshake::GenerateClientKey( CORE::CString& clientKey )
{GUCEF_TRACE;

    UInt8 randomBytes[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( randomBytes, sizeof( randomBytes ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWebSocketHandshake: Failed to obtain random data for the client key" );
        return false;
    }
    clientKey = CORE::Base64Encode( randomBytes, sizeof( randomBytes ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketHandshake::ComputeAcceptKey( const CORE::CString& clientKey ,
                                       CORE::CString& acceptKey       )
{GUCEF_TRACE;

    acceptKey.Clear();

    CORE::CCodecRegistry::TICodecPtr sha1Codec;
    if ( !GetSha1RawCodec( sha1Codec ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWebSocketHandshake: The sha1-raw codec is not available, is codecspluginSTBRUMMEHASH loaded?" );
        return false;
    }

    // RFC 6455 section 4.2.2: the digest of the key and GUID is the raw 20 bytes, not its hex representation
    CORE::CString combined = clientKey + WEBSOCKET_GUID;
    CORE::CDynamicBuffer inBuffer;
    inBuffer.CopyFrom( combined.Length(), combined.C_String() );
    CORE::CDynamicBufferAccess inBufferAccess( inBuffer );

    CORE::CDynamicBuffer outBuffer( true );
    CORE::CDynamicBufferAccess outBufferAccess( outBuffer );

    if ( !sha1Codec->Encode( inBufferAccess, outBufferAccess ) || 20 != outBuffer.GetDataSize() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWebSocketHandshake: The sha1-raw codec failed to produce a digest" );
        return false;
    }

    acceptKey = CORE::Base64Encode( outBuffer.GetBufferPtr(), outBuffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

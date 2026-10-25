/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#include <string.h>

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#include "TestSigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

CTestSigner::CTestSigner( const CORE::CString& keyIdPrefix )
    : AUTH::ILinkSigner()
    , m_publicKeyHex()
    , m_keyId()
    , m_signCount( 0 )
    , m_valid( false )
{GUCEF_TRACE;

    memset( m_seed, 0, sizeof( m_seed ) );
    if ( !CORE::CRandom::GetRandomBytes( m_seed, sizeof( m_seed ) ) )
        return;
    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, "ed25519" );
    if ( codec.IsNULL() )
        return;

    CORE::CValueList params;
    params.Set( "privateKey", CORE::CVariant( m_seed, sizeof( m_seed ) ) );
    params.Set( "output", CORE::CVariant( "publicKey" ) );
    CORE::CICodecInstancePtr instance;
    CORE::CDynamicBuffer publicKey;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() ||
         !instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, GUCEF_NULL, 0, GUCEF_NULL, publicKey ) || 32 != publicKey.GetDataSize() )
        return;
    m_publicKeyHex = AUTH::CAuthCrypto::ToHex( publicKey.GetConstBufferPtr(), publicKey.GetDataSize() );
    m_keyId = keyIdPrefix + "-" + m_publicKeyHex.SubstrToIndex( 16, true );
    m_valid = true;
}

/*-------------------------------------------------------------------------*/

CTestSigner::~CTestSigner()
{GUCEF_TRACE;

    AUTH::CAuthCrypto::Wipe( m_seed, sizeof( m_seed ) );
}

/*-------------------------------------------------------------------------*/

bool
CTestSigner::IsValid( void ) const
{GUCEF_TRACE;

    return m_valid;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestSigner::GetSignCount( void ) const
{GUCEF_TRACE;

    return m_signCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestSigner::GetKeyId( void ) const
{GUCEF_TRACE;

    return m_keyId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestSigner::GetPublicKeyHex( void ) const
{GUCEF_TRACE;

    return m_publicKeyHex;
}

/*-------------------------------------------------------------------------*/

bool
CTestSigner::Sign( const void* data, UInt32 dataSize, CORE::CDynamicBuffer& signature )
{GUCEF_TRACE;

    if ( !m_valid )
        return false;
    CORE::CCodecRegistry::TICodecPtr codec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::SignatureCodec, "ed25519" );
    if ( codec.IsNULL() )
        return false;
    CORE::CValueList params;
    params.Set( "privateKey", CORE::CVariant( m_seed, sizeof( m_seed ) ) );
    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
        return false;
    signature.SetDataSize( 0 );
    if ( !instance->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, data, dataSize, GUCEF_NULL, signature ) )
        return false;
    ++m_signCount;
    return 64 == signature.GetDataSize();
}

/*-------------------------------------------------------------------------*/

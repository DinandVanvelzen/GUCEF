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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefDRN_CDRNResourceHasher.h"

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

CDRNResourceHasher::CDRNResourceHasher( void )
    : m_instance()
    , m_fallback()
    , m_output()
    , m_useCodec( false )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtr codec = CDRNCrypto::GetCodec( CORE::CoreCodecTypes::CryptographicHashCodec, "sha256" );
    if ( !codec.IsNULL() )
    {
        CORE::CValueList params;
        params.Set( "output", CORE::CVariant( "raw" ) );
        if ( codec->CreateInstance( params, m_instance ) && !m_instance.IsNULL() )
            m_useCodec = true;
        else
            m_instance.Unlink();
    }
    m_output.SetBufferSize( 2 * DigestSize, false );
}

/*-------------------------------------------------------------------------*/

CDRNResourceHasher::~CDRNResourceHasher()
{GUCEF_TRACE;

    m_instance.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHasher::IsUsingCodec( void ) const
{GUCEF_TRACE;

    return m_useCodec;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::DropCodec( void )
{GUCEF_TRACE;

    // a hash codec that fails is not used again by this object; the message under way is lost with it, which the
    // caller sees as a digest that does not match (a chunk is fetched again, a final check fails)
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNResourceHasher: The sha256 codec failed, using the built in SHA-256 from now on" );
    m_useCodec = false;
    m_instance.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::Begin( void )
{GUCEF_TRACE;

    m_fallback.Reset();
    if ( m_useCodec )
    {
        m_output.SetDataSize( 0 );
        if ( !m_instance->Process( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_ENCODE, GUCEF_NULL, 0, GUCEF_NULL, m_output ) )
            DropCodec();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::Update( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return;
    if ( m_useCodec )
    {
        if ( m_instance->Process( GUCEF_CODEC_PROCESS_UPDATE | GUCEF_CODEC_PROCESS_ENCODE, data, size, GUCEF_NULL, m_output ) )
            return;
        DropCodec();
    }
    m_fallback.Update( data, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::Final( UInt8* digest )
{GUCEF_TRACE;

    if ( m_useCodec )
    {
        m_output.SetDataSize( 0 );
        if ( m_instance->Process( GUCEF_CODEC_PROCESS_FINAL | GUCEF_CODEC_PROCESS_ENCODE, GUCEF_NULL, 0, GUCEF_NULL, m_output ) &&
             DigestSize == m_output.GetDataSize() )
        {
            memcpy( digest, m_output.GetConstBufferPtr(), DigestSize );
            return;
        }
        DropCodec();
    }
    m_fallback.Final( digest );
    m_fallback.Reset();
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::Hash( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    Begin();
    Update( data, size );
    Final( digest );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::HashLeaf( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    static const UInt8 leafPrefix = 0x00;
    Begin();
    Update( &leafPrefix, 1 );
    Update( data, size );
    Final( digest );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHasher::HashNode( const UInt8* left, const UInt8* right, UInt8* digest )
{GUCEF_TRACE;

    UInt8 node[ 1 + 2 * DigestSize ];
    node[ 0 ] = 0x01;
    memcpy( node + 1, left, DigestSize );
    memcpy( node + 1 + DigestSize, right, DigestSize );
    Hash( node, sizeof( node ), digest );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

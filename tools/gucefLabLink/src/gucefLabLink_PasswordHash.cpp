/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CPBKDF2HMACSHA256_H
#include "gucefCORE_CPbkdf2HmacSha256.h"
#define GUCEF_CORE_CPBKDF2HMACSHA256_H
#endif /* GUCEF_CORE_CPBKDF2HMACSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefLabLink_PasswordHash.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CPasswordHash::Argon2id = "argon2id";
const char* CPasswordHash::Pbkdf2Sha256 = "pbkdf2-sha256";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Writes over memory in a way the compiler does not leave out */
static void
WipeMemory( void* memory, size_t size )
{GUCEF_TRACE;

    volatile UInt8* bytes = static_cast< volatile UInt8* >( memory );
    for ( size_t i=0; i<size; ++i )
        bytes[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

static CORE::CCodecRegistry::TICodecPtr
Argon2idCodec( void )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::KeyDerivationCodec, CPasswordHash::Argon2id );
}

/*-------------------------------------------------------------------------*/

/** The value of "name=" in a list such as "m=65536,t=3,p=1"; 0 when it is not there */
static UInt32
ParameterOf( const CORE::CString& list, const char* name )
{GUCEF_TRACE;

    CORE::CString::StringVector items = list.ParseElements( ',', false );
    const CORE::CString prefix = CORE::CString( name ) + "=";
    for ( size_t i=0; i<items.size(); ++i )
    {
        if ( items[ i ].StartsWith( prefix ) )
            return CORE::StringToUInt32( items[ i ].CutChars( prefix.Length(), true ), 0 );
    }
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CPasswordHash::IsArgon2idAvailable( void )
{GUCEF_TRACE;

    return !Argon2idCodec().IsNULL();
}

/*-------------------------------------------------------------------------*/

SPasswordParams
CPasswordHash::Effective( const SPasswordParams& wanted )
{GUCEF_TRACE;

    if ( wanted.algorithm == Argon2id && IsArgon2idAvailable() )
        return wanted;
    if ( wanted.algorithm == Pbkdf2Sha256 )
        return wanted;
    SPasswordParams fallback;
    fallback.algorithm = Pbkdf2Sha256;
    fallback.memoryInKiB = 0;
    fallback.iterations = Pbkdf2Iterations;
    fallback.lanes = 0;
    return fallback;
}

/*-------------------------------------------------------------------------*/

bool
CPasswordHash::Derive( const CORE::CString& password, const SPasswordParams& params, const void* salt, UInt32 saltSize, UInt8* hash )
{GUCEF_TRACE;

    const UInt32 passwordSize = password.Length();
    if ( params.algorithm == Pbkdf2Sha256 )
        return params.iterations > 0 && CORE::CPbkdf2HmacSha256::Derive( password.C_String(), passwordSize, salt, saltSize, params.iterations, hash, HashSize );
    if ( params.algorithm != Argon2id )
        return false;

    CORE::CCodecRegistry::TICodecPtr codec = Argon2idCodec();
    if ( codec.IsNULL() )
        return false;
    CORE::CValueList codecParams;
    codecParams.Set( "salt", CORE::CVariant( salt, saltSize ) );
    codecParams.Set( "memory", CORE::CVariant( params.memoryInKiB ) );
    codecParams.Set( "iterations", CORE::CVariant( params.iterations ) );
    codecParams.Set( "lanes", CORE::CVariant( params.lanes ) );
    codecParams.Set( "length", CORE::CVariant( HashSize ) );

    // the buffers are sized once, so that no copy of the password is left in memory a growing buffer gave back,
    // and wiped before they go
    CORE::CDynamicBuffer input;
    if ( passwordSize > 0 )
    {
        input.SetBufferSize( passwordSize );
        input.Append( password.C_String(), passwordSize );
    }
    CORE::CDynamicBuffer derived;
    derived.SetBufferSize( HashSize );
    bool ok = false;
    {
        CORE::CDynamicBufferAccess inputAccess( input );
        CORE::CDynamicBufferAccess outputAccess( derived );
        ok = codec->Encode( inputAccess, outputAccess, codecParams );
    }
    ok = ok && derived.GetDataSize() == HashSize;
    if ( ok )
        ::memcpy( hash, derived.GetConstBufferPtr(), HashSize );
    if ( input.GetBufferSize() > 0 )
        WipeMemory( input.GetBufferPtr(), input.GetBufferSize() );
    if ( derived.GetBufferSize() > 0 )
        WipeMemory( derived.GetBufferPtr(), derived.GetBufferSize() );
    return ok;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPasswordHash::Format( const SPasswordParams& params, const void* salt, const void* hash )
{GUCEF_TRACE;

    CORE::CString saltText;
    CORE::CString hashText;
    CORE::Base64UrlEncode( salt, SaltSize, saltText, false );
    CORE::Base64UrlEncode( hash, HashSize, hashText, false );
    if ( params.algorithm == Pbkdf2Sha256 )
        return CORE::CString( "$" ) + Pbkdf2Sha256 + "$i=" + CORE::ToString( params.iterations ) + "$" + saltText + "$" + hashText;
    return CORE::CString( "$" ) + Argon2id + "$m=" + CORE::ToString( params.memoryInKiB ) + ",t=" + CORE::ToString( params.iterations ) +
           ",p=" + CORE::ToString( params.lanes ) + "$" + saltText + "$" + hashText;
}

/*-------------------------------------------------------------------------*/

bool
CPasswordHash::MakeRecord( const CORE::CString& password, const SPasswordParams& params, CORE::CString& record )
{GUCEF_TRACE;

    record.Clear();
    UInt8 salt[ SaltSize ];
    UInt8 hash[ HashSize ];
    if ( !CORE::CRandom::GetRandomBytes( salt, SaltSize ) )
        return false;
    const bool ok = Derive( password, params, salt, SaltSize, hash );
    if ( ok )
        record = Format( params, salt, hash );
    WipeMemory( hash, sizeof( hash ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CPasswordHash::ParseRecord( const CORE::CString& record, SPasswordParams& params, CORE::CDynamicBuffer& salt, CORE::CDynamicBuffer& hash )
{GUCEF_TRACE;

    // "", algorithm, parameters, salt, hash
    CORE::CString::StringVector parts = record.ParseElements( '$', true );
    if ( parts.size() != 5 || !parts[ 0 ].IsNULLOrEmpty() )
        return false;
    params = SPasswordParams();
    params.algorithm = parts[ 1 ];
    if ( params.algorithm == Argon2id )
    {
        params.memoryInKiB = ParameterOf( parts[ 2 ], "m" );
        params.iterations = ParameterOf( parts[ 2 ], "t" );
        params.lanes = ParameterOf( parts[ 2 ], "p" );
        if ( 0 == params.memoryInKiB || 0 == params.iterations || 0 == params.lanes )
            return false;
    }
    else
    if ( params.algorithm == Pbkdf2Sha256 )
    {
        params.memoryInKiB = 0;
        params.lanes = 0;
        params.iterations = ParameterOf( parts[ 2 ], "i" );
        if ( 0 == params.iterations )
            return false;
    }
    else
        return false;
    return CORE::Base64UrlDecode( parts[ 3 ], salt ) && salt.GetDataSize() == SaltSize &&
           CORE::Base64UrlDecode( parts[ 4 ], hash ) && hash.GetDataSize() == HashSize;
}

/*-------------------------------------------------------------------------*/

bool
CPasswordHash::Verify( const CORE::CString& password, const CORE::CString& record, const SPasswordParams& current, bool& needsUpgrade )
{GUCEF_TRACE;

    needsUpgrade = false;
    SPasswordParams params;
    CORE::CDynamicBuffer salt;
    CORE::CDynamicBuffer expected;
    if ( !ParseRecord( record, params, salt, expected ) )
        return false;
    UInt8 hash[ HashSize ];
    if ( !Derive( password, params, salt.GetConstBufferPtr(), SaltSize, hash ) )
        return false;
    const bool equal = CORE::CSha256::ConstantTimeEquals( hash, expected.GetConstBufferPtr(), HashSize );
    WipeMemory( hash, sizeof( hash ) );
    const SPasswordParams wanted = Effective( current );
    needsUpgrade = equal && ( params.algorithm != wanted.algorithm || params.memoryInKiB != wanted.memoryInKiB ||
                              params.iterations != wanted.iterations || params.lanes != wanted.lanes );
    return equal;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPasswordHash::MakeDummyRecord( const SPasswordParams& params )
{GUCEF_TRACE;

    // a random hash: the work of verifying against it is that of a real record with the same parameters
    UInt8 salt[ SaltSize ];
    UInt8 hash[ HashSize ];
    if ( !CORE::CRandom::GetRandomBytes( salt, SaltSize ) || !CORE::CRandom::GetRandomBytes( hash, HashSize ) )
        return CORE::CString::Empty;
    return Format( Effective( params ), salt, hash );
}

/*-------------------------------------------------------------------------*/

void
CPasswordHash::Wipe( CORE::CString& text )
{GUCEF_TRACE;

    const UInt32 length = text.Length();
    if ( length > 0 )
        WipeMemory( text.C_String(), length );
    text.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

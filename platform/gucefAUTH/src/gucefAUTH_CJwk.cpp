/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHJSON_H
#include "gucefAUTH_CAuthJson.h"
#define GUCEF_AUTH_CAUTHJSON_H
#endif /* GUCEF_AUTH_CAUTHJSON_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CJwk.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CJwk::MinModulusBits;
const UInt32 CJwk::MaxModulusBits;
const UInt32 CJwks::MaxDocumentBytes;
const UInt32 CJwks::MaxKeys;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJwk::CJwk( void )
    : keyType()
    , keyId()
    , algorithm()
    , use()
    , modulus()
    , exponent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwk::CJwk( const CJwk& src )
    : keyType( src.keyType )
    , keyId( src.keyId )
    , algorithm( src.algorithm )
    , use( src.use )
    , modulus( src.modulus )
    , exponent( src.exponent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwk::~CJwk()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwk&
CJwk::operator=( const CJwk& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        keyType = src.keyType;
        keyId = src.keyId;
        algorithm = src.algorithm;
        use = src.use;
        modulus = src.modulus;
        exponent = src.exponent;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

/** Decodes a base64url big endian integer and drops leading zero bytes */
static bool
DecodeInteger( const CORE::CString& text, CORE::CDynamicBuffer& number )
{GUCEF_TRACE;

    number.Clear( true );
    CORE::CDynamicBuffer decoded;
    if ( text.IsNULLOrEmpty() || !CORE::Base64UrlDecode( text, decoded ) || 0 == decoded.GetDataSize() )
        return false;
    const UInt8* bytes = static_cast< const UInt8* >( decoded.GetConstBufferPtr() );
    UInt32 skip = 0;
    while ( skip < decoded.GetDataSize() && 0 == bytes[ skip ] )
        ++skip;
    if ( skip >= decoded.GetDataSize() )
        return false;
    number.CopyFrom( decoded.GetDataSize() - skip, bytes + skip );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJwk::Load( const CORE::CDataNode& jwk )
{GUCEF_TRACE;

    *this = CJwk();

    CORE::CString type;
    if ( !CAuthJson::GetString( jwk, "kty", type ) || type != "RSA" )
        return false;
    CAuthJson::GetString( jwk, "kid", keyId );
    CAuthJson::GetString( jwk, "alg", algorithm );
    CAuthJson::GetString( jwk, "use", use );
    if ( !use.IsNULLOrEmpty() && use != "sig" )
        return false;
    if ( !algorithm.IsNULLOrEmpty() && algorithm != "RS256" && algorithm != "RS384" && algorithm != "RS512" )
        return false;

    CORE::CString n, e;
    if ( !CAuthJson::GetString( jwk, "n", n ) || !CAuthJson::GetString( jwk, "e", e ) )
        return false;
    if ( !DecodeInteger( n, modulus ) || !DecodeInteger( e, exponent ) )
        return false;
    keyType = type;
    if ( !IsUsable() )
    {
        *this = CJwk();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CJwk::GetModulusBits( void ) const
{GUCEF_TRACE;

    if ( 0 == modulus.GetDataSize() )
        return 0;
    UInt8 top = static_cast< const UInt8* >( modulus.GetConstBufferPtr() )[ 0 ];
    UInt32 bits = ( modulus.GetDataSize() - 1 ) * 8;
    while ( top > 0 )
    {
        ++bits;
        top = (UInt8) ( top >> 1 );
    }
    return bits;
}

/*-------------------------------------------------------------------------*/

bool
CJwk::IsUsable( void ) const
{GUCEF_TRACE;

    if ( keyType != "RSA" || 0 == exponent.GetDataSize() || exponent.GetDataSize() > 8 )
        return false;
    UInt32 bits = GetModulusBits();
    return bits >= MinModulusBits && bits <= MaxModulusBits;
}

/*-------------------------------------------------------------------------*/

bool
CJwk::CanVerify( const CORE::CString& tokenAlgorithm ) const
{GUCEF_TRACE;

    if ( !IsUsable() )
        return false;
    if ( tokenAlgorithm != "RS256" && tokenAlgorithm != "RS384" && tokenAlgorithm != "RS512" )
        return false;
    if ( !use.IsNULLOrEmpty() && use != "sig" )
        return false;
    return algorithm.IsNULLOrEmpty() || algorithm == tokenAlgorithm;
}

/*-------------------------------------------------------------------------*/

CJwks::CJwks( void )
    : m_keys()
    , m_skipped( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwks::CJwks( const CJwks& src )
    : m_keys( src.m_keys )
    , m_skipped( src.m_skipped )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwks::~CJwks()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwks&
CJwks::operator=( const CJwks& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_keys = src.m_keys;
        m_skipped = src.m_skipped;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CJwks::Parse( const void* data, UInt32 size )
{GUCEF_TRACE;

    Clear();

    CORE::CDataNode root;
    if ( !CAuthJson::Parse( data, size, MaxDocumentBytes, 6, true, root ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Jwks: The key set is not acceptable JSON" );
        return false;
    }

    const CORE::CDataNode& document = CAuthJson::GetDocument( root );
    const CORE::CDataNode* keys = document.FindChild( "keys" );
    if ( GUCEF_NULL == keys || GUCEF_DATATYPE_ARRAY != keys->GetNodeType() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Jwks: The document has no list of keys" );
        return false;
    }

    UInt32 total = 0;
    CORE::CDataNode::const_iterator i = keys->ConstBegin();
    while ( i != keys->ConstEnd() )
    {
        if ( ++total > MaxKeys )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Jwks: The key set has more than " << MaxKeys << " keys" );
            Clear();
            return false;
        }

        CJwk key;
        if ( key.Load( *(*i) ) )
        {
            if ( key.keyId.IsNULLOrEmpty() || m_keys.find( key.keyId ) != m_keys.end() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Jwks: A key has no key id or a key id occurs twice" );
                Clear();
                return false;
            }
            m_keys[ key.keyId ] = key;
        }
        else
            ++m_skipped;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJwks::Parse( const CORE::CString& text )
{GUCEF_TRACE;

    return Parse( text.C_String(), GetByteLength( text ) );
}

/*-------------------------------------------------------------------------*/

const CJwk*
CJwks::Find( const CORE::CString& keyId ) const
{GUCEF_TRACE;

    TKeyMap::const_iterator i = m_keys.find( keyId );
    return i == m_keys.end() ? GUCEF_NULL : &(*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CJwks::Add( const CJwk& key )
{GUCEF_TRACE;

    if ( !key.IsUsable() || key.keyId.IsNULLOrEmpty() || m_keys.size() >= MaxKeys )
        return false;
    m_keys[ key.keyId ] = key;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CJwks::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_keys.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CJwks::GetSkippedCount( void ) const
{GUCEF_TRACE;

    return m_skipped;
}

/*-------------------------------------------------------------------------*/

void
CJwks::Clear( void )
{GUCEF_TRACE;

    m_keys.clear();
    m_skipped = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

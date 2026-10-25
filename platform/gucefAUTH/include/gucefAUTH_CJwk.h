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

#ifndef GUCEF_AUTH_CJWK_H
#define GUCEF_AUTH_CJWK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A public key of a JSON Web Key set (RFC 7517) that can verify signatures: an RSA key of 2048 to 8192 bits. Keys of
 *  other types (EC, symmetric) are not usable here and are never loaded, so a symmetric "key" in a set cannot be
 *  pressed into service for an HMAC.
 */
class GUCEF_AUTH_EXPORT_CPP CJwk
{
    public:

    static const UInt32 MinModulusBits = 2048;
    static const UInt32 MaxModulusBits = 8192;

    CORE::CString keyType;                  /**< "RSA" */
    CORE::CString keyId;                    /**< kid */
    CORE::CString algorithm;                /**< alg, empty when the key does not pin one */
    CORE::CString use;                      /**< "sig" or empty */
    CORE::CDynamicBuffer modulus;           /**< n, big endian, no leading zero byte */
    CORE::CDynamicBuffer exponent;          /**< e, big endian, no leading zero byte */

    CJwk( void );
    CJwk( const CJwk& src );
    ~CJwk();
    CJwk& operator=( const CJwk& src );

    /** Reads the key from the node of a JWK object. False when it is not a usable RSA signature key */
    bool Load( const CORE::CDataNode& jwk );

    /** An RSA key of an acceptable size with an exponent */
    bool IsUsable( void ) const;

    /** Whether the key may verify a token of the algorithm (RS256, RS384, RS512): the key type is RSA, it is for signatures, and a pinned algorithm is the same */
    bool CanVerify( const CORE::CString& tokenAlgorithm ) const;

    /** The size of the modulus in bits */
    UInt32 GetModulusBits( void ) const;
};

/*-------------------------------------------------------------------------*/

/** A JSON Web Key set: the keys of a provider by key id */
class GUCEF_AUTH_EXPORT_CPP CJwks
{
    public:

    static const UInt32 MaxDocumentBytes = 262144;
    static const UInt32 MaxKeys = 64;

    CJwks( void );
    CJwks( const CJwks& src );
    ~CJwks();
    CJwks& operator=( const CJwks& src );

    /**
     *  Reads the set from the text of the document ({"keys":[...]}). The keys that are not usable (other key types) are
     *  skipped and counted. False when the text is not a key set, a key id occurs twice, a usable key has no key id or
     *  there are more than MaxKeys keys; the set is empty then.
     */
    bool Parse( const void* data, UInt32 size );

    bool Parse( const CORE::CString& text );

    /** The key with the key id; GUCEF_NULL when there is none */
    const CJwk* Find( const CORE::CString& keyId ) const;

    bool Add( const CJwk& key );

    UInt32 GetCount( void ) const;

    /** How many keys of the document were not usable */
    UInt32 GetSkippedCount( void ) const;

    void Clear( void );

    private:

    typedef GUCEF::map< CORE::CString, CJwk > TKeyMap;

    TKeyMap m_keys;
    UInt32 m_skipped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CJWK_H ? */

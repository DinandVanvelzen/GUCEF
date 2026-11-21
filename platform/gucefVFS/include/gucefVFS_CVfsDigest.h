/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSDIGEST_H
#define GUCEF_VFS_CVFSDIGEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A digest of the content of a file together with the name of the algorithm that made it.
 *
 *  CArchive::GetFileHash() gives a string without saying what it is a hash of (an MD5 in one form, an ETag, a SHA-256 in
 *  upper case, nothing); a digest that is compared with another or checked against content has to name its algorithm.
 *  The value of a digest of a known algorithm is lower case hexadecimal of the full length of that algorithm, which makes
 *  two digests of the same content equal as strings. The constructors normalise to that form; a value that does not have it
 *  (a wrong length, characters that are no hexadecimal digits) is kept as given and IsWellFormed() says false.
 *
 *  A digest with ALGORITHM_UNKNOWN carries whatever string the source gave, unchanged, and never matches anything.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsDigest
{
    public:

    enum EAlgorithm
    {
        ALGORITHM_UNKNOWN = 0,
        ALGORITHM_MD5,
        ALGORITHM_SHA1,
        ALGORITHM_SHA256
    };

    EAlgorithm algorithm;
    CORE::CString value;                        /**< lower case hexadecimal for a known algorithm, empty = no digest */

    /** No digest: unknown algorithm, empty value */
    CVfsDigest( void );

    /** Takes the value as given: a known algorithm gets the value in the normal form (lower case, surrounding white space removed) */
    CVfsDigest( EAlgorithm digestAlgorithm, const CORE::CString& digestValue );

    CVfsDigest( const CVfsDigest& src );

    CVfsDigest& operator=( const CVfsDigest& src );

    /** Makes the digest of the raw bytes of a hash (16, 20 or 32 bytes by algorithm): lower case hexadecimal */
    static CVfsDigest FromBytes( EAlgorithm digestAlgorithm, const void* digestBytes, UInt32 digestByteCount );

    /** Whether there is no digest (no value) */
    bool IsEmpty( void ) const;

    /** Whether the algorithm is known, the value has the length of that algorithm and only has hexadecimal digits in lower case */
    bool IsWellFormed( void ) const;

    /** Whether both digests are well formed, name the same known algorithm and have the same value */
    bool Matches( const CVfsDigest& other ) const;

    /** The raw bytes of a well formed digest; false (and 0 bytes) for another; 'bytes' has room for 32 bytes */
    bool ToBytes( UInt8* bytes, UInt32& byteCount ) const;

    /** The value in the normal form: lower case, surrounding white space removed */
    static CORE::CString NormalizeValue( const CORE::CString& digestValue );

    /** The number of hexadecimal characters of a digest of the algorithm: 32, 40, 64 and 0 for an unknown algorithm */
    static UInt32 GetHexLength( EAlgorithm digestAlgorithm );

    /** The number of bytes of a digest of the algorithm: 16, 20, 32 and 0 for an unknown algorithm */
    static UInt32 GetByteLength( EAlgorithm digestAlgorithm );

    /** "md5", "sha1", "sha256" and "unknown" */
    static const char* GetAlgorithmName( EAlgorithm digestAlgorithm );

    /** The algorithm of a name as GetAlgorithmName() gives it (case does not matter, "sha-256" and "sha-1" are taken too); ALGORITHM_UNKNOWN for another name */
    static EAlgorithm GetAlgorithmFromName( const CORE::CString& name );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#define GUCEF_ASSETSTORE_CASSETDIGEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSHA1_H
#include "gucefCORE_CSha1.h"
#define GUCEF_CORE_CSHA1_H
#endif /* GUCEF_CORE_CSHA1_H ? */
#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A digest of the content of a file, SHA-1 or SHA-256, computed in one pass or in pieces. Stores give either (section
 *  4.4); the primary store identifies files by SHA-256 (section 8, step 2). Algorithms are named "sha1" and "sha256",
 *  digests are lower case hexadecimal.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetDigest
{
    public:

    static const CORE::CString Sha1;        /**< "sha1" */
    static const CORE::CString Sha256;      /**< "sha256" */

    /** A digest of the algorithm; an unknown algorithm gives SHA-256 */
    explicit CAssetDigest( const CORE::CString& algorithm = Sha256 );

    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** The digest of everything given since the last Reset(), after which the object is reset */
    CORE::CString FinalHex( void );

    const CORE::CString& GetAlgorithm( void ) const;

    static bool IsSupportedAlgorithm( const CORE::CString& algorithm );

    /** The digest of the file at the VFS path; false when it cannot be read */
    static bool ComputeVfsFileDigest( const CORE::CString& vfsPath, const CORE::CString& algorithm, CORE::CString& digestHex, UInt64* fileSize = GUCEF_NULL );

    /** Whether two hexadecimal digests are the same, ignoring case */
    static bool AreEqual( const CORE::CString& digestA, const CORE::CString& digestB );

    private:

    CORE::CString m_algorithm;
    CORE::CSha1 m_sha1;
    CORE::CSha256 m_sha256;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */

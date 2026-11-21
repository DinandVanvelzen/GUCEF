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

#ifndef GUCEF_VFS_CVFSDIGESTHASHER_H
#define GUCEF_VFS_CVFSDIGESTHASHER_H

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

#ifndef md5_INCLUDED
#include "md5.h"
#endif /* md5_INCLUDED ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

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
 *  Computes a CVfsDigest of data that is given in pieces: the one place that maps CVfsDigest::EAlgorithm onto the hash
 *  classes of gucefCORE (md5.h, CSha1, CSha256), so that the block cache and the archives do not each have a switch of their own.
 *
 *  Update() as often as needed, then Final(); Reset() starts a new message. An unknown algorithm hashes nothing and
 *  gives an empty digest.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsDigestHasher
{
    public:

    explicit CVfsDigestHasher( CVfsDigest::EAlgorithm algorithm );

    /** Whether the algorithm is one that can be computed */
    bool IsSupported( void ) const;

    CVfsDigest::EAlgorithm GetAlgorithm( void ) const;

    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** The digest of what was given: after this Reset() has to be called before the hasher is used again */
    CVfsDigest Final( void );

    /** One shot digest of a buffer */
    static CVfsDigest Compute( CVfsDigest::EAlgorithm algorithm, const void* data, UInt32 size );

    private:

    CVfsDigestHasher( const CVfsDigestHasher& src );
    CVfsDigestHasher& operator=( const CVfsDigestHasher& src );

    CVfsDigest::EAlgorithm m_algorithm;
    md5_state_t m_md5;
    CORE::CSha1 m_sha1;
    CORE::CSha256 m_sha256;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSDIGESTHASHER_H ? */

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

#ifndef GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#define GUCEF_DRN_CDRNRESOURCEHASHTREE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHER_H
#include "gucefDRN_CDRNResourceHasher.h"
#define GUCEF_DRN_CDRNRESOURCEHASHER_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The hash tree of a resource: a binary SHA-256 tree over its chunks of DRN_RESOURCE_CHUNK_SIZE, shaped as the
 *  Merkle tree of RFC 9162 section 2.1 (leaf SHA-256( 0x00 || chunk ), node SHA-256( 0x01 || left || right ), split
 *  at the largest power of two below the leaf count, an empty resource hashing to SHA-256 of nothing). Kept as levels,
 *  an unpaired last node carried up unchanged, which gives the same root. An audit path proves one chunk against the
 *  root; the digest binds the root to the resource size:
 *      digest = SHA-256( "GUCEF.DRN.RES.TREE.1" || u64 big endian size || root )
 *  Memory: about 64 bytes per chunk (1 MiB per GiB of resource).
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceHashTree
{
    public:

    static const UInt32 HashSize = CDRNResourceHasher::DigestSize;

    /** The longest audit path: a tree of 2^64 leaves */
    static const UInt32 MaxAuditPathLength = 64;

    CDRNResourceHashTree( void );
    CDRNResourceHashTree( const CDRNResourceHashTree& src );
    ~CDRNResourceHashTree();
    CDRNResourceHashTree& operator=( const CDRNResourceHashTree& src );

    void Clear( void );

    /** Starts a tree for a resource of this size; false when the leaf level does not fit in memory */
    bool Reset( UInt64 resourceSize );

    /** The hash of a leaf (CDRNResourceHasher::HashLeaf of the chunk) */
    bool SetLeafHash( UInt64 leafIndex, const UInt8* leafHash );

    /** Computes the levels above the leaves, after every leaf hash was set */
    bool Finish( CDRNResourceHasher& hasher );

    bool IsComplete( void ) const;
    UInt64 GetResourceSize( void ) const;
    UInt64 GetLeafCount( void ) const;

    /** The leaf hash, of a tree with its leaves set */
    bool GetLeafHash( UInt64 leafIndex, UInt8* leafHash ) const;

    bool GetRoot( UInt8* root ) const;

    /** The digest of the resource: root and size (see the class description) */
    bool GetDigest( CDRNResourceHasher& hasher, UInt8* digest ) const;

    /**
     *  The audit path of a leaf, sibling hashes from the leaf up, into 'path' (room for MaxAuditPathLength hashes).
     *  Returns their number; 0 for a tree of one leaf, and when the tree is not complete or the index is outside it
     *  (check with IsComplete() and GetLeafCount()).
     */
    UInt32 GetAuditPath( UInt64 leafIndex, UInt8* path ) const;

    /** Bytes the tree holds */
    UInt64 GetMemoryBytes( void ) const;

    /** The digest of a resource of this size with this root */
    static void ComputeDigest( CDRNResourceHasher& hasher, UInt64 resourceSize, const UInt8* root, UInt8* digest );

    /** Checks a leaf hash against a root with its audit path (RFC 9162 section 2.1.3.2) */
    static bool VerifyAuditPath( CDRNResourceHasher& hasher, UInt64 leafIndex, UInt64 leafCount, const UInt8* leafHash,
                                 const UInt8* path, UInt32 pathLength, const UInt8* root );

    /** The root of the tree of an empty resource */
    static void GetEmptyRoot( CDRNResourceHasher& hasher, UInt8* root );

    private:

    typedef GUCEF::vector< CORE::CDynamicBuffer > TLevelVector;

    TLevelVector m_levels;      /**< level 0 the leaves, the last one the root */
    UInt64 m_resourceSize;
    UInt64 m_leafCount;
    bool m_complete;
    UInt8 m_emptyRoot[ HashSize ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEHASHTREE_H ? */

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

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#define GUCEF_DRN_CDRNRESOURCEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

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
 *  A resource as its provider resolved it: what a requester learns when it opens one (INFO), and what a provider
 *  checks a read against (the revision). Sizes and offsets are 64 bit throughout.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceInfo
{
    public:

    static const UInt32 DigestSize = 32;

    CORE::CString name;                 /**< the resource name within its provider */
    UInt64 size;                        /**< bytes */
    UInt64 modifiedUtcMs;               /**< last modification, milliseconds since the Unix epoch in UTC, 0 when unknown */
    CORE::CString revision;             /**< changes whenever the content may have changed; by default "<size>-<modifiedUtcMs>" */
    bool hasSha256;
    UInt8 sha256[ DigestSize ];         /**< SHA-256 of the whole resource, when the provider knows it */
    bool hasTreeRoot;
    UInt8 treeRoot[ DigestSize ];       /**< root of the hash tree over its chunks (CDRNResourceHashTree), when there is one */

    CDRNResourceInfo( void );
    CDRNResourceInfo( const CDRNResourceInfo& src );
    CDRNResourceInfo& operator=( const CDRNResourceInfo& src );

    void Clear( void );

    /** Chunks of DRN_RESOURCE_CHUNK_SIZE that make up the resource, the last one possibly shorter */
    UInt64 GetChunkCount( void ) const;

    /** Bytes in a chunk of the resource, 0 beyond its end */
    UInt32 GetChunkBytes( UInt64 chunkIndex ) const;

    /** Same size and revision, and the same tree root when both have one */
    bool IsSameRevisionAs( const CDRNResourceInfo& other ) const;

    /** The default revision tag of a provider that knows only size and modification time */
    static CORE::CString MakeRevision( UInt64 size, UInt64 modifiedUtcMs );

    static UInt64 ChunkCountFor( UInt64 size );
};

typedef CORE::CTCloneableObj< CDRNResourceInfo > CDRNResourceInfoEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

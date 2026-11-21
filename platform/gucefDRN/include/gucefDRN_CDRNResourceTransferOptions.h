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

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H
#define GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

/** How one resource transfer is done and checked */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTransferOptions
{
    public:

    static const UInt32 DigestSize = 32;

    enum TVerifyMode
    {
        VERIFY_LINK                 = 0,    /**< the packet protection of the link proves where the bytes come from; nothing is hashed per chunk */
        VERIFY_TREE                 = 1,    /**< every chunk is checked against the hash tree of the resource; NO_TREE when the serving side has none */
        VERIFY_TREE_IF_AVAILABLE    = 2     /**< as VERIFY_TREE when the serving side has a tree, else as VERIFY_LINK */
    };

    TVerifyMode verifyMode;             /**< default VERIFY_LINK */
    bool hasTreeDigest;
    UInt8 treeDigest[ DigestSize ];     /**< the digest the resource must have (CDRNResourceHashTree::GetDigest()); implies VERIFY_TREE */
    bool hasSha256;
    UInt8 sha256[ DigestSize ];         /**< the SHA-256 the whole resource must have, checked when it completed (whole resources only) */
    CORE::CString expectedRevision;     /**< the revision the resource must have (CHANGED otherwise), empty for any */
    UInt32 windowChunks;                /**< chunks asked for ahead of what was written, default 16 (1 MiB) */
    UInt32 openTimeoutMs;               /**< waiting for the serving side to open the resource, default 30000 */
    UInt32 idleTimeoutMs;               /**< no data for this long while data is due ends the transfer with TIMEOUT, default 30000 */
    UInt32 corruptChunkRetries;         /**< times a chunk that fails its check is asked for again, default 3 */
    bool resume;                        /**< a resumable target continues with what it holds of the same revision, default true */
    bool restartIfChanged;              /**< a resumable target that holds another revision starts over (else CHANGED), default true */

    CDRNResourceTransferOptions( void );
    CDRNResourceTransferOptions( const CDRNResourceTransferOptions& src );
    CDRNResourceTransferOptions& operator=( const CDRNResourceTransferOptions& src );

    /** Whether the hash tree is needed (VERIFY_TREE or a tree digest) */
    bool RequiresTree( void ) const;

    /** Whether the hash tree is asked for */
    bool WantsTree( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETRANSFEROPTIONS_H ? */

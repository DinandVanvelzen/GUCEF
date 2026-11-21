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

#ifndef GUCEF_DRN_CDRNRESOURCETREESTORE_H
#define GUCEF_DRN_CDRNRESOURCETREESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#include "gucefDRN_CDRNResourceInfo.h"
#define GUCEF_DRN_CDRNRESOURCEINFO_H
#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#include "gucefDRN_CDRNResourceHashTree.h"
#define GUCEF_DRN_CDRNRESOURCEHASHTREE_H
#endif /* GUCEF_DRN_CDRNRESOURCEHASHTREE_H ? */

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
 *  Hash trees kept on disk across runs, in the directory an application configured
 *  (CDRNResourceTransferSettings::treeCacheDirectory); nothing is ever written next to what is served unless that
 *  directory is put there.
 *
 *  One file per tree, "<first 32 hex digits of SHA-256( provider \n name \n revision )>.drntree": magic "GUDRNRT1",
 *  u8 format version 1, string provider, string name, varint64 size, varint64 modifiedUtcMs, string revision,
 *  varint64 leaf count, the leaf hashes, then the SHA-256 of all of it. Written atomically (CORE::WriteFileAtomically).
 *  A tree is taken only when the file is whole and provider, name, size, modification time and revision are those of
 *  the resource as resolved now; the levels above the leaves are computed again when it is loaded.
 *  Called on threads of the pool; files are replaced whole, so concurrent use of one directory is safe.
 */
class GUCEF_DRN_EXPORT_CPP CDRNResourceTreeStore
{
    public:

    static const UInt8 FormatVersion = 1;

    static CORE::CString GetTreeFilePath( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info );

    /** Loads the tree of the resource as resolved; false when there is none, or none that fits */
    static bool Load( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info, CDRNResourceHashTree& tree );

    static bool Save( const CORE::CString& directory, const CORE::CString& provider, const CDRNResourceInfo& info, const CDRNResourceHashTree& tree );

    /** Removes trees written longer ago than maxAgeSeconds (0: none), then the oldest until at most maxBytes remain */
    static void Prune( const CORE::CString& directory, UInt64 maxBytes, UInt32 maxAgeSeconds );

    private:

    CDRNResourceTreeStore( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCETREESTORE_H ? */

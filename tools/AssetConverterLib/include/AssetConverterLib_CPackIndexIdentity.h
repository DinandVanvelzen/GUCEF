/*
 *  AssetConverterLib: the identity of a pack in the pack index: its key, its fingerprint and where it came from
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H
#define ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  { packKey, fingerprint: { fileCount, totalBytes, treeHash }, name, title?, storePackId?, source?, root, contentHash? }.
 *  The packKey is the SHA-256 of the sorted lines "path, size" of the files of the pack as listed (the treeHash is the hash of
 *  the same lines): it identifies the pack as long as no content is read, so building the structure of a pack reads no file.
 *  The contentHash, when a person asked for the content to be verified, is the SHA-256 over the lines "path, size, sha256(file)".
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexIdentity
{
    public:

    CORE::CString packKey;
    UInt64 fileCount;
    UInt64 totalBytes;
    CORE::CString treeHash;
    CORE::CString name;
    CORE::CString title;
    CORE::CString storePackId;
    CORE::CString sourceStoreId;
    CORE::CString sourceItemId;
    CORE::CString sourceVersion;
    CORE::CString sourceUrl;
    CORE::CString root;
    CORE::CString contentHash;

    CPackIndexIdentity( void );

    bool operator==( const CPackIndexIdentity& other ) const;
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXIDENTITY_H ? */


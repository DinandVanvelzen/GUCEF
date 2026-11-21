/*
 *  AssetConverterLib: reads a pack index document: inline or sharded, one reader for both
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXREADER_H
#define ASSETCONVERTERLIB_CPACKINDEXREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

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
 *  The reader of the pack index. It reads the manifest first (small, always), and the shards only when asked, so a view of the
 *  first two levels reads the manifest and nothing else, and a group opened reads its shards (ReadShard()). Readers never lock:
 *  they rely on the atomic rename of the writer and on the hashes.
 *
 *  - A document of another format is refused (result NotAPackIndex).
 *  - A document of a higher version than this build reads is refused with a message that says so (result VersionTooNew): the
 *    reader never guesses at members whose meaning it does not know. Members this build does not know of a readable version
 *    are ignored.
 *  - The documentHash of the manifest and the sha256 of each shard are checked; a mismatch is result Corrupt.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexReader
{
    public:

    enum EResult
    {
        RESULT_OK = 0       ,
        RESULT_MISSING      ,   /**< there is no manifest */
        RESULT_CORRUPT      ,   /**< it cannot be parsed, a hash does not match or a shard is gone */
        RESULT_NOT_A_PACK_INDEX ,
        RESULT_VERSION_TOO_NEW
    };

    CPackIndexReader( void );

    /** Reads the manifest of the index in the folder; with a sharded index the resources are not read */
    EResult ReadManifest( const CORE::CString& indexFolder, CPackIndexDocument& document );

    /** Reads one shard of a manifest, checks its hash and appends its records */
    EResult ReadShard( const CORE::CString& indexFolder, const CPackIndexShardInfo& shard, CPackIndexResource::TResourceVector& resources );

    /** The manifest and all the records */
    EResult Read( const CORE::CString& indexFolder, CPackIndexDocument& document );

    /** Renames a corrupt manifest to pack-index.json.bad so that it is treated as missing and rebuilt */
    bool MarkBad( const CORE::CString& indexFolder );

    /** What the last call said in words: the reason of a result other than OK */
    const CORE::CString& GetMessage( void ) const;

    static CORE::CString ToString( EResult result );

    private:

    CORE::CString m_message;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXREADER_H ? */


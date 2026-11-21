/*
 *  AssetConverterLib: the single place that decides whether a stored pack index result may be used
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

#ifndef ASSETCONVERTERLIB_CPACKWORKGATE_H
#define ASSETCONVERTERLIB_CPACKWORKGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKGATEDECISION_H
#include "AssetConverterLib_CPackGateDecision.h"
#define ASSETCONVERTERLIB_CPACKGATEDECISION_H
#endif /* ASSETCONVERTERLIB_CPACKGATEDECISION_H ? */

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#include "AssetConverterLib_CPackRedoRequest.h"
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXREADER_H
#include "AssetConverterLib_CPackIndexReader.h"
#define ASSETCONVERTERLIB_CPACKINDEXREADER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXREADER_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#include "AssetConverterLib_CAssetCacheEntry.h"
#define ASSETCONVERTERLIB_CASSETCACHEENTRY_H
#endif /* ASSETCONVERTERLIB_CASSETCACHEENTRY_H ? */

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
 *  The work gate (design 4.3): every question "is the stored result of this step still valid" of the pack index goes through
 *  here, and no other code compares keys. A stored result is valid when the key recorded with it equals the key the step computes
 *  now from its inputs, the scheme of the conversion cache. For the conversion steps the gate wraps CAssetCacheEntry::CompareWith()
 *  and does not repeat the comparison; for recognise, index, analyse and thumbnail it makes the same comparison on the parts of
 *  its key and names the first part that differs with a TActivityReason.
 *
 *  A redo request turns a fresh answer into a forced one for the steps it names (CPackRedoRequest, design 5.1).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackWorkGate
{
    public:

    /** Recognise: the listing paths and the versions of the recognisers (the recognition key of the profile) */
    static CPackGateDecision DecideRecognise( const CPackIndexDocument* stored, const CPackIndexDocument& current, const CPackRedoRequest& request );

    /**
     *  Index (structure and grouping): the stored manifest, as the reader gave it, against the inputs of now (CPackIndexBuilder::PrepareInputs()).
     *  The fast check is the fingerprint (file count, bytes, tree hash); the content hash is compared only when the request verifies it.
     */
    static CPackGateDecision DecideIndex( CPackIndexReader::EResult storedResult, const CPackIndexDocument& stored, const CPackIndexDocument& current, const CPackRedoRequest& request );

    /** Analyse, per resource: the stored record against the key of now. A step that failed stays failed until the key changes or the request retries it */
    static CPackGateDecision DecideAnalyse( const CPackIndexResource* stored, const CORE::CString& currentKey, UInt64 currentSize, const CPackRedoRequest& request );

    /** Convert (import and export): the cache entry of the asset against the entry of now; the licence refusal wins */
    static CPackGateDecision DecideConvert( const CAssetCacheEntry* stored, const CAssetCacheEntry& current, bool licenceRefused, const CORE::CString& licenceReason, const CPackRedoRequest& request );

    /** Thumbnail: the wanted key (digest, renderer, version, size) against the thumbnails of the record */
    static CPackGateDecision DecideThumbnail( const CPackIndexResource* stored, const CPackIndexThumbnail& wanted, const CPackRedoRequest& request );

    /** The key of an analysis step: the analyser, its version, its settings and the source (the digest when known, else the size) */
    static CORE::CString MakeAnalyseKey( const CORE::CString& analyserId, UInt32 analyserVersion, const CORE::CString& settingsHash, const CORE::CString& digest, UInt64 size );

    private:

    CPackWorkGate( void );
    ~CPackWorkGate();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKWORKGATE_H ? */


/*
 *  AssetConverterLib: one resource record of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H
#define ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXSTATUS_H
#include "AssetConverterLib_CPackIndexStatus.h"
#define ASSETCONVERTERLIB_CPACKINDEXSTATUS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSTATUS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXREFERENCE_H
#include "AssetConverterLib_CPackIndexReference.h"
#define ASSETCONVERTERLIB_CPACKINDEXREFERENCE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXREFERENCE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H
#include "AssetConverterLib_CPackIndexOutputs.h"
#define ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXTHUMBNAIL_H
#include "AssetConverterLib_CPackIndexThumbnail.h"
#define ASSETCONVERTERLIB_CPACKINDEXTHUMBNAIL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXTHUMBNAIL_H ? */

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
 *  A resource of a pack: the record of schema version 1 (section 2.4 of the pack index design). "Absent means unknown, never
 *  zero": a member that is empty or has hasBounds false is not written. The statistics are a map of scalars whose members
 *  depend on the kind (a model: triangles, vertices, subMeshes, bones, lods, animations, materials; a texture or image:
 *  width, height, channels, bitDepth; an animation: lengthS, frameRate; audio: lengthS, channels, sampleRate), so a reader
 *  of an older or newer build keeps what it does not know.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexResource
{
    public:

    typedef GUCEF::vector< CPackIndexResource > TResourceVector;
    typedef GUCEF::vector< CPackIndexReference > TReferenceVector;
    typedef GUCEF::vector< CPackIndexThumbnail > TThumbnailVector;
    typedef GUCEF::map< CORE::CString, CORE::CVariant > TValueMap;

    CORE::CString id;                       /**< "as" + the first 20 hex of SHA-256( idBase + "\n" + path ) */
    CORE::CString path;                     /**< relative to the pack root, with / separators */
    CORE::CString name;                     /**< the file name as the pack spells it */
    CORE::CString title;                    /**< display name derived from the name */
    CORE::CString kind;                     /**< CAssetKindRegistry::GetKindId() */
    CORE::CString storeKind;                /**< the vocabulary kind of the store when the name says it (static-model, skeletal-model, texture, ...) */
    CORE::CString group;                    /**< function group id (CPackGroupVocabulary) */
    CORE::CString subGroup;                 /**< path with / for nesting */
    CORE::CString groupRule;                /**< diagnostic: which stage decided */
    CORE::CString format;                   /**< the extension in lower case */
    UInt64 size;
    CORE::CString digest;                   /**< SHA-256 of the file once hashed */
    CORE::CString::StringVector metadataFiles;
    TValueMap stats;
    bool hasBounds;
    Float32 boundsMin[ 3 ];
    Float32 boundsMax[ 3 ];
    TReferenceVector refs;
    CORE::CString::StringVector unresolvedRefs;
    CORE::CString presentedUnder;
    CPackIndexOutputs outputs;
    TThumbnailVector thumbnails;
    TValueMap presentation;                 /**< per resource defaults as presentation ids: camera, loop, materialPreview */
    CORE::CString::StringVector tags;
    CORE::CString::StringVector held;
    CORE::CString licenceRef;
    CPackIndexStatus status;

    CPackIndexResource( void );

    /** The id of a resource: the formula of the store, with the base the pack key (or the store pack id) */
    static CORE::CString MakeId( const CORE::CString& idBase, const CORE::CString& path );

    /** The function of the canonical order of resources: group in the fixed order, then sub-group, then path */
    bool operator<( const CPackIndexResource& other ) const;

    void SetStat( const CORE::CString& name, UInt64 value );
    bool GetStat( const CORE::CString& name, UInt64& value ) const;
    bool HasStats( void ) const;

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

#endif /* ASSETCONVERTERLIB_CPACKINDEXRESOURCE_H ? */


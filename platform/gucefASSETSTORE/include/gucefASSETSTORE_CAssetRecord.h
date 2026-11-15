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

#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#define GUCEF_ASSETSTORE_CASSETRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETMODELPROPS_H
#include "gucefASSETSTORE_CAssetModelProps.h"
#define GUCEF_ASSETSTORE_CASSETMODELPROPS_H
#endif /* GUCEF_ASSETSTORE_CASSETMODELPROPS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETTEXTUREPROPS_H
#include "gucefASSETSTORE_CAssetTextureProps.h"
#define GUCEF_ASSETSTORE_CASSETTEXTUREPROPS_H
#endif /* GUCEF_ASSETSTORE_CASSETTEXTUREPROPS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETMATERIALPROPS_H
#include "gucefASSETSTORE_CAssetMaterialProps.h"
#define GUCEF_ASSETSTORE_CASSETMATERIALPROPS_H
#endif /* GUCEF_ASSETSTORE_CASSETMATERIALPROPS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETANIMATIONPROPS_H
#include "gucefASSETSTORE_CAssetAnimationProps.h"
#define GUCEF_ASSETSTORE_CASSETANIMATIONPROPS_H
#endif /* GUCEF_ASSETSTORE_CASSETANIMATIONPROPS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETAUDIOPROPS_H
#include "gucefASSETSTORE_CAssetAudioProps.h"
#define GUCEF_ASSETSTORE_CASSETAUDIOPROPS_H
#endif /* GUCEF_ASSETSTORE_CASSETAUDIOPROPS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETFILERECORD_H
#include "gucefASSETSTORE_CAssetFileRecord.h"
#define GUCEF_ASSETSTORE_CASSETFILERECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETFILERECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORD_H
#include "gucefASSETSTORE_CAssetKeyword.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORD_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORD_H ? */
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

typedef GUCEF::vector< CAssetFileRecord > TAssetFileRecordVector;
typedef GUCEF::vector< CAssetKeyword > TAssetKeywordVector;

/**
 *  The normalised record of one usable item of a pack (section 7.1): a mesh, a texture, a material, an animation, a
 *  sound, a prefab or scene, a font, a document. Paths are relative to the root of the pack, which is relative to the
 *  storage location of the pack. The typed properties of its kind are filled by content analysis; 'analysed' tells
 *  that every analyser that could look at it did, so a resumed ingest does not analyse it again.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetRecord : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString assetId;
    CORE::CString packId;
    CORE::CString kind;                     /**< an id of the kinds of the vocabulary: "static-model", "texture", ... */
    CORE::CString name;                     /**< the file name without its extension */
    CORE::CString originalPath;             /**< relative to the root of the pack */
    CORE::CString convertedPath;            /**< relative to the root of the pack, empty when not converted */
    CORE::CString sourceFormat;             /**< a format id: "fbx", "gltf", "png", ... */
    CORE::CString convertedFormat;
    Int64 sizeBytes;
    CORE::CString digest;                   /**< SHA-256 of the original file */
    CORE::CString categoryId;               /**< empty when the asset has no category of its own */
    TAssetFileRecordVector files;
    TAssetKeywordVector keywords;           /**< the keywords of the asset itself: its name and what was derived */
    CAssetModelProps model;
    CAssetTextureProps texture;
    CAssetMaterialProps material;
    CAssetAnimationProps animation;
    CAssetAudioProps audio;
    bool analysed;
    CORE::CString::StringSet analyzers;     /**< "name/version" of the analysers that looked at it */

    CAssetRecord( void );
    virtual ~CAssetRecord() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetRecord > TAssetRecordVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */

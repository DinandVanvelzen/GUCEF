/*
 *  AssetConverterLib: a normalized relation of an asset to another file of its pack
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

#ifndef ASSETCONVERTERLIB_CASSETREFERENCE_H
#define ASSETCONVERTERLIB_CASSETREFERENCE_H

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
 *  One normalized relation of an asset (converter design section 5.8): what the importer resolved while it read the source.
 *  'target' is the file the relation leads to: a VFS path while the importer fills CAssetScene::references, a path in the
 *  pack in the sidecar member "relations". An unresolved relation has no target and keeps the reference as the source
 *  wrote it in 'via' (a GUID, a res:// path, an object path, a file name).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetReference
{
    public:

    static const CORE::CString KindUsesMaterial;      /**< "usesMaterial": a material file */
    static const CORE::CString KindUsesTexture;       /**< "usesTexture": an image of a material or texture slot */
    static const CORE::CString KindUsesMesh;          /**< "usesMesh": a model file whose geometry the asset uses */
    static const CORE::CString KindUsesSkeleton;      /**< "usesSkeleton" */
    static const CORE::CString KindUsesAnimation;     /**< "usesAnimation" */
    static const CORE::CString KindInstancesPrefab;   /**< "instancesPrefab": a nested prefab or instanced scene */
    static const CORE::CString KindInScene;           /**< "inScene": the reverse of instancesPrefab and usesMesh, derived, not followed */
    static const CORE::CString KindFitsSkeleton;      /**< "fitsSkeleton": information, not followed */
    static const CORE::CString KindDependsOn;         /**< "dependsOn": another source the import read */
    static const CORE::CString KindUsesFile;          /**< "usesFile": a payload file without a meaning of its own (a buffer, a .meta file) */

    /** The bounds of a closure over relations (converter design section 5.8.4) */
    static const UInt32 MaxClosureDepth;              /**< 8 relation steps */
    static const UInt32 MaxClosureFiles;              /**< 4096 files */

    CORE::CString kind;
    CORE::CString target;
    CORE::CString via;
    bool optional;

    CAssetReference( void );

    CAssetReference( const CORE::CString& relationKind, const CORE::CString& relationTarget, const CORE::CString& original, bool isOptional );

    bool IsResolved( void ) const;

    /** The canonical order: by kind, then target, then the original reference, then optional */
    bool operator<( const CAssetReference& other ) const;

    bool operator==( const CAssetReference& other ) const;

    /** { kind, path?, via?, optional? }: 'target' as "path", optional only when true */
    bool Serialize( CORE::CDataNode& node ) const;

    /** False when the node has no kind */
    bool Deserialize( const CORE::CDataNode& node );

    /** Whether the kind is one of the vocabulary */
    static bool IsKnownKind( const CORE::CString& kind );

    /** Whether a closure for companion files follows relations of the kind: every known kind but inScene and fitsSkeleton */
    static bool IsFollowedKind( const CORE::CString& kind );

    /** The kind of the safety net for a dependency an importer did not record as a relation: usesFile for .meta and .import files, dependsOn else */
    static const CORE::CString& GetKindOfDependency( const CORE::CString& path );

    typedef GUCEF::vector< CAssetReference > TReferenceVector;

    /** Sorts into the canonical order and removes duplicates */
    static void Normalise( TReferenceVector& references );

    /** Writes the list as an array child 'name' of 'parent' in the canonical order */
    static void SerializeList( const TReferenceVector& references, CORE::CDataNode& parent, const char* name );

    /** Reads the array child 'name' of 'parent'; an absent list gives an empty result. False when an entry cannot be read */
    static bool DeserializeList( const CORE::CDataNode& parent, const char* name, TReferenceVector& references );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETREFERENCE_H ? */

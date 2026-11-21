/*
 *  AssetConverterLib: a hidden face set of the intermediate model: the faces of a body that an outfit hides while it is worn
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

#ifndef ASSETCONVERTERLIB_CASSETHIDDENFACESET_H
#define ASSETCONVERTERLIB_CASSETHIDDENFACESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A hidden face set: the faces of a body that an outfit covers and that are not drawn while the outfit is worn, so that the
 *  body does not show through it. The faces are not removed: the triangles of a set are kept in submeshes of their own, and
 *  every submesh holds in CAssetSubMesh::hiddenBy the bits of the sets that hide it (bit i is the set at index i of
 *  CAssetScene::hiddenFaceSets). The body therefore stays complete and works with every outfit; a consumer hides a submesh
 *  when one of its sets is active.
 *
 *  A set is active when its outfit is worn. The importer activates the outfits it assembles onto the character; a cook
 *  setting or an application can choose others.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetHiddenFaceSet
{
    public:

    typedef GUCEF::vector< CAssetHiddenFaceSet > THiddenFaceSetVector;

    static const UInt32 MaxSets = 32;           /**< the bits of CAssetSubMesh::hiddenBy */
    static const CORE::CString SceneExtrasKey;  /**< "gucefHiddenFaceSets": the sets in the extras of a scene (glTF) */
    static const CORE::CString SubMeshExtrasKey;/**< "gucefHiddenBy": the names of the sets of a submesh in the extras of a glTF primitive */

    CORE::CString name;                         /**< the outfit, unique within the scene (for example the wardrobe item of the source) */
    CORE::CString source;                       /**< what the faces come from, for the report: the hidden face map, or the rule used */
    CORE::CString::StringVector garments;       /**< the names of the nodes of the garments of the outfit */
    bool active;                                /**< the outfit is worn: its faces are not drawn */
    UInt32 hiddenTriangles;                     /**< the triangles of the set in all levels of detail, for the report */

    CAssetHiddenFaceSet( void );

    /** The bit of the set at 'index' in CAssetSubMesh::hiddenBy; 0 for an index beyond MaxSets */
    static UInt32 BitOf( UInt32 index );

    /** The bits of the active sets */
    static UInt32 GetActiveMask( const THiddenFaceSetVector& sets );

    /** The index of the set with the name, -1 when there is none */
    static Int32 Find( const THiddenFaceSetVector& sets, const CORE::CString& setName );

    /** The names of the sets whose bits are in 'mask', in the order of the sets, joined with 'separator' */
    static CORE::CString NamesOf( const THiddenFaceSetVector& sets, UInt32 mask, const CORE::CString& separator );

    /**
     *  Makes exactly the named sets active ("none" or an empty list: no set). Names that are not sets are added to 'unknown'.
     *  @return the number of sets that are active afterwards
     */
    static UInt32 Activate( THiddenFaceSetVector& sets, const CORE::CString::StringVector& names, CORE::CString::StringVector& unknown );

    bool Serialize( CORE::CDataNode& node ) const;

    bool Deserialize( const CORE::CDataNode& node );

    /** The sets as an array of objects; false when one cannot be written */
    static bool SerializeList( const THiddenFaceSetVector& sets, CORE::CDataNode& array );

    /** The sets of an array written by SerializeList(); false when an entry cannot be read (the readable ones are kept) */
    static bool DeserializeList( const CORE::CDataNode& array, THiddenFaceSetVector& sets );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETHIDDENFACESET_H ? */

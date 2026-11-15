/*
 *  AssetConverterLib: the built-in post-processor that reads the hints in node and animation names
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

#ifndef ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H
#define ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#include "AssetConverterLib_CIAssetPostProcessor.h"
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */


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

class CAssetScene;

/*-------------------------------------------------------------------------*/

/**
 *  "NameHints": reads the conventions that artists put in names in their modelling tool.
 *
 *  The set comes from its setting "set", else from the setting "nameHints" of the asset, else "auto": "unreal",
 *  "godot", "both" or "none". "auto" is "unreal" for assets of the Unreal importer or of Unreal projects, "godot" for
 *  those of the Godot importer or projects, "none" for those of Unity projects and "both" for loose files.
 *
 *  - Unreal: a node named UCX_<mesh>[_nn], UBX_, USP_ or UCP_ is a convex, box, sphere or capsule collider of the node
 *    named <mesh>. The collider node goes; its vertices, in the frame of that node, give the shape: the convex hull
 *    (gucefGEOMETRY CTQuickHull3D), the principal-axes box, the smallest enclosing sphere, or a capsule along the longest
 *    axis of that box that holds every vertex.
 *  - Both: nodes named <name>_LOD<n> are level n of the LOD group <name>, held by a sibling named <name>, by their
 *    parent when it holds only them, else by a new node <name> they are moved under.
 *  - Godot: a node named ...-noimp goes with everything below it. -col and -convcol add a trimesh or convex collider
 *    made of the mesh of the node, which stays visible; -colonly and -convcolonly make the mesh only a collider. An
 *    animation named ...-loop or ...-cycle loops. The suffixes are taken off the names.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CNameHintsPostProcessor : public CIAssetPostProcessor
{
    public:

    static const CORE::CString TypeName;    /**< "NameHints" */

    CNameHintsPostProcessor( void );

    virtual ~CNameHintsPostProcessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Process( CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    /** The set "auto" stands for, for the provenance of the asset */
    static CORE::CString ResolveSet( const CORE::CString& set, const CAssetScene& scene );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H ? */

/*
 *  assetpluginGODOT: adds the scene of a model file to a scene
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Adds the content of a scene (what the importer of a glTF or FBX file made) to another scene, below a node of it:
 *  the instancing of a model file in a Godot scene. Nodes, skins, animations, LOD groups and prefabs are added with
 *  their references moved to their new places. The meshes, materials, textures and lights are added once per source
 *  scene: a second instance of the same file (SBases of the first) uses those again and adds only its nodes, so a
 *  model that a scene places many times is in the result once.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotSceneMerger
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    /** Where the meshes, materials, textures and lights of a source scene went in the target */
    struct SBases
    {
        bool added;
        UInt32 meshBase;
        UInt32 materialBase;
        UInt32 textureBase;
        UInt32 lightBase;
        SBases( void ) : added( false ), meshBase( 0 ), materialBase( 0 ), textureBase( 0 ), lightBase( 0 ) {}
    };

    /**
     *  Adds the scene below the node 'parent' of the target (a root node of the target for -1). 'newRoots' gets the
     *  nodes of the target that the root nodes of the source became; the result is the index of the first node added.
     */
    static UInt32 Append( ASSETCONVERTER::CAssetScene& target, const ASSETCONVERTER::CAssetScene& source, Int32 parent, SBases& bases, TUInt32Vector& newRoots );

    private:

    CGodotSceneMerger( void );
    ~CGodotSceneMerger();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTSCENEMERGER_H ? */

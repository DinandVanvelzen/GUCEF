/*
 *  assetpluginGODOT: the importer of Godot projects
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

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
 *  The importer "godot": reads the scenes (.tscn) and the mesh resources (.tres of an ArrayMesh or a primitive mesh) of
 *  Godot 4 projects through the VFS into the model (CGodotSceneReader). The model files the scenes instance (glTF,
 *  GLB, FBX) are read through the importers "gltf" and "fbx" of their plugins, which must be loaded. IndexPack()
 *  fills the index "uid" of the pack from the .import, .tscn and .tres files, so uid:// references resolve. It keeps
 *  no state between calls, so the tasks of the pipeline can use it at the same time.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotImporter : public ASSETCONVERTER::CIAssetImporter
{
    public:

    CGodotImporter( void );

    virtual ~CGodotImporter();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanImport( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Import( const CORE::CString& vfsPath, ASSETCONVERTER::CAssetPackContext& context, ASSETCONVERTER::CAssetScene& out, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IndexPack( ASSETCONVERTER::CAssetPackContext& context ) GUCEF_VIRTUAL_OVERRIDE;
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTER_H ? */

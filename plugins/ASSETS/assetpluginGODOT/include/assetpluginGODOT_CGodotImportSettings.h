/*
 *  assetpluginGODOT: the import settings of a file of a Godot project
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#include "assetpluginGODOT_CGodotDocument.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

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
 *  The import settings Godot keeps in <file>.import next to a source file of a project: the section [remap] with the
 *  importer and the uid, and the section [params] with the settings of the importer for the scene importer of
 *  glTF, FBX and other model files:
 *
 *  - "nodes/root_scale": the scale of the root of the imported scene (1 without); it multiplies the roots of the scene;
 *  - "nodes/root_name" and "nodes/root_type": the name and the type of the root node;
 *  - "animation/import": whether the animations are imported (true without);
 *  - "meshes/generate_lods": whether Godot generates levels of detail (true without);
 *  - "_subresources": per material "use_external/enabled" with "use_external/path" (a .tres that replaces the material
 *    of the file), per animation "settings/loop_mode" (0 none, 1 linear, 2 ping pong), and per node ("PATH:<path>")
 *    "import/skip_import".
 *
 *  Settings that do not change the geometry or the appearance of the scene are carried to the sidecar as they are
 *  (AddPlain()).
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotImportSettings
{
    public:

    CGodotImportSettings( void );

    ~CGodotImportSettings();

    /** Reads <assetPath>.import when the pack has it; false when it has not (the defaults then apply) */
    bool Load( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath );

    bool IsPresent( void ) const;

    /** The path of the .import file in the pack */
    const CORE::CString& GetSettingsPath( void ) const;

    CORE::CString GetImporter( void ) const;

    CORE::CString GetUid( void ) const;

    Float64 GetRootScale( void ) const;

    CORE::CString GetRootName( void ) const;

    CORE::CString GetRootType( void ) const;

    bool GetImportAnimations( void ) const;

    bool GetGenerateLods( void ) const;

    /** Whether the settings of an animation by its name say how it loops; the loop flag */
    bool GetAnimationLoop( const CORE::CString& animationName, bool& loop ) const;

    /** The external material that replaces the material of the file with the name, as written (path and uid) */
    bool GetExternalMaterial( const CORE::CString& materialName, CORE::CString& path, CORE::CString& uid ) const;

    /** Whether the settings skip the node at the path in the imported scene */
    bool IsNodeSkipped( const CORE::CString& nodePath ) const;

    /** The settings of [params] as plain data for the sidecar */
    void AddPlain( CORE::CDataNode& object ) const;

    private:

    CGodotImportSettings( const CGodotImportSettings& src );            /**< not implemented */
    CGodotImportSettings& operator=( const CGodotImportSettings& src ); /**< not implemented */

    const CORE::CDataNode* FindParam( const CORE::CString& key ) const;
    const CORE::CDataNode* FindSubresource( const CORE::CString& group, const CORE::CString& name ) const;

    private:

    CGodotDocument m_document;
    CORE::CString m_settingsPath;
    const CORE::CDataNode* m_remap;
    const CORE::CDataNode* m_params;
    bool m_present;
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTIMPORTSETTINGS_H ? */

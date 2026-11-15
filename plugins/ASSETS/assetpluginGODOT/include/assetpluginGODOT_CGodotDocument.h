/*
 *  assetpluginGODOT: a parsed Godot file
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

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
 *  A Godot file read into the tree of CGodotTextReader (a scene, a resource, an import file or the project file) with
 *  the sections found by their kind: the external resources and the sub resources by id, the nodes of a scene in file
 *  order and the main resource of a resource file. The references into the tree stay valid as long as the document
 *  lives; it is a value that is not copied.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotDocument
{
    public:

    typedef GUCEF::vector< const CORE::CDataNode* > TSectionVector;

    CGodotDocument( void );

    ~CGodotDocument();

    /** Reads and parses the file at the path in the pack; false with the problem when it cannot be read or is not valid */
    bool Load( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath );

    /** Parses text as the file at the path */
    bool LoadText( const CORE::CString& text, const CORE::CString& assetPath );

    bool IsLoaded( void ) const;

    const CORE::CString& GetProblem( void ) const;

    /** The path of the file in the pack */
    const CORE::CString& GetAssetPath( void ) const;

    const CORE::CDataNode& GetRoot( void ) const;

    /** "gd_scene" or "gd_resource"; empty for other files */
    CORE::CString GetKind( void ) const;

    /** The type of a resource file ("ArrayMesh", "StandardMaterial3D", ...); empty for others */
    CORE::CString GetResourceType( void ) const;

    /** The uid of the file header; empty without one */
    CORE::CString GetUid( void ) const;

    /** The format number of the file header (3 for Godot 4); 0 without one */
    Int32 GetFormat( void ) const;

    /** The first section with the tag */
    const CORE::CDataNode* FindSection( const CORE::CString& tag ) const;

    /** The ext_resource or sub_resource section with the id; NULL when there is none */
    const CORE::CDataNode* FindExtResource( const CORE::CString& id ) const;
    const CORE::CDataNode* FindSubResource( const CORE::CString& id ) const;

    /** The type, path and uid of an external resource; false when the id is not there */
    bool GetExtResourceInfo( const CORE::CString& id, CORE::CString& type, CORE::CString& path, CORE::CString& uid ) const;

    /** The [node] sections in file order */
    const TSectionVector& GetNodeSections( void ) const;

    /** The [resource] section of a resource file */
    const CORE::CDataNode* GetMainResource( void ) const;

    private:

    CGodotDocument( const CGodotDocument& src );            /**< not implemented */
    CGodotDocument& operator=( const CGodotDocument& src ); /**< not implemented */

    void BuildIndex( void );

    private:

    typedef GUCEF::map< CORE::CString, const CORE::CDataNode* > TSectionMap;

    CORE::CDataNode m_root;
    CORE::CString m_assetPath;
    CORE::CString m_problem;
    bool m_loaded;
    TSectionMap m_extResources;
    TSectionMap m_subResources;
    TSectionVector m_nodes;
    const CORE::CDataNode* m_main;
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

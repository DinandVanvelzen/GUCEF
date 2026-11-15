/*
 *  AssetViewerLib: the scene view: a DotScene file in a stage, its node tree for the browser, the selection highlighted, its properties for the inspector
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

#ifndef ASSETVIEWERLIB_CSCENEVIEW_H
#define ASSETVIEWERLIB_CSCENEVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETVIEWERLIB_CVIEWERSTAGE_H
#include "AssetViewerLib_CViewerStage.h"
#define ASSETVIEWERLIB_CVIEWERSTAGE_H
#endif /* ASSETVIEWERLIB_CVIEWERSTAGE_H ? */

#ifndef ASSETVIEWERLIB_CSCENETREEFILESYSTEMACCESS_H
#include "AssetViewerLib_CSceneTreeFileSystemAccess.h"
#define ASSETVIEWERLIB_CSCENETREEFILESYSTEMACCESS_H
#endif /* ASSETVIEWERLIB_CSCENETREEFILESYSTEMACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An Ogre scene in the DotScene format (.scene, read by the codec of OgrePlugin_DotScene) in a stage. The nodes that the file
 *  makes hang below one node of the stage; their tree is given to a content browser as a file system
 *  (CSceneTreeFileSystemAccess), a node being a folder and its objects files. Selecting a node or an object draws the bounding
 *  box of it and what it holds in the world, and the inspector lists its properties: the transform of a node and what is attached
 *  to it, the type and mesh of an entity, the light type and colours of a light.
 *
 *  The meshes that the file names are looked up in the resource group of the scene, in the folder of the scene and the mount.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CSceneView
{
    public:

    explicit CSceneView( CViewerStage& stage );

    ~CSceneView();

    /** Loads a .scene of a resource group (by its name in the group); false when the file or the codec is not there */
    bool Load( const CORE::CString& groupName, const CORE::CString& sceneName );

    void Unload( void );

    bool IsLoaded( void ) const;

    /** The node the scene hangs on; NULL when nothing is loaded */
    Ogre::SceneNode* GetRoot( void ) const;

    /** How many nodes the scene has, not counting the one the stage gives it */
    UInt32 GetNodeCount( void ) const;
    UInt32 GetObjectCount( void ) const;

    /** The tree for the content browser; made with the view, its contents change with every Load() (Refresh() the form after it) */
    CORE::TFileSystemAccessPtr GetFileSystem( void ) const;

    /** The box of the whole scene in the world: the center and the radius for the camera */
    void GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const;

    /** Selects a node by its path in the tree ("/" for the whole scene), or an object by its file path ("/Parent/Child/TheCube.entity") */
    bool Select( const CORE::CString& path );
    const CORE::CString& GetSelectedPath( void ) const;

    /** The properties of what is selected, for the property inspector */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    private:

    CSceneView( const CSceneView& src );
    CSceneView& operator=( const CSceneView& src );

    Ogre::SceneNode* FindNode( const CORE::CString& path ) const;
    Ogre::MovableObject* FindObject( const CORE::CString& filePath ) const;
    void AddNodeToTree( Ogre::SceneNode* node, const CORE::CString& path, CSceneTreeFileSystemAccess::TNodeVector& nodes );
    void DestroyTree( Ogre::SceneNode* node );
    void ShowBox( const Ogre::AxisAlignedBox& box );
    static Ogre::AxisAlignedBox GetSubtreeBox( Ogre::SceneNode* node );
    static CORE::CString GetObjectKind( const Ogre::MovableObject& object );

    private:

    CViewerStage& m_stage;
    Ogre::SceneNode* m_root;
    CSceneTreeFileSystemAccess* m_tree;
    CORE::TFileSystemAccessPtr m_treePtr;
    CORE::CString m_groupName;
    CORE::CString m_sceneName;
    CORE::CString m_selectedPath;
    UInt32 m_nodeCount;
    UInt32 m_objectCount;
    CORE::CString::StringVector m_meshNames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CSCENEVIEW_H ? */

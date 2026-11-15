/*
 *  AssetViewerLib: the model view: an Ogre mesh in a stage with display modes, bounds, bones, animation, LOD and sub mesh controls
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

#ifndef ASSETVIEWERLIB_CMODELVIEW_H
#define ASSETVIEWERLIB_CMODELVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreEntity.h>
#include <OgreMesh.h>

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#include "gucefGUI_CPropertyInspectorForm.h"
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERSTAGE_H
#include "AssetViewerLib_CViewerStage.h"
#define ASSETVIEWERLIB_CVIEWERSTAGE_H
#endif /* ASSETVIEWERLIB_CVIEWERSTAGE_H ? */

#ifndef ASSETVIEWERLIB_CMESHINSPECTOR_H
#include "AssetViewerLib_CMeshInspector.h"
#define ASSETVIEWERLIB_CMESHINSPECTOR_H
#endif /* ASSETVIEWERLIB_CMESHINSPECTOR_H ? */

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
 *  One mesh in a stage: an Ogre .mesh or a source model that the Assimp codec of Ogre reads (the resource name has the
 *  extension of the format). What it offers:
 *    - display modes: shaded, wireframe (the polygon mode of the camera), normals and tangents (the vectors drawn as lines over
 *      the shaded model), UV checker (a checkerboard material on every sub mesh) and vertex colours (an unlit material that shows
 *      the colours of the vertices);
 *    - the bounding box and the bones of the skeleton as lines;
 *    - the clips of the skeleton, played by time (SetClipTime());
 *    - the level of detail: automatic by distance, or one level forced, and the distance and level of this frame;
 *    - visibility and material of each sub mesh;
 *    - the statistics, and the inspector data and descriptors for the properties above;
 *    - the UV layout of a sub mesh as a picture.
 *  Not made: shading of the normals as colours, and the collider overlay.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CModelView
{
    public:

    enum EDisplayMode
    {
        DISPLAYMODE_SHADED = 0      ,
        DISPLAYMODE_WIREFRAME       ,
        DISPLAYMODE_NORMALS         ,
        DISPLAYMODE_TANGENTS        ,
        DISPLAYMODE_UV_CHECKER      ,
        DISPLAYMODE_VERTEX_COLOURS  ,
        DISPLAYMODE_COUNT
    };

    struct SClip
    {
        CORE::CString name;
        UInt32 lengthInMs;
    };
    typedef GUCEF::vector< SClip > TClipVector;

    explicit CModelView( CViewerStage& stage );

    ~CModelView();

    /** Loads a mesh of a resource group and puts it in the stage; the mesh of a name that is loaded already is loaded again */
    bool LoadMesh( const CORE::CString& groupName, const CORE::CString& meshName );

    /** Takes the mesh and what was drawn for it out of the stage, and out of Ogre */
    void Unload( void );

    bool IsLoaded( void ) const;
    const Ogre::MeshPtr& GetMesh( void ) const;
    Ogre::Entity* GetEntity( void ) const;
    const SMeshStatistics& GetStatistics( void ) const;
    const CORE::CString& GetMeshName( void ) const;

    /** The bounding sphere of the mesh: the center and the radius */
    void GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const;

    /** Points the controller at the mesh and sets the grid to a size that suits it */
    void FrameInController( COrbitController& controller );

    /*
     *  Display
     */

    void SetDisplayMode( EDisplayMode mode );
    EDisplayMode GetDisplayMode( void ) const;
    static const char* GetDisplayModeName( EDisplayMode mode );

    void SetBoundsVisible( bool visible );
    bool IsBoundsVisible( void ) const;

    void SetSkeletonVisible( bool visible );
    bool IsSkeletonVisible( void ) const;

    /*
     *  Sub meshes
     */

    UInt32 GetSubMeshCount( void ) const;
    bool IsSubMeshVisible( UInt32 index ) const;
    void SetSubMeshVisible( UInt32 index, bool visible );
    CORE::CString GetSubMeshMaterial( UInt32 index ) const;

    /**
     *  Gives a sub mesh another material (the display mode can still override it); false when there is no such material. The group
     *  says where to look first, the group of the mesh and the groups of the other sub meshes follow
     */
    bool SetSubMeshMaterial( UInt32 index, const CORE::CString& materialName, const CORE::CString& groupName = CORE::CString::Empty );

    /** The names of the materials of the resource group of the mesh, to pick from */
    void GetMaterialChoices( CORE::CString::StringVector& names ) const;

    /*
     *  Level of detail
     */

    UInt32 GetLodLevelCount( void ) const;

    /** -1 for automatic, else the level that is drawn whatever the distance */
    void SetForcedLod( Int32 level );
    Int32 GetForcedLod( void ) const;

    /** The level that was drawn at the last frame and the distance of the camera to the mesh at the last Update() */
    UInt32 GetCurrentLod( void ) const;
    Float32 GetCameraDistance( void ) const;

    /*
     *  Animation
     */

    const TClipVector& GetClips( void ) const;

    /** The clip that is shown (-1: none, the bind pose) */
    bool SelectClip( Int32 index );
    Int32 GetSelectedClip( void ) const;

    /** Puts the selected clip at a time in milliseconds */
    void SetClipTime( UInt32 timeInMs );

    /*
     *  Frame
     */

    /** Brings what depends on the camera or the animation up to date: the distance, the bones. Called before the stage draws */
    void Update( const COrbitController& controller );

    /*
     *  Inspector
     */

    /** The tree for the property inspector: View, Statistics, Features, Bounds and Submeshes */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    /** The editors of the editable values of that tree */
    void GetInspectorDescriptors( GUI::CPropertyInspectorForm::TDescriptorMap& descriptors ) const;

    /** Applies an edit of the inspector (by its path); false for a path or value that is not for the view */
    bool ApplyProperty( const CORE::CString& path, const CORE::CString& value );

    /*
     *  UV layout
     */

    /** The edges of the triangles of a sub mesh in texture space as a picture of size x size pixels; false without texture coordinates */
    bool RenderUvLayout( UInt32 subMeshIndex, UInt32 size, IMAGE::TPixelMapPtr& pixels ) const;

    private:

    CModelView( const CModelView& src );
    CModelView& operator=( const CModelView& src );

    void ApplyMaterials( void );
    void RebuildVectorLines( void );
    void RebuildBoundsLines( void );
    void RebuildSkeletonLines( void );
    void CollectClips( void );
    void ClearLines( void );
    void RemoveMissingTextures( void );

    private:

    CViewerStage& m_stage;
    Ogre::MeshPtr m_mesh;
    Ogre::Entity* m_entity;
    Ogre::SceneNode* m_node;
    CORE::CString m_meshName;
    CORE::CString m_groupName;
    SMeshStatistics m_statistics;
    EDisplayMode m_displayMode;
    bool m_boundsVisible;
    bool m_skeletonVisible;
    Int32 m_forcedLod;
    Float32 m_cameraDistance;
    Int32 m_selectedClip;
    TClipVector m_clips;
    CORE::CString::StringVector m_baseMaterials;
    CORE::CString::StringVector m_baseMaterialGroups;
    GUCEF::vector< bool > m_subMeshVisible;
    CORE::CString::StringVector m_ownedMaterials;
    bool m_meshIsManual;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CMODELVIEW_H ? */

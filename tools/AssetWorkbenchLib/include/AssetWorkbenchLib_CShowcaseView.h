/*
 *  AssetWorkbenchLib: the models of a generated showcase in one stage: a grid grouped by folder with the name of every model on the ground in front of it
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

#ifndef ASSETWORKBENCHLIB_CSHOWCASEVIEW_H
#define ASSETWORKBENCHLIB_CSHOWCASEVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H
#include "AssetWorkbenchLib_CWorkbenchStage.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H ? */

#ifndef ASSETWORKBENCHLIB_CMODELVIEW_H
#include "AssetWorkbenchLib_CModelView.h"
#define ASSETWORKBENCHLIB_CMODELVIEW_H
#endif /* ASSETWORKBENCHLIB_CMODELVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H
#include "AssetWorkbenchLib_CPackShowcaseLayout.h"
#define ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H
#endif /* ASSETWORKBENCHLIB_CPACKSHOWCASELAYOUT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Many models in the stage at once, arranged by CPackShowcaseLayout: one scene node below the content node of the stage holds one
 *  node per model, so the shared light set and the ground grid of the stage serve all of them. Models are added one at a time on the
 *  thread of the renderer (the work of a time slice of the loader); the preparation of the file is done on a thread of a pool
 *  (CModelView::PrepareMesh()). The name of every model is drawn as lines on the ground in front of it, through a line object of the stage.
 *  A model that cannot be read or has no geometry is skipped and counted.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CShowcaseView
{
    public:

    explicit CShowcaseView( CWorkbenchStage& stage );
    ~CShowcaseView();

    /** Adds a model: makes its mesh, entity and node. 'path' is the path in the pack, the label is its file name. False when it is skipped */
    bool AddModel( const CORE::CString& groupName, const CORE::CString& meshName, const CORE::CString& path, CModelView::SPreparedMesh& prepared );

    /** Puts the models in the grid (and the labels in front of them); to be called after the last AddModel(), or in between to show the progress */
    bool Arrange( const CPackShowcaseLayout::SOptions& options );

    /** Takes the models and labels out of the stage and out of Ogre */
    void Unload( void );

    bool IsLoaded( void ) const;
    UInt32 GetModelCount( void ) const;
    UInt32 GetSkippedCount( void ) const;
    const CORE::CString& GetModelPath( UInt32 index ) const;
    /** The node of a model, NULL for an index that is not there */
    Ogre::SceneNode* GetModelNode( UInt32 index ) const;
    /** The cell of a model in the arrangement; false before Arrange() */
    bool GetModelCell( UInt32 index, CPackShowcaseLayout::SCell& cell ) const;
    const CPackShowcaseLayout& GetLayout( void ) const;
    /** The sphere that holds everything that is shown: the camera frames it */
    void GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const;
    /** The paths of the pack that are shown, the folders and the number of models in each, for the inspector */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    /** The model whose cell holds the ground point (x, z) of the arrangement; -1 for none (the click of the user on the ground) */
    Int32 FindModelAtGroundPoint( Float32 x, Float32 z ) const;

    /** The lines of a text on the ground, for the labels and the tests: the text in capitals of 5 by 7 dots, one line per run of dots, the left end of the text at (x, z), running along +X */
    static void AddTextLines( CWorkbenchStage::TLineVertexVector& lines, const CORE::CString& text, Float32 x, Float32 z, Float32 height, Float32 red, Float32 green, Float32 blue );

    /** The same text standing up in the plane z = 'z' (readable from the side of the camera), its baseline at y = 'y', running along +X */
    static void AddUprightTextLines( CWorkbenchStage::TLineVertexVector& lines, const CORE::CString& text, Float32 x, Float32 y, Float32 z, Float32 height, Float32 red, Float32 green, Float32 blue );

    /**
     *  The height of the letters of a label, and the number of letters it keeps, for a text of 'length' letters in a cell that is 'pitch' wide:
     *  as high as the cell allows (a fifth of its width), less when the text would be wider than the cell, and cut where the letters would
     *  be too small to read
     */
    static Float32 GetLabelHeight( Float32 pitch, UInt32 length, UInt32& keptLength );

    /** The height, in pixels of the view, that letters of this height have at this distance from the camera, and whether they can be read there */
    static Float32 GetLabelPixelHeight( Float32 letterHeight, Float32 distance, Float32 fieldOfViewDegrees, UInt32 viewHeightInPixels );
    static bool IsLabelReadable( Float32 pixelHeight );

    private:

    struct SEntry
    {
        CORE::CString path;
        CORE::CString label;
        Ogre::MeshPtr mesh;
        Ogre::Entity* entity;
        Ogre::SceneNode* node;
        Ogre::Vector3 size;             /**< the extents of the bounds as the model stands */
        Ogre::Vector3 centre;           /**< the middle of the bounds as the model stands (before the scale) */
        Float32 bottom;                 /**< the lowest point as the model stands */
        CORE::CString::StringVector ownedMaterials;
        CORE::CString::StringVector textureNames;
        CORE::CString groupName;
        UInt32 triangleCount;
        SEntry( void );
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    CShowcaseView( const CShowcaseView& src );
    CShowcaseView& operator=( const CShowcaseView& src );
    void DestroyEntry( SEntry& entry );
    void RemoveFader( void );

    private:

    CWorkbenchStage& m_stage;
    Ogre::SceneNode* m_root;
    TEntryVector m_entries;
    CPackShowcaseLayout m_layout;
    CORE::CString::StringVector m_sortedPaths;
    GUCEF::vector< UInt32 > m_sortedToEntry;
    UInt32 m_skipped;
    bool m_arranged;
    class CLabelFader;
    CLabelFader* m_fader;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSHOWCASEVIEW_H ? */

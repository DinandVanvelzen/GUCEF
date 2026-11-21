/*
 *  AssetWorkbenchLib: a scene, a camera and a render texture that an asset is shown in
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H
#define ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreLight.h>
#include <OgreManualObject.h>
#include <OgreTexture.h>
#include <OgreRenderTexture.h>
#include <OgreViewport.h>

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#include "AssetWorkbenchLib_COrbitController.h"
#define ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CORBITCONTROLLER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H
#include "AssetWorkbenchLib_CWorkbenchViewLayout.h"
#define ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHVIEWLAYOUT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

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

class CStageViewportListener;

/**
 *  A scene of its own with a camera, lights, a grid and axes, that draws into a render texture: the surface of the model,
 *  material and scene views, and the render path of the capture and of the thumbnails. The asset hangs on GetContentNode().
 *
 *  The texture is drawn when Render() says so, which has to be inside a frame of the window (a frame listener, or between the
 *  steps of CAssetCapture): the render systems that need a frame around every pass (Vulkan) and the shader generator that
 *  adds the scene manager at the start of a frame rely on it. The picture reads back with ReadPixels() after that.
 *
 *  The lights follow the camera, so a model is lit the same from every side. The presets are the key light of a studio, a
 *  three point set, ambient light only (flat) and a dim single light. A single ambient colour stands in for an environment:
 *  image based lighting is not made.
 *
 *  The surface is one view (a perspective camera that the orbit controller moves) or, with SetViewLayout(), several views of
 *  the same scene (CWorkbenchViewLayout): every view is a viewport of the one render texture with a camera of its own, so the
 *  views share the objects, the selection and the lights and cost one texture and one pass per view, drawn only when something
 *  changed. The orthographic views look at the point of the controller from a fixed side with an extent that follows the bounds
 *  of the controller; the lights turn with the camera of the view that is being drawn. Every view has its own grid, axes and
 *  bounding box display.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchStage
{
    public:

    enum ELightPreset
    {
        LIGHTPRESET_STUDIO = 0  ,
        LIGHTPRESET_THREE_POINT ,
        LIGHTPRESET_FLAT        ,
        LIGHTPRESET_DIM         ,
        LIGHTPRESET_COUNT
    };

    explicit CWorkbenchStage( const CORE::CString& name );

    ~CWorkbenchStage();

    /** Makes the scene manager, the camera and the texture of this size; needs the Root of Ogre with a render system */
    bool Initialize( UInt32 width, UInt32 height );

    void Shutdown( void );

    bool IsInitialized( void ) const;

    /** Makes the texture of another size; the picture is gone until Render() draws it again */
    bool Resize( UInt32 width, UInt32 height );

    UInt32 GetWidth( void ) const;
    UInt32 GetHeight( void ) const;

    Ogre::SceneManager* GetSceneManager( void ) const;
    Ogre::Camera* GetCamera( void ) const;
    Ogre::SceneNode* GetContentNode( void ) const;
    Ogre::TexturePtr GetRenderTexture( void ) const;
    const CORE::CString& GetName( void ) const;

    /*
     *  The view
     */

    /** Moves the camera to where the controller says, with its field of view, aspect ratio and clip distances */
    void ApplyController( const COrbitController& controller );

    void SetBackgroundColour( Float32 red, Float32 green, Float32 blue );

    void SetLightPreset( ELightPreset preset );
    ELightPreset GetLightPreset( void ) const;
    static const char* GetLightPresetName( ELightPreset preset );

    void SetGridVisible( bool visible );
    bool IsGridVisible( void ) const;
    void SetAxesVisible( bool visible );
    bool IsAxesVisible( void ) const;

    /** The size of one grid cell and of the axes, in the units of the asset; the grid is made again */
    void SetGridUnit( Float32 unit );
    Float32 GetGridUnit( void ) const;

    /** Draws the polygons as lines (the polygon mode of the camera) */
    void SetWireframe( bool wireframe );
    bool IsWireframe( void ) const;

    /*
     *  The views
     */

    /** Divides the surface into the views of the layout; the first cell has to be a perspective view. False (the layout stays) otherwise */
    bool SetViewLayout( const CWorkbenchViewLayout& layout );

    const CWorkbenchViewLayout& GetViewLayout( void ) const;

    UInt32 GetViewCount( void ) const;

    /** What the view draws of the grid, the axes and the bounds, on top of the display of the whole stage */
    bool SetViewDisplay( UInt32 viewIndex, bool showGrid, bool showAxes, bool showBounds );

    /** The pixels of the texture that the view draws in; false for a view that is not there */
    bool GetViewRect( UInt32 viewIndex, UInt32& left, UInt32& top, UInt32& width, UInt32& height ) const;

    /** The camera of a view; NULL for a view that is not there */
    Ogre::Camera* GetViewCamera( UInt32 viewIndex ) const;

    /** Pixels of the texture between two views */
    enum
    {
        VIEW_GAP = 2
    };

    /*
     *  Drawing
     */

    /** Draws the scene into the texture; call it inside a frame of the window */
    bool Render( void );

    /** Whether something changed since the texture was drawn last (or an animation runs): the application draws when it does */
    bool IsDirty( void ) const;
    void MarkDirty( void );

    /** The picture of the texture as RGBA with 8 bits per channel, the first row at the top; false when the read back fails */
    bool ReadPixels( IMAGE::TPixelMapPtr& pixels );

    /** A line object that this stage owns, for the overlays of the views, attached to the content node: made once per name */
    Ogre::ManualObject* GetLineObject( const CORE::CString& name );

    /** Takes the line objects away again, with what they draw */
    void ClearLineObjects( void );

    /** Rewrites a line object with segments (pairs of points) of a colour each */
    struct SLineVertex
    {
        Float32 x, y, z;
        Float32 r, g, b;
    };
    typedef GUCEF::vector< SLineVertex > TLineVertexVector;
    static void FillLineObject( Ogre::ManualObject& object, const TLineVertexVector& vertices );

    /** Adds the twelve edges of a box to a list of lines */
    static void AddBoxLines( TLineVertexVector& lines, const Ogre::AxisAlignedBox& box, Float32 red, Float32 green, Float32 blue );

    private:

    friend class CStageViewportListener;

    struct SViewSlot
    {
        Ogre::Camera* camera;
        Ogre::SceneNode* node;
        Ogre::Viewport* viewport;
        UInt32 left, top, width, height;

        SViewSlot( void ) : camera( GUCEF_NULL ), node( GUCEF_NULL ), viewport( GUCEF_NULL ), left( 0 ), top( 0 ), width( 0 ), height( 0 ) {}
    };

    typedef GUCEF::vector< SViewSlot > TViewSlotVector;

    CWorkbenchStage( const CWorkbenchStage& src );
    CWorkbenchStage& operator=( const CWorkbenchStage& src );

    bool MakeRenderTexture( UInt32 width, UInt32 height );
    void ReleaseRenderTexture( void );
    static void ReleaseRenderTexture( Ogre::TexturePtr& texture, Ogre::RenderTarget* renderTarget );
    void RebuildGrid( void );
    void RebuildViewports( void );
    bool EnsureViewCameras( UInt32 count );
    UInt32 GetViewMask( const CWorkbenchViewLayout::SCell& cell ) const;
    void PositionOrthographic( UInt32 viewIndex, const COrbitController& controller );
    void OnViewportUpdate( Ogre::Viewport* viewport );

    private:

    CORE::CString m_name;
    Ogre::SceneManager* m_sceneManager;
    Ogre::Camera* m_camera;
    Ogre::SceneNode* m_cameraNode;
    Ogre::SceneNode* m_contentNode;
    Ogre::SceneNode* m_lightRig;
    TViewSlotVector m_slots;
    CWorkbenchViewLayout m_layout;
    Ogre::Viewport* m_backdrop;
    CStageViewportListener* m_listener;
    Ogre::ColourValue m_background;
    Ogre::Light* m_lights[ 3 ];
    Ogre::SceneNode* m_lightNodes[ 3 ];
    Ogre::ManualObject* m_grid;
    Ogre::ManualObject* m_axes;
    GUCEF::map< CORE::CString, Ogre::ManualObject* > m_lineObjects;
    Ogre::TexturePtr m_texture;
    Ogre::RenderTarget* m_renderTarget;
    Ogre::Viewport* m_viewport;
    UInt32 m_width;
    UInt32 m_height;
    ELightPreset m_lightPreset;
    Float32 m_gridUnit;
    bool m_gridVisible;
    bool m_axesVisible;
    bool m_wireframe;
    bool m_dirty;
    UInt32 m_textureCounter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H ? */

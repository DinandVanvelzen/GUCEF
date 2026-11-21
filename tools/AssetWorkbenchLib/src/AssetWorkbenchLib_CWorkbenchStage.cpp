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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#ifndef PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#include "pluginglueOGRE_COgreShaderGeneratorSceneTracker.h"
#define PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#endif /* PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H
#include "AssetWorkbenchLib_CWorkbenchMaterials.h"
#define ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H ? */

#include <OgreRenderTargetListener.h>

#include "AssetWorkbenchLib_CWorkbenchStage.h"

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

/** Turns the lights to the camera of the view that is about to be drawn */
class CStageViewportListener : public Ogre::RenderTargetListener
{
    public:

    explicit CStageViewportListener( CWorkbenchStage& stage ) : m_stage( stage ) {}

    virtual void preViewportUpdate( const Ogre::RenderTargetViewportEvent& evt ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_stage.OnViewportUpdate( evt.source );
    }

    private:

    CWorkbenchStage& m_stage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 GridHalfCells = 8;

// the visibility flags of what a view can leave out; everything else has all flags
const UInt32 VisibilityGrid = 0x0100;
const UInt32 VisibilityAxes = 0x0200;
const UInt32 VisibilityBounds = 0x0400;

struct SLightSetting
{
    Float32 key;
    Float32 fill;
    Float32 rim;
    Float32 ambient;
};

/** Per preset: the strength of the key, fill and rim light and the ambient light */
const SLightSetting LightSettings[ CWorkbenchStage::LIGHTPRESET_COUNT ] =
{
    { 0.95f, 0.30f, 0.00f, 0.22f },     // studio
    { 0.85f, 0.35f, 0.60f, 0.12f },     // three point
    { 0.00f, 0.00f, 0.00f, 1.00f },     // flat: ambient light only
    { 0.35f, 0.00f, 0.00f, 0.08f }      // dim
};

void
AddLine( CWorkbenchStage::TLineVertexVector& lines, Float32 x1, Float32 y1, Float32 z1, Float32 x2, Float32 y2, Float32 z2, Float32 r, Float32 g, Float32 b )
{
    CWorkbenchStage::SLineVertex a = { x1, y1, z1, r, g, b };
    CWorkbenchStage::SLineVertex c = { x2, y2, z2, r, g, b };
    lines.push_back( a );
    lines.push_back( c );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWorkbenchStage::CWorkbenchStage( const CORE::CString& name )
    : m_name( name )
    , m_sceneManager( GUCEF_NULL )
    , m_camera( GUCEF_NULL )
    , m_cameraNode( GUCEF_NULL )
    , m_contentNode( GUCEF_NULL )
    , m_grid( GUCEF_NULL )
    , m_axes( GUCEF_NULL )
    , m_lineObjects()
    , m_texture()
    , m_renderTarget( GUCEF_NULL )
    , m_viewport( GUCEF_NULL )
    , m_lightRig( GUCEF_NULL )
    , m_slots()
    , m_layout( CWorkbenchViewLayout::MakeSingle() )
    , m_backdrop( GUCEF_NULL )
    , m_listener( GUCEF_NULL )
    , m_background( 0.17f, 0.18f, 0.21f, 1.0f )
    , m_width( 0 )
    , m_height( 0 )
    , m_lightPreset( LIGHTPRESET_STUDIO )
    , m_gridUnit( 0.2f )
    , m_gridVisible( true )
    , m_axesVisible( true )
    , m_wireframe( false )
    , m_dirty( true )
    , m_textureCounter( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ] = GUCEF_NULL;
        m_lightNodes[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CWorkbenchStage::~CWorkbenchStage()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::Initialize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( IsInitialized() )
        return true;
    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || !CWorkbenchMaterials::Ensure() )
        return false;

    try
    {
        Ogre::Root& root = Ogre::Root::getSingleton();
        m_sceneManager = root.createSceneManager( "DefaultSceneManager", m_name.STL_String() );

        // the shader generator knows a scene manager once it is tracked, which the tracker does at the next frame of the Root:
        // a stage that is drawn before that frame (a render texture update) would draw without an active scene manager there
        PLUGINGLUE::OGRE::COgreGlobal* ogreGlobal = PLUGINGLUE::OGRE::COgreGlobal::Instance();
        PLUGINGLUE::OGRE::COgreShaderGeneratorSceneTracker* tracker = GUCEF_NULL == ogreGlobal ? GUCEF_NULL : ogreGlobal->GetRootSetup().GetShaderGeneratorSceneTracker();
        if ( GUCEF_NULL != tracker )
            tracker->AddNewSceneManagers();
        m_camera = m_sceneManager->createCamera( "Camera" );
        m_cameraNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
        m_cameraNode->attachObject( m_camera );
        // the camera does not roll: the vertical axis of the world stays vertical on the picture
        m_cameraNode->setFixedYawAxis( true, Ogre::Vector3::UNIT_Y );
        m_contentNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();

        // the lights hang on a rig that turns with the camera of the view that is drawn: key from the upper left, fill from the lower right, rim from behind
        m_lightRig = m_sceneManager->getRootSceneNode()->createChildSceneNode();
        const char* const lightNames[ 3 ] = { "Key", "Fill", "Rim" };
        const Ogre::Vector3 directions[ 3 ] = { Ogre::Vector3( 0.5f, -0.7f, -1.0f ), Ogre::Vector3( -0.6f, 0.3f, -1.0f ), Ogre::Vector3( 0.2f, -0.2f, 1.0f ) };
        for ( UInt32 i=0; i<3; ++i )
        {
            m_lights[ i ] = m_sceneManager->createLight( lightNames[ i ], Ogre::Light::LT_DIRECTIONAL );
            m_lightNodes[ i ] = m_lightRig->createChildSceneNode();
            m_lightNodes[ i ]->attachObject( m_lights[ i ] );
            m_lightNodes[ i ]->setDirection( directions[ i ], Ogre::Node::TS_PARENT );
        }

        m_grid = m_sceneManager->createManualObject( "Grid" );
        m_grid->setDynamic( true );
        m_axes = m_sceneManager->createManualObject( "Axes" );
        m_axes->setDynamic( true );
        m_grid->setVisibilityFlags( VisibilityGrid );
        m_axes->setVisibilityFlags( VisibilityAxes );
        m_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( m_grid );
        m_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( m_axes );
        RebuildGrid();

        m_slots.clear();
        SViewSlot first;
        first.camera = m_camera;
        first.node = m_cameraNode;
        m_slots.push_back( first );
        if ( !MakeRenderTexture( width, height ) )
        {
            Shutdown();
            return false;
        }
        SetLightPreset( m_lightPreset );
        SetBackgroundColour( 0.17f, 0.18f, 0.21f );
        m_dirty = true;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The stage " << m_name << " cannot be made: " << e.getFullDescription() );
        Shutdown();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::Shutdown( void )
{GUCEF_TRACE;

    ReleaseRenderTexture();
    if ( GUCEF_NULL != m_sceneManager && GUCEF_NULL != Ogre::Root::getSingletonPtr() )
    {
        try
        {
            Ogre::Root::getSingleton().destroySceneManager( m_sceneManager );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The scene manager of " << m_name << " cannot be destroyed: " << e.getFullDescription() );
        }
    }
    m_sceneManager = GUCEF_NULL;
    m_camera = GUCEF_NULL;
    m_cameraNode = GUCEF_NULL;
    m_lightRig = GUCEF_NULL;
    m_slots.clear();
    m_layout = CWorkbenchViewLayout::MakeSingle();
    m_contentNode = GUCEF_NULL;
    m_grid = GUCEF_NULL;
    m_axes = GUCEF_NULL;
    m_lineObjects.clear();
    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ] = GUCEF_NULL;
        m_lightNodes[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::IsInitialized( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_sceneManager && GUCEF_NULL != m_renderTarget;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::MakeRenderTexture( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( width < 1 )
        width = 1;
    if ( height < 1 )
        height = 1;

    // a new name for every texture: the old one may still be known to the render system for a frame
    const CORE::CString textureName = "AssetWorkbench/Stage/" + m_name + "/" + CORE::UInt32ToString( ++m_textureCounter );
    m_texture = Ogre::TextureManager::getSingleton().createManual( textureName.STL_String(), Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                  width, height, 0, Ogre::PF_BYTE_RGBA, Ogre::TU_RENDERTARGET );
    if ( !m_texture )
        return false;
    m_renderTarget = m_texture->getBuffer()->getRenderTarget();
    if ( GUCEF_NULL == m_renderTarget )
        return false;
    m_renderTarget->setAutoUpdated( false );
    // a depth buffer of its own: the one of an earlier texture of another size is not shared with this one
    static Ogre::uint16 depthPools = 100;
    m_renderTarget->setDepthBufferPool( ++depthPools );
    m_width = width;
    m_height = height;
    RebuildViewports();
    m_camera->setAspectRatio( (Float32) width / (Float32) height );
    m_dirty = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::ReleaseRenderTexture( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_renderTarget && GUCEF_NULL != m_listener )
        m_renderTarget->removeListener( m_listener );
    GUCEF_DELETE m_listener;
    m_listener = GUCEF_NULL;
    for ( size_t i=0; i<m_slots.size(); ++i )
        m_slots[ i ].viewport = GUCEF_NULL;
    m_backdrop = GUCEF_NULL;
    ReleaseRenderTexture( m_texture, m_renderTarget );
    m_viewport = GUCEF_NULL;
    m_renderTarget = GUCEF_NULL;
    m_texture.reset();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::ReleaseRenderTexture( Ogre::TexturePtr& texture, Ogre::RenderTarget* renderTarget )
{GUCEF_TRACE;

    if ( GUCEF_NULL != renderTarget )
        renderTarget->removeAllViewports();
    if ( texture && GUCEF_NULL != Ogre::TextureManager::getSingletonPtr() )
        Ogre::TextureManager::getSingleton().remove( texture );
    texture.reset();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::Resize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    if ( width == m_width && height == m_height )
        return true;

    // the new texture and its viewport are made before the old ones go, so a failure leaves the stage as it was
    Ogre::TexturePtr oldTexture = m_texture;
    Ogre::RenderTarget* oldTarget = m_renderTarget;
    try
    {
        if ( !MakeRenderTexture( width, height ) )
        {
            m_texture = oldTexture;
            m_renderTarget = oldTarget;
            return false;
        }
        ReleaseRenderTexture( oldTexture, oldTarget );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The texture of " << m_name << " cannot be resized: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchStage::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchStage::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneManager*
CWorkbenchStage::GetSceneManager( void ) const
{GUCEF_TRACE;

    return m_sceneManager;
}

/*-------------------------------------------------------------------------*/

Ogre::Camera*
CWorkbenchStage::GetCamera( void ) const
{GUCEF_TRACE;

    return m_camera;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneNode*
CWorkbenchStage::GetContentNode( void ) const
{GUCEF_TRACE;

    return m_contentNode;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CWorkbenchStage::GetRenderTexture( void ) const
{GUCEF_TRACE;

    return m_texture;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchStage::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::ApplyController( const COrbitController& controller )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return;
    const COrbitController::SVector3 eye = controller.GetEye();
    const COrbitController::SVector3& target = controller.GetTarget();
    m_cameraNode->setPosition( eye.x, eye.y, eye.z );
    m_cameraNode->lookAt( Ogre::Vector3( target.x, target.y, target.z ), Ogre::Node::TS_WORLD );
    m_camera->setFOVy( Ogre::Degree( controller.GetFieldOfView() ) );
    m_camera->setNearClipDistance( controller.GetNearClipDistance() );
    m_camera->setFarClipDistance( controller.GetFarClipDistance() );
    m_camera->setAspectRatio( m_slots.empty() || 0 == m_slots[ 0 ].height ? (Float32) m_width / (Float32) ( m_height > 0 ? m_height : 1 ) : (Float32) m_slots[ 0 ].width / (Float32) m_slots[ 0 ].height );
    m_lightRig->setOrientation( m_cameraNode->getOrientation() );

    // the other views: perspective ones follow the camera, orthographic ones look at the same point from their side
    for ( UInt32 i=1; i<(UInt32) m_slots.size() && i<m_layout.GetCellCount(); ++i )
    {
        if ( CWorkbenchViewLayout::PROJECTION_PERSPECTIVE == m_layout.GetCell( i ).projection )
        {
            Ogre::Camera* camera = m_slots[ i ].camera;
            m_slots[ i ].node->setPosition( eye.x, eye.y, eye.z );
            m_slots[ i ].node->lookAt( Ogre::Vector3( target.x, target.y, target.z ), Ogre::Node::TS_WORLD );
            camera->setFOVy( Ogre::Degree( controller.GetFieldOfView() ) );
            camera->setNearClipDistance( controller.GetNearClipDistance() );
            camera->setFarClipDistance( controller.GetFarClipDistance() );
            camera->setAspectRatio( 0 == m_slots[ i ].height ? 1.0f : (Float32) m_slots[ i ].width / (Float32) m_slots[ i ].height );
        }
        else
            PositionOrthographic( i, controller );
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::PositionOrthographic( UInt32 viewIndex, const COrbitController& controller )
{GUCEF_TRACE;

    SViewSlot& slot = m_slots[ viewIndex ];
    // a box is looked at from its middle: the target of the perspective camera is moved to centre what the box covers in that view
    const COrbitController::SVector3& target = controller.IsBoxCentredOnCovered() ? controller.GetBoundsCenter() : controller.GetTarget();
    const Float32 radius = controller.GetBoundsRadius();

    // the axes the camera looks along, to its right and up; a picture from the front looks down -z with +x to the right, from the
    // side down -x with -z to the right, from the top down -y with +x to the right and -z up
    Ogre::Vector3 forward( 0.0f, 0.0f, -1.0f ), right( 1.0f, 0.0f, 0.0f ), up( 0.0f, 1.0f, 0.0f );
    switch ( m_layout.GetCell( viewIndex ).direction )
    {
        case CWorkbenchViewLayout::DIRECTION_SIDE:  { forward = Ogre::Vector3( -1.0f, 0.0f, 0.0f ); right = Ogre::Vector3( 0.0f, 0.0f, -1.0f ); break; }
        case CWorkbenchViewLayout::DIRECTION_TOP:   { forward = Ogre::Vector3( 0.0f, -1.0f, 0.0f ); up = Ogre::Vector3( 0.0f, 0.0f, -1.0f ); break; }
        default:                                    { break; }
    }
    const Ogre::Vector3 center( target.x, target.y, target.z );
    const Ogre::Vector3 eye = center - forward * ( 4.0f * radius );
    slot.node->setFixedYawAxis( false );
    slot.node->setPosition( eye );
    slot.node->setOrientation( Ogre::Quaternion( right, up, -forward ) );

    // the extent holds the whole sphere of the bounds, whatever the shape of the view is, and does not follow the zoom of the perspective view
    const Float32 aspect = 0 == slot.height ? 1.0f : (Float32) slot.width / (Float32) slot.height;
    Float32 extent = 2.0f * radius * 1.12f;
    if ( aspect < 1.0f )
        extent /= aspect;
    if ( controller.HasBoundsBox() )
    {
        // a box: the sides that this view shows decide, and the box fills the part of the view that the controller asks for
        const COrbitController::SVector3& low = controller.GetBoundsMinimum();
        const COrbitController::SVector3& high = controller.GetBoundsMaximum();
        const Float32 size[ 3 ] = { high.x - low.x, high.y - low.y, high.z - low.z };
        const Float32 width = fabsf( right.x ) * size[ 0 ] + fabsf( right.y ) * size[ 1 ] + fabsf( right.z ) * size[ 2 ];
        const Float32 height = fabsf( up.x ) * size[ 0 ] + fabsf( up.y ) * size[ 1 ] + fabsf( up.z ) * size[ 2 ];
        const Float32 fill = controller.GetFillFraction();
        const Float32 fromHeight = height / fill;
        const Float32 fromWidth = width / ( fill * aspect );
        extent = fromHeight > fromWidth ? fromHeight : fromWidth;
        if ( extent < 1e-5f )
            extent = 1e-5f;
    }
    slot.camera->setProjectionType( Ogre::PT_ORTHOGRAPHIC );
    slot.camera->setOrthoWindowHeight( extent );
    slot.camera->setAspectRatio( aspect );
    slot.camera->setNearClipDistance( 0.5f * radius );
    slot.camera->setFarClipDistance( 8.0f * radius );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetBackgroundColour( Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    m_background = Ogre::ColourValue( red, green, blue, 1.0f );
    for ( size_t i=0; i<m_slots.size(); ++i )
        if ( GUCEF_NULL != m_slots[ i ].viewport )
            m_slots[ i ].viewport->setBackgroundColour( m_background );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetLightPreset( ELightPreset preset )
{GUCEF_TRACE;

    if ( (Int32) preset < 0 || (Int32) preset >= (Int32) LIGHTPRESET_COUNT )
        return;
    m_lightPreset = preset;
    if ( GUCEF_NULL == m_sceneManager )
        return;
    const SLightSetting& setting = LightSettings[ preset ];
    const Float32 strengths[ 3 ] = { setting.key, setting.fill, setting.rim };
    const Ogre::ColourValue colours[ 3 ] = { Ogre::ColourValue( 1.0f, 0.97f, 0.92f ), Ogre::ColourValue( 0.78f, 0.86f, 1.0f ), Ogre::ColourValue( 1.0f, 1.0f, 1.0f ) };
    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ]->setVisible( strengths[ i ] > 0.0f );
        m_lights[ i ]->setDiffuseColour( colours[ i ] * strengths[ i ] );
        m_lights[ i ]->setSpecularColour( colours[ i ] * ( 0.35f * strengths[ i ] ) );
    }
    m_sceneManager->setAmbientLight( Ogre::ColourValue( setting.ambient, setting.ambient, setting.ambient, 1.0f ) );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

CWorkbenchStage::ELightPreset
CWorkbenchStage::GetLightPreset( void ) const
{GUCEF_TRACE;

    return m_lightPreset;
}

/*-------------------------------------------------------------------------*/

const char*
CWorkbenchStage::GetLightPresetName( ELightPreset preset )
{GUCEF_TRACE;

    switch ( preset )
    {
        case LIGHTPRESET_STUDIO:        return "Studio";
        case LIGHTPRESET_THREE_POINT:   return "Three point";
        case LIGHTPRESET_FLAT:          return "Flat";
        default:                        return "Dim";
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetGridVisible( bool visible )
{GUCEF_TRACE;

    m_gridVisible = visible;
    if ( GUCEF_NULL != m_grid )
        m_grid->setVisible( visible );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::IsGridVisible( void ) const
{GUCEF_TRACE;

    return m_gridVisible;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetAxesVisible( bool visible )
{GUCEF_TRACE;

    m_axesVisible = visible;
    if ( GUCEF_NULL != m_axes )
        m_axes->setVisible( visible );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::IsAxesVisible( void ) const
{GUCEF_TRACE;

    return m_axesVisible;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetGridUnit( Float32 unit )
{GUCEF_TRACE;

    if ( unit <= 0.0f )
        return;
    m_gridUnit = unit;
    RebuildGrid();
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

Float32
CWorkbenchStage::GetGridUnit( void ) const
{GUCEF_TRACE;

    return m_gridUnit;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::RebuildGrid( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_grid || GUCEF_NULL == m_axes )
        return;

    TLineVertexVector gridLines;
    const Float32 extent = m_gridUnit * (Float32) GridHalfCells;
    for ( Int32 i=-(Int32) GridHalfCells; i<=(Int32) GridHalfCells; ++i )
    {
        if ( 0 == i )
            continue;   // the axes take the middle lines
        const Float32 at = m_gridUnit * (Float32) i;
        const Float32 grey = 0 == ( i % 5 ) ? 0.48f : 0.30f;
        AddLine( gridLines, -extent, 0.0f, at, extent, 0.0f, at, grey, grey, grey );
        AddLine( gridLines, at, 0.0f, -extent, at, 0.0f, extent, grey, grey, grey );
    }
    FillLineObject( *m_grid, gridLines );

    TLineVertexVector axisLines;
    AddLine( axisLines, -extent, 0.0f, 0.0f, extent, 0.0f, 0.0f, 0.75f, 0.2f, 0.2f );
    AddLine( axisLines, 0.0f, 0.0f, -extent, 0.0f, 0.0f, extent, 0.2f, 0.35f, 0.8f );
    AddLine( axisLines, 0.0f, 0.0f, 0.0f, 0.0f, extent * 0.5f, 0.0f, 0.25f, 0.8f, 0.25f );
    FillLineObject( *m_axes, axisLines );
    m_grid->setVisible( m_gridVisible );
    m_axes->setVisible( m_axesVisible );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::SetWireframe( bool wireframe )
{GUCEF_TRACE;

    m_wireframe = wireframe;
    for ( size_t i=0; i<m_slots.size(); ++i )
        if ( GUCEF_NULL != m_slots[ i ].camera )
            m_slots[ i ].camera->setPolygonMode( wireframe ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::IsWireframe( void ) const
{GUCEF_TRACE;

    return m_wireframe;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchStage::GetViewMask( const CWorkbenchViewLayout::SCell& cell ) const
{GUCEF_TRACE;

    UInt32 mask = 0xFFFFFFFFu;
    if ( !cell.showGrid )
        mask &= ~VisibilityGrid;
    if ( !cell.showAxes )
        mask &= ~VisibilityAxes;
    if ( !cell.showBounds )
        mask &= ~VisibilityBounds;
    return mask;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::EnsureViewCameras( UInt32 count )
{GUCEF_TRACE;

    try
    {
        while ( (UInt32) m_slots.size() < count )
        {
            const CORE::CString name = "View" + CORE::UInt32ToString( (UInt32) m_slots.size() );
            SViewSlot slot;
            slot.camera = m_sceneManager->createCamera( name.STL_String() );
            slot.node = m_sceneManager->getRootSceneNode()->createChildSceneNode();
            slot.node->attachObject( slot.camera );
            slot.camera->setPolygonMode( m_wireframe ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID );
            m_slots.push_back( slot );
        }
        while ( (UInt32) m_slots.size() > count && m_slots.size() > 1 )
        {
            SViewSlot& slot = m_slots.back();
            slot.node->detachObject( slot.camera );
            m_sceneManager->destroyCamera( slot.camera );
            m_sceneManager->destroySceneNode( slot.node );
            m_slots.pop_back();
        }
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The cameras of the views of " << m_name << " cannot be made: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::RebuildViewports( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderTarget || m_width < 1 || m_height < 1 )
        return;

    const UInt32 cells = m_layout.GetCellCount();
    const bool several = cells > 1;
    m_renderTarget->removeAllViewports();
    m_backdrop = GUCEF_NULL;
    m_viewport = GUCEF_NULL;
    if ( GUCEF_NULL == m_listener )
        m_listener = GUCEF_NEW CStageViewportListener( *this );
    m_renderTarget->addListener( m_listener );

    if ( several )
    {
        // the gaps between the views are the colour of the viewport that covers the whole texture and draws nothing
        m_backdrop = m_renderTarget->addViewport( m_camera, 0 );
        m_backdrop->setClearEveryFrame( true );
        m_backdrop->setOverlaysEnabled( false );
        m_backdrop->setVisibilityMask( 0 );
        m_backdrop->setBackgroundColour( Ogre::ColourValue( 0.07f, 0.075f, 0.09f, 1.0f ) );
    }
    for ( UInt32 i=0; i<cells; ++i )
    {
        const CWorkbenchViewLayout::SCell& cell = m_layout.GetCell( i );
        SViewSlot& slot = m_slots[ i ];

        // the pixels of the view: the edges between two views are given back half the gap each
        const Float32 right = cell.left + cell.width;
        const Float32 bottom = cell.top + cell.height;
        UInt32 left = (UInt32) ( cell.left * (Float32) m_width + 0.5f );
        UInt32 top = (UInt32) ( cell.top * (Float32) m_height + 0.5f );
        UInt32 rightPixel = (UInt32) ( right * (Float32) m_width + 0.5f );
        UInt32 bottomPixel = (UInt32) ( bottom * (Float32) m_height + 0.5f );
        if ( several )
        {
            if ( cell.left > 0.001f ) left += VIEW_GAP / 2;
            if ( cell.top > 0.001f ) top += VIEW_GAP / 2;
            if ( right < 0.999f ) rightPixel -= VIEW_GAP / 2;
            if ( bottom < 0.999f ) bottomPixel -= VIEW_GAP / 2;
        }
        if ( rightPixel > m_width ) rightPixel = m_width;
        if ( bottomPixel > m_height ) bottomPixel = m_height;
        if ( rightPixel <= left ) rightPixel = left + 1;
        if ( bottomPixel <= top ) bottomPixel = top + 1;
        slot.left = left;
        slot.top = top;
        slot.width = rightPixel - left;
        slot.height = bottomPixel - top;

        // a little over the pixel so that the viewport does not round to the pixel before
        const Float32 fw = (Float32) m_width;
        const Float32 fh = (Float32) m_height;
        slot.viewport = m_renderTarget->addViewport( slot.camera, (int) i + 1, ( (Float32) left + 0.01f ) / fw, ( (Float32) top + 0.01f ) / fh,
                                                     ( (Float32) slot.width + 0.01f ) / fw, ( (Float32) slot.height + 0.01f ) / fh );
        slot.viewport->setClearEveryFrame( true );
        slot.viewport->setOverlaysEnabled( false );
        slot.viewport->setBackgroundColour( m_background );
        slot.viewport->setVisibilityMask( GetViewMask( cell ) );
        slot.width = (UInt32) slot.viewport->getActualWidth();
        slot.height = (UInt32) slot.viewport->getActualHeight();
        slot.left = (UInt32) slot.viewport->getActualLeft();
        slot.top = (UInt32) slot.viewport->getActualTop();
        if ( CWorkbenchViewLayout::PROJECTION_PERSPECTIVE == cell.projection )
        {
            slot.camera->setProjectionType( Ogre::PT_PERSPECTIVE );
            slot.camera->setAspectRatio( (Float32) slot.width / (Float32) ( slot.height > 0 ? slot.height : 1 ) );
        }
    }
    m_viewport = m_slots[ 0 ].viewport;
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::SetViewLayout( const CWorkbenchViewLayout& layout )
{GUCEF_TRACE;

    if ( !IsInitialized() || 0 == layout.GetCellCount() || CWorkbenchViewLayout::PROJECTION_PERSPECTIVE != layout.GetCell( 0 ).projection )
        return false;
    if ( layout == m_layout )
        return true;
    try
    {
        if ( !EnsureViewCameras( layout.GetCellCount() ) )
            return false;
        m_layout = layout;
        RebuildViewports();
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The views of " << m_name << " cannot be made: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

const CWorkbenchViewLayout&
CWorkbenchStage::GetViewLayout( void ) const
{GUCEF_TRACE;

    return m_layout;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchStage::GetViewCount( void ) const
{GUCEF_TRACE;

    return m_layout.GetCellCount();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::SetViewDisplay( UInt32 viewIndex, bool showGrid, bool showAxes, bool showBounds )
{GUCEF_TRACE;

    if ( viewIndex >= m_layout.GetCellCount() || !m_layout.SetCellDisplay( viewIndex, showGrid, showAxes, showBounds ) )
        return false;
    if ( viewIndex < (UInt32) m_slots.size() && GUCEF_NULL != m_slots[ viewIndex ].viewport )
        m_slots[ viewIndex ].viewport->setVisibilityMask( GetViewMask( m_layout.GetCell( viewIndex ) ) );
    m_dirty = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::GetViewRect( UInt32 viewIndex, UInt32& left, UInt32& top, UInt32& width, UInt32& height ) const
{GUCEF_TRACE;

    if ( viewIndex >= m_layout.GetCellCount() || viewIndex >= (UInt32) m_slots.size() )
        return false;
    left = m_slots[ viewIndex ].left;
    top = m_slots[ viewIndex ].top;
    width = m_slots[ viewIndex ].width;
    height = m_slots[ viewIndex ].height;
    return true;
}

/*-------------------------------------------------------------------------*/

Ogre::Camera*
CWorkbenchStage::GetViewCamera( UInt32 viewIndex ) const
{GUCEF_TRACE;

    return viewIndex < m_layout.GetCellCount() && viewIndex < (UInt32) m_slots.size() ? m_slots[ viewIndex ].camera : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::OnViewportUpdate( Ogre::Viewport* viewport )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_lightRig )
        return;
    for ( size_t i=0; i<m_slots.size(); ++i )
    {
        if ( m_slots[ i ].viewport == viewport && GUCEF_NULL != m_slots[ i ].node )
        {
            m_lightRig->setOrientation( m_slots[ i ].node->getOrientation() );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::Render( void )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    try
    {
        m_renderTarget->update();
        m_dirty = false;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The stage " << m_name << " cannot be drawn: " << e.getFullDescription() );
        return false;
    }
    catch ( const std::exception& e )
    {
        // the shader generator reports a material it cannot build with an exception of the standard library: that is not the end of the program
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The stage " << m_name << " cannot be drawn: " << e.what() );
        m_dirty = false;
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::IsDirty( void ) const
{GUCEF_TRACE;

    return m_dirty;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::MarkDirty( void )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchStage::ReadPixels( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    try
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, m_width, m_height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        Ogre::PixelBox box( m_width, m_height, 1, Ogre::PF_BYTE_RGBA, pixels->GetDataPtr() );
        m_renderTarget->copyContentsToMemory( Ogre::Box( 0, 0, m_width, m_height ), box );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The picture of " << m_name << " cannot be read back: " << e.getFullDescription() );
        pixels.Unlink();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

Ogre::ManualObject*
CWorkbenchStage::GetLineObject( const CORE::CString& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sceneManager )
        return GUCEF_NULL;
    GUCEF::map< CORE::CString, Ogre::ManualObject* >::iterator i = m_lineObjects.find( name );
    if ( i != m_lineObjects.end() )
        return i->second;
    try
    {
        Ogre::ManualObject* object = m_sceneManager->createManualObject( ( "Lines/" + name ).STL_String() );
        object->setDynamic( true );
        if ( name == "Bounds" )
            object->setVisibilityFlags( VisibilityBounds );
        m_contentNode->createChildSceneNode()->attachObject( object );
        m_lineObjects[ name ] = object;
        return object;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWorkbenchStage: The line object " << name << " cannot be made: " << e.getFullDescription() );
        return GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::ClearLineObjects( void )
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, Ogre::ManualObject* >::iterator i = m_lineObjects.begin();
    while ( i != m_lineObjects.end() )
    {
        i->second->clear();
        ++i;
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::FillLineObject( Ogre::ManualObject& object, const TLineVertexVector& vertices )
{GUCEF_TRACE;

    object.clear();
    if ( vertices.empty() )
        return;
    object.begin( CWorkbenchMaterials::LinesMaterialName.STL_String(), Ogre::RenderOperation::OT_LINE_LIST );
    for ( size_t i=0; i<vertices.size(); ++i )
    {
        object.position( vertices[ i ].x, vertices[ i ].y, vertices[ i ].z );
        object.colour( vertices[ i ].r, vertices[ i ].g, vertices[ i ].b, 1.0f );
    }
    object.end();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchStage::AddBoxLines( TLineVertexVector& lines, const Ogre::AxisAlignedBox& box, Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    if ( !box.isFinite() )
        return;
    const Ogre::Vector3& lo = box.getMinimum();
    const Ogre::Vector3& hi = box.getMaximum();
    const Float32 x[ 2 ] = { lo.x, hi.x };
    const Float32 y[ 2 ] = { lo.y, hi.y };
    const Float32 z[ 2 ] = { lo.z, hi.z };
    for ( UInt32 a=0; a<2; ++a )
    {
        for ( UInt32 b=0; b<2; ++b )
        {
            AddLine( lines, x[ 0 ], y[ a ], z[ b ], x[ 1 ], y[ a ], z[ b ], red, green, blue );
            AddLine( lines, x[ a ], y[ 0 ], z[ b ], x[ a ], y[ 1 ], z[ b ], red, green, blue );
            AddLine( lines, x[ a ], y[ b ], z[ 0 ], x[ a ], y[ b ], z[ 1 ], red, green, blue );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

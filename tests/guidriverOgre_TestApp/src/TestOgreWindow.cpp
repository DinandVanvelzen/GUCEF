/*
 *  guidriverOgre_TestApp: Test application for the window backend Ogre (guidriverOgre)
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

#include <string.h>

#include <Ogre.h>
#include <OgreCodec.h>
#include <OgreRenderWindow.h>
#include <OgreRenderTargetListener.h>
#include <OgreWorkQueue.h>

#if ( defined( __linux__ ) )
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#include "guidriverOgre_CIOgreWindowOverlay.h"
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_H
#include "pluginglueOGRE.h"
#define GUCEF_PLUGINGLUE_OGRE_H
#endif /* GUCEF_PLUGINGLUE_OGRE_H ? */

#include "TestOgreSupport.h"
#include "TestOgreWindow.h"
#include "TestOgreWorkQueueWindow.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< CORE::CString > TStepVector;

/** Writes down the steps of a frame as the application, a GUI context and an overlay see them */
class CFrameRecorder : public Ogre::FrameListener                ,
                       public Ogre::RenderTargetListener         ,
                       public CORE::CObserver                    ,
                       public GUIDRIVEROGRE::CIOgreWindowOverlay
{
    public:

    explicit CFrameRecorder( Int32 order ) : m_order( order ), steps() {}
    virtual ~CFrameRecorder() { UnsubscribeAllFromObserver(); }

    virtual bool frameStarted( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE { steps.push_back( "frameStarted" ); return true; }
    virtual bool frameRenderingQueued( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE { steps.push_back( "frameRenderingQueued" ); return true; }
    virtual bool frameEnded( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE { steps.push_back( "frameEnded" ); return true; }
    virtual void preViewportUpdate( const Ogre::RenderTargetViewportEvent& evt ) GUCEF_VIRTUAL_OVERRIDE { steps.push_back( "viewport" ); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CWindowContext::WindowContextRedrawEvent == eventId )
            steps.push_back( "redraw" );
        else
        if ( GUI::CWindowContext::WindowContextSizeEvent == eventId )
            steps.push_back( "size" );
        else
        if ( GUI::CWindowContext::WindowContextCloseRequestEvent == eventId )
            steps.push_back( "closeRequest" );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CFrameRecorder";
        return classTypeName;
    }

    virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_order; }

    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem, Ogre::Viewport& viewport, Ogre::SceneManager* sceneManager, bool isCapture ) GUCEF_VIRTUAL_OVERRIDE
    {
        steps.push_back( CORE::CString( isCapture ? "capture" : "overlay" ) + CORE::Int32ToString( m_order ) );
        if ( GUCEF_NULL != sceneManager )
            steps.push_back( "sceneManager" );
    }

    CORE::CString Joined( void ) const
    {
        CORE::CString text;
        for ( size_t i=0; i<steps.size(); ++i )
            text += ( 0 == i ? "" : "," ) + steps[ i ];
        return text;
    }

    Int32 m_order;
    TStepVector steps;
};

/*-------------------------------------------------------------------------*/

/** Draws quads with the GUI material of guidriverOgre, optionally textured */
class CQuadOverlay : public GUIDRIVEROGRE::CIOgreWindowOverlay
{
    public:

    CQuadOverlay( GUIDRIVEROGRE::COgreGuiMaterial::TBlendMode blendMode, Int32 order )
        : m_material( blendMode ), m_order( order ), m_batch(), m_texture(), m_drawn( 0 ) {}

    virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_order; }

    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem, Ogre::Viewport& viewport, Ogre::SceneManager* sceneManager, bool isCapture ) GUCEF_VIRTUAL_OVERRIDE
    {
        const Ogre::Matrix4 projection = GUIDRIVEROGRE::COgreGuiMaterial::MakeProjection( 0.0f, 0.0f, 1.0f, 1.0f,
            (UInt32) viewport.getActualWidth(), (UInt32) viewport.getActualHeight(), viewport.getTarget()->requiresTextureFlipping() );
        m_material.BeginDrawing( renderSystem, projection );
        m_material.SetTexture( renderSystem, m_texture );
        m_material.Render( renderSystem, m_batch.GetOperation() );
        ++m_drawn;
    }

    GUIDRIVEROGRE::COgreGuiMaterial m_material;
    Int32 m_order;
    CTestQuadBatch m_batch;
    Ogre::TexturePtr m_texture;
    UInt32 m_drawn;
};

/*-------------------------------------------------------------------------*/

/** The image codec of the test: "OGW1" and the four bytes of a colour become a 4 x 4 RGBA picture of that colour */
class CTestTextureCodec : public IMAGE::CIImageCodec
{
    public:

    CTestTextureCodec( void ) : IMAGE::CIImageCodec() {}
    virtual ~CTestTextureCodec() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool Encode( const IMAGE::CImage& inputImage, CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_CODEC_CAPABILITY_DECODE; }
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ogwtex"; }
    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ImageCodec"; }
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestTextureCodec(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestTextureCodec";
        return classTypeName;
    }

    virtual bool Decode( CORE::CIOAccess& encodedInput, IMAGE::CImage& outputImage ) GUCEF_VIRTUAL_OVERRIDE
    {
        char header[ 8 ];
        if ( 8 != encodedInput.Read( header, 1, 8 ) || 0 != memcmp( header, "OGW1", 4 ) )
            return false;
        UInt8 pixels[ 4 * 4 * 4 ];
        for ( UInt32 i=0; i<16; ++i )
            memcpy( pixels + i*4, header + 4, 4 );
        outputImage.Assign( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( pixels, 4, 4, IMAGE::PSF_RGBA, DATATYPE_UINT8 ) ) );
        return true;
    }
};

typedef CORE::CTSharedPtr< CTestTextureCodec, MT::CMutex > TTestTextureCodecPtr;

/*-------------------------------------------------------------------------*/

struct SFrameCountAbove
{
    const GUIDRIVEROGRE::COgreWindowContext* window;
    UInt64 count;
    SFrameCountAbove( const GUIDRIVEROGRE::COgreWindowContext* w, UInt64 c ) : window( w ), count( c ) {}
    bool operator()( void ) const { return window->GetFrameCount() > count; }
};

struct SRenderWindowWidth
{
    const GUIDRIVEROGRE::COgreWindowContext* window;
    UInt32 width;
    SRenderWindowWidth( const GUIDRIVEROGRE::COgreWindowContext* w, UInt32 wd ) : window( w ), width( wd ) {}
    bool operator()( void ) const { return GUCEF_NULL != window->GetOgreRenderWindow() && window->GetOgreRenderWindow()->getWidth() == width; }
};

struct SHasStep
{
    const CFrameRecorder& recorder;
    CORE::CString step;
    SHasStep( const CFrameRecorder& r, const CORE::CString& s ) : recorder( r ), step( s ) {}
    bool operator()( void ) const
    {
        for ( size_t i=0; i<recorder.steps.size(); ++i )
            if ( recorder.steps[ i ] == step )
                return true;
        return false;
    }
};

/** A scene manager with a camera and a viewport of one colour over the whole window */
Ogre::Viewport*
AddColourViewport( Ogre::RenderWindow* window, const char* name, const Ogre::ColourValue& colour )
{
    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::SceneManager* sceneManager = root.hasSceneManager( name ) ? root.getSceneManager( name ) : root.createSceneManager( "DefaultSceneManager", name );
    Ogre::Camera* camera = sceneManager->hasCamera( name ) ? sceneManager->getCamera( name ) : sceneManager->createCamera( name );
    if ( GUCEF_NULL == camera->getParentSceneNode() )
        sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( camera );
    Ogre::Viewport* viewport = window->addViewport( camera );
    viewport->setBackgroundColour( colour );
    viewport->setOverlaysEnabled( false );
    return viewport;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOgreWindowTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverOgre" );

    bool setUp = false;
    GUCEF_TESTFW_TESTCASE( "Backend: the support library, the Ogre plugins and the window backend Ogre load" )
    try
    {
        // the image codec of the texture test is in gucefIMAGE before the codec plugin of Ogre starts
        TTestTextureCodecPtr codec( GUCEF_NEW CTestTextureCodec() );
        IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().TryRegister( "ogwtex", codec );

        setUp = SetUpOgreForTests();
        ASSERT_TRUE( setUp );
        ASSERT_TRUE( GUCEF_NULL != Ogre::Root::getSingletonPtr() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    if ( !setUp )
        return;

    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );

    GUCEF_TESTFW_TESTCASE( "Backend: Ogre windows take no GraphicsApi value, and an unknown platform backend gives no window" )
    try
    {
        ASSERT_FALSE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiGL ) );
        ASSERT_FALSE( backend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiNone ) );
        CORE::CValueList gl;
        gl.Set( GUI::CWindowManagerBackend::GraphicsApiParameterName, "GL" );
        ASSERT_TRUE( backend->CreateWindowContext( "OgreTest", 64, 48, false, gl ).IsNULL() );
        CORE::CValueList unknown;
        unknown.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, "DoesNotExist" );
        ASSERT_TRUE( backend->CreateWindowContext( "OgreTest", 64, 48, false, unknown ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    const ETestWindowMode mode = GetTestWindowMode();
    const CORE::CString skipReason = GetNoWindowReason();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: window mode " << (Int32) mode << ( TESTWINDOWMODE_NONE == mode ? GetNoWindowReason() : CORE::CString() ) );

    GUIDRIVEROGRE::COgreWindowContext* window = GUCEF_NULL;
    GUCEF_TESTFW_TESTCASE( "Window: an Ogre render window on the platform window, with typed accessors and the properties of the platform window" )
    if ( TESTWINDOWMODE_NONE == mode )
        GUCEF_TESTFW_SKIP( skipReason.C_String() );
    try
    {
        const UInt32 threadsBefore = CountProcessThreads();
        GUI::TWindowContextPtr windowPtr = GetMainTestWindow();
        window = AsOgreWindow( windowPtr );
        ASSERT_TRUE( GUCEF_NULL != window );
        if ( GUCEF_NULL != window )
        {
            const UInt32 threadsAfter = CountProcessThreads();
            // the work queue of the Root starts with the first render window, on the GUCEF thread pool: Ogre makes no
            // worker threads of its own (it names them "[Ogre] Worker")
            const UInt32 ogreWorkers = CountProcessThreadsNamed( "[Ogre] Worker" );
            const UInt32 poolWorkers = CountProcessThreadsNamed( "OgreWork" );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: threads before the first render window " << threadsBefore << ", after it " << threadsAfter
                << "; worker threads of Ogre: " << ogreWorkers << ", of the GUCEF thread pool of the work queue: " << poolWorkers );
            #if ( defined( __linux__ ) )
            ASSERT_TRUE( 0 == ogreWorkers );
            ASSERT_TRUE( poolWorkers >= 2 );
            #endif
            ASSERT_TRUE( GUCEF_NULL != PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().GetTaskWorkQueue() );

            ASSERT_TRUE( GUCEF_NULL != window->GetOgreRenderWindow() );
            ASSERT_TRUE( GUCEF_NULL != window->GetOgreRoot() && window->GetOgreRoot()->isInitialised() );
            ASSERT_TRUE( "OpenGL 3+ Rendering Subsystem" == window->GetOgreRoot()->getRenderSystem()->getName() );
            ASSERT_TRUE( 320 == window->GetWidth() && 240 == window->GetHeight() );
            ASSERT_TRUE( 320 == window->GetOgreRenderWindow()->getWidth() && 240 == window->GetOgreRenderWindow()->getHeight() );
            ASSERT_FALSE( window->GetOgreRenderWindow()->isAutoUpdated() );
            ASSERT_TRUE( GUIDRIVEROGRE::COgreWindowContext::GraphicsApiName == window->GetProperty( "GRAPHICS_API" ) );
            ASSERT_TRUE( window->GetProperty( "GL_GETPROCADDRESS" ).IsNULLOrEmpty() );
            ASSERT_TRUE( window->GetProperty( "GL_API" ).IsNULLOrEmpty() );
            if ( TESTWINDOWMODE_X11 == mode )
            {
                // the platform window has no GL context of its own; input drivers attach to its handles through the Ogre window
                ASSERT_TRUE( "XWinGL" == window->GetPlatformBackendName() );
                ASSERT_FALSE( window->GetPlatformWindowContext().IsNULL() );
                ASSERT_TRUE( "None" == window->GetPlatformWindowContext()->GetProperty( "GRAPHICS_API" ) );
                ASSERT_TRUE( 0 != CORE::StringToUInt32( window->GetProperty( "WINDOW" ), 0 ) );
                ASSERT_TRUE( window->GetPlatformWindowContext()->GetProperty( "WINDOW" ) == window->GetProperty( "WINDOW" ) );
                ASSERT_TRUE( GUCEF_NULL != CORE::StringToPointer( window->GetProperty( "DISPLAY" ) ) );
            }
            else
            if ( TESTWINDOWMODE_WAYLAND == mode )
            {
                // Ogre's wl_egl_window is on the surface of the platform window, made on the connection of pluginglueWAYLAND
                ASSERT_TRUE( "Wayland" == window->GetPlatformBackendName() );
                ASSERT_FALSE( window->GetPlatformWindowContext().IsNULL() );
                ASSERT_TRUE( "None" == window->GetPlatformWindowContext()->GetProperty( "GRAPHICS_API" ) );
                ASSERT_TRUE( GUCEF_NULL != CORE::StringToPointer( window->GetProperty( "WL_SURFACE" ) ) );
                ASSERT_TRUE( GUCEF_NULL != CORE::StringToPointer( window->GetProperty( "DISPLAY" ) ) );
            }
            else
            {
                ASSERT_TRUE( GUIDRIVEROGRE::COgreWindowContext::OffscreenPlatformBackend == window->GetPlatformBackendName() );
                ASSERT_TRUE( window->GetPlatformWindowContext().IsNULL() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    if ( GUCEF_NULL == window )
    {
        TearDownOgreForTests();
        return;
    }
    Ogre::RenderWindow* renderWindow = window->GetOgreRenderWindow();

    GUCEF_TESTFW_TESTCASE( "Frames: a redraw runs the frame listeners, the GUI contexts, the viewports, the overlays in their order, then ends the frame" )
    try
    {
        Ogre::Viewport* viewport = AddColourViewport( renderWindow, "FrameOrder", Ogre::ColourValue( 0.0f, 0.0f, 1.0f, 1.0f ) );
        CFrameRecorder recorder( 5 );
        CFrameRecorder lower( -3 );
        Ogre::Root::getSingleton().addFrameListener( &recorder );
        renderWindow->addListener( &recorder );
        recorder.SubscribeTo( window );
        window->RegisterOverlay( &recorder );
        window->RegisterOverlay( &lower );
        ASSERT_TRUE( 2 == window->GetOverlayCount() );

        const UInt64 frames = window->GetFrameCount();
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( frames + 1 == window->GetFrameCount() );

        // each recorder keeps its own steps; the order between overlays is the next test
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: frame steps " << recorder.Joined() );
        ASSERT_TRUE( "frameStarted,redraw,viewport,overlay5,sceneManager,frameRenderingQueued,frameEnded" == recorder.Joined() );
        ASSERT_TRUE( "overlay-3,sceneManager" == lower.Joined() );

        // the application does not draw the window: Root::renderOneFrame() leaves it alone
        recorder.steps.clear();
        Ogre::Root::getSingleton().renderOneFrame();
        ASSERT_TRUE( "frameStarted,frameRenderingQueued,frameEnded" == recorder.Joined() );

        window->UnregisterOverlay( &recorder );
        window->UnregisterOverlay( &lower );
        ASSERT_TRUE( 0 == window->GetOverlayCount() );
        recorder.UnsubscribeAllFromObserver();
        renderWindow->removeListener( &recorder );
        Ogre::Root::getSingleton().removeFrameListener( &recorder );
        renderWindow->removeViewport( viewport->getZOrder() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: overlays draw from the lowest order up, equal orders in the order they were registered" )
    try
    {
        // one shared list: each overlay writes into it
        CFrameRecorder a( 2 ), b( 1 ), c( 2 );
        window->RegisterOverlay( &a );
        window->RegisterOverlay( &b );
        window->RegisterOverlay( &c );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( 1 == a.steps.size() && 1 == b.steps.size() && 1 == c.steps.size() );
        window->UnregisterOverlay( &a );
        window->UnregisterOverlay( &b );
        window->UnregisterOverlay( &c );

        // the order itself: a sequence counter shared through the steps
        class COrderOverlay : public GUIDRIVEROGRE::CIOgreWindowOverlay
        {
            public:
            COrderOverlay( Int32 order, const char* name, CORE::CString& log ) : m_order( order ), m_name( name ), m_log( log ) {}
            virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_order; }
            virtual void RenderOverlay( Ogre::RenderSystem&, Ogre::Viewport&, Ogre::SceneManager*, bool ) GUCEF_VIRTUAL_OVERRIDE { m_log += m_name; }
            Int32 m_order;
            const char* m_name;
            CORE::CString& m_log;
        };
        CORE::CString log;
        COrderOverlay first( 2, "A", log ), second( 1, "B", log ), third( 2, "C", log ), fourth( -1, "D", log );
        window->RegisterOverlay( &first );
        window->RegisterOverlay( &second );
        window->RegisterOverlay( &third );
        window->RegisterOverlay( &fourth );
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( "DBAC" == log );
        log = CORE::CString();
        first.m_order = 3;
        ASSERT_TRUE( window->RenderFrame() );
        ASSERT_TRUE( "DBCA" == log );
        window->UnregisterOverlay( &first );
        window->UnregisterOverlay( &second );
        window->UnregisterOverlay( &third );
        window->UnregisterOverlay( &fourth );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pixels: the viewport of the application, a GUI material quad and a textured quad over it, read back from the window" )
    try
    {
        Ogre::Viewport* viewport = AddColourViewport( renderWindow, "Pixels", Ogre::ColourValue( 0.0f, 0.0f, 1.0f, 1.0f ) );
        CQuadOverlay quads( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_PREMULTIPLIED, 0 );
        ASSERT_TRUE( quads.m_material.Initialize() );
        quads.m_batch.AddQuad( 10.0f, 20.0f, 100.0f, 50.0f, 255, 0, 0, 255 );
        // half transparent green, premultiplied: half of it over the blue
        quads.m_batch.AddQuad( 200.0f, 100.0f, 40.0f, 40.0f, 0, 128, 0, 128 );
        ASSERT_TRUE( quads.m_batch.Build() );
        window->RegisterOverlay( &quads );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
        ASSERT_TRUE( !pixels.IsNULL() && 320 == pixels->GetWidthInPixels() && 240 == pixels->GetHeightInPixels() );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 300, 230, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 10, 20, 255, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 109, 69, 255, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 110, 69, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 109, 70, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 9, 20, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 220, 120, 0, 128, 127 ) );

        // a texture times the white vertex colour, through the same material
        window->UnregisterOverlay( &quads );
        CQuadOverlay textured( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_PREMULTIPLIED, 0 );
        ASSERT_TRUE( textured.m_material.Initialize() );
        textured.m_batch.AddQuad( 50.0f, 50.0f, 64.0f, 32.0f, 255, 255, 255, 255 );
        ASSERT_TRUE( textured.m_batch.Build() );
        textured.m_texture = MakeSolidTexture( "OgreTest/Yellow", 4, 4, 255, 255, 0, 255 );
        window->RegisterOverlay( &textured );
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
        ASSERT_TRUE( PixelIs( pixels, 60, 60, 255, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 49, 60, 0, 0, 255 ) );
        window->UnregisterOverlay( &textured );
        Ogre::TextureManager::getSingleton().remove( textured.m_texture );
        textured.m_texture.reset();

        renderWindow->removeViewport( viewport->getZOrder() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pixels: a window without viewports is cleared to black under its overlays" )
    try
    {
        CQuadOverlay quads( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_STRAIGHT, 0 );
        ASSERT_TRUE( quads.m_material.Initialize() );
        // straight alpha: the colour is weighted by its alpha
        quads.m_batch.AddQuad( 0.0f, 0.0f, 32.0f, 32.0f, 255, 255, 255, 128 );
        ASSERT_TRUE( quads.m_batch.Build() );
        window->RegisterOverlay( &quads );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
        ASSERT_TRUE( PixelIs( pixels, 100, 100, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 10, 10, 128, 128, 128 ) );
        window->UnregisterOverlay( &quads );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Pixels: a texture loaded through the GucefVFS archive and decoded by gucefIMAGE through OgreCodec_GucefIMAGE" )
    if ( !Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefVFS" ) || !Ogre::Codec::isCodecRegistered( "ogwtex" ) )
        GUCEF_TESTFW_SKIP( "OgrePlugin_GucefVFS or OgreCodec_GucefIMAGE is not loaded" );
    try
    {
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "guidriverOgre_TestApp_Textures" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( dir ) );
        {
            CORE::CFileAccess file( CORE::CombinePath( dir, "magenta.ogwtex" ), "wb" );
            const UInt8 data[ 8 ] = { 'O', 'G', 'W', '1', 255, 0, 255, 255 };
            ASSERT_TRUE( file.IsValid() && 1 == file.Write( data, 8, 1 ) );
        }
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( dir, "guidriverOgreTestTextures", false, false, "/ogretest" ) );

        Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
        groups.createResourceGroup( "OgreWindowTest", false );
        groups.addResourceLocation( "/ogretest", "GucefVFS", "OgreWindowTest", false, true );
        groups.initialiseResourceGroup( "OgreWindowTest" );
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().load( "magenta.ogwtex", "OgreWindowTest" );
        ASSERT_TRUE( texture && 4 == texture->getWidth() );

        CQuadOverlay textured( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_PREMULTIPLIED, 0 );
        ASSERT_TRUE( textured.m_material.Initialize() );
        textured.m_batch.AddQuad( 20.0f, 20.0f, 40.0f, 40.0f, 255, 255, 255, 255 );
        ASSERT_TRUE( textured.m_batch.Build() );
        textured.m_texture = texture;
        window->RegisterOverlay( &textured );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
        ASSERT_TRUE( PixelIs( pixels, 30, 30, 255, 0, 255 ) );
        window->UnregisterOverlay( &textured );
        textured.m_texture.reset();
        texture.reset();
        groups.destroyResourceGroup( "OgreWindowTest" );
        vfs.UnmountArchiveByName( "guidriverOgreTestTextures" );
        CORE::DeleteDir( dir, true );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "guidriverOgre_TestApp: " << e.getFullDescription() );
        ERRORHERE;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Capture: an overlay drawn alone into a render texture, on opaque or transparent black" )
    try
    {
        CQuadOverlay quads( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_PREMULTIPLIED, 0 );
        ASSERT_TRUE( quads.m_material.Initialize() );
        quads.m_batch.AddQuad( 2.0f, 3.0f, 10.0f, 6.0f, 0, 255, 0, 255 );
        ASSERT_TRUE( quads.m_batch.Build() );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->CaptureOverlay( quads, 40, 30, true, pixels ) );
        ASSERT_TRUE( !pixels.IsNULL() && 40 == pixels->GetWidthInPixels() && 30 == pixels->GetHeightInPixels() );
        UInt8 rgba[ 4 ];
        GetPixel( pixels, 0, 0, rgba );
        ASSERT_TRUE( 0 == rgba[ 0 ] && 0 == rgba[ 1 ] && 0 == rgba[ 2 ] && 255 == rgba[ 3 ] );
        // the top row of the quad is the third row of the picture: rows from the top, as in a window
        ASSERT_TRUE( PixelIs( pixels, 2, 3, 0, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 11, 8, 0, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 12, 8, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 2, 2, 0, 0, 0 ) );
        ASSERT_TRUE( window->CaptureOverlay( quads, 40, 30, false, pixels ) );
        GetPixel( pixels, 0, 0, rgba );
        ASSERT_TRUE( 0 == rgba[ 3 ] );
        GetPixel( pixels, 5, 5, rgba );
        ASSERT_TRUE( 255 == rgba[ 3 ] );
        ASSERT_TRUE( 2 == quads.m_drawn );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Video settings: vsync is applied to the render window and reported back with the size and anti aliasing" )
    try
    {
        GUI::TWindowContextPtr windowPtr = GetMainTestWindow();
        GUI::CVideoSettings settings = window->GetVideoSettings();
        ASSERT_TRUE( 320 == settings.GetResolutionWidthInPixels() && 240 == settings.GetResolutionHeightInPixels() );
        ASSERT_FALSE( settings.GetVSyncState() );
        settings.SetVSyncState( true );
        ASSERT_TRUE( backend->ApplyVideoSettings( windowPtr, settings ) );
        // on Wayland the frame callbacks of the platform window are the vertical sync: Ogre's swap does not wait
        if ( TESTWINDOWMODE_WAYLAND == mode )
            ASSERT_FALSE( renderWindow->isVSyncEnabled() );
        else
            ASSERT_TRUE( renderWindow->isVSyncEnabled() );
        ASSERT_TRUE( window->GetVideoSettings().GetVSyncState() );
        ASSERT_TRUE( window->GetVideoSettings().GetAntiAliasingFactor() == renderWindow->getFSAA() );
        settings.SetVSyncState( false );
        ASSERT_TRUE( backend->ApplyVideoSettings( windowPtr, settings ) );
        ASSERT_FALSE( renderWindow->isVSyncEnabled() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Platform window: its redraws pace the frames, its size changes resize the render window, its close requests are passed on" )
    if ( TESTWINDOWMODE_X11 != mode && TESTWINDOWMODE_WAYLAND != mode )
        GUCEF_TESTFW_SKIP( "the window has no platform window" );
    try
    {
        GUI::TWindowContextPtr windowPtr = GetMainTestWindow();
        CFrameRecorder recorder( 0 );
        recorder.SubscribeTo( window );

        // the redraws of the platform window come with the pulses of the application (on Wayland when the compositor showed the frame before)
        const UInt64 frames = window->GetFrameCount();
        ASSERT_TRUE( PulseUntil( SFrameCountAbove( window, frames ) ) );

        // a new size through the platform window, the render window (Ogre's child window) follows
        GUI::CVideoSettings settings = window->GetVideoSettings();
        settings.SetResolution( 400, 300, 32 );
        ASSERT_TRUE( backend->ApplyVideoSettings( windowPtr, settings ) );
        ASSERT_TRUE( PulseUntil( SRenderWindowWidth( window, 400 ) ) );
        ASSERT_TRUE( 400 == window->GetWidth() && 300 == window->GetHeight() );
        ASSERT_TRUE( 300 == renderWindow->getHeight() );
        ASSERT_TRUE( SHasStep( recorder, "size" )() );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) && 400 == pixels->GetWidthInPixels() );

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        // the window manager's close request: WM_DELETE_WINDOW to the platform window
        if ( TESTWINDOWMODE_X11 == mode )
        {
            ::Display* display = (::Display*) CORE::StringToPointer( window->GetProperty( "DISPLAY" ) );
            const ::Window xwindow = (::Window) CORE::StringToUInt32( window->GetProperty( "WINDOW" ), 0 );
            ASSERT_TRUE( GUCEF_NULL != display && 0 != xwindow );
            if ( GUCEF_NULL != display && 0 != xwindow )
            {
                ::XEvent event;
                memset( &event, 0, sizeof( event ) );
                event.xclient.type = ClientMessage;
                event.xclient.window = xwindow;
                event.xclient.message_type = ::XInternAtom( display, "WM_PROTOCOLS", False );
                event.xclient.format = 32;
                event.xclient.data.l[ 0 ] = (long) ::XInternAtom( display, "WM_DELETE_WINDOW", False );
                event.xclient.data.l[ 1 ] = CurrentTime;
                ::XSendEvent( display, xwindow, False, NoEventMask, &event );
                ::XFlush( display );
                ASSERT_TRUE( PulseUntil( SHasStep( recorder, "closeRequest" ) ) );
            }
        }
        #endif

        settings.SetResolution( 320, 240, 32 );
        ASSERT_TRUE( backend->ApplyVideoSettings( windowPtr, settings ) );
        ASSERT_TRUE( PulseUntil( SRenderWindowWidth( window, 320 ) ) );
        recorder.UnsubscribeAllFromObserver();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Windows: a second Ogre window draws and goes while the first stays" )
    try
    {
        GUI::TWindowContextPtr secondPtr = MakeTestOgreWindow( 100, 80 );
        GUIDRIVEROGRE::COgreWindowContext* second = AsOgreWindow( secondPtr );
        ASSERT_TRUE( GUCEF_NULL != second );
        if ( GUCEF_NULL != second )
        {
            ASSERT_TRUE( second->GetOgreRenderWindow() != renderWindow );
            Ogre::Viewport* viewport = AddColourViewport( second->GetOgreRenderWindow(), "Second", Ogre::ColourValue( 1.0f, 1.0f, 0.0f, 1.0f ) );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( second->RenderFrame( &pixels ) );
            ASSERT_TRUE( !pixels.IsNULL() && 100 == pixels->GetWidthInPixels() );
            ASSERT_TRUE( PixelIs( pixels, 50, 40, 255, 255, 0 ) );
            second->GetOgreRenderWindow()->removeViewport( viewport->getZOrder() );
            backend->DestroyWindowContext( secondPtr );
            ASSERT_TRUE( GUCEF_NULL == second->GetOgreRenderWindow() );
        }
        // the first one still draws
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // Ogre's work queue on the GUCEF thread pool, with this window
    PerformOgreWorkQueueWindowTests( window );

    GUCEF_TESTFW_TESTCASE( "Shutdown: the window goes, then the plugins and the Root" )
    try
    {
        ReleaseMainTestWindow();
        TearDownOgreForTests();
        ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
        ASSERT_TRUE( GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

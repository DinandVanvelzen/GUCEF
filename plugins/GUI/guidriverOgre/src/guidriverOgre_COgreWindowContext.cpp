/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#include <stdlib.h>

#include <Ogre.h>
#include <OgreRenderWindow.h>
#include <OgreHardwarePixelBuffer.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGER_H
#include "gucefGUI_CWindowManager.h"
#define GUCEF_GUI_CWINDOWMANAGER_H
#endif /* GUCEF_GUI_CWINDOWMANAGER_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#ifndef GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H
#include "guidriverOgre_COgrePlatformWindowSupport.h"
#define GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H
#endif /* GUIDRIVEROGRE_COGREPLATFORMWINDOWSUPPORT_H ? */

#include "guidriverOgre_COgreWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COgreWindowContext::ClassTypeName = "GUCEF::GUIDRIVEROGRE::COgreWindowContext";
// the same text as COgrePlatformWindowSupport::PlatformBackendParameterName and OffscreenBackend; literals, because the
// order in which the constants of two source files are made is not defined
const CORE::CString COgreWindowContext::PlatformBackendParameterName = "PlatformBackend";
const CORE::CString COgreWindowContext::OffscreenPlatformBackend = "Offscreen";
const CORE::CString COgreWindowContext::GraphicsApiName = "Ogre";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Every render window of Ogre needs a name of its own */
UInt32 g_windowCounter = 0;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWindowContext::COgreWindowContext( void )
    : GUI::CWindowContext()
    , Ogre::RenderTargetListener()
    , m_guiContext()
    , m_id( ++g_windowCounter )
    , m_name()
    , m_platformBackendName()
    , m_platformBackend()
    , m_platformWindow()
    , m_renderWindow( GUCEF_NULL )
    , m_overlayViewport( GUCEF_NULL )
    , m_captureTexture()
    , m_captureViewport( GUCEF_NULL )
    , m_overlays()
    , m_overlaySequence( 0 )
    , m_frameCount( 0 )
    , m_inFrame( false )
    , m_videoSettings()
    , m_surfaceState()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWindowContext::~COgreWindowContext()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreWindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
COgreWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
COgreWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

GUI::CString
COgreWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return !m_platformWindow.IsNULL() && m_platformWindow->IsActive();
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
COgreWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    if ( !m_platformWindow.IsNULL() )
        return m_platformWindow->GetWidth();
    return GUCEF_NULL != m_renderWindow ? (GUI::UInt32) m_renderWindow->getWidth() : 0;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
COgreWindowContext::GetHeight( void ) const
{GUCEF_TRACE;

    if ( !m_platformWindow.IsNULL() )
        return m_platformWindow->GetHeight();
    return GUCEF_NULL != m_renderWindow ? (GUI::UInt32) m_renderWindow->getHeight() : 0;
}

/*-------------------------------------------------------------------------*/

GUI::CString
COgreWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "GRAPHICS_API" == propertyName )
        return GraphicsApiName;
    if ( propertyName.StartsWith( "GL_" ) )
        return GUI::CString::Empty;
    if ( "PLATFORM_BACKEND" == propertyName )
        return m_platformBackendName;
    if ( "Ogre::RenderWindow" == propertyName )
        return CORE::PointerToString( m_renderWindow );
    if ( "Ogre::SceneManager" == propertyName )
    {
        // the scene manager of the camera of the last viewport of the window, the one the overlays draw over
        Ogre::SceneManager* sceneManager = GUCEF_NULL;
        if ( GUCEF_NULL != m_renderWindow && m_renderWindow->getNumViewports() > 0 )
        {
            Ogre::Viewport* last = m_renderWindow->getViewport( m_renderWindow->getNumViewports() - 1 );
            if ( GUCEF_NULL != last && GUCEF_NULL != last->getCamera() )
                sceneManager = last->getCamera()->getSceneManager();
        }
        return CORE::PointerToString( sceneManager );
    }
    if ( !m_platformWindow.IsNULL() )
        return m_platformWindow->GetProperty( propertyName );
    return GUI::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::SetSystemCursorVisible( bool visible )
{GUCEF_TRACE;

    if ( m_platformWindow.IsNULL() )
        return false;
    return m_platformWindow->SetSystemCursorVisible( visible );
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::SetSystemCursorShape( GUI::TCursorShape shape )
{GUCEF_TRACE;

    if ( m_platformWindow.IsNULL() )
        return false;
    return m_platformWindow->SetSystemCursorShape( shape );
}

/*-------------------------------------------------------------------------*/

Ogre::RenderWindow*
COgreWindowContext::GetOgreRenderWindow( void ) const
{GUCEF_TRACE;

    return m_renderWindow;
}

/*-------------------------------------------------------------------------*/

Ogre::Root*
COgreWindowContext::GetOgreRoot( void ) const
{GUCEF_TRACE;

    return Ogre::Root::getSingletonPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowContext::GetPlatformWindowContext( void ) const
{GUCEF_TRACE;

    return m_platformWindow;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
COgreWindowContext::GetPlatformBackendName( void ) const
{GUCEF_TRACE;

    return m_platformBackendName;
}

/*-------------------------------------------------------------------------*/

UInt64
COgreWindowContext::GetFrameCount( void ) const
{GUCEF_TRACE;

    return m_frameCount;
}

/*-------------------------------------------------------------------------*/

const COgreSurfaceState&
COgreWindowContext::GetSurfaceState( void ) const
{GUCEF_TRACE;

    return m_surfaceState;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::Initialize( const GUI::CString& title                ,
                                const GUI::CVideoSettings& videoSettings ,
                                const CORE::CValueList* params           )
{GUCEF_TRACE;

    Shutdown();

    m_name = title;
    m_videoSettings = videoSettings;
    m_platformBackendName = COgrePlatformWindowSupport::SelectPlatformBackend( params, COgrePlatformWindowSupport::GetDefaultPlatformBackend() );
    if ( !COgrePlatformWindowSupport::IsKnownPlatformBackend( m_platformBackendName ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: Ogre render windows on platform windows of the backend \""
            << m_platformBackendName << "\" are not supported" );
        return false;
    }

    // the Root of the support library, initialised with the render system of its configuration
    PLUGINGLUE::OGRE::COgreRootSetup& rootSetup = PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup();
    if ( !rootSetup.CreateRoot() || !rootSetup.InitialiseRoot() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: There is no initialised Ogre Root with a render system, no window is made" );
        return false;
    }

    if ( OffscreenPlatformBackend == m_platformBackendName )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        // with a display server Ogre's EGL support makes a top level X11 window of its own instead of a headless surface
        const char* display = ::getenv( "DISPLAY" );
        if ( GUCEF_NULL != display && '\0' != display[ 0 ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: An offscreen Ogre window needs a process without an X11 display "
                "(DISPLAY is " << display << "): Ogre would show a window of its own. Use the platform backend XWinGL" );
            return false;
        }
        #endif
    }
    else
    if ( !CreatePlatformWindow( title, videoSettings, params ) )
    {
        Shutdown();
        return false;
    }

    if ( !CreateRenderWindow( title, videoSettings ) )
    {
        Shutdown();
        return false;
    }

    const UInt32 pulseInterval = COgrePlatformWindowSupport::GetPulseIntervalInMs( videoSettings.GetFrequency() );
    if ( m_platformWindow.IsNULL() && pulseInterval > 0 )
    {
        // without a platform window the window paces its own frames
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestPeriodicPulses( this, pulseInterval );
        TEventCallback pulse( this, &COgreWindowContext::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: Made the Ogre render window \"" << m_renderWindow->getName() << "\" of "
        << m_renderWindow->getWidth() << " x " << m_renderWindow->getHeight() << " on the platform backend " << m_platformBackendName
        << " with " << GetOgreRoot()->getRenderSystem()->getName() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::CreatePlatformWindow( const GUI::CString& title                ,
                                          const GUI::CVideoSettings& videoSettings ,
                                          const CORE::CValueList* params           )
{GUCEF_TRACE;

    m_platformBackend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( m_platformBackendName );
    if ( m_platformBackend.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The platform window backend \"" << m_platformBackendName
            << "\" is not there: load its window driver plugin first" );
        return false;
    }
    if ( !m_platformBackend->IsGraphicsApiSupported( GUI::CWindowManagerBackend::GraphicsApiNone ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The platform window backend \"" << m_platformBackendName
            << "\" cannot make windows without a graphics context (GraphicsApi None), which an Ogre render window needs" );
        return false;
    }

    // the window without a GL context of its own: Ogre makes the surface
    CORE::CValueList platformParams;
    if ( GUCEF_NULL != params )
        platformParams = *params;
    platformParams.Delete( PlatformBackendParameterName );
    platformParams.Set( GUI::CWindowManagerBackend::GraphicsApiParameterName, GUI::CWindowManagerBackend::GraphicsApiNone );
    m_platformWindow = m_platformBackend->CreateWindowContext( title, videoSettings, &platformParams );
    if ( m_platformWindow.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The platform window backend \"" << m_platformBackendName << "\" made no window" );
        return false;
    }

    TEventCallback redraw( this, &COgreWindowContext::OnPlatformRedraw );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextRedrawEvent, redraw );
    TEventCallback size( this, &COgreWindowContext::OnPlatformSize );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextSizeEvent, size );
    TEventCallback activate( this, &COgreWindowContext::OnPlatformActivate );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextActivateEvent, activate );
    TEventCallback closeRequest( this, &COgreWindowContext::OnPlatformCloseRequest );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextCloseRequestEvent, closeRequest );
    TEventCallback surfaceLost( this, &COgreWindowContext::OnPlatformSurfaceLost );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextSurfaceLostEvent, surfaceLost );
    TEventCallback surfaceRestored( this, &COgreWindowContext::OnPlatformSurfaceRestored );
    SubscribeTo( m_platformWindow.GetPointerAlways(), GUI::CWindowContext::WindowContextSurfaceRestoredEvent, surfaceRestored );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::CreateRenderWindow( const GUI::CString& title                ,
                                        const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    Ogre::Root* root = GetOgreRoot();
    Ogre::NameValuePairList miscParams;
    COgrePlatformWindowSupport::AddVideoSettingParams( m_platformBackendName, videoSettings, title, miscParams );

    UInt32 width = videoSettings.GetResolutionWidthInPixels();
    UInt32 height = videoSettings.GetResolutionHeightInPixels();
    bool fullscreen = false;
    if ( !m_platformWindow.IsNULL() )
    {
        // the native handles of the platform window, as its properties give them
        CORE::CString problem;
        if ( !COgrePlatformWindowSupport::AddNativeWindowParams( m_platformBackendName, *m_platformWindow, miscParams, problem ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: No render window can be made: " << problem );
            return false;
        }
        width = m_platformWindow->GetWidth();
        height = m_platformWindow->GetHeight();
    }
    if ( 0 == width || 0 == height )
    {
        width = width > 0 ? width : 1;
        height = height > 0 ? height : 1;
    }

    try
    {
        const Ogre::String name = ( title + " #" + CORE::UInt32ToString( m_id ) ).C_String();
        m_renderWindow = root->createRenderWindow( name, width, height, fullscreen, &miscParams );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: Ogre made no render window: " << e.getFullDescription() );
        m_renderWindow = GUCEF_NULL;
        return false;
    }
    if ( GUCEF_NULL == m_renderWindow )
        return false;

    // the window is drawn by this context in the order of its frames, never by Root::renderOneFrame()
    m_renderWindow->setAutoUpdated( false );
    m_renderWindow->addListener( this );
    m_overlayViewport = OGRE_NEW Ogre::Viewport( GUCEF_NULL, m_renderWindow, 0.0f, 0.0f, 1.0f, 1.0f, 0 );
    m_surfaceState.OnRenderWindowCreated();
    return true;
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::Shutdown( void )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestStopOfPeriodicUpdates( this );
    UnsubscribeFrom( pulseGenerator.GetPointerAlways() );

    Ogre::Root* root = GetOgreRoot();
    if ( GUCEF_NULL != root )
    {
        if ( !m_captureTexture.isNull() )
        {
            Ogre::RenderTarget* target = m_captureTexture->getBuffer()->getRenderTarget();
            if ( GUCEF_NULL != target )
                target->removeAllViewports();
            Ogre::TextureManager::getSingleton().remove( m_captureTexture );
            // the last reference goes while the GL context of the render window is still there
            m_captureTexture.reset();
        }
        if ( GUCEF_NULL != m_overlayViewport )
            OGRE_DELETE m_overlayViewport;
        if ( GUCEF_NULL != m_renderWindow )
        {
            m_renderWindow->removeListener( this );
            root->destroyRenderTarget( m_renderWindow );
        }
    }
    m_captureTexture.reset();
    m_captureViewport = GUCEF_NULL;
    m_overlayViewport = GUCEF_NULL;
    m_renderWindow = GUCEF_NULL;
    m_surfaceState.OnRenderWindowDestroyed();

    if ( !m_platformWindow.IsNULL() )
    {
        UnsubscribeFrom( m_platformWindow.GetPointerAlways() );
        if ( !m_platformBackend.IsNULL() )
            m_platformBackend->DestroyWindowContext( m_platformWindow );
    }
    m_platformWindow.Unlink();
    m_platformBackend.Unlink();
    m_frameCount = 0;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::ApplyVideoSettings( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderWindow )
        return false;
    bool success = true;
    if ( !m_platformWindow.IsNULL() )
        success = m_platformBackend->ApplyVideoSettings( m_platformWindow, videoSettings );
    else
        m_renderWindow->resize( videoSettings.GetResolutionWidthInPixels(), videoSettings.GetResolutionHeightInPixels() );
    // on Wayland the frame callbacks of the platform window are the vertical sync, Ogre's swap does not wait
    m_renderWindow->setVSyncEnabled( videoSettings.GetVSyncState() && COgrePlatformWindowSupport::WaylandBackend != m_platformBackendName );
    if ( videoSettings.GetAntiAliasingFactor() != m_videoSettings.GetAntiAliasingFactor() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The anti aliasing of a render window is chosen when it is made, "
            "the new factor applies to windows made from now on" );
    }
    if ( m_platformWindow.IsNULL() && videoSettings.GetFrequency() != m_videoSettings.GetFrequency() )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        const UInt32 pulseInterval = COgrePlatformWindowSupport::GetPulseIntervalInMs( videoSettings.GetFrequency() );
        if ( pulseInterval > 0 )
            pulseGenerator->RequestPeriodicPulses( this, pulseInterval );
    }
    m_videoSettings = videoSettings;
    return success;
}

/*-------------------------------------------------------------------------*/

GUI::CVideoSettings
COgreWindowContext::GetVideoSettings( void ) const
{GUCEF_TRACE;

    GUI::CVideoSettings settings( m_videoSettings );
    if ( GUCEF_NULL != m_renderWindow )
    {
        settings.SetResolution( GetWidth(), GetHeight(), m_videoSettings.GetResolutionDepthInBits() );
        const bool platformVSync = COgrePlatformWindowSupport::WaylandBackend == m_platformBackendName;
        settings.SetVSyncState( platformVSync ? m_videoSettings.GetVSyncState() : m_renderWindow->isVSyncEnabled() );
        settings.SetAntiAliasingFactor( m_renderWindow->getFSAA() );
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::RegisterOverlay( CIOgreWindowOverlay* overlay )
{GUCEF_TRACE;

    if ( GUCEF_NULL == overlay )
        return;
    for ( TOverlayVector::iterator i=m_overlays.begin(); i!=m_overlays.end(); ++i )
        if ( (*i).overlay == overlay )
            return;
    SOverlayEntry entry;
    entry.overlay = overlay;
    entry.sequence = ++m_overlaySequence;
    m_overlays.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::UnregisterOverlay( CIOgreWindowOverlay* overlay )
{GUCEF_TRACE;

    for ( TOverlayVector::iterator i=m_overlays.begin(); i!=m_overlays.end(); ++i )
    {
        if ( (*i).overlay == overlay )
        {
            m_overlays.erase( i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
COgreWindowContext::GetOverlayCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_overlays.size();
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::SortOverlays( void )
{GUCEF_TRACE;

    // insertion sort on order, then sequence: few overlays, and their order seldom changes
    for ( size_t i=1; i<m_overlays.size(); ++i )
    {
        SOverlayEntry entry = m_overlays[ i ];
        const Int32 order = entry.overlay->GetOverlayOrder();
        size_t j = i;
        while ( j > 0 )
        {
            const SOverlayEntry& previous = m_overlays[ j-1 ];
            const Int32 previousOrder = previous.overlay->GetOverlayOrder();
            if ( previousOrder < order || ( previousOrder == order && previous.sequence < entry.sequence ) )
                break;
            m_overlays[ j ] = previous;
            --j;
        }
        m_overlays[ j ] = entry;
    }
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::DrawOverlays( Ogre::Viewport& viewport, Ogre::SceneManager* sceneManager, bool isCapture )
{GUCEF_TRACE;

    SortOverlays();
    Ogre::RenderSystem* renderSystem = GetOgreRoot()->getRenderSystem();
    // a copy: an overlay may unregister while it draws
    TOverlayVector overlays( m_overlays );
    for ( TOverlayVector::iterator i=overlays.begin(); i!=overlays.end(); ++i )
    {
        try
        {
            (*i).overlay->RenderOverlay( *renderSystem, viewport, sceneManager, isCapture );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: An overlay failed to draw: " << e.getFullDescription() );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::postRenderTargetUpdate( const Ogre::RenderTargetEvent& evt )
{GUCEF_TRACE;

    if ( evt.source != m_renderWindow || GUCEF_NULL == m_overlayViewport )
        return;

    Ogre::RenderSystem* renderSystem = GetOgreRoot()->getRenderSystem();
    Ogre::SceneManager* sceneManager = GUCEF_NULL;
    const unsigned short viewportCount = m_renderWindow->getNumViewports();
    if ( viewportCount > 0 )
    {
        Ogre::Viewport* last = m_renderWindow->getViewport( viewportCount - 1 );
        if ( GUCEF_NULL != last && GUCEF_NULL != last->getCamera() )
            sceneManager = last->getCamera()->getSceneManager();
    }

    m_overlayViewport->_updateDimensions();
    renderSystem->_setViewport( m_overlayViewport );
    renderSystem->_beginFrame();
    if ( 0 == viewportCount )
    {
        // nothing of the application drew in the window: the overlays draw on black
        renderSystem->clearFrameBuffer( Ogre::FBT_COLOUR | Ogre::FBT_DEPTH | Ogre::FBT_STENCIL, Ogre::ColourValue::Black );
    }
    DrawOverlays( *m_overlayViewport, sceneManager, false );
    renderSystem->_endFrame();
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::RenderFrame( IMAGE::TPixelMapPtr* capture )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderWindow || m_inFrame || !m_surfaceState.CanDraw() )
        return false;
    Ogre::Root* root = GetOgreRoot();
    if ( GUCEF_NULL == root || GUCEF_NULL == root->getRenderSystem() )
        return false;

    m_inFrame = true;
    bool success = true;
    try
    {
        // 1. the frame listeners of the application
        root->_fireFrameStarted();

        // 2. the GUI contexts of the window update and prepare
        NotifyObservers( WindowContextRedrawEvent );

        if ( GUCEF_NULL != m_renderWindow )
        {
            // 3. the viewports of the application, then the overlays (postRenderTargetUpdate)
            m_renderWindow->update( false );
            if ( GUCEF_NULL != capture )
                success = ReadPixels( *m_renderWindow, (UInt32) m_renderWindow->getWidth(), (UInt32) m_renderWindow->getHeight(), *capture );

            // 4. present
            root->_fireFrameRenderingQueued();
            m_renderWindow->swapBuffers();
            const Ogre::SceneManagerInstanceMap& sceneManagers = root->getSceneManagers();
            for ( Ogre::SceneManagerInstanceMap::const_iterator i=sceneManagers.begin(); i!=sceneManagers.end(); ++i )
                (*i).second->_handleLodEvents();
        }
        else
        {
            success = false;
        }

        // 5. the end of the frame, with the main thread tasks of Ogre's work queue
        root->_fireFrameEnded();
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The frame failed: " << e.getFullDescription() );
        success = false;
    }
    ++m_frameCount;
    m_inFrame = false;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::ReadPixels( Ogre::RenderTarget& target, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height )
        return false;
    if ( pixels.IsNULL() || pixels->GetWidthInPixels() != width || pixels->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }
    try
    {
        // PF_BYTE_RGBA: the bytes R, G, B, A in memory, as the pixel map has them; Ogre gives the rows from the top.
        // The OpenGL render windows read through their first viewport: a window without viewports gets one for the
        // reading, which draws nothing
        Ogre::Viewport* readViewport = GUCEF_NULL;
        if ( 0 == target.getNumViewports() )
            readViewport = target.addViewport( GUCEF_NULL );
        Ogre::PixelBox box( width, height, 1, Ogre::PF_BYTE_RGBA, pixels->GetDataPtr() );
        target.copyContentsToMemory( Ogre::Box( 0, 0, width, height ), box, Ogre::RenderTarget::FB_AUTO );
        if ( GUCEF_NULL != readViewport )
            target.removeViewport( readViewport->getZOrder() );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The picture could not be read back: " << e.getFullDescription() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowContext::CaptureOverlay( CIOgreWindowOverlay& overlay ,
                                    UInt32 width                 ,
                                    UInt32 height                ,
                                    bool opaque                  ,
                                    IMAGE::TPixelMapPtr& pixels  )
{GUCEF_TRACE;

    Ogre::Root* root = GetOgreRoot();
    if ( GUCEF_NULL == m_renderWindow || GUCEF_NULL == root || 0 == width || 0 == height )
        return false;
    Ogre::RenderSystem* renderSystem = root->getRenderSystem();
    try
    {
        if ( m_captureTexture.isNull() || m_captureTexture->getWidth() != width || m_captureTexture->getHeight() != height )
        {
            if ( !m_captureTexture.isNull() )
            {
                m_captureTexture->getBuffer()->getRenderTarget()->removeAllViewports();
                Ogre::TextureManager::getSingleton().remove( m_captureTexture );
                m_captureTexture.reset();
                m_captureViewport = GUCEF_NULL;
            }
            const Ogre::String name = "GucefOgreWindow/Capture/" + Ogre::StringConverter::toString( m_id );
            m_captureTexture = Ogre::TextureManager::getSingleton().createManual( name, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D, width, height, 0,
                                                                                   Ogre::PF_BYTE_RGBA, Ogre::TU_RENDERTARGET );
            Ogre::RenderTarget* target = m_captureTexture->getBuffer()->getRenderTarget();
            target->setAutoUpdated( false );
            m_captureViewport = target->addViewport( GUCEF_NULL );
            m_captureViewport->setClearEveryFrame( false );
        }
        Ogre::RenderTarget* target = m_captureTexture->getBuffer()->getRenderTarget();
        renderSystem->_setViewport( m_captureViewport );
        renderSystem->_beginFrame();
        renderSystem->clearFrameBuffer( Ogre::FBT_COLOUR | Ogre::FBT_DEPTH | Ogre::FBT_STENCIL, Ogre::ColourValue( 0.0f, 0.0f, 0.0f, opaque ? 1.0f : 0.0f ) );
        overlay.RenderOverlay( *renderSystem, *m_captureViewport, GUCEF_NULL, true );
        renderSystem->_endFrame();
        return ReadPixels( *target, width, height, pixels );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The overlay could not be captured: " << e.getFullDescription() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformRedraw( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RenderFrame( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RenderFrame( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformSize( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_renderWindow && !m_platformWindow.IsNULL() && m_surfaceState.CanDraw() )
    {
        if ( COgrePlatformWindowSupport::RESIZEMETHOD_FROM_NATIVE == COgrePlatformWindowSupport::GetResizeMethod( m_platformBackendName ) )
        {
            // Ogre reads the size of the native window it was given
            m_renderWindow->windowMovedOrResized();
        }
        else
        {
            const UInt32 width = m_platformWindow->GetWidth();
            const UInt32 height = m_platformWindow->GetHeight();
            if ( width > 0 && height > 0 )
                m_renderWindow->resize( width, height );
        }
    }
    NotifyObservers( WindowContextSizeEvent );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformSurfaceLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( COgreSurfaceState::SURFACEACTION_RELEASE_SURFACE != m_surfaceState.OnSurfaceLost() )
        return;
    if ( !COgrePlatformWindowSupport::CanLoseSurface( m_platformBackendName ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The platform window of backend " << m_platformBackendName
            << " lost its surface, which Ogre cannot be told on this platform: nothing is drawn until it comes back" );
        return;
    }
    #if ( OGRE_PLATFORM == OGRE_PLATFORM_ANDROID ) || ( OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN )
    try
    {
        // while the native window still exists: Ogre lets go of its EGL surface, the context stays
        m_renderWindow->_notifySurfaceDestroyed();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The native surface of \"" << m_name << "\" is gone, no frames are drawn until it comes back" );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: Ogre could not let go of the surface: " << e.getFullDescription() );
    }
    #else
    // the render windows of Ogre are told of a lost surface only in the Android and web builds of Ogre
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: The render window cannot be told that its surface is gone on this platform" );
    #endif
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformSurfaceRestored( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_platformWindow.IsNULL() )
        return;
    void* nativeWindow = CORE::StringToPointer( m_platformWindow->GetProperty( "WINDOW" ) );
    if ( COgreSurfaceState::SURFACEACTION_ADOPT_SURFACE != m_surfaceState.OnSurfaceRestored( GUCEF_NULL != nativeWindow ) )
        return;
    if ( !COgrePlatformWindowSupport::CanLoseSurface( m_platformBackendName ) )
        return;
    #if ( OGRE_PLATFORM == OGRE_PLATFORM_ANDROID ) || ( OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN )
    try
    {
        void* config = CORE::StringToPointer( m_platformWindow->GetProperty( "ANDROID_CONFIG" ) );
        m_renderWindow->_notifySurfaceCreated( nativeWindow, config );
        m_renderWindow->windowMovedOrResized();
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: \"" << m_name << "\" draws on a new native surface of "
            << m_renderWindow->getWidth() << " x " << m_renderWindow->getHeight() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OgreWindowContext: Ogre could not take the new surface: " << e.getFullDescription() );
    }
    #endif
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformActivate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( WindowContextActivateEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowContext::OnPlatformCloseRequest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( WindowContextCloseRequestEvent, eventData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

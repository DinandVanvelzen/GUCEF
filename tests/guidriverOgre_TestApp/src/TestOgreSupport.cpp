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

#include <stdlib.h>
#include <string.h>

#if ( defined( __linux__ ) )
#include <dirent.h>
#endif

#include <Ogre.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwarePixelBuffer.h>

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* GUCEF_PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */


#include "TestOgreSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::TWindowContextPtr g_mainWindow;
const char* const GuiPluginGroup = "guidriverOgreTestGuiPlugins";
const char* const OgrePluginGroup = "guidriverOgreTestOgrePlugins";

bool
EnvIs( const char* name, char value )
{
    const char* text = ::getenv( name );
    return GUCEF_NULL != text && value == text[ 0 ];
}

bool
HasWaylandDisplay( void )
{
    const char* display = ::getenv( "WAYLAND_DISPLAY" );
    return GUCEF_NULL != display && '\0' != display[ 0 ];
}

bool
HasDisplay( void )
{
    const char* display = ::getenv( "DISPLAY" );
    return GUCEF_NULL != display && '\0' != display[ 0 ];
}

void
AddOgrePluginEntry( CORE::CDataNode& group, const CORE::CString& pluginName, bool loadFailAllowed )
{
    CORE::CDataNode* entry = group.AddChild( "PluginMetaData" );
    entry->SetAttribute( "Description", pluginName );
    entry->SetAttribute( "Path", "$MODULEDIR$" );
    entry->SetAttribute( "Filename", pluginName );
    entry->SetAttribute( "AltFilename", pluginName + "_d" );
    entry->SetAttribute( "Type", "OgrePlugin" );
    entry->SetAttribute( "LoadFailAllowed", loadFailAllowed );
}

}

/*-------------------------------------------------------------------------*/

ETestWindowMode
GetTestWindowMode( void )
{GUCEF_TRACE;

    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return TESTWINDOWMODE_NONE;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( HasWaylandDisplay() && EnvIs( "GUCEF_TEST_WAYLAND_WINDOWS", '1' ) )
        return TESTWINDOWMODE_WAYLAND;
    if ( HasDisplay() )
        return EnvIs( "GUCEF_TEST_X11_WINDOWS", '1' ) ? TESTWINDOWMODE_X11 : TESTWINDOWMODE_NONE;
    return TESTWINDOWMODE_OFFSCREEN;
    #else
    return TESTWINDOWMODE_NONE;
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetNoWindowReason( void )
{GUCEF_TRACE;

    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return "GUCEF_TEST_RENDER_GPU=0 skips the rendering tests";
    if ( HasDisplay() )
        return "windows on the X server of DISPLAY are made only with GUCEF_TEST_X11_WINDOWS=1 (a server for tests such as Xvfb)";
    return "the platform has no window mode for these tests";
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetTestRenderer( void )
{GUCEF_TRACE;

    // Ogre's headless surface is one of EGL: offscreen windows are OpenGL ones
    const char* renderer = ::getenv( "GUCEF_TEST_OGRE_RENDERER" );
    if ( GUCEF_NULL == renderer || '\0' == renderer[ 0 ] || TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() )
        return "OpenGL";
    return renderer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetExpectedTestRenderer( void )
{GUCEF_TRACE;

    CORE::CString::StringVector renderers;
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().GetAvailableRenderers( renderers );
    const CORE::CString asked = GetTestRenderer();
    for ( UInt32 i=0; i<(UInt32)renderers.size(); ++i )
        if ( renderers[ i ].Equals( asked, false ) )
            return renderers[ i ];
    return renderers.empty() ? CORE::CString::Empty : renderers.front();
}

/*-------------------------------------------------------------------------*/

bool
SetUpOgreForTests( void )
{GUCEF_TRACE;

    // the systems of the support library, as its plugin entry point makes them when the bootstrap configuration loads
    // it first (the entry points of the plugins are C functions of the same name, so a test linking several plugins
    // loads them through the plugin control or uses their C++ API)
    if ( GUCEF_NULL == PLUGINGLUE::OGRE::COgreGlobal::Instance() )
        return false;

    // the Ogre plugins as the bootstrap configuration lists them
    CORE::CDataNode config;
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", OgrePluginGroup );
    group->SetAttribute( "LoadImmediately", true );
    // Ogre chooses X11 or Wayland when it is compiled: Wayland windows need the Wayland flavour of the render system
    const bool wayland = TESTWINDOWMODE_WAYLAND == GetTestWindowMode();
    AddOgrePluginEntry( *group, wayland ? "OgreRenderSystem_GL3Plus_Wayland" : "OgreRenderSystem_GL3Plus", true );
    // Vulkan: the render system, which finds the Vulkan devices when it loads, and the compiler of its programs
    AddOgrePluginEntry( *group, wayland ? "OgreRenderSystem_Vulkan_Wayland" : "OgreRenderSystem_Vulkan", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GLSLangProgramManager", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GucefVFS", true );
    AddOgrePluginEntry( *group, "OgreCodec_GucefIMAGE", true );
    CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config );

    // OpenGL is the renderer of the configuration; the test windows ask for GetTestRenderer() in their video settings
    CORE::CString::StringVector renderSystems;
    renderSystems.push_back( "OpenGL 3+ Rendering Subsystem" );
    renderSystems.push_back( "Vulkan Rendering Subsystem" );
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().SetRenderSystems( renderSystems );

    // the window backend and the platform window driver
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverOgre", GuiPluginGroup, true ) )
        pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverOgre_d", GuiPluginGroup, true );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( !pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverXWinGL", GuiPluginGroup, true ) )
        pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverXWinGL_d", GuiPluginGroup, true );
    if ( wayland && !pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverWAYLAND", GuiPluginGroup, true ) )
        pluginControl.AddPluginFromDir( "$MODULEDIR$/guidriverWAYLAND_d", GuiPluginGroup, true );
    #endif
    return !GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

void
TearDownOgreForTests( void )
{GUCEF_TRACE;

    // the order of an application: the windows, the GUI and window plugins (their GPU resources go while the Root
    // exists), the Root (Ogre shuts down while the code of its plugins is loaded), the Ogre plugin modules
    ReleaseMainTestWindow();
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    pluginControl.UnloadPluginGroup( GuiPluginGroup );
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().DestroyRoot();
    pluginControl.UnloadPluginGroup( OgrePluginGroup );
    PLUGINGLUE::OGRE::COgreGlobal::Deinstance();
}

/*-------------------------------------------------------------------------*/

CORE::CValueList
GetTestWindowParams( void )
{GUCEF_TRACE;

    CORE::CValueList params;
    if ( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() )
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, GUIDRIVEROGRE::COgreWindowContext::OffscreenPlatformBackend );
    else
    if ( TESTWINDOWMODE_WAYLAND == GetTestWindowMode() )
    {
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, "Wayland" );
        // a headless compositor has no decorations of its own; the tests need none, and libdecor would load GTK
        params.Set( "DECORATIONS", "none" );
    }
    else
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, "XWinGL" );
    return params;
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
MakeTestOgreWindow( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
    if ( backend.IsNULL() || TESTWINDOWMODE_NONE == GetTestWindowMode() )
        return GUI::TWindowContextPtr();
    GUI::CVideoSettings settings;
    settings.SetResolution( width, height, 32 );
    settings.SetFullscreenState( false );
    settings.SetVSyncState( false );
    settings.SetRenderer( GetTestRenderer() );
    // the tests draw the frames themselves; the X11 window paces its own (60 a second) when the application pulses
    settings.SetFrequency( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() ? 0 : 60 );
    CORE::CValueList params = GetTestWindowParams();
    return backend->CreateWindowContext( "OgreTest", settings, &params );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
GetMainTestWindow( void )
{GUCEF_TRACE;

    if ( g_mainWindow.IsNULL() )
        g_mainWindow = MakeTestOgreWindow( 320, 240 );
    return g_mainWindow;
}

/*-------------------------------------------------------------------------*/

void
ReleaseMainTestWindow( void )
{GUCEF_TRACE;

    if ( g_mainWindow.IsNULL() )
        return;
    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
    if ( !backend.IsNULL() )
        backend->DestroyWindowContext( g_mainWindow );
    g_mainWindow.Unlink();
}

/*-------------------------------------------------------------------------*/

GUIDRIVEROGRE::COgreWindowContext*
AsOgreWindow( GUI::TWindowContextPtr window )
{GUCEF_TRACE;

    if ( window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != window->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< GUIDRIVEROGRE::COgreWindowContext* >( window.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

UInt32
CountProcessThreads( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    // a directory per thread
    UInt32 count = 0;
    DIR* tasks = ::opendir( "/proc/self/task" );
    if ( GUCEF_NULL == tasks )
        return 0;
    struct dirent* entry = GUCEF_NULL;
    while ( GUCEF_NULL != ( entry = ::readdir( tasks ) ) )
        if ( '.' != entry->d_name[ 0 ] )
            ++count;
    ::closedir( tasks );
    return count;
    #else
    return 0;
    #endif
}

/*-------------------------------------------------------------------------*/

void
GetPixel( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y, UInt8 rgba[ 4 ] )
{GUCEF_TRACE;

    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    memcpy( rgba, data, 4 );
}

/*-------------------------------------------------------------------------*/

bool
PixelIs( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y, UInt8 r, UInt8 g, UInt8 b, Int32 tolerance )
{GUCEF_TRACE;

    if ( pixels.IsNULL() || x < 0 || y < 0 || x >= (Int32) pixels->GetWidthInPixels() || y >= (Int32) pixels->GetHeightInPixels() )
        return false;
    UInt8 rgba[ 4 ];
    GetPixel( pixels, x, y, rgba );
    const bool same = abs( (Int32) rgba[ 0 ] - r ) <= tolerance && abs( (Int32) rgba[ 1 ] - g ) <= tolerance && abs( (Int32) rgba[ 2 ] - b ) <= tolerance;
    if ( !same )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PixelIs: pixel " << x << "," << y << " is " << (UInt32) rgba[ 0 ] << "," << (UInt32) rgba[ 1 ] << ","
            << (UInt32) rgba[ 2 ] << "," << (UInt32) rgba[ 3 ] << ", expected " << (UInt32) r << "," << (UInt32) g << "," << (UInt32) b );
    }
    return same;
}

/*-------------------------------------------------------------------------*/

CTestQuadBatch::CTestQuadBatch( void )
    : m_vertices()
    , m_indices()
    , m_vertexData( GUCEF_NULL )
    , m_indexData( GUCEF_NULL )
    , m_operation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestQuadBatch::~CTestQuadBatch()
{GUCEF_TRACE;

    OGRE_DELETE m_vertexData;
    OGRE_DELETE m_indexData;
}

/*-------------------------------------------------------------------------*/

void
CTestQuadBatch::AddQuad( float x, float y, float width, float height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

    const UInt32 first = (UInt32) m_vertices.size();
    const float xs[ 4 ] = { x, x + width, x + width, x };
    const float ys[ 4 ] = { y, y, y + height, y + height };
    const float us[ 4 ] = { 0.0f, 1.0f, 1.0f, 0.0f };
    const float vs[ 4 ] = { 0.0f, 0.0f, 1.0f, 1.0f };
    for ( UInt32 i=0; i<4; ++i )
    {
        SVertex vertex;
        vertex.x = xs[ i ];
        vertex.y = ys[ i ];
        vertex.colour[ 0 ] = r;
        vertex.colour[ 1 ] = g;
        vertex.colour[ 2 ] = b;
        vertex.colour[ 3 ] = a;
        vertex.u = us[ i ];
        vertex.v = vs[ i ];
        m_vertices.push_back( vertex );
    }
    const UInt32 indices[ 6 ] = { 0, 1, 2, 0, 2, 3 };
    for ( UInt32 i=0; i<6; ++i )
        m_indices.push_back( first + indices[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CTestQuadBatch::Build( void )
{GUCEF_TRACE;

    if ( m_vertices.empty() )
        return false;
    Ogre::HardwareBufferManager& buffers = Ogre::HardwareBufferManager::getSingleton();
    m_vertexData = OGRE_NEW Ogre::VertexData();
    m_vertexData->vertexStart = 0;
    m_vertexData->vertexCount = m_vertices.size();
    Ogre::VertexDeclaration* declaration = m_vertexData->vertexDeclaration;
    declaration->addElement( 0, 0, Ogre::VET_FLOAT2, Ogre::VES_POSITION );
    declaration->addElement( 0, 8, Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE );
    declaration->addElement( 0, 12, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0 );
    Ogre::HardwareVertexBufferSharedPtr vertexBuffer = buffers.createVertexBuffer( sizeof( SVertex ), m_vertices.size(), Ogre::HBU_GPU_ONLY );
    vertexBuffer->writeData( 0, m_vertices.size() * sizeof( SVertex ), &m_vertices[ 0 ], true );
    m_vertexData->vertexBufferBinding->setBinding( 0, vertexBuffer );
    m_indexData = OGRE_NEW Ogre::IndexData();
    m_indexData->indexStart = 0;
    m_indexData->indexCount = m_indices.size();
    m_indexData->indexBuffer = buffers.createIndexBuffer( Ogre::HardwareIndexBuffer::IT_32BIT, m_indices.size(), Ogre::HBU_GPU_ONLY );
    m_indexData->indexBuffer->writeData( 0, m_indices.size() * sizeof( UInt32 ), &m_indices[ 0 ], true );
    m_operation.vertexData = m_vertexData;
    m_operation.indexData = m_indexData;
    m_operation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    m_operation.useIndexes = true;
    return true;
}

/*-------------------------------------------------------------------------*/

const Ogre::RenderOperation&
CTestQuadBatch::GetOperation( void ) const
{GUCEF_TRACE;

    return m_operation;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
MakeSolidTexture( const char* name, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

    GUCEF::vector< UInt8 > pixels( (size_t) width * height * 4 );
    for ( size_t i=0; i<pixels.size(); i+=4 )
    {
        pixels[ i ] = r;
        pixels[ i+1 ] = g;
        pixels[ i+2 ] = b;
        pixels[ i+3 ] = a;
    }
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual( name, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_BYTE_RGBA );
    texture->getBuffer()->blitFromMemory( Ogre::PixelBox( width, height, 1, Ogre::PF_BYTE_RGBA, &pixels[ 0 ] ) );
    return texture;
}

/*-------------------------------------------------------------------------*/

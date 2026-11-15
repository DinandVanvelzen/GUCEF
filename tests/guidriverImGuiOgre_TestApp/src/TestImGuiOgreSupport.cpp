/*
 *  guidriverImGuiOgre_TestApp: tests of the gucefGUI driver ImGuiOgre
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

#include <Ogre.h>
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

#include "guidriverImGuiOgre_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIOGRETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTestSoftwareDriver* g_softwareDriver = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestSoftwareDriver::CTestSoftwareDriver( void )
    : GUIDRIVERIMGUI::CImGuiDriver()
{
}

CTestSoftwareDriver::~CTestSoftwareDriver()
{
}

GUI::TGuiContextPtr
CTestSoftwareDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{
    if ( !CanCreateContext() )
        return GUI::TGuiContextPtr();
    return AddContext( GUCEF_NEW GUIDRIVERIMGUI::CImGuiContext( this, windowContext ) );
}

CString
CTestSoftwareDriver::GetDriverName( void )
{
    return "ImGuiSoftwareTest";
}

/*-------------------------------------------------------------------------*/

CNonOgreWindow::CNonOgreWindow( void )
    : GUI::CWindowContext()
{
}

UInt32 CNonOgreWindow::GetWidth( void ) const { return 640; }
UInt32 CNonOgreWindow::GetHeight( void ) const { return 480; }
UInt32 CNonOgreWindow::GetID( void ) const { return 1; }
bool CNonOgreWindow::IsActive( void ) const { return true; }
CString CNonOgreWindow::GetName( void ) const { return "NonOgreWindow"; }
void CNonOgreWindow::SetGuiContext( GUI::TGuiContextPtr& context ) {}
GUI::TGuiContextPtr CNonOgreWindow::GetGuiContext( void ) { return GUI::TGuiContextPtr(); }
CString CNonOgreWindow::GetProperty( const CString& propertyName ) const { return CString::Empty; }

/*-------------------------------------------------------------------------*/

CTestFrameBuilder::CTestFrameBuilder( TBuildFunction function, void* userData )
    : GUIDRIVERIMGUI::CImGuiFrameBuilder()
    , m_function( function )
    , m_userData( userData )
{
}

void
CTestFrameBuilder::SetFunction( TBuildFunction function, void* userData )
{
    m_function = function;
    m_userData = userData;
}

void
CTestFrameBuilder::OnBuildFrame( GUIDRIVERIMGUI::CImGuiContext& context, UInt32 deltaInMs )
{
    if ( GUCEF_NULL != m_function )
        m_function( context, deltaInMs, m_userData );
}

/*-------------------------------------------------------------------------*/

CLogTextCounter::CLogTextCounter( const CORE::CString& text )
    : CORE::CILogger()
    , m_text( text )
    , m_count( 0 )
    , m_lock()
{
}

void
CLogTextCounter::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    MT::CScopeMutex lock( m_lock );
    if ( logMessage.HasSubstr( m_text ) >= 0 )
        ++m_count;
}

void
CLogTextCounter::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

void
CLogTextCounter::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage, threadId, timestamp );
}

void
CLogTextCounter::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{
    Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
}

void CLogTextCounter::FlushLog( void ) {}
void CLogTextCounter::SetMinimalLogLevel( const Int32 minimalLogLevel ) {}
Int32 CLogTextCounter::GetMinimalLogLevel( void ) const { return CORE::LOGLEVEL_EVERYTHING; }

/*-------------------------------------------------------------------------*/

CTestSoftwareDriver&
GetSoftwareDriver( void )
{
    if ( GUCEF_NULL == g_softwareDriver )
        g_softwareDriver = GUCEF_NEW CTestSoftwareDriver();
    return *g_softwareDriver;
}

void
DestroySoftwareDriver( void )
{
    GUCEF_DELETE g_softwareDriver;
    g_softwareDriver = GUCEF_NULL;
}

bool
LoadTestPlugin( const char* moduleName, const char* pluginGroup )
{
    CString release = CString( "$MODULEDIR$/" ) + moduleName;
    CString debug = release + "_d";
    #ifdef GUCEF_DEBUG_MODE
    const CString& first = debug;
    const CString& second = release;
    #else
    const CString& first = release;
    const CString& second = debug;
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, pluginGroup, true ) || pluginControl.AddPluginFromDir( second, pluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
LoadFreeTypeFontPlugin( void )
{
    GUI::CFontRasterizerRegistry& registry = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
    if ( GUCEF_NULL == registry.Get( "FreeType" ) )
    {
        LoadTestPlugin( "fontpluginFREETYPE", "fontplugin" );
    }
    return GUCEF_NULL != registry.Get( "FreeType" );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateSoftwareContext( void )
{
    GUI::TGuiContextPtr context = GetSoftwareDriver().CreateGUIContext( GUI::TWindowContextPtr() );
    GUIDRIVERIMGUI::CImGuiContext* imguiContext = AsImGui( context );
    if ( GUCEF_NULL != imguiContext )
        imguiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

GUIDRIVERIMGUI::CImGuiContext*
AsImGui( const GUI::TGuiContextPtr& context )
{
    if ( context.IsNULL() )
        return GUCEF_NULL;
    return GUIDRIVERIMGUI::CImGuiGlobal::Instance()->FindContext( context.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        data[ i*4 ] = r;
        data[ i*4+1 ] = g;
        data[ i*4+2 ] = b;
        data[ i*4+3 ] = a;
    }
    return pixels;
}

/*-------------------------------------------------------------------------*/

GUI::TBitmapFontPtr
MakeTestFont( Int32 width, Int32 height )
{
    UInt32 count = 126 - 32 + 1;
    GUI::TBitmapFontPtr font( GUCEF_NEW GUI::CBitmapFont() );
    font->SetAtlas( MakeSolidImage( (UInt32) width * count, (UInt32) height, 255, 255, 255, 255 ) );
    for ( UInt32 cp=32; cp<=126; ++cp )
    {
        GUI::CBitmapFont::TGlyph glyph;
        glyph.source = GUI::CRect( (Int32) ( cp - 32 ) * width, 0, width, height );
        glyph.advance = width;
        font->SetGlyph( cp, glyph );
    }
    font->SetLineHeight( height );
    font->SetBaseline( height );
    font->SetFallbackCodePoint( '?' );
    return font;
}

/*-------------------------------------------------------------------------*/

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

/*-------------------------------------------------------------------------*/

bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance &&
           abs( (Int32) a.b - (Int32) b.b ) <= tolerance && abs( (Int32) a.a - (Int32) b.a ) <= tolerance;
}


/*-------------------------------------------------------------------------*/

namespace {

GUI::TWindowContextPtr g_mainWindow;
const char* const GuiPluginGroup = "guidriverImGuiOgreTestGuiPlugins";
const char* const OgrePluginGroup = "guidriverImGuiOgreTestOgrePlugins";

bool
EnvIs( const char* name, char value )
{
    const char* text = ::getenv( name );
    return GUCEF_NULL != text && value == text[ 0 ];
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
{
    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return TESTWINDOWMODE_NONE;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( HasDisplay() )
        return EnvIs( "GUCEF_TEST_X11_WINDOWS", '1' ) ? TESTWINDOWMODE_X11 : TESTWINDOWMODE_NONE;
    return TESTWINDOWMODE_OFFSCREEN;
    #else
    return TESTWINDOWMODE_NONE;
    #endif
}

/*-------------------------------------------------------------------------*/

CString
GetNoWindowReason( void )
{
    if ( EnvIs( "GUCEF_TEST_RENDER_GPU", '0' ) )
        return "GUCEF_TEST_RENDER_GPU=0 skips the rendering tests";
    if ( HasDisplay() )
        return "windows on the X server of DISPLAY are made only with GUCEF_TEST_X11_WINDOWS=1 (a server for tests such as Xvfb)";
    return "the platform has no window mode for these tests";
}

/*-------------------------------------------------------------------------*/

bool
SetUpOgreForTests( void )
{
    // the systems of the support library, as its plugin entry point makes them when the bootstrap configuration loads it
    if ( GUCEF_NULL == PLUGINGLUE::OGRE::COgreGlobal::Instance() )
        return false;

    // the Ogre plugins as the bootstrap configuration lists them
    CORE::CDataNode config;
    CORE::CDataNode* group = config.AddChild( "PluginGroup" );
    group->SetAttribute( "GroupName", OgrePluginGroup );
    group->SetAttribute( "LoadImmediately", true );
    AddOgrePluginEntry( *group, "OgreRenderSystem_GL3Plus", true );
    AddOgrePluginEntry( *group, "OgrePlugin_GucefVFS", true );
    AddOgrePluginEntry( *group, "OgreCodec_GucefIMAGE", true );
    CORE::CCoreGlobal::Instance()->GetPluginControl().LoadConfig( config );

    CORE::CString::StringVector renderSystems;
    renderSystems.push_back( "OpenGL 3+ Rendering Subsystem" );
    PLUGINGLUE::OGRE::COgreGlobal::Instance()->GetRootSetup().SetRenderSystems( renderSystems );

    // the window backend and the platform window driver
    LoadTestPlugin( "guidriverOgre", GuiPluginGroup );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    LoadTestPlugin( "guidriverXWinGL", GuiPluginGroup );
    #endif

    // the GUI driver of the tests, the one the layer order test needs, and the reference
    LoadTestPlugin( "guidriverImGuiOgre", GuiPluginGroup );
    LoadTestPlugin( "guidriverRmlUIOgre", GuiPluginGroup );
    return !GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" ).IsNULL() &&
           GUCEF_NULL != GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( GUIDRIVERIMGUIOGRE::CImGuiOgreDriver::RegistrationName );
}

/*-------------------------------------------------------------------------*/

void
TearDownOgreForTests( void )
{
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

GUI::TWindowContextPtr
MakeTestOgreWindow( UInt32 width, UInt32 height )
{
    GUI::TWindowManagerBackendPtr backend = GUI::CGuiGlobal::Instance()->GetWindowManager().GetBackend( "Ogre" );
    if ( backend.IsNULL() || TESTWINDOWMODE_NONE == GetTestWindowMode() )
        return GUI::TWindowContextPtr();
    GUI::CVideoSettings settings;
    settings.SetResolution( width, height, 32 );
    settings.SetFullscreenState( false );
    settings.SetVSyncState( false );
    // the tests draw the frames themselves; the X11 window paces its own (60 a second) when the application pulses
    settings.SetFrequency( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() ? 0 : 60 );
    CORE::CValueList params;
    if ( TESTWINDOWMODE_OFFSCREEN == GetTestWindowMode() )
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, GUIDRIVEROGRE::COgreWindowContext::OffscreenPlatformBackend );
    else
        params.Set( GUIDRIVEROGRE::COgreWindowContext::PlatformBackendParameterName, "XWinGL" );
    return backend->CreateWindowContext( "OgreTest", settings, &params );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
GetMainTestWindow( void )
{
    if ( g_mainWindow.IsNULL() )
        g_mainWindow = MakeTestOgreWindow( 640, 480 );
    return g_mainWindow;
}

/*-------------------------------------------------------------------------*/

void
ReleaseMainTestWindow( void )
{
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
{
    if ( window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != window->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< GUIDRIVEROGRE::COgreWindowContext* >( window.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateImGuiOgreContext( GUI::TWindowContextPtr window )
{
    if ( window.IsNULL() )
        window = GetMainTestWindow();
    if ( window.IsNULL() )
        return GUI::TGuiContextPtr();
    GUI::TGuiContextPtr context = GUI::CGuiGlobal::Instance()->GetGuiManager().CreateGUIContext( GUIDRIVERIMGUIOGRE::CImGuiOgreDriver::RegistrationName, window );
    GUIDRIVERIMGUI::CImGuiContext* imguiContext = AsImGui( context );
    if ( GUCEF_NULL != imguiContext )
        imguiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

UInt32
CountImGuiOgreTextures( void )
{
    if ( GUCEF_NULL == Ogre::TextureManager::getSingletonPtr() )
        return 0;
    UInt32 count = 0;
    Ogre::ResourceManager::ResourceMapIterator textures = Ogre::TextureManager::getSingleton().getResourceIterator();
    while ( textures.hasMoreElements() )
    {
        if ( 0 == textures.getNext()->getName().compare( 0, 18, "ImGuiOgre/Texture/" ) )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
MakeSolidTexture( const char* name, UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIOGRETEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  guidriverImGui_TestApp: tests of the gucefGUI driver library of Dear ImGui
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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
#include "fontpluginFREETYPE_pluginAPI.h"
#endif

#include "guidriverImGui_TestSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTestImGuiDriver* g_driver = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestImGuiDriver::CTestImGuiDriver( void )
    : GUIDRIVERIMGUI::CImGuiDriver()
{
}

/*-------------------------------------------------------------------------*/

CTestImGuiDriver::~CTestImGuiDriver()
{
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CTestImGuiDriver::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{
    if ( !CanCreateContext() )
        return GUI::TGuiContextPtr();
    return AddContext( GUCEF_NEW GUIDRIVERIMGUI::CImGuiContext( this, windowContext ) );
}

/*-------------------------------------------------------------------------*/

CString
CTestImGuiDriver::GetDriverName( void )
{
    return "ImGuiTest";
}

/*-------------------------------------------------------------------------*/

CRecordingWindow::CRecordingWindow( void )
    : GUI::CWindowContext()
    , shapeCalls( 0 )
    , lastShape( GUI::CURSORSHAPE_ARROW )
    , supportsShapes( true )
    , active( true )
{
}

UInt32 CRecordingWindow::GetWidth( void ) const { return 640; }
UInt32 CRecordingWindow::GetHeight( void ) const { return 480; }
UInt32 CRecordingWindow::GetID( void ) const { return 1; }
bool CRecordingWindow::IsActive( void ) const { return active; }
CString CRecordingWindow::GetName( void ) const { return "RecordingWindow"; }
void CRecordingWindow::SetGuiContext( GUI::TGuiContextPtr& context ) {}
GUI::TGuiContextPtr CRecordingWindow::GetGuiContext( void ) { return GUI::TGuiContextPtr(); }
CString CRecordingWindow::GetProperty( const CString& propertyName ) const { return CString::Empty; }
bool CRecordingWindow::SetSystemCursorVisible( bool visible ) { return true; }

bool
CRecordingWindow::SetSystemCursorShape( GUI::TCursorShape shape )
{
    ++shapeCalls;
    lastShape = shape;
    return supportsShapes;
}

void
CRecordingWindow::SetActive( bool isActive )
{
    active = isActive;
    NotifyObservers( WindowContextActivateEvent );
}

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

CEventCounter::CEventCounter( void )
    : CORE::CObserver()
    , counts()
{
}

UInt32
CEventCounter::Count( const CORE::CEvent& eventId ) const
{
    TCountMap::const_iterator i = counts.find( eventId );
    return i == counts.end() ? 0 : (*i).second;
}

const CString&
CEventCounter::GetClassTypeName( void ) const
{
    static const CString classTypeName = "GUCEF::IMGUITEST::CEventCounter";
    return classTypeName;
}

void
CEventCounter::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++counts[ eventId ];
}

/*-------------------------------------------------------------------------*/

CTestImGuiDriver&
GetTestDriver( void )
{
    if ( GUCEF_NULL == g_driver )
        g_driver = GUCEF_NEW CTestImGuiDriver();
    return *g_driver;
}

/*-------------------------------------------------------------------------*/

void
DestroyTestDriver( void )
{
    GUCEF_DELETE g_driver;
    g_driver = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

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
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
        FONTPLUGINFREETYPE::fontpluginFREETYPE_GUCEFPlugin_Load( 0, GUCEF_NULL );
        #else
        LoadTestPlugin( "fontpluginFREETYPE", "fontplugin" );
        #endif
    }
    return GUCEF_NULL != registry.Get( "FreeType" );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateImGuiContext( GUI::TWindowContextPtr window )
{
    GUI::TGuiContextPtr context = GetTestDriver().CreateGUIContext( window );
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

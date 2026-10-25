/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefGAMEGUI_TestCommon.h"

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

bool
LoadTestPlugin( const char* moduleName, const char* pluginGroup )
{GUCEF_TRACE;

    CORE::CString release = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    CORE::CString debug = release + "_d";
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString& first = debug;
    const CORE::CString& second = release;
    #else
    const CORE::CString& first = release;
    const CORE::CString& second = debug;
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, pluginGroup, true ) || pluginControl.AddPluginFromDir( second, pluginGroup, true );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateNullContext( void )
{GUCEF_TRACE;

    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    if ( GUCEF_NULL == manager.GetGuiDriver( "Null" ) )
        LoadTestPlugin( "guidriverNull", "guidriver" );
    GUI::TGuiContextPtr context = manager.CreateGUIContext( "Null", GUI::TWindowContextPtr() );
    GUI::CGUIContext* guiContext = context.IsNULL() ? GUCEF_NULL : context->AsGUIContext();
    if ( GUCEF_NULL != guiContext )
        guiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{GUCEF_TRACE;

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
{GUCEF_TRACE;

    // a white atlas with one cell per glyph; every glyph is a full block
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

GUI::TGuiContextPtr
CreateFormContext( void )
{GUCEF_TRACE;

    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( 1280, 960 );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateSizedFormContext( UInt32 width, UInt32 height, bool realFont )
{GUCEF_TRACE;

    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = ContextOf( contextPtr );
    if ( GUCEF_NULL == context )
        return contextPtr;
    context->SetLogicalResolution( width, height );

    CORE::CString fontName = "Test";
    if ( realFont )
    {
        GUI::CFontRasterizerRegistry& registry = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
        if ( GUCEF_NULL == registry.Get( "FreeType" ) )
            LoadTestPlugin( "fontpluginFREETYPE", "fontplugin" );
        if ( GUCEF_NULL != registry.Get( "FreeType" ) && GUI::CGuiGlobal::Instance()->MountData() &&
             context->RegisterFont( "UI", GUI::CGuiGlobal::DefaultUiFontPath, 18 ) )
            fontName = "UI";
    }
    if ( "Test" == fontName )
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );

    CORE::CDataNode theme;
    GUI::BuildDarkTheme( theme, fontName );
    context->SetTheme( theme );
    return contextPtr;
}

/*-------------------------------------------------------------------------*/

bool
HasRealFont( GUI::CGUIContext* context )
{GUCEF_TRACE;

    return GUCEF_NULL != context && !context->GetBitmapFont( "UI" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CaptureFrame( GUI::CGUIContext* context )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixels;
    if ( GUCEF_NULL != context )
    {
        context->RenderFrame( 16 );
        context->CaptureFrame( pixels );
    }
    return pixels;
}

/*-------------------------------------------------------------------------*/

namespace {

UInt32
Crc32( const UInt8* data, size_t size, UInt32 crc )
{
    static UInt32 table[ 256 ];
    static bool tableMade = false;
    if ( !tableMade )
    {
        for ( UInt32 n=0; n<256; ++n )
        {
            UInt32 c = n;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? 0xedb88320u ^ ( c >> 1 ) : c >> 1;
            table[ n ] = c;
        }
        tableMade = true;
    }
    crc = ~crc;
    for ( size_t i=0; i<size; ++i )
        crc = table[ ( crc ^ data[ i ] ) & 0xff ] ^ ( crc >> 8 );
    return ~crc;
}

void
Put32( GUCEF::vector< UInt8 >& out, UInt32 value )
{
    out.push_back( (UInt8) ( value >> 24 ) );
    out.push_back( (UInt8) ( value >> 16 ) );
    out.push_back( (UInt8) ( value >> 8 ) );
    out.push_back( (UInt8) value );
}

void
PutChunk( GUCEF::vector< UInt8 >& out, const char* type, const GUCEF::vector< UInt8 >& data )
{
    Put32( out, (UInt32) data.size() );
    GUCEF::vector< UInt8 > body( type, type + 4 );
    body.insert( body.end(), data.begin(), data.end() );
    out.insert( out.end(), body.begin(), body.end() );
    Put32( out, Crc32( &body[ 0 ], body.size(), 0 ) );
}

}

bool
WriteShot( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString directory = CORE::GetEnv( "GUCEF_TEST_SHOT_DIR" );
    if ( directory.IsNULLOrEmpty() || pixels.IsNULL() )
        return false;
    UInt32 width = pixels->GetWidthInPixels();
    UInt32 height = pixels->GetHeightInPixels();
    const UInt8* rgba = static_cast< const UInt8* >( pixels->GetDataPtr() );

    // the scan lines, each with filter type 0, in stored (not compressed) deflate blocks
    GUCEF::vector< UInt8 > raw;
    raw.reserve( (size_t) height * ( width * 4 + 1 ) );
    for ( UInt32 y=0; y<height; ++y )
    {
        raw.push_back( 0 );
        raw.insert( raw.end(), rgba + (size_t) y * width * 4, rgba + (size_t) ( y + 1 ) * width * 4 );
    }
    GUCEF::vector< UInt8 > zlib;
    zlib.push_back( 0x78 );
    zlib.push_back( 0x01 );
    size_t position = 0;
    UInt32 a = 1, b = 0;
    for ( size_t i=0; i<raw.size(); ++i )
    {
        a = ( a + raw[ i ] ) % 65521;
        b = ( b + a ) % 65521;
    }
    do
    {
        size_t count = raw.size() - position;
        if ( count > 65535 )
            count = 65535;
        bool last = position + count >= raw.size();
        zlib.push_back( last ? 1 : 0 );
        zlib.push_back( (UInt8) ( count & 0xff ) );
        zlib.push_back( (UInt8) ( count >> 8 ) );
        zlib.push_back( (UInt8) ( ~count & 0xff ) );
        zlib.push_back( (UInt8) ( ( ~count >> 8 ) & 0xff ) );
        zlib.insert( zlib.end(), raw.begin() + position, raw.begin() + position + count );
        position += count;
    }
    while ( position < raw.size() );
    Put32( zlib, ( b << 16 ) | a );

    GUCEF::vector< UInt8 > header;
    Put32( header, width );
    Put32( header, height );
    header.push_back( 8 );      // bits per channel
    header.push_back( 6 );      // RGBA
    header.push_back( 0 );
    header.push_back( 0 );
    header.push_back( 0 );

    GUCEF::vector< UInt8 > png;
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
    png.insert( png.end(), signature, signature + 8 );
    PutChunk( png, "IHDR", header );
    PutChunk( png, "IDAT", zlib );
    PutChunk( png, "IEND", GUCEF::vector< UInt8 >() );

    CORE::CString path = directory;
    CORE::AppendToPath( path, name + ".png" );
    CORE::CFileAccess file( path, "wb" );
    return file.IsValid() && png.size() == file.Write( &png[ 0 ], 1, (UInt32) png.size() );
}

/*-------------------------------------------------------------------------*/

GUI::CGUIContext*
ContextOf( GUI::TGuiContextPtr& contextPtr )
{GUCEF_TRACE;

    return static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
ClickAt( GUI::CGUIContext* context, Int32 x, Int32 y, GUI::TMouseButton button )
{GUCEF_TRACE;

    context->FlushChanges();
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( button, true );
    context->InjectMouseButton( button, false );
}

/*-------------------------------------------------------------------------*/

void
ClickWidget( GUI::CGUIContext* context, GUI::CWidget* widget )
{GUCEF_TRACE;

    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    ClickAt( context, rect.left + rect.width / 2, rect.top + rect.height / 2 );
}

/*-------------------------------------------------------------------------*/

void
RowPoint( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, Int32& x, Int32& y )
{GUCEF_TRACE;

    grid->ScrollToRow( row );
    context->FlushChanges();
    const GUI::CRect& rect = grid->GetRect();
    Int32 rowHeight = grid->GetRowHeight();
    x = rect.left + 12;
    y = rect.top + rowHeight + ( row - grid->GetIntProperty( "ScrollPosition" ) ) * rowHeight + rowHeight / 2;
}

/*-------------------------------------------------------------------------*/

void
ClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, GUI::TMouseButton button )
{GUCEF_TRACE;

    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    ClickAt( context, x, y, button );
    context->RenderFrame( 500 );         // past the double click time: the next click is a click of its own
}

/*-------------------------------------------------------------------------*/

void
DoubleClickRow( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row )
{GUCEF_TRACE;

    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    ClickAt( context, x, y );
    ClickAt( context, x, y );
    context->RenderFrame( 300 );
}

/*-------------------------------------------------------------------------*/

void
ClickColumnTitle( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 column )
{GUCEF_TRACE;

    context->FlushChanges();
    const GUI::CRect& rect = grid->GetRect();
    GUCEF::vector< Int32 > widths = grid->GetColumnWidths();
    Int32 x = rect.left + 4;
    for ( Int32 i=0; i<column && i<(Int32)widths.size(); ++i )
        x += widths[ i ];
    ClickAt( context, x, rect.top + 4 );
    context->RenderFrame( 500 );
}

/*-------------------------------------------------------------------------*/

void
PressKey( GUI::CGUIContext* context, INPUT::KeyCode key )
{GUCEF_TRACE;

    context->InjectKey( key, true, 0 );
    context->InjectKey( key, false, 0 );
}

/*-------------------------------------------------------------------------*/

void
TypeInto( GUI::CGUIContext* context, GUI::CWidget* field, const CORE::CString& text, bool enter )
{GUCEF_TRACE;

    context->FlushChanges();
    context->SetFocus( field );
    field->SetProperty( "Text", CORE::CString::Empty );
    context->InjectText( text );
    if ( enter )
        PressKey( context, INPUT::KEYCODE_RETURN );
}

/*-------------------------------------------------------------------------*/

bool
PickItem( GUI::CGUIContext* context, GUI::CWidget* combobox, Int32 index )
{GUCEF_TRACE;

    if ( GUCEF_NULL == combobox )
        return false;
    ClickWidget( context, combobox );
    context->FlushChanges();
    GUI::CListbox* list = GAMEGUI::WidgetAs< GUI::CListbox >( context->GetPopup(), "Listbox" );
    if ( GUCEF_NULL == list )
        return false;
    const GUI::CRect& rect = list->GetRect();
    Int32 rowHeight = list->GetRowHeight();
    context->InjectMouseMove( rect.left + 4, rect.top + index * rowHeight + rowHeight / 2 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CString::StringVector& lines, const CORE::CString& text )
{GUCEF_TRACE;

    for ( size_t i=0; i<lines.size(); ++i )
        if ( lines[ i ] == text )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
AnyLineHas( const CORE::CString::StringVector& lines, const CORE::CString& text )
{GUCEF_TRACE;

    for ( size_t i=0; i<lines.size(); ++i )
        if ( lines[ i ].HasSubstr( text, true ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
IsWidgetEnabled( GUI::CForm& form, const CORE::CString& name )
{GUCEF_TRACE;

    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL != widget && widget->GetBoolProperty( "Enabled" );
}

/*-------------------------------------------------------------------------*/

void
PumpFor( UInt32 durationInMs )
{GUCEF_TRACE;

    for ( UInt32 waited=0; waited<durationInMs; ++waited )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

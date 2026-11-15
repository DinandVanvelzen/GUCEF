/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#include "gucefCORE_CTimeZoneRules.h"
#define GUCEF_CORE_CTIMEZONERULES_H
#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefDRNUTILSGUI_TestCommon.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                   //
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

namespace {

IMAGE::TPixelMapPtr
MakeSolidImage( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 i=0; i<width*height*4; ++i )
        data[ i ] = 255;
    return pixels;
}

/** A bitmap font with fixed width glyphs for ASCII 32..126, each glyph 'width' x 'height' */
GUI::TBitmapFontPtr
MakeTestFont( Int32 width, Int32 height )
{
    UInt32 count = 126 - 32 + 1;
    GUI::TBitmapFontPtr font( GUCEF_NEW GUI::CBitmapFont() );
    font->SetAtlas( MakeSolidImage( (UInt32) width * count, (UInt32) height ) );
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

}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateFormContext( void )
{GUCEF_TRACE;

    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    if ( GUCEF_NULL == manager.GetGuiDriver( "Null" ) )
        LoadTestPlugin( "guidriverNull", "guidriver" );
    GUI::TGuiContextPtr contextPtr = manager.CreateGUIContext( "Null", GUI::TWindowContextPtr() );
    GUI::CIGUIContext* baseContext = contextPtr.GetPointerAlways();
    GUI::CGUIContext* context = GUCEF_NULL != baseContext ? baseContext->AsGUIContext() : GUCEF_NULL;
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

GUI::CGUIContext*
ContextOf( GUI::TGuiContextPtr& contextPtr )
{GUCEF_TRACE;

    return static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

void
ClickWidget( GUI::CGUIContext* context, GUI::CWidget* widget )
{GUCEF_TRACE;

    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
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

bool
IsWidgetEnabled( GUI::CForm& form, const CORE::CString& name )
{GUCEF_TRACE;

    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL != widget && widget->GetBoolProperty( "Enabled" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
TextOfWidget( GUI::CForm& form, const CORE::CString& name )
{GUCEF_TRACE;

    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL != widget ? widget->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int64
UtcMs( Int32 year, Int32 month, Int32 day, Int32 hour, Int32 minute, Int32 second )
{GUCEF_TRACE;

    return ( CORE::CTimeZoneRules::DaysFromCivil( year, month, day ) * 86400 + hour * 3600 + minute * 60 + second ) * 1000;
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

/*
 *  guidriverMyGUI_TestApp: Application for testing the MyGUI GUI drivers
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

#include <math.h>
#include <stdlib.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#include "guidriverMyGUI_CMyGUIContext.h"
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */

#include "MyGUI_Widget.h"
#include "MyGUI_TextBox.h"
#include "MyGUI_FontManager.h"
#include "MyGUI_IFont.h"
#include "MyGUI_ITexture.h"

#include "guidriverMyGUI_TestCommon.h"

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

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const char* const FontAsset = "MyGUI.Engine/DejaVuSans.ttf";
const char* const FontName = "DejaVuSans";

// The glyph "I" of DejaVuSans.ttf is a rectangle of 202 by 1493 font units: the ideal picture of the glyph at
// any size is a rectangle with that proportion
const double GlyphStemUnits = 202.0;
const double GlyphHeightUnits = 1493.0;

/** What the bitmap font path measured, for the MSDF cases to compare with */
struct SBitmapResults
{
    SBitmapResults( void ) : valid( false ), textHeight( 0 ), measuredWidth( 0 ), measuredHeight( 0 ), error( 0.0 ), glyphHeight( 0.0 ) {}

    bool valid;
    Int32 textHeight;
    Int32 measuredWidth;
    Int32 measuredHeight;
    double error;
    double glyphHeight;
};

SBitmapResults g_bitmap;

/** How well a picture of the glyph "I" is the ideal rectangle */
struct SGlyphQuality
{
    SGlyphQuality( void ) : found( false ), height( 0.0 ), stemWidth( 0.0 ), error( 0.0 ) {}

    bool found;
    double height;     // the height of the glyph in pixels, from the coverage of the column through the stem
    double stemWidth;  // the width of the stem in pixels, from the coverage of the row through the middle
    double error;      // the difference with the ideal rectangle of that height and position, in pixels, 0 is exact
};

double
CoverageAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    if ( x < 0 || y < 0 || x >= (Int32) pixels->GetWidthInPixels() || y >= (Int32) pixels->GetHeightInPixels() )
        return 0.0;
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return (double) data[ 0 ] / 255.0;
}

/** How many pixels of the rectangle are not black */
UInt32
CountLit( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect )
{
    UInt32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( CoverageAt( pixels, x, y ) > 0.04 )
                ++count;
    return count;
}

/** The overlap of the pixel [x, x+1] with the interval [left, right] */
double
PixelOverlap( Int32 x, double left, double right )
{
    double from = left > (double) x ? left : (double) x;
    double to = right < (double) ( x + 1 ) ? right : (double) ( x + 1 );
    return to > from ? to - from : 0.0;
}

/**
 *  Measures the white glyph "I" on black in the rectangle: its height from the column through the stem, the
 *  width of the stem from the row through the middle, and how far the row is from the ideal rectangle: the
 *  coverage of the glyph in proportion 202 to 1493 with the height and the position that were measured.
 *  A sharp glyph is within a fraction of a pixel, a blurred one is off by about half the width of its edges.
 */
SGlyphQuality
MeasureGlyph( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect )
{
    SGlyphQuality quality;
    Int32 minX = 1 << 30, maxX = -1, minY = 1 << 30, maxY = -1;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( CoverageAt( pixels, x, y ) >= 0.5 )
            {
                if ( x < minX ) minX = x;
                if ( x > maxX ) maxX = x;
                if ( y < minY ) minY = y;
                if ( y > maxY ) maxY = y;
            }
    if ( maxX < 0 )
        return quality;

    const Int32 row = ( minY + maxY ) / 2;
    double stem = 0.0, moment = 0.0;
    for ( Int32 x=minX-6; x<=maxX+6; ++x )
    {
        const double c = CoverageAt( pixels, x, row );
        stem += c;
        moment += ( (double) x + 0.5 ) * c;
    }
    if ( stem < 0.5 )
        return quality;
    const double centre = moment / stem;

    // the height: the coverage along the column through the stem, in proportion to the strongest pixel of it
    // (a thin stem that is blurred never gets to full coverage)
    const Int32 column = (Int32) floor( centre );
    double peak = 0.0;
    for ( Int32 y=minY-6; y<=maxY+6; ++y )
        if ( CoverageAt( pixels, column, y ) > peak )
            peak = CoverageAt( pixels, column, y );
    if ( peak < 0.25 )
        return quality;
    double height = 0.0;
    for ( Int32 y=minY-6; y<=maxY+6; ++y )
        height += CoverageAt( pixels, column, y ) / peak;

    const double idealWidth = height * GlyphStemUnits / GlyphHeightUnits;
    double error = 0.0;
    for ( Int32 x=minX-6; x<=maxX+6; ++x )
        error += fabs( CoverageAt( pixels, x, row ) - PixelOverlap( x, centre - idealWidth / 2.0, centre + idealWidth / 2.0 ) );

    quality.found = true;
    quality.height = height;
    quality.stemWidth = stem;
    quality.error = error;
    return quality;
}

bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels, UInt32 width, UInt32 height )
{
    context->RenderFrame( 16 );
    bool captured = context->CaptureFrame( pixels );
    return captured && !pixels.IsNULL() && width == pixels->GetWidthInPixels() && height == pixels->GetHeightInPixels();
}

GUI::CGUIContext*
AsContext( GUI::TGuiContextPtr& pointer )
{
    return pointer.IsNULL() ? GUCEF_NULL : pointer->AsGUIContext();
}

guidriverMyGUI::CMyGUIContext*
AsMyGUIContext( GUI::TGuiContextPtr& pointer )
{
    return static_cast< guidriverMyGUI::CMyGUIContext* >( AsContext( pointer ) );
}

GUI::CWidget*
MakeScreen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

/** A label of the logical rectangle 20,20 300 x 60 with a white text in the font */
GUI::CWidget*
MakeLabel( GUI::CGUIContext* context, GUI::CWidget* parent, const char* text, const char* font )
{
    GUI::CWidget* label = context->CreateWidget( "Label", "Label", parent );
    if ( GUCEF_NULL == label )
        return GUCEF_NULL;
    label->SetProperty( "Position", "0,20 0,20" );
    label->SetProperty( "Size", "0,300 0,60" );
    label->SetProperty( "Text", text );
    label->SetProperty( "Font", font );
    label->SetColourProperty( "TextColour", GUI::CColour( 255, 255, 255, 255 ) );
    return label;
}

/** Makes the MSDF fonts setting of the driver through its configuration */
void
ConfigureDriver( GUI::CGUIDriver* driver, bool msdfFonts, const char* platform )
{
    CORE::CDataNode config( "MyGUI" );
    config.SetAttribute( "MsdfFonts", msdfFonts ? "true" : "false" );
    config.SetAttribute( "Platform", platform );
    driver->LoadConfig( config );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMyGUIMsdfTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MyGUI MSDF fonts" );

    GUI::CGUIDriver* driver = GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "MyGUIOpenGL" );
    if ( GUCEF_NULL == driver )
    {
        // the render tests make the driver known: without it there is nothing to test here
        GUI::TGuiContextPtr first = CreateMyGUIContext();
        driver = GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( "MyGUIOpenGL" );
        if ( GUCEF_NULL == driver )
            return;
    }

    GUCEF_TESTFW_TESTCASE( "Fonts are bitmap fonts by default, the setting is in the configuration of the driver" )
    try
    {
        ASSERT_TRUE( "false" == driver->GetDriverProperty( "MsdfFonts" ) );
        ASSERT_TRUE( "32" == driver->GetDriverProperty( "MsdfFontSize" ) );

        // the configuration is read and written
        CORE::CDataNode config( "MyGUI" );
        config.SetAttribute( "MsdfFonts", "true" );
        config.SetAttribute( "MsdfFontSize", "48" );
        ASSERT_TRUE( driver->LoadConfig( config ) );
        ASSERT_TRUE( "true" == driver->GetDriverProperty( "MsdfFonts" ) );
        ASSERT_TRUE( "48" == driver->GetDriverProperty( "MsdfFontSize" ) );
        CORE::CDataNode saved( "Root" );
        ASSERT_TRUE( driver->SaveConfig( saved ) );
        const CORE::CDataNode* section = saved.FindChild( "MyGUI" );
        ASSERT_TRUE( GUCEF_NULL != section );
        if ( GUCEF_NULL != section )
        {
            ASSERT_TRUE( section->GetAttributeValueOrChildValueByName( "MsdfFonts" ).AsBool( false, true ) );
            ASSERT_TRUE( 48 == section->GetAttributeValueOrChildValueByName( "MsdfFontSize" ).AsUInt32( 0, true ) );
        }
        config.SetAttribute( "MsdfFonts", "false" );
        config.SetAttribute( "MsdfFontSize", "32" );
        ASSERT_TRUE( driver->LoadConfig( config ) );
        ASSERT_TRUE( "false" == driver->GetDriverProperty( "MsdfFonts" ) );
        ASSERT_TRUE( "32" == driver->GetDriverProperty( "MsdfFontSize" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A bitmap font drawn at four times the size is the reference that is blurred" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            throw 0;  // the failed assertion above is the report
        ASSERT_TRUE( context->GetDriver()->HasCapability( "MsdfFonts" ) );
        ASSERT_TRUE( context->GetDriver()->LoadFontFromAsset( FontAsset ) );
        guidriverMyGUI::CMyGUIContext* myguiContext = AsMyGUIContext( contextPtr );
        ASSERT_FALSE( myguiContext->IsMsdfFont( FontName ) );
        ASSERT_TRUE( 0 == myguiContext->GetMyGUIFontHeight( FontName ) );

        // the font manager answers the default font for a name it does not know: it has to be the font itself
        MyGUI::IFont* font = MyGUI::FontManager::getInstance().getByName( FontName );
        ASSERT_TRUE( GUCEF_NULL != font && FontName == font->getResourceName() );
        g_bitmap.textHeight = GUCEF_NULL != font ? font->getDefaultHeight() : 0;
        ASSERT_TRUE( g_bitmap.textHeight > 6 );
        ASSERT_TRUE( context->MeasureText( FontName, "Hello World", 1000, g_bitmap.measuredWidth, g_bitmap.measuredHeight ) );

        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* label = MakeLabel( context, screen, "I", FontName );

        // at render scale 4 the picture is four times as large: a bitmap font is made at one size, so to be
        // as large as the text of the MSDF font it is stretched four times
        context->SetRenderScale( 4 );
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels, 2560, 1920 ) );
        MyGUI::TextBox* text = static_cast< MyGUI::Widget* >( label->GetImplementationPtr() )->castType< MyGUI::TextBox >( false );
        ASSERT_TRUE( GUCEF_NULL != text );
        if ( GUCEF_NULL != text )
            text->setFontHeight( g_bitmap.textHeight * 4 );
        ASSERT_TRUE( Capture( context, pixels, 2560, 1920 ) );

        SGlyphQuality quality = MeasureGlyph( pixels, GUI::CRect( 60, 60, 1000, 320 ) );
        ASSERT_TRUE( quality.found );
        ASSERT_TRUE( quality.height > (double) g_bitmap.textHeight * 4.0 * 0.4 );
        g_bitmap.error = quality.error;
        g_bitmap.glyphHeight = quality.height;
        g_bitmap.valid = quality.found;
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "A TrueType font of the VFS becomes an MSDF font, the text has the size of the bitmap font and stays sharp when it is larger" )
    try
    {
        ConfigureDriver( driver, true, "auto" );
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            throw 0;  // the failed assertion above is the report
        ASSERT_TRUE( "OpenGL3" == driver->GetDriverProperty( "Renderer" ) );
        ASSERT_TRUE( driver->HasCapability( "MsdfFonts" ) );

        // the font that was asked for before the context existed is made as an MSDF font
        guidriverMyGUI::CMyGUIContext* myguiContext = AsMyGUIContext( contextPtr );
        ASSERT_TRUE( myguiContext->SupportsMsdfFonts() );
        ASSERT_TRUE( myguiContext->IsMsdfFont( FontName ) );
        ASSERT_TRUE( g_bitmap.valid );

        // the text is as large as the same text in the bitmap font
        const Int32 logicalHeight = myguiContext->GetMyGUIFontHeight( FontName );
        ASSERT_TRUE( abs( logicalHeight - g_bitmap.textHeight ) <= 1 );
        Int32 width = 0, height = 0;
        ASSERT_TRUE( context->MeasureText( FontName, "Hello World", 1000, width, height ) );
        ASSERT_TRUE( abs( height - g_bitmap.measuredHeight ) <= 1 );
        ASSERT_TRUE( abs( width - g_bitmap.measuredWidth ) * 10 <= g_bitmap.measuredWidth );

        // a font that is added once the context runs is an MSDF font as well, the same name replaces nothing
        ASSERT_TRUE( driver->LoadFontFromAsset( FontAsset ) );
        ASSERT_TRUE( myguiContext->IsMsdfFont( FontName ) );
        ASSERT_FALSE( driver->LoadFontFromAsset( "MyGUI.Engine/no_such_font.ttf" ) );

        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* label = MakeLabel( context, screen, "I", FontName );

        // at the render scale of 1 the glyph is there, at 4 the text is four times the size
        IMAGE::TPixelMapPtr small;
        ASSERT_TRUE( Capture( context, small, 640, 480 ) );
        SGlyphQuality atOne = MeasureGlyph( small, GUI::CRect( 15, 15, 250, 70 ) );
        ASSERT_TRUE( atOne.found );

        context->SetRenderScale( 4 );
        IMAGE::TPixelMapPtr large;
        ASSERT_TRUE( Capture( context, large, 2560, 1920 ) );
        SGlyphQuality atFour = MeasureGlyph( large, GUI::CRect( 60, 60, 1000, 320 ) );
        ASSERT_TRUE( atFour.found );
        if ( atOne.found && atFour.found )
        {
            // four times as high, and the same size as the bitmap font stretched to it
            ASSERT_TRUE( fabs( atFour.height / atOne.height - 4.0 ) < 0.8 );
            ASSERT_TRUE( g_bitmap.valid && fabs( atFour.height - g_bitmap.glyphHeight ) < g_bitmap.glyphHeight * 0.12 );
            // the stem is the proportion of the glyph the font says
            ASSERT_TRUE( fabs( atFour.stemWidth - atFour.height * GlyphStemUnits / GlyphHeightUnits ) < 0.35 );
            // sharp: the picture is the ideal rectangle within a pixel of error along the row, and closer to it than the bitmap font
            ASSERT_TRUE( atFour.error < 0.8 );
            ASSERT_TRUE( g_bitmap.valid && atFour.error < g_bitmap.error );
            ASSERT_TRUE( g_bitmap.valid && atFour.error * 2.0 < g_bitmap.error );
        }

        // the widgets are placed again when the scale changes, the text grows with it, back to one it is the first size again
        context->SetRenderScale( 1 );
        ASSERT_TRUE( Capture( context, small, 640, 480 ) );
        SGlyphQuality again = MeasureGlyph( small, GUI::CRect( 15, 15, 250, 70 ) );
        ASSERT_TRUE( again.found && atOne.found && fabs( again.height - atOne.height ) < 0.01 );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "The font is read through the VFS: a font that only the VFS has, under a name that is no file, is loaded as an MSDF font" )
    try
    {
        ConfigureDriver( driver, true, "auto" );

        // a copy of the font in a directory that is mounted elsewhere in the VFS: the VFS path is not a file path
        CORE::CString directory = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( directory, "mygui_msdf_font_dir" );
        CORE::CreateDirs( directory );
        CORE::CString source = CORE::RelativePath( "$MODULEDIR$/../data/MyGUI.Engine/DejaVuSans.ttf" );
        CORE::CString copy = directory;
        CORE::AppendToPath( copy, "VfsOnlyFont.ttf" );
        ASSERT_TRUE( CORE::CopyFile( source, copy, true ) );

        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const char* mountPoint = "mygui_msdf_vfs/fonts";
        ASSERT_TRUE( vfs.MountArchive( "mygui_msdf_fonts", directory, VFS::CVFS::FileSystemArchiveTypeName, mountPoint, false, false, directory ) );
        const CORE::CString asset = CORE::CString( mountPoint ) + "/VfsOnlyFont.ttf";
        ASSERT_TRUE( vfs.FileExists( asset ) );
        ASSERT_FALSE( CORE::FileExists( asset ) );

        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            guidriverMyGUI::CMyGUIContext* myguiContext = AsMyGUIContext( contextPtr );
            ASSERT_TRUE( driver->LoadFontFromAsset( asset ) );
            ASSERT_TRUE( driver->GetAvailableFonts().count( "VfsOnlyFont" ) > 0 );
            ASSERT_TRUE( myguiContext->IsMsdfFont( "VfsOnlyFont" ) );

            // MyGUI has one texture of distance fields for the font
            MyGUI::IFont* font = MyGUI::FontManager::getInstance().getByName( "VfsOnlyFont" );
            ASSERT_TRUE( GUCEF_NULL != font && GUCEF_NULL != font->getTextureFont() );
            if ( GUCEF_NULL != font && GUCEF_NULL != font->getTextureFont() )
                ASSERT_TRUE( font->getTextureFont()->getWidth() > 64 && font->getTextureFont()->getHeight() > 64 );

            GUI::CWidget* screen = MakeScreen( context );
            MakeLabel( context, screen, "Font from the VFS", "VfsOnlyFont" );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( context, pixels, 640, 480 ) );
            ASSERT_TRUE( CountLit( pixels, GUI::CRect( 20, 20, 300, 60 ) ) > 30 );
            context->DestroyWidget( screen );
        }
        contextPtr = GUI::TGuiContextPtr();
        vfs.UnmountArchiveByName( "mygui_msdf_fonts" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "On the legacy OpenGL platform, which has no shaders, the fonts stay bitmap fonts and are drawn" )
    try
    {
        ConfigureDriver( driver, true, "OpenGL" );
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( "OpenGL" == driver->GetDriverProperty( "Renderer" ) );
            ASSERT_TRUE( "true" == driver->GetDriverProperty( "MsdfFonts" ) );
            ASSERT_FALSE( driver->HasCapability( "MsdfFonts" ) );

            guidriverMyGUI::CMyGUIContext* myguiContext = AsMyGUIContext( contextPtr );
            ASSERT_FALSE( myguiContext->SupportsMsdfFonts() );

            // the font that was asked for is there, as a bitmap font, and its text is drawn
            ASSERT_TRUE( myguiContext->HasMyGUIFont( FontName ) );
            ASSERT_FALSE( myguiContext->IsMsdfFont( FontName ) );
            ASSERT_TRUE( 0 == myguiContext->GetMyGUIFontHeight( FontName ) );
            ASSERT_TRUE( driver->LoadFontFromAsset( FontAsset ) );
            ASSERT_FALSE( myguiContext->IsMsdfFont( FontName ) );
            ASSERT_FALSE( myguiContext->LoadMsdfFont( "NoMsdf", FontAsset, 12, 32 ) );
            ASSERT_FALSE( myguiContext->HasMyGUIFont( "NoMsdf" ) );

            // the text is measured with the bitmap font and a label with the font draws without trouble; what it
            // draws is not checked: the offscreen EGL window has a core profile context, which has no fixed
            // function pipeline for the legacy platform to draw with
            GUI::CWidget* screen = MakeScreen( context );
            MakeLabel( context, screen, "Bitmap font", FontName );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( context, pixels, 640, 480 ) );
            Int32 width = 0, height = 0;
            ASSERT_TRUE( context->MeasureText( FontName, "Hello World", 1000, width, height ) );
            ASSERT_TRUE( width == g_bitmap.measuredWidth && height == g_bitmap.measuredHeight );
            context->DestroyWidget( screen );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "With MSDF fonts off again the next context has bitmap fonts, on the OpenGL 3 platform" )
    try
    {
        ConfigureDriver( driver, false, "auto" );
        GUI::TGuiContextPtr contextPtr = CreateMyGUIContext();
        GUI::CGUIContext* context = AsContext( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( "OpenGL3" == driver->GetDriverProperty( "Renderer" ) );
            guidriverMyGUI::CMyGUIContext* myguiContext = AsMyGUIContext( contextPtr );
            ASSERT_TRUE( myguiContext->HasMyGUIFont( FontName ) );
            ASSERT_FALSE( myguiContext->IsMsdfFont( FontName ) );
            ASSERT_TRUE( myguiContext->SupportsMsdfFonts() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

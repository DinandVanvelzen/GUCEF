/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#include "guidriverRmlUI_CRmlUIContext.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */

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

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

/** Same colour, give or take a little for the rounding of blending on a GPU */
bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance = 2 )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance &&
           abs( (Int32) a.b - (Int32) b.b ) <= tolerance;
}

/** Counts the pixels of a colour in a rectangle */
UInt32
CountColour( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect, const GUI::CColour& colour )
{
    UInt32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( Near( PixelAt( pixels, x, y ), colour ) )
                ++count;
    return count;
}

GUI::CWidget*
Screen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels )
{
    context->RenderFrame( 16 );
    return context->CaptureFrame( pixels ) && !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() && 480 == pixels->GetHeightInPixels();
}

GUI::CVideoOptions::TDisplayMode
TestDisplayMode( UInt32 width, UInt32 height )
{
    GUI::CVideoOptions::TDisplayMode mode;
    mode.widthInPixels = width;
    mode.heightInPixels = height;
    mode.depthInBits = 32;
    mode.frequency = 60;
    return mode;
}

const GUI::CColour Black( 0, 0, 0, 255 );
const GUI::CColour Red( 255, 0, 0, 255 );
const GUI::CColour Green( 0, 255, 0, 255 );
const GUI::CColour Blue( 0, 0, 255, 255 );
const GUI::CColour White( 255, 255, 255, 255 );
const GUI::CColour Yellow( 255, 255, 0, 255 );

/** A picture with a different colour for every pixel */
IMAGE::TPixelMapPtr
MakeGradientImage( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr pixels = MakeSolidImage( width, height, 0, 0, 0, 255 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt8* p = data + ( (size_t) y * width + x ) * 4;
            p[ 0 ] = (UInt8) ( 20 + x * 25 );
            p[ 1 ] = (UInt8) ( 20 + y * 25 );
            p[ 2 ] = (UInt8) ( 200 - ( x + y ) * 10 );
        }
    return pixels;
}

/** The picture blown up 'factor' times with every pixel a block */
IMAGE::TPixelMapPtr
Blocks( const IMAGE::CPixelMap& source, UInt32 factor )
{
    UInt32 width = source.GetWidthInPixels(), height = source.GetHeightInPixels();
    IMAGE::TPixelMapPtr result = MakeSolidImage( width * factor, height * factor, 0, 0, 0, 0 );
    const UInt8* in = static_cast< const UInt8* >( source.GetDataPtr() );
    UInt8* out = static_cast< UInt8* >( result->GetDataPtr() );
    for ( UInt32 y=0; y<height*factor; ++y )
        for ( UInt32 x=0; x<width*factor; ++x )
            memcpy( out + ( (size_t) y * width * factor + x ) * 4, in + ( (size_t) ( y / factor ) * width + x / factor ) * 4, 4 );
    return result;
}

/** At a render scale of 2: pictures and a font of density 2 next to ones of density 1, a fill, a line and text */
void
BuildDensityScene( GUI::CGUIContext* context )
{
    context->SetRenderScale( 2 );

    IMAGE::TPixelMapPtr fine = MakeGradientImage( 8, 8 );
    IMAGE::TPixelMapPtr coarse = MakeGradientImage( 4, 4 );
    context->RegisterImage( "Fine", fine, 2 );
    context->RegisterImage( "Coarse", coarse );

    // the same font as a density 2 copy with its atlas blown up
    GUI::TBitmapFontPtr font = MakeTestFont( 6, 9 );
    GUI::TBitmapFontPtr font2( GUCEF_NEW GUI::CBitmapFont( *font ) );
    font2->SetAtlas( Blocks( *font->GetAtlas(), 2 ) );
    font2->SetDensity( 2 );
    GUI::CBitmapFont::TGlyphMap glyphs = font->GetGlyphs();
    for ( GUI::CBitmapFont::TGlyphMap::iterator g=glyphs.begin(); g!=glyphs.end(); ++g )
    {
        GUI::CBitmapFont::TGlyph glyph = (*g).second;
        glyph.source = GUI::CRect( glyph.source.left * 2, glyph.source.top * 2, glyph.source.width * 2, glyph.source.height * 2 );
        font2->SetGlyph( (*g).first, glyph );
    }
    context->RegisterFont( "Block", font );
    context->RegisterFont( "Block2", font2 );

    GUI::CWidget* screen = Screen( context );
    screen->SetColourProperty( "BackgroundColour", GUI::CColour( 30, 30, 30, 255 ) );
    const char* names[] = { "Fine", "Coarse" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* frame = context->CreateWidget( "ImageFrame", names[ i ], screen );
        frame->SetProperty( "Position", "0," + CORE::Int32ToString( 10 + (Int32) i * 20 ) + " 0,10" );
        frame->SetProperty( "Size", "0,4 0,4" );
        frame->SetProperty( "Image", names[ i ] );
        frame->SetProperty( "ScaleMode", "None" );
    }
    GUI::CWidget* panel = context->CreateWidget( "Panel", "Fill", screen );
    panel->SetProperty( "Position", "0,60 0,10" );
    panel->SetProperty( "Size", "0,5 0,5" );
    panel->SetColourProperty( "BackgroundColour", GUI::CColour( 250, 100, 0, 255 ) );
    const char* fonts[] = { "Block", "Block2" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* label = context->CreateWidget( "Label", CORE::CString( "Text" ) + fonts[ i ], screen );
        label->SetProperty( "Position", "0,10 0," + CORE::Int32ToString( 40 + (Int32) i * 20 ) );
        label->SetProperty( "Size", "0,100 0,15" );
        label->SetProperty( "Font", fonts[ i ] );
        label->SetProperty( "Text", "AbCd" );
        label->SetColourProperty( "TextColour", GUI::CColour( 255, 255, 255, 255 ) );
    }
    GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Surface", screen ) );
    surface->SetProperty( "Position", "0,120 0,10" );
    surface->SetProperty( "Size", "0,40 0,40" );
    surface->DrawLine( 0, 0, 30, 20, GUI::CColour( 0, 255, 0, 255 ) );
    surface->Commit();
}

/** Panels, borders, clipping, opacity, scaled and tinted pictures, text, lines and points in one picture */
void
BuildMixedScene( GUI::CGUIContext* context )
{
    context->RegisterFont( "Block", MakeTestFont( 6, 9 ) );
    IMAGE::TPixelMapPtr checker = MakeSolidImage( 4, 4, 255, 255, 255, 255 );
    UInt8* data = static_cast< UInt8* >( checker->GetDataPtr() );
    for ( UInt32 i=0; i<16; ++i )
    {
        if ( 0 == ( ( i % 4 ) + ( i / 4 ) ) % 2 )
        {
            data[ i*4 ] = 200; data[ i*4+1 ] = 30; data[ i*4+2 ] = 60; data[ i*4+3 ] = ( i % 3 ) ? 255 : 128;
        }
    }
    context->RegisterImage( "Checker", checker );

    GUI::CWidget* screen = Screen( context );
    screen->SetColourProperty( "BackgroundColour", GUI::CColour( 20, 40, 60, 255 ) );

    GUI::CWidget* panel = context->CreateWidget( "Panel", "Panel", screen );
    panel->SetProperty( "Position", "0,13 0,17" );
    panel->SetProperty( "Size", "0,201 0,143" );
    panel->SetColourProperty( "BackgroundColour", GUI::CColour( 90, 120, 30, 200 ) );
    panel->SetColourProperty( "BorderColour", White );
    panel->SetBoolProperty( "Clip", true );
    GUI::CWidget* inner = context->CreateWidget( "Panel", "Inner", panel );
    inner->SetProperty( "Position", "0,150 0,100" );
    inner->SetProperty( "Size", "0,90 0,70" );
    inner->SetColourProperty( "BackgroundColour", GUI::CColour( 250, 200, 10, 255 ) );
    inner->SetFloatProperty( "Opacity", 0.6f );

    GUI::CWidget* frame = context->CreateWidget( "ImageFrame", "Checker", screen );
    frame->SetProperty( "Position", "0,240 0,20" );
    frame->SetProperty( "Size", "0,48 0,36" );
    frame->SetProperty( "Image", "Checker" );
    frame->SetProperty( "ScaleMode", "Stretch" );

    GUI::CWidget* label = context->CreateWidget( "Label", "Label", screen );
    label->SetProperty( "Position", "0,300 0,40" );
    label->SetProperty( "Size", "0,120 0,40" );
    label->SetProperty( "Font", "Block" );
    label->SetProperty( "Text", "Hello there wide world" );
    label->SetBoolProperty( "WordWrap", true );
    label->SetColourProperty( "TextColour", GUI::CColour( 120, 220, 255, 230 ) );

    GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Surface", screen ) );
    surface->SetProperty( "Position", "0,20 0,200" );
    surface->SetProperty( "Size", "0,300 0,200" );
    surface->FillRect( GUI::CRect( 5, 5, 40, 30 ), GUI::CColour( 255, 0, 255, 160 ) );
    surface->StrokeRect( GUI::CRect( 50, 5, 40, 30 ), Green, 3 );
    surface->DrawLine( 0, 199, 299, 50, Yellow );
    surface->DrawLine( 10, 60, 120, 190, Red, 3 );
    surface->DrawLine( 150, 120, 150, 180, White );
    GUI::CDrawSurface::TPointVector points;
    for ( Int32 i=0; i<30; ++i )
    {
        GUI::CDrawSurface::SPoint point;
        point.x = 200 + i * 3;
        point.y = 20 + ( i * 7 ) % 40;
        points.push_back( point );
    }
    surface->DrawPoints( points, White );
    surface->PushClip( GUI::CRect( 100, 60, 50, 50 ) );
    surface->DrawImage( "Checker", GUI::CRect( 90, 50, 70, 70 ), GUI::CRect( 1, 1, 3, 3 ), GUI::CColour( 255, 255, 128, 200 ) );
    surface->PopClip();
    surface->DrawString( "Block", "Text!", 170, 150, GUI::CColour( 255, 160, 0, 255 ) );
    surface->Commit();
}

/*-------------------------------------------------------------------------*/

/** The same drawing tests for every driver that can capture what it draws */
void
RunRenderTests( const char* driverName, GUI::TGuiContextPtr ( *createContext )( void ) )
{
    CORE::CString prefix = CORE::CString( "Render (" ) + driverName + "): ";

    GUCEF_TESTFW_TESTCASE( ( prefix + "Panels fill and draw borders, children are cut off where Clip is set" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = ( contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* panel = context->CreateWidget( "Panel", "Panel", screen );
        panel->SetProperty( "Position", "0,10 0,10" );
        panel->SetProperty( "Size", "0,100 0,50" );
        panel->SetColourProperty( "BackgroundColour", Blue );
        panel->SetColourProperty( "BorderColour", White );
        GUI::CWidget* child = context->CreateWidget( "Panel", "Child", panel );
        child->SetProperty( "Position", "0,80 0,20" );
        child->SetProperty( "Size", "0,50 0,10" );
        child->SetColourProperty( "BackgroundColour", Red );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 5, 5 ), Black ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 10, 10 ), White ) && Near( PixelAt( pixels, 109, 59 ), White ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 50, 30 ), Blue ) );
        // the child sticks out of the panel: shown outside without Clip
        ASSERT_TRUE( Near( PixelAt( pixels, 95, 35 ), Red ) && Near( PixelAt( pixels, 130, 35 ), Red ) );
        panel->SetBoolProperty( "Clip", true );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 95, 35 ), Red ) && Near( PixelAt( pixels, 130, 35 ), Black ) );

        // half transparent over blue
        child->SetProperty( "Position", "0,20 0,20" );
        child->SetFloatProperty( "Opacity", 0.5f );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 35 ), GUI::CColour( 128, 0, 128, 255 ), 3 ) );

        // hidden: gone
        panel->SetVisibility( false );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 50, 30 ), Black ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( prefix + "A group box draws a frame whose top line has a gap for the caption" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = ( contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidget* screen = Screen( context );
        GUI::CGroupBox* group = GUI::CWidget::As< GUI::CGroupBox >( context->CreateWidget( "GroupBox", "Group", screen ) );
        ASSERT_TRUE( GUCEF_NULL != group );
        if ( GUCEF_NULL == group )
            return;
        group->SetProperty( "Position", "0,20 0,20" );
        group->SetProperty( "Size", "0,200 0,0" );
        group->SetProperty( "AutoHeight", "false" );
        group->SetProperty( "Size", "0,200 0,100" );
        group->SetColourProperty( "FrameColour", White );
        group->SetCaption( "Group" );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        const Int32 lineY = 20 + group->GetCaptionHeight() / 2;
        // the sides and the bottom
        ASSERT_TRUE( Near( PixelAt( pixels, 20, 80 ), White ) && Near( PixelAt( pixels, 219, 80 ), White ) && Near( PixelAt( pixels, 100, 119 ), White ) );
        // the top line left of the caption and right of it, and the gap before the caption
        ASSERT_TRUE( Near( PixelAt( pixels, 24, lineY ), White ) && Near( PixelAt( pixels, 210, lineY ), White ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 30, lineY ), Black ) );
        // nothing above the line
        ASSERT_TRUE( Near( PixelAt( pixels, 20, 20 ), Black ) || 0 == group->GetCaptionHeight() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( prefix + "Picture buttons show the picture of their state, images scale, cut out and tint" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterImage( "Normal", MakeSolidImage( 20, 10, 255, 0, 0, 255 ) );
        context->RegisterImage( "Hover", MakeSolidImage( 20, 10, 0, 255, 0, 255 ) );
        context->RegisterImage( "Pushed", MakeSolidImage( 20, 10, 0, 0, 255, 255 ) );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* button = context->CreateWidget( "Button", "Button", screen );
        button->SetProperty( "Position", "0,100 0,100" );
        button->SetProperty( "NormalImage", "Normal" );
        button->SetProperty( "HoverImage", "Hover" );
        button->SetProperty( "PushedImage", "Pushed" );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 200 == CountColour( pixels, GUI::CRect( 100, 100, 20, 10 ), Red ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 120, 105 ), Black ) );
        context->InjectMouseMove( 105, 105 );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 105, 105 ), Green ) );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 105, 105 ), Blue ) );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );

        // a 2x2 picture of four colours, made larger: nearest pixel, no blurring
        IMAGE::TPixelMapPtr quad = MakeSolidImage( 2, 2, 0, 0, 0, 255 );
        UInt8* q = static_cast< UInt8* >( quad->GetDataPtr() );
        q[ 0 ] = 255;                 // top left red
        q[ 5 ] = 255;                 // top right green
        q[ 10 ] = 255;                // bottom left blue
        q[ 12 ] = q[ 13 ] = q[ 14 ] = 255;  // bottom right white
        context->RegisterImage( "Quad", quad );
        GUI::CWidget* frame = context->CreateWidget( "ImageFrame", "Frame", screen );
        frame->SetProperty( "Position", "0,200 0,200" );
        frame->SetProperty( "Size", "0,40 0,20" );
        frame->SetProperty( "Image", "Quad" );
        ASSERT_TRUE( Capture( context, pixels ) );
        // stretched
        ASSERT_TRUE( 200 == CountColour( pixels, GUI::CRect( 200, 200, 20, 10 ), Red ) );
        ASSERT_TRUE( 200 == CountColour( pixels, GUI::CRect( 220, 210, 20, 10 ), White ) );
        // fitted: 20x20 in the middle of 40x20
        frame->SetProperty( "ScaleMode", "Fit" );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 205, 205 ), Black ) && Near( PixelAt( pixels, 212, 205 ), Red ) && Near( PixelAt( pixels, 227, 215 ), White ) );
        // one pixel of it, tinted
        frame->SetProperty( "ScaleMode", "Stretch" );
        frame->SetProperty( "SubRect", "1,1,1,1" );
        frame->SetColourProperty( "Tint", GUI::CColour( 255, 255, 0, 255 ) );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 800 == CountColour( pixels, GUI::CRect( 200, 200, 40, 20 ), Yellow ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( prefix + "Text in a bitmap font is drawn in its colour, aligned in the label" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Block", MakeTestFont( 8, 10 ) );
        GUI::CWidget* screen = Screen( context );
        GUI::CWidget* label = context->CreateWidget( "Label", "Label", screen );
        label->SetProperty( "Position", "0,100 0,50" );
        label->SetProperty( "Size", "0,100 0,30" );
        label->SetProperty( "Font", "Block" );
        label->SetProperty( "Text", "abc" );
        label->SetColourProperty( "TextColour", Yellow );
        label->SetProperty( "HTextAlign", "Center" );
        label->SetProperty( "VTextAlign", "Center" );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        // 24x10 of text in the middle of 100x30: from (138,60)
        ASSERT_TRUE( 240 == CountColour( pixels, GUI::CRect( 100, 50, 100, 30 ), Yellow ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 138, 60 ), Yellow ) && Near( PixelAt( pixels, 161, 69 ), Yellow ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 137, 60 ), Black ) && Near( PixelAt( pixels, 162, 60 ), Black ) );

        // a picture with transparent parts lets the background through
        IMAGE::TPixelMapPtr holes = MakeSolidImage( 2, 1, 255, 0, 0, 255 );
        static_cast< UInt8* >( holes->GetDataPtr() )[ 7 ] = 0;
        context->RegisterImage( "Holes", holes );
        GUI::CWidget* frame = context->CreateWidget( "ImageFrame", "Holes", screen );
        frame->SetProperty( "Position", "0,300 0,300" );
        frame->SetProperty( "Size", "0,20 0,10" );
        frame->SetProperty( "Image", "Holes" );
        screen->SetColourProperty( "BackgroundColour", Blue );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 305, 305 ), Red ) && Near( PixelAt( pixels, 315, 305 ), Blue ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( prefix + "A draw surface draws its committed commands within its clip, roots stack in order, the cursor is on top" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Block", MakeTestFont( 8, 10 ) );
        context->RegisterImage( "Dot", MakeSolidImage( 3, 3, 0, 255, 0, 255 ) );
        GUI::CWidget* screen = Screen( context );
        GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Map", screen ) );
        surface->SetProperty( "Position", "0,10 0,10" );
        surface->SetProperty( "Size", "0,200 0,100" );
        surface->FillRect( GUI::CRect( 0, 0, 10, 10 ), Red );
        surface->DrawLine( 20, 5, 29, 5, White );
        GUI::CDrawSurface::TPointVector points( 1 );
        points[ 0 ].x = 40; points[ 0 ].y = 40;
        surface->DrawPoints( points, Yellow );
        surface->DrawImage( "Dot", GUI::CRect( 50, 50, 6, 6 ) );
        surface->PushClip( GUI::CRect( 100, 0, 5, 5 ) );
        surface->FillRect( GUI::CRect( 100, 0, 10, 10 ), Blue );
        surface->PopClip();
        surface->DrawString( "Block", "x", 150, 0, Yellow );
        // outside the surface: cut off
        surface->FillRect( GUI::CRect( 190, 90, 20, 20 ), Red );
        surface->Commit();

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 100 == CountColour( pixels, GUI::CRect( 10, 10, 10, 10 ), Red ) );
        ASSERT_TRUE( 10 == CountColour( pixels, GUI::CRect( 30, 15, 10, 1 ), White ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 50, 50 ), Yellow ) );
        ASSERT_TRUE( 36 == CountColour( pixels, GUI::CRect( 60, 60, 6, 6 ), Green ) );
        ASSERT_TRUE( 25 == CountColour( pixels, GUI::CRect( 110, 10, 10, 10 ), Blue ) );
        ASSERT_TRUE( 80 == CountColour( pixels, GUI::CRect( 160, 10, 8, 10 ), Yellow ) );
        ASSERT_TRUE( 100 == CountColour( pixels, GUI::CRect( 200, 100, 20, 20 ), Red ) );

        // a second root over the first, then the cursor over both
        GUI::CWidget* top = context->CreateWidget( "Panel", "Top", GUCEF_NULL );
        top->SetProperty( "Position", "0,10 0,10" );
        top->SetProperty( "Size", "0,5 0,5" );
        top->SetColourProperty( "BackgroundColour", White );
        context->ShowRoot( top, false );
        context->RegisterImage( "Cursor", MakeSolidImage( 4, 4, 255, 0, 255, 255 ) );
        context->SetCursor( "Cursor", 1, 1 );
        context->InjectMouseMove( 300, 300 );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 12, 12 ), White ) && Near( PixelAt( pixels, 16, 16 ), Red ) );
        ASSERT_TRUE( 16 == CountColour( pixels, GUI::CRect( 299, 299, 4, 4 ), GUI::CColour( 255, 0, 255, 255 ) ) );
        context->SetCursorVisible( false );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 0 == CountColour( pixels, GUI::CRect( 299, 299, 4, 4 ), GUI::CColour( 255, 0, 255, 255 ) ) );

        // the tooltip comes when the pointer rests
        top->SetProperty( "Tooltip", "hi" );
        top->SetProperty( "Font", "Block" );
        context->InjectMouseMove( 12, 12 );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( 0 == CountColour( pixels, GUI::CRect( 24, 28, 24, 14 ), GUI::CColour( 250, 250, 210, 255 ) ) );
        context->RenderFrame( 700 );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( CountColour( pixels, GUI::CRect( 24, 28, 24, 14 ), GUI::CColour( 250, 250, 210, 255 ) ) > 0 );
        context->DestroyWidget( top );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( prefix + "A window has a fold button and folds up to its title bar, a scroll panel cuts off what is scrolled out and draws its scroll bar" ).C_String() )
    try
    {
        GUI::TGuiContextPtr contextPtr = createContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = Screen( context );

        // a window: the title bar with a glyph on the fold button, the body with a red child
        GUI::CWindow* window = static_cast< GUI::CWindow* >( context->CreateWidget( "Window", "Window", screen ) );
        window->SetProperty( "Position", "0,20 0,20" );
        window->SetProperty( "Size", "0,200 0,100" );
        window->SetIntProperty( "TitleHeight", 20 );
        window->SetColourProperty( "BackgroundColour", Blue );
        window->SetColourProperty( "TitleBarColour", GUI::CColour( 40, 60, 110, 255 ) );
        window->SetColourProperty( "TitleColour", White );
        GUI::CWidget* child = context->CreateWidget( "Panel", "Child", window );
        child->SetProperty( "Position", "0,10 0,40" );
        child->SetProperty( "Size", "0,50 0,20" );
        child->SetColourProperty( "BackgroundColour", Red );

        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 25 ), GUI::CColour( 40, 60, 110, 255 ) ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 105 ), Blue ) && Near( PixelAt( pixels, 40, 70 ), Red ) );
        UInt32 glyph = 0;
        for ( Int32 y=20; y<40; ++y )
            for ( Int32 x=200; x<220; ++x )
                if ( Near( PixelAt( pixels, x, y ), White ) )
                    ++glyph;
        ASSERT_TRUE( glyph >= 10 && glyph <= 40 );

        // folded up: the title bar and the glyph, nothing of the body and its child
        window->SetBoolProperty( "Collapsed", true );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 25 ), GUI::CColour( 40, 60, 110, 255 ) ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 105 ), Black ) && Near( PixelAt( pixels, 40, 70 ), Black ) );
        UInt32 foldedGlyph = 0;
        for ( Int32 y=20; y<40; ++y )
            for ( Int32 x=200; x<220; ++x )
                if ( Near( PixelAt( pixels, x, y ), White ) )
                    ++foldedGlyph;
        ASSERT_TRUE( foldedGlyph == glyph );
        window->SetBoolProperty( "Collapsed", false );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 40, 70 ), Red ) );

        // a scroll panel of 100 x 60 with a content of 100: a red and a blue part, the scroll bar on the right
        GUI::CScrollPanel* scroll = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Scroll", screen ) );
        scroll->SetProperty( "Position", "0,300 0,140" );
        scroll->SetProperty( "Size", "0,100 0,60" );
        scroll->SetColourProperty( "TrackColour", GUI::CColour( 30, 30, 30, 255 ) );
        scroll->SetColourProperty( "ThumbColour", Yellow );
        GUI::CWidget* top = context->CreateWidget( "Panel", "Top", scroll );
        top->SetProperty( "Size", "1,0 0,50" );
        top->SetColourProperty( "BackgroundColour", Red );
        GUI::CWidget* bottom = context->CreateWidget( "Panel", "Bottom", scroll );
        bottom->SetProperty( "Position", "0,0 0,50" );
        bottom->SetProperty( "Size", "1,0 0,50" );
        bottom->SetColourProperty( "BackgroundColour", Blue );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 320, 160 ), Red ) && Near( PixelAt( pixels, 320, 195 ), Blue ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 320, 205 ), Black ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 394, 142 ), Yellow ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 394, 198 ), GUI::CColour( 30, 30, 30, 255 ) ) );

        // scrolled to the end: the blue part is at the top, the thumb at the bottom
        scroll->SetScrollPosition( 40 );
        ASSERT_TRUE( Capture( context, pixels ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 320, 145 ), Red ) && Near( PixelAt( pixels, 320, 195 ), Blue ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 394, 195 ), Yellow ) && Near( PixelAt( pixels, 394, 142 ), GUI::CColour( 30, 30, 30, 255 ) ) );
        ASSERT_TRUE( Near( PixelAt( pixels, 320, 139 ), Black ) && Near( PixelAt( pixels, 320, 201 ), Black ) );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/** Counts the log messages that contain a text */
class CLogTextCounter : public CORE::CILogger
{
    public:

    CORE::CString m_text;
    UInt32 m_count;
    MT::CMutex m_lock;

    CLogTextCounter( const CORE::CString& text ) : CORE::CILogger(), m_text( text ), m_count( 0 ), m_lock() {}

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        if ( logMessage.HasSubstr( m_text ) >= 0 )
            ++m_count;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const UInt32 threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiRenderTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Render" );

    // the null driver draws with the reference software renderer
    RunRenderTests( "Null", &CreateNullContext );

    // RmlUi drawing with OpenGL in an offscreen window (WebGL in a canvas in a browser): the same tests, then the
    // same pixels as the reference
    bool haveRmlUi = false;
    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The RmlUi driver makes a context in a window, offscreen with EGL or a WebGL canvas in a browser" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !IsInBrowser() )
        GUCEF_TESTFW_SKIP( "there is no web page to draw in outside a browser" );
    #endif
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateRmlUiContext();
        GUI::CGUIContext* context = ( contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext() );
        ASSERT_TRUE( GUCEF_NULL != context );
        ASSERT_TRUE( GUCEF_NULL != context && "RmlUi" == context->GetDriver()->GetDriverName() );
        ASSERT_TRUE( GUCEF_NULL != context && context->GetDriver()->HasCapability( "CaptureFrame" ) );
        haveRmlUi = GUCEF_NULL != context;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !haveRmlUi )
        return;
    RunRenderTests( "RmlUi", &CreateRmlUiContext );

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): A scene of every kind of drawing has the same pixels as the software reference" )
    try
    {
        GUI::TGuiContextPtr referencePtr = CreateNullContext();
        GUI::TGuiContextPtr rmlPtr = CreateRmlUiContext();
        GUI::CGUIContext* reference = static_cast< GUI::CGUIContext* >( referencePtr.GetPointerAlways() );
        GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
        BuildMixedScene( reference );
        BuildMixedScene( rml );

        IMAGE::TPixelMapPtr expected, actual;
        ASSERT_TRUE( Capture( reference, expected ) );
        ASSERT_TRUE( Capture( rml, actual ) );
        UInt32 different = 0;
        Int32 firstX = -1, firstY = -1;
        for ( Int32 y=0; y<480; ++y )
        {
            for ( Int32 x=0; x<640; ++x )
            {
                if ( !Near( PixelAt( expected, x, y ), PixelAt( actual, x, y ) ) )
                {
                    if ( 0 == different )
                    {
                        firstX = x;
                        firstY = y;
                    }
                    ++different;
                }
            }
        }
        if ( 0 != different )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUi differs from the reference in " << different << " pixels, the first at " << firstX << "," << firstY );
        ASSERT_TRUE( 0 == different );

        // nothing changed: the second frame draws from the geometry and textures of the first
        UInt32 textures = static_cast< guidriverRmlUI::CRmlUIContext* >( rml )->GetTextureCreationCount();
        ASSERT_TRUE( Capture( rml, actual ) );
        ASSERT_TRUE( textures == static_cast< guidriverRmlUI::CRmlUIContext* >( rml )->GetTextureCreationCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The file open dialog in the dark theme with a TrueType font has the same pixels as the software reference" )
    if ( !LoadFreeTypeFontPlugin() )
        GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
    try
    {
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefGUI_TestApp_DialogRender" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( CORE::CombinePath( dir, "Saves" ) ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "disc.cue" ), "FILE disc.bin BINARY" ) );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "notes.txt" ), "notes" ) );
        CORE::TFileSystemAccessPtr native = ( GUCEF_NEW CORE::CNativeFileSystemAccess( "This computer" ) )->CreateSharedPtr();

        GUI::TGuiContextPtr referencePtr = CreateNullContext();
        GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( 1280, 960 );
        GUI::CGUIContext* contexts[ 2 ] = { static_cast< GUI::CGUIContext* >( referencePtr.GetPointerAlways() ), static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() ) };
        ASSERT_TRUE( GUCEF_NULL != contexts[ 0 ] && GUCEF_NULL != contexts[ 1 ] && GUI::CGuiGlobal::Instance()->MountData() );

        GUI::CFileOpenDialogForm* dialogs[ 2 ] = { GUCEF_NULL, GUCEF_NULL };
        IMAGE::TPixelMapPtr frames[ 2 ];
        for ( UInt32 c=0; c<2 && GUCEF_NULL != contexts[ 1 ]; ++c )
        {
            GUI::CGUIContext* context = contexts[ c ];
            context->SetLogicalResolution( 1280, 960 );
            ASSERT_TRUE( context->RegisterFont( "UI", GUI::CGuiGlobal::DefaultUiFontPath, 18 ) );
            CORE::CDataNode theme;
            GUI::BuildDarkTheme( theme, "UI" );
            context->SetTheme( theme );
            Screen( context )->SetColourProperty( "BackgroundColour", GUI::CColour( 60, 70, 40, 255 ) );

            dialogs[ c ] = GUCEF_NEW GUI::CFileOpenDialogForm( context );
            dialogs[ c ]->GetOptions().acceptDirectoriesToo = true;
            dialogs[ c ]->AddRoot( "This computer", native, dir );
            CORE::CString::StringVector disc;
            disc.push_back( "*.cue" );
            disc.push_back( "*.iso" );
            dialogs[ c ]->AddFileType( "Game disc", disc );
            ASSERT_TRUE( dialogs[ c ]->Open() );
            context->RenderFrame( 16 );
            CORE::CString::StringVector pick;
            pick.push_back( "disc.cue" );
            ASSERT_TRUE( dialogs[ c ]->SelectNames( pick ) );

            context->RenderFrame( 16 );
            ASSERT_TRUE( context->CaptureFrame( frames[ c ] ) && !frames[ c ].IsNULL() && 1280 == frames[ c ]->GetWidthInPixels() );
        }
        if ( !frames[ 0 ].IsNULL() && !frames[ 1 ].IsNULL() )
        {
            UInt32 different = 0;
            for ( Int32 y=0; y<960; ++y )
                for ( Int32 x=0; x<1280; ++x )
                    if ( !Near( PixelAt( frames[ 0 ], x, y ), PixelAt( frames[ 1 ], x, y ) ) )
                        ++different;
            if ( 0 != different )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUi differs from the reference in " << different << " pixels" );
            ASSERT_TRUE( 0 == different );
        }
        for ( UInt32 c=0; c<2; ++c )
            GUCEF_DELETE dialogs[ c ];
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The settings form in the dark theme with a TrueType font has the same pixels as the software reference" )
    if ( !LoadFreeTypeFontPlugin() )
        GUCEF_TESTFW_SKIP( "the FreeType font plugin is not there" );
    try
    {
        GUI::TGuiContextPtr referencePtr = CreateNullContext();
        GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( 1280, 960 );
        GUI::CGUIContext* contexts[ 2 ] = { static_cast< GUI::CGUIContext* >( referencePtr.GetPointerAlways() ), static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() ) };
        ASSERT_TRUE( GUCEF_NULL != contexts[ 0 ] && GUCEF_NULL != contexts[ 1 ] && GUI::CGuiGlobal::Instance()->MountData() );

        GUI::CSettingsForm* forms[ 2 ] = { GUCEF_NULL, GUCEF_NULL };
        IMAGE::TPixelMapPtr frames[ 2 ];
        for ( UInt32 c=0; c<2 && GUCEF_NULL != contexts[ 1 ]; ++c )
        {
            GUI::CGUIContext* context = contexts[ c ];
            context->SetLogicalResolution( 1280, 960 );
            ASSERT_TRUE( context->RegisterFont( "UI", GUI::CGuiGlobal::DefaultUiFontPath, 18 ) );
            CORE::CDataNode theme;
            GUI::BuildDarkTheme( theme, "UI" );
            context->SetTheme( theme );
            Screen( context )->SetColourProperty( "BackgroundColour", GUI::CColour( 60, 70, 40, 255 ) );

            forms[ c ] = GUCEF_NEW GUI::CSettingsForm( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( forms[ c ]->AddPage( "Video" ) );
            forms[ c ]->AddPage( "General" );
            GUI::CVideoOptions options;
            options.AddDisplayMode( TestDisplayMode( 1920, 1080 ) );
            video->SetVideoOptions( options );
            GUI::CVideoSettingsForm::TStringVector filters;
            filters.push_back( "fsr1" );
            video->SetAvailablePresentFilters( filters );
            ASSERT_TRUE( forms[ c ]->Open() );
            video->ShowProgress( "Upscaling the pictures for 1920 x 1080", 0.4f );

            context->RenderFrame( 16 );
            ASSERT_TRUE( context->CaptureFrame( frames[ c ] ) && !frames[ c ].IsNULL() && 1280 == frames[ c ]->GetWidthInPixels() );
        }
        if ( !frames[ 0 ].IsNULL() && !frames[ 1 ].IsNULL() )
        {
            UInt32 different = 0;
            for ( Int32 y=0; y<960; ++y )
                for ( Int32 x=0; x<1280; ++x )
                    if ( !Near( PixelAt( frames[ 0 ], x, y ), PixelAt( frames[ 1 ], x, y ) ) )
                        ++different;
            if ( 0 != different )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUi differs from the reference in " << different << " pixels" );
            ASSERT_TRUE( 0 == different );
        }
        for ( UInt32 c=0; c<2; ++c )
            GUCEF_DELETE forms[ c ];
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): At a render scale of 2 pictures and fonts of density 2 are drawn pixel for pixel, the same as the reference" )
    try
    {
        GUI::TGuiContextPtr referencePtr = CreateNullContext();
        GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( 1280, 960 );
        GUI::CGUIContext* contexts[ 2 ] = { static_cast< GUI::CGUIContext* >( referencePtr.GetPointerAlways() ), static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() ) };
        ASSERT_TRUE( GUCEF_NULL != contexts[ 0 ] && GUCEF_NULL != contexts[ 1 ] );
        IMAGE::TPixelMapPtr frames[ 2 ];
        for ( UInt32 c=0; c<2 && GUCEF_NULL != contexts[ 1 ]; ++c )
        {
            BuildDensityScene( contexts[ c ] );
            ASSERT_TRUE( 1280 == contexts[ c ]->GetRenderWidth() && 960 == contexts[ c ]->GetRenderHeight() );
            Int32 w = 0, h = 0;
            ASSERT_TRUE( contexts[ c ]->GetImageLogicalSize( "Fine", w, h ) && 4 == w && 4 == h && 2 == contexts[ c ]->GetImageDensity( "Fine" ) );
            contexts[ c ]->RenderFrame( 16 );
            ASSERT_TRUE( contexts[ c ]->CaptureFrame( frames[ c ] ) && !frames[ c ].IsNULL() && 1280 == frames[ c ]->GetWidthInPixels() && 960 == frames[ c ]->GetHeightInPixels() );
        }

        if ( !frames[ 0 ].IsNULL() && !frames[ 1 ].IsNULL() )
        {
            // the density 2 picture pixel for pixel, the density 1 picture in blocks of 2 x 2
            IMAGE::TPixelMapPtr fine = MakeGradientImage( 8, 8 ), coarse = MakeGradientImage( 4, 4 );
            UInt32 wrong = 0;
            for ( Int32 y=0; y<8; ++y )
                for ( Int32 x=0; x<8; ++x )
                {
                    if ( !Near( PixelAt( frames[ 0 ], 20 + x, 20 + y ), PixelAt( fine, x, y ), 0 ) )
                        ++wrong;
                    if ( !Near( PixelAt( frames[ 0 ], 60 + x, 20 + y ), PixelAt( coarse, x / 2, y / 2 ), 0 ) )
                        ++wrong;
                }
            ASSERT_TRUE( 0 == wrong );

            // the fill covers 2 x 2 pixels for every logical pixel
            ASSERT_TRUE( Near( PixelAt( frames[ 0 ], 120, 20 ), GUI::CColour( 250, 100, 0, 255 ) ) && Near( PixelAt( frames[ 0 ], 129, 29 ), GUI::CColour( 250, 100, 0, 255 ) ) );
            ASSERT_FALSE( Near( PixelAt( frames[ 0 ], 130, 30 ), GUI::CColour( 250, 100, 0, 255 ) ) );

            // the font of density 2 draws what the scaled up font of density 1 draws, and both draw something
            UInt32 textDiffers = 0, inked = 0;
            for ( Int32 y=0; y<30; ++y )
                for ( Int32 x=0; x<200; ++x )
                {
                    GUI::CColour a = PixelAt( frames[ 0 ], 20 + x, 80 + y ), b = PixelAt( frames[ 0 ], 20 + x, 120 + y );
                    if ( !Near( a, b, 0 ) )
                        ++textDiffers;
                    if ( Near( a, GUI::CColour( 255, 255, 255, 255 ) ) )
                        ++inked;
                }
            ASSERT_TRUE( 0 == textDiffers && inked > 50 );

            // RmlUi draws the same pixels
            UInt32 different = 0;
            for ( Int32 y=0; y<960; ++y )
                for ( Int32 x=0; x<1280; ++x )
                    if ( !Near( PixelAt( frames[ 0 ], x, y ), PixelAt( frames[ 1 ], x, y ) ) )
                        ++different;
            if ( 0 != different )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "At render scale 2 RmlUi differs from the reference in " << different << " pixels" );
            ASSERT_TRUE( 0 == different );
        }

        // back at render scale 1 the frame is the logical resolution again
        for ( UInt32 c=0; c<2 && GUCEF_NULL != contexts[ 1 ]; ++c )
        {
            contexts[ c ]->SetRenderScale( 1 );
            contexts[ c ]->RenderFrame( 16 );
            ASSERT_TRUE( contexts[ c ]->CaptureFrame( frames[ c ] ) && 640 == frames[ c ]->GetWidthInPixels() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The present filters the driver offers upscale the picture as each of them should" )
    try
    {
        GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( 1280, 960 );
        GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != rml && rml->GetDriver()->HasCapability( "PresentFilters" ) );
        if ( GUCEF_NULL != rml )
        {
            GUI::CGUIContext::TPresentFilterNames filters = rml->GetAvailablePresentFilters();
            bool hasMmpx = false, hasFsr = false, hasAnime4k = false;
            for ( UInt32 i=0; i<(UInt32)filters.size(); ++i )
            {
                hasMmpx = hasMmpx || "mmpx" == filters[ i ];
                hasFsr = hasFsr || "fsr" == filters[ i ];
                hasAnime4k = hasAnime4k || "anime4k" == filters[ i ];
            }
            ASSERT_TRUE( hasMmpx && hasFsr );
            GUI::CGUIContext::TPresentFilterParameters noParameters;
            ASSERT_FALSE( rml->SetPresentFilter( "no such filter", noParameters ) );
            ASSERT_TRUE( rml->GetPresentFilter().IsNULLOrEmpty() );

            // pixel art: lines of 1:1 and 2:1 slopes, a thick diagonal, a flat block, on a flat background
            const GUI::CColour background( 20, 30, 60, 255 ), white( 255, 255, 255, 255 ), red( 220, 40, 40, 255 ), yellow( 240, 200, 40, 255 );
            IMAGE::TPixelMapPtr pattern = MakeSolidImage( 640, 480, background.r, background.g, background.b, 255 );
            UInt8* data = static_cast< UInt8* >( pattern->GetDataPtr() );
            struct SPut { static void At( UInt8* d, Int32 x, Int32 y, const GUI::CColour& c ) { UInt8* p = d + ( (size_t) y * 640 + x ) * 4; p[ 0 ] = c.r; p[ 1 ] = c.g; p[ 2 ] = c.b; p[ 3 ] = 255; } };
            for ( Int32 i=0; i<200; ++i )
            {
                SPut::At( data, 100 + i, 100 + i, white );
                SPut::At( data, 100 + i, 300 + i / 2, white );
                for ( Int32 t=0; t<4; ++t )
                    SPut::At( data, 360 + i + t, 60 + i, red );
            }
            for ( Int32 y=320; y<420; ++y )
                for ( Int32 x=400; x<560; ++x )
                    SPut::At( data, x, y, yellow );
            rml->RegisterImage( "Pattern", pattern );
            GUI::CWidget* frame = rml->CreateWidget( "ImageFrame", "Pattern", GUCEF_NULL );
            frame->SetProperty( "Size", "1,0 1,0" );
            frame->SetProperty( "Image", "Pattern" );
            frame->SetProperty( "ScaleMode", "Stretch" );
            rml->ShowRoot( frame, false );

            IMAGE::TPixelMapPtr nearest, filtered;
            rml->RenderFrame( 16 );
            ASSERT_TRUE( rml->CapturePresentedFrame( nearest ) && 1280 == nearest->GetWidthInPixels() );
            ASSERT_TRUE( Near( PixelAt( nearest, 1000, 700 ), yellow ) && Near( PixelAt( nearest, 201, 201 ), white ) );

            struct SFilterCheck { const char* name; bool available; };
            const SFilterCheck checks[] = { { "mmpx", hasMmpx }, { "fsr", hasFsr }, { "anime4k", hasAnime4k } };
            for ( UInt32 c=0; c<3 && !nearest.IsNULL(); ++c )
            {
                if ( !checks[ c ].available )
                    continue;
                GUI::CGUIContext::TPresentFilterParameters parameters;
                parameters[ "sharpness" ] = "0.5";
                ASSERT_TRUE( rml->SetPresentFilter( checks[ c ].name, parameters ) );
                rml->RenderFrame( 16 );
                ASSERT_TRUE( rml->CapturePresentedFrame( filtered ) && !filtered.IsNULL() );
                if ( filtered.IsNULL() )
                    continue;

                UInt32 differ = 0, notInPalette = 0, farOff = 0;
                UInt64 totalDifference = 0;
                for ( Int32 y=0; y<960; ++y )
                {
                    for ( Int32 x=0; x<1280; ++x )
                    {
                        GUI::CColour a = PixelAt( filtered, x, y ), n = PixelAt( nearest, x, y );
                        Int32 d = abs( (Int32) a.r - n.r ) + abs( (Int32) a.g - n.g ) + abs( (Int32) a.b - n.b );
                        totalDifference += (UInt64) d;
                        if ( d > 3 )
                            ++differ;
                        if ( d > 96 )
                            ++farOff;
                        if ( !Near( a, background, 1 ) && !Near( a, white, 1 ) && !Near( a, red, 1 ) && !Near( a, yellow, 1 ) )
                            ++notInPalette;
                    }
                }
                Float64 meanDifference = (Float64) totalDifference / ( 1280.0 * 960.0 );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Present filter " << checks[ c ].name << ": " << differ << " pixels differ from nearest, " << notInPalette << " are not in the palette, mean difference " << meanDifference );

                // every filter keeps a flat area flat and only changes the picture near edges
                ASSERT_TRUE( Near( PixelAt( filtered, 960, 760 ), yellow, 6 ) && Near( PixelAt( filtered, 40, 900 ), background, 6 ) );
                ASSERT_TRUE( differ > 200 && meanDifference < 8.0 );
                if ( 0 == c )
                {
                    // pixel art stays pixel art: only colours of the picture
                    ASSERT_TRUE( 0 == notInPalette );
                }
                else
                {
                    // the smooth upscalers blend edges
                    ASSERT_TRUE( notInPalette > 200 && farOff < 20000 );
                }
            }

            // without a filter it is the picture scaled as the present mode says again
            ASSERT_TRUE( rml->SetPresentFilter( "", GUI::CGUIContext::TPresentFilterParameters() ) );
            rml->RenderFrame( 16 );
            ASSERT_TRUE( rml->CapturePresentedFrame( filtered ) && !filtered.IsNULL() && !nearest.IsNULL() );
            UInt32 changed = 0;
            for ( Int32 y=0; y<960 && !filtered.IsNULL(); y+=3 )
                for ( Int32 x=0; x<1280; x+=3 )
                    if ( !Near( PixelAt( filtered, x, y ), PixelAt( nearest, x, y ) ) )
                        ++changed;
            ASSERT_TRUE( 0 == changed );
            rml->DestroyWidget( frame );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The present pass shows the picture in the window as the present mode says" )
    try
    {
        // each window pixel shows the picture pixel at its centre: blocks when scaled up, black around the picture
        struct SCase { UInt32 windowWidth; UInt32 windowHeight; GUI::TPresentMode mode; };
        const SCase cases[] = { { 1280, 960, GUI::PRESENTMODE_NEAREST }, { 1000, 600, GUI::PRESENTMODE_NEAREST }, { 1000, 600, GUI::PRESENTMODE_INTEGER } };
        for ( size_t c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
        {
            GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( cases[ c ].windowWidth, cases[ c ].windowHeight );
            GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != rml );
            if ( GUCEF_NULL == rml )
                continue;
            rml->SetPresentMode( cases[ c ].mode );
            BuildMixedScene( rml );
            rml->RenderFrame( 16 );

            IMAGE::TPixelMapPtr picture, presented;
            ASSERT_TRUE( rml->CaptureFrame( picture ) );
            ASSERT_TRUE( rml->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( !presented.IsNULL() && cases[ c ].windowWidth == presented->GetWidthInPixels() && cases[ c ].windowHeight == presented->GetHeightInPixels() );
            if ( picture.IsNULL() || presented.IsNULL() )
                continue;

            const GUI::CRect area = rml->GetPresentRect();
            UInt32 wrong = 0;
            for ( Int32 y=0; y<(Int32) cases[ c ].windowHeight; ++y )
            {
                for ( Int32 x=0; x<(Int32) cases[ c ].windowWidth; ++x )
                {
                    const GUI::CColour actual = PixelAt( presented, x, y );
                    if ( x < area.left || x >= area.Right() || y < area.top || y >= area.Bottom() )
                    {
                        if ( !Near( actual, Black ) )
                            ++wrong;
                        continue;
                    }

                    // a pixel centre exactly on the edge of two picture pixels may show either of them
                    const double fx = ( ( x - area.left ) + 0.5 ) * 640.0 / area.width;
                    const double fy = ( ( y - area.top ) + 0.5 ) * 480.0 / area.height;
                    const Int32 px = (Int32) fx;
                    const Int32 py = (Int32) fy;
                    const bool edgeX = px > 0 && fx - px < 1e-6;
                    const bool edgeY = py > 0 && fy - py < 1e-6;
                    bool match = Near( actual, PixelAt( picture, px, py ) );
                    if ( !match && edgeX )
                        match = Near( actual, PixelAt( picture, px - 1, py ) );
                    if ( !match && edgeY )
                        match = Near( actual, PixelAt( picture, px, py - 1 ) );
                    if ( !match && edgeX && edgeY )
                        match = Near( actual, PixelAt( picture, px - 1, py - 1 ) );
                    if ( !match )
                        ++wrong;
                }
            }
            if ( 0 != wrong )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Present case " << (UInt32) c << ": " << wrong << " pixels differ" );
            ASSERT_TRUE( 0 == wrong );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): The pointer of the window is where the widget is drawn: a thin bar is hit at its drawn pixels in windows of every size and present mode" )
    try
    {
        struct SWindow { UInt32 width; UInt32 height; };
        const SWindow windows[] = { { 1079, 843 }, { 1280, 960 }, { 1600, 1200 }, { 3840, 1080 } };
        const GUI::TPresentMode modes[] = { GUI::PRESENTMODE_NEAREST, GUI::PRESENTMODE_INTEGER, GUI::PRESENTMODE_LINEAR, GUI::PRESENTMODE_STRETCH };
        const GUI::CColour barColour( 255, 0, 255, 255 );
        for ( size_t w=0; w<sizeof( windows ) / sizeof( windows[ 0 ] ); ++w )
        {
            // the picture at its own size (a game screen) and the picture that is the size of the window (a layer over it)
            for ( UInt32 own=0; own<2; ++own )
            {
                for ( size_t m=0; m<sizeof( modes ) / sizeof( modes[ 0 ] ); ++m )
                {
                    if ( 1 == own && GUI::PRESENTMODE_NEAREST != modes[ m ] )
                        continue;
                    GUI::TGuiContextPtr rmlPtr = CreateRmlUiContextInWindow( windows[ w ].width, windows[ w ].height );
                    GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
                    ASSERT_TRUE( GUCEF_NULL != rml );
                    if ( GUCEF_NULL == rml )
                        continue;
                    if ( 1 == own )
                        rml->SetLogicalResolution( 0, 0 );
                    rml->SetPresentMode( modes[ m ] );

                    // a 16 pixel bar of the picture, near its right edge, as high as most of the picture
                    const Int32 pictureWidth = (Int32) rml->GetLogicalWidth();
                    const Int32 pictureHeight = (Int32) rml->GetLogicalHeight();
                    GUI::CWidget* screen = Screen( rml );
                    GUI::CWidget* bar = rml->CreateWidget( "Panel", "Bar", screen );
                    bar->SetProperty( "Position", "0," + CORE::Int32ToString( pictureWidth - 40 ) + " 0,10" );
                    bar->SetProperty( "Size", "0,16 0," + CORE::Int32ToString( pictureHeight - 20 ) );
                    bar->SetColourProperty( "BackgroundColour", barColour );
                    rml->FlushChanges();
                    rml->RenderFrame( 16 );
                    IMAGE::TPixelMapPtr presented;
                    ASSERT_TRUE( rml->CapturePresentedFrame( presented ) && !presented.IsNULL() );
                    if ( presented.IsNULL() )
                        continue;

                    // the pixels of the window that show the bar, found in the picture the window shows, and what the pointer says there
                    Int32 left = -1, right = -1, top = -1, bottom = -1;
                    for ( Int32 y=0; y<(Int32) windows[ w ].height; ++y )
                    {
                        for ( Int32 x=0; x<(Int32) windows[ w ].width; ++x )
                        {
                            if ( Near( PixelAt( presented, x, y ), barColour ) )
                            {
                                if ( left < 0 || x < left ) left = x;
                                if ( x > right ) right = x;
                                if ( top < 0 || y < top ) top = y;
                                if ( y > bottom ) bottom = y;
                            }
                        }
                    }
                    ASSERT_TRUE( left >= 0 && right - left >= 8 && bottom - top >= 100 );
                    if ( left < 0 )
                        continue;
                    UInt32 wrong = 0;
                    const Int32 centreY = ( top + bottom ) / 2;
                    // inside the drawn bar: hit; clearly outside it: not hit
                    for ( Int32 x=left+1; x<right; ++x )
                    {
                        rml->ProcessWindowMouseMove( x, centreY );
                        if ( rml->HitTest( rml->GetPointerX(), rml->GetPointerY() ) != bar )
                            ++wrong;
                    }
                    rml->ProcessWindowMouseMove( left + 1, top + 2 );
                    if ( rml->HitTest( rml->GetPointerX(), rml->GetPointerY() ) != bar )
                        ++wrong;
                    rml->ProcessWindowMouseMove( right - 1, bottom - 2 );
                    if ( rml->HitTest( rml->GetPointerX(), rml->GetPointerY() ) != bar )
                        ++wrong;
                    // (a picture that is scaled up with a filter has a soft edge of up to a picture pixel, 8 window pixels is further out than that is wide)
                    rml->ProcessWindowMouseMove( left - 8, centreY );
                    if ( rml->HitTest( rml->GetPointerX(), rml->GetPointerY() ) == bar )
                        ++wrong;
                    rml->ProcessWindowMouseMove( right + 8, centreY );
                    if ( rml->HitTest( rml->GetPointerX(), rml->GetPointerY() ) == bar )
                        ++wrong;
                    if ( 0 != wrong )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Window " << windows[ w ].width << "x" << windows[ w ].height << ", mode " << (Int32) modes[ m ] << ", own " << own << ": " << wrong << " places where the bar is drawn and the pointer says otherwise, the bar is drawn at " << left << ".." << right );
                    ASSERT_TRUE( 0 == wrong );
                    rml->DestroyWidget( screen );
                }
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): A higher layer is drawn over the lower one in the same window, at its own resolution" )
    try
    {
        GUI::TGuiContextPtr bottomPtr = CreateRmlUiContext();
        GUI::CGUIContext* bottom = static_cast< GUI::CGUIContext* >( bottomPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != bottom );
        if ( GUCEF_NULL != bottom )
        {
            GUI::TGuiContextPtr topPtr = GUI::CGuiGlobal::Instance()->GetGuiManager().CreateGUIContext( "RmlUi", bottom->GetWindowContext() );
            GUI::CGUIContext* top = static_cast< GUI::CGUIContext* >( topPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != top );
            top->SetLogicalResolution( 0, 0 );

            // the layer numbers decide the order, not the order of making
            bottom->SetLayer( 1 );
            ASSERT_TRUE( top->IsBottomLayer() && !bottom->IsBottomLayer() );
            top->SetLayer( 2 );
            ASSERT_TRUE( bottom->IsBottomLayer() && !top->IsBottomLayer() );

            GUI::CWidget* red = Screen( bottom );
            red->SetColourProperty( "BackgroundColour", Red );
            GUI::CWidget* overlay = top->CreateWidget( "Panel", "Overlay", GUCEF_NULL );
            overlay->SetProperty( "Position", "0,100 0,100" );
            overlay->SetProperty( "Size", "0,50 0,50" );
            overlay->SetColourProperty( "BackgroundColour", Green );
            top->ShowRoot( overlay, false );

            bottom->RenderWindowLayers( 16 );
            IMAGE::TPixelMapPtr presented;
            ASSERT_TRUE( top->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( Near( PixelAt( presented, 10, 10 ), Red ) );
            ASSERT_TRUE( Near( PixelAt( presented, 120, 120 ), Green ) );
            ASSERT_TRUE( Near( PixelAt( presented, 160, 160 ), Red ) );

            // without the higher layer the lower one is all there is
            topPtr.Unlink();
            ASSERT_TRUE( 1 == bottom->GetWindowContext()->GetGuiLayers().size() );
            bottom->RenderWindowLayers( 16 );
            ASSERT_TRUE( bottom->CapturePresentedFrame( presented ) );
            ASSERT_TRUE( Near( PixelAt( presented, 120, 120 ), Red ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUi): A context that goes gives everything back to RmlUi before its renderer goes" )
    try
    {
        // RmlUi keeps what it made through a renderer until it is told to let go; a renderer deleted before that is
        // used after it is gone when RmlUi shuts down
        CLogTextCounter counter( "RenderInterface is being destroyed" );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &counter );
        {
            GUI::TGuiContextPtr rmlPtr = CreateRmlUiContext();
            GUI::CGUIContext* rml = static_cast< GUI::CGUIContext* >( rmlPtr.GetPointerAlways() );
            BuildMixedScene( rml );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( rml, pixels ) );
        }
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &counter );
        ASSERT_TRUE( 0 == counter.m_count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

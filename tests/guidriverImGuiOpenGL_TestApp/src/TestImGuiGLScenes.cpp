/*
 *  guidriverImGuiOpenGL_TestApp: tests of the gucefGUI driver ImGui (OpenGL)
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

#include "imgui.h"

#include "TestImGuiGLScenes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUIGLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUIDRIVERIMGUI;

const GUI::CColour Black( 0, 0, 0, 255 );
const GUI::CColour Red( 255, 0, 0, 255 );
const GUI::CColour Green( 0, 255, 0, 255 );
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

GUI::CWidget*
Screen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

/** Panels, borders, clipping, opacity, scaled and tinted pictures, text, lines and points in one picture (as gucefGUI_TestApp draws it) */
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

/** At a render scale of 2: pictures and a font of density 2 next to ones of density 1, a fill, a line and text */
void
BuildDensityScene( GUI::CGUIContext* context )
{
    context->SetRenderScale( 2 );

    IMAGE::TPixelMapPtr fine = MakeGradientImage( 8, 8 );
    IMAGE::TPixelMapPtr coarse = MakeGradientImage( 4, 4 );
    context->RegisterImage( "Fine", fine, 2 );
    context->RegisterImage( "Coarse", coarse );

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

/** The pixels that differ, outside an optional rectangle that is left out */
UInt32
CountDifferences( const IMAGE::TPixelMapPtr& expected, const IMAGE::TPixelMapPtr& actual, Int32 tolerance, const GUI::CRect& leftOut, const char* what )
{
    if ( expected.IsNULL() || actual.IsNULL() || expected->GetWidthInPixels() != actual->GetWidthInPixels() || expected->GetHeightInPixels() != actual->GetHeightInPixels() )
        return 0xFFFFFFFF;
    UInt32 different = 0;
    Int32 firstX = -1, firstY = -1;
    GUI::CColour firstExpected, firstActual;
    for ( Int32 y=0; y<(Int32)expected->GetHeightInPixels(); ++y )
    {
        for ( Int32 x=0; x<(Int32)expected->GetWidthInPixels(); ++x )
        {
            if ( leftOut.Contains( x, y ) )
                continue;
            GUI::CColour e = PixelAt( expected, x, y );
            GUI::CColour a = PixelAt( actual, x, y );
            if ( !Near( e, a, tolerance ) )
            {
                if ( 0 == different )
                {
                    firstX = x;
                    firstY = y;
                    firstExpected = e;
                    firstActual = a;
                }
                ++different;
            }
        }
    }
    if ( 0 != different )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui render test \"" << what << "\": " << different << " pixels differ from the reference, the first at " << firstX << "," << firstY
                         << ": " << firstActual.ToString() << " instead of " << firstExpected.ToString() );
    return different;
}

/** The pixels that differ by more than the tolerance in some channel, as a per mille of all pixels */
UInt32
DifferentPerMille( const IMAGE::TPixelMapPtr& expected, const IMAGE::TPixelMapPtr& actual, Int32 tolerance, UInt32& maxDifference )
{
    maxDifference = 0;
    if ( expected.IsNULL() || actual.IsNULL() || expected->GetWidthInPixels() != actual->GetWidthInPixels() || expected->GetHeightInPixels() != actual->GetHeightInPixels() )
        return 1000;
    UInt32 different = 0;
    for ( Int32 y=0; y<(Int32)expected->GetHeightInPixels(); ++y )
    {
        for ( Int32 x=0; x<(Int32)expected->GetWidthInPixels(); ++x )
        {
            const GUI::CColour e = PixelAt( expected, x, y );
            const GUI::CColour a = PixelAt( actual, x, y );
            UInt32 delta = (UInt32) abs( (Int32) e.r - (Int32) a.r );
            delta = ( (UInt32) abs( (Int32) e.g - (Int32) a.g ) > delta ) ? (UInt32) abs( (Int32) e.g - (Int32) a.g ) : delta;
            delta = ( (UInt32) abs( (Int32) e.b - (Int32) a.b ) > delta ) ? (UInt32) abs( (Int32) e.b - (Int32) a.b ) : delta;
            delta = ( (UInt32) abs( (Int32) e.a - (Int32) a.a ) > delta ) ? (UInt32) abs( (Int32) e.a - (Int32) a.a ) : delta;
            if ( delta > maxDifference )
                maxDifference = delta;
            if ( delta > (UInt32) tolerance )
                ++different;
        }
    }
    return (UInt32) ( (UInt64) different * 1000 / ( (UInt64) expected->GetWidthInPixels() * expected->GetHeightInPixels() ) );
}

/** A window with the usual widgets, the same in every context */
void
BuildScriptedWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    static bool check = true;
    static float value = 0.25f;
    ImGui::SetNextWindowPos( ImVec2( 330.0f, 20.0f ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( 280.0f, 200.0f ), ImGuiCond_Always );
    ImGui::Begin( "Scripted" );
    ImGui::Text( "Text %d", 42 );
    ImGui::Button( "Button" );
    ImGui::Checkbox( "Check", &check );
    ImGui::SliderFloat( "Slider", &value, 0.0f, 1.0f );
    ImGui::Separator();
    ImGui::BulletText( "Bullet" );
    context.Image( "Checker" );
    ImGui::End();
}

/** Drawing straight into the foreground draw list, to test the rules of the rasterizer */
void
BuildRasterizerProbes( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->Flags &= ~( ImDrawFlags_AAFill | ImDrawFlags_AALines | ImDrawFlags_AALineEnds );
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();

    // fill rule: the pixels whose centre is inside (edges off the pixel centres: a GPU decides a tie by rounding)
    list->AddRectFilled( ImVec2( 10.25f, 10.25f ), ImVec2( 20.25f, 20.25f ), IM_COL32( 255, 255, 255, 255 ) );
    // two triangles sharing an edge at half alpha: no pixel is blended twice
    list->AddQuadFilled( ImVec2( 30.0f, 10.0f ), ImVec2( 50.0f, 10.0f ), ImVec2( 50.0f, 30.0f ), ImVec2( 30.0f, 30.0f ), IM_COL32( 255, 0, 0, 128 ) );
    // scissor
    list->PushClipRect( ImVec2( 60.0f, 10.0f ), ImVec2( 70.0f, 20.0f ), true );
    list->AddRectFilled( ImVec2( 55.0f, 5.0f ), ImVec2( 80.0f, 30.0f ), IM_COL32( 0, 255, 0, 255 ) );
    list->PopClipRect();
    // sampling: a black and a white texel over four pixels, at the nearest texel and linearly
    ImTextureRef ramp = context.GetImageTextureRef( "Ramp" );
    list->AddCallback( platformIO.DrawCallback_SetSamplerNearest, GUCEF_NULL );
    list->AddImage( ramp, ImVec2( 100.0f, 10.0f ), ImVec2( 104.0f, 11.0f ) );
    list->AddCallback( platformIO.DrawCallback_SetSamplerLinear, GUCEF_NULL );
    list->AddImage( ramp, ImVec2( 100.0f, 20.0f ), ImVec2( 104.0f, 21.0f ) );
    // straight alpha blending over black
    list->AddRectFilled( ImVec2( 120.0f, 10.0f ), ImVec2( 130.0f, 20.0f ), IM_COL32( 255, 0, 0, 128 ) );
}

/** More vertices than 16-bit indices can address */
void
BuildManyRects( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    ImDrawList* list = ImGui::GetForegroundDrawList();
    for ( UInt32 i=0; i<20000; ++i )
    {
        const float x = (float) ( i % 200 ) * 3.0f;
        const float y = (float) ( i / 200 ) * 3.0f;
        list->AddRectFilled( ImVec2( x, y ), ImVec2( x + 2.0f, y + 2.0f ), IM_COL32( 0, 0, 255, 255 ) );
    }
}

bool
RecolourImage( GUI::CGUIContext* context, const char* alias, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{
    IMAGE::TPixelMapPtr pixels;
    UInt32 revision = 0;
    if ( !context->ResolveImage( alias, pixels, revision ) || pixels.IsNULL() )
        return false;
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 i=0; i<pixels->GetWidthInPixels() * pixels->GetHeightInPixels(); ++i )
    {
        data[ i*4 ] = r;
        data[ i*4+1 ] = g;
        data[ i*4+2 ] = b;
        data[ i*4+3 ] = a;
    }
    return context->UpdateImage( alias );
}

/*-------------------------------------------------------------------------*/

void
BuildButtonWindow( CImGuiContext& context, UInt32 deltaInMs, void* userData )
{
    SButtonState* state = static_cast< SButtonState* >( userData );
    ImGui::SetNextWindowPos( ImVec2( 20.0f, 20.0f ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( 200.0f, 100.0f ), ImGuiCond_Always );
    ImGui::Begin( "Buttons" );
    if ( ImGui::Button( "Press" ) )
        ++state->clicks;
    const ImVec2 itemMin = ImGui::GetItemRectMin();
    const ImVec2 itemMax = ImGui::GetItemRectMax();
    state->minX = itemMin.x;
    state->minY = itemMin.y;
    state->maxX = itemMax.x;
    state->maxY = itemMax.y;
    state->hovered = ImGui::IsItemHovered();
    ++state->frames;
    ImGui::End();
}

/*-------------------------------------------------------------------------*/

void
BuildFillScene( GUI::CGUIContext* context )
{
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

    GUI::CWidget* other = context->CreateWidget( "Panel", "Other", screen );
    other->SetProperty( "Position", "0,300 0,40" );
    other->SetProperty( "Size", "0,120 0,90" );
    other->SetColourProperty( "BackgroundColour", GUI::CColour( 200, 30, 60, 255 ) );
    other->SetColourProperty( "BorderColour", Green );
}

/*-------------------------------------------------------------------------*/

/** Compares the draw data of two contexts: lists, vertices, indices, commands and clip rectangles */
bool
SameDrawData( const ImDrawData& a, const ImDrawData& b )
{
    if ( a.CmdLists.Size != b.CmdLists.Size || a.TotalVtxCount != b.TotalVtxCount || a.TotalIdxCount != b.TotalIdxCount )
        return false;
    for ( int n=0; n<a.CmdLists.Size; ++n )
    {
        const ImDrawList* la = a.CmdLists[ n ];
        const ImDrawList* lb = b.CmdLists[ n ];
        if ( la->VtxBuffer.Size != lb->VtxBuffer.Size || la->IdxBuffer.Size != lb->IdxBuffer.Size || la->CmdBuffer.Size != lb->CmdBuffer.Size )
            return false;
        if ( 0 != memcmp( la->VtxBuffer.Data, lb->VtxBuffer.Data, (size_t) la->VtxBuffer.Size * sizeof( ImDrawVert ) ) )
            return false;
        if ( 0 != memcmp( la->IdxBuffer.Data, lb->IdxBuffer.Data, (size_t) la->IdxBuffer.Size * sizeof( ImDrawIdx ) ) )
            return false;
        for ( int c=0; c<la->CmdBuffer.Size; ++c )
        {
            const ImDrawCmd& ca = la->CmdBuffer[ c ];
            const ImDrawCmd& cb = lb->CmdBuffer[ c ];
            if ( ca.ElemCount != cb.ElemCount || ca.IdxOffset != cb.IdxOffset || ca.VtxOffset != cb.VtxOffset ||
                 0 != memcmp( &ca.ClipRect, &cb.ClipRect, sizeof( ImVec4 ) ) || ( GUCEF_NULL == ca.UserCallback ) != ( GUCEF_NULL == cb.UserCallback ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUIGLTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

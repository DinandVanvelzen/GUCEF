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

#include <string.h>

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUIDRIVERIMGUI;

namespace {

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

    // fill rule: the pixels whose centre is inside, left and top edges included
    list->AddRectFilled( ImVec2( 10.5f, 10.5f ), ImVec2( 20.5f, 20.5f ), IM_COL32( 255, 255, 255, 255 ) );
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

}

/*-------------------------------------------------------------------------*/

void
PerformImGuiRenderTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "Render" );

    GUCEF_TESTFW_TESTCASE( "Render: a retained scene of every kind of drawing has the pixels of the software reference renderer" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildMixedScene( context );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( context->CaptureFrame( actual ) );
            ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *context, expected ) );
            ASSERT_TRUE( 0 == CountDifferences( expected, actual, 0, GUI::CRect(), "mixed scene" ) );
            ASSERT_TRUE( 0 == context->GetSoftwareRenderer().GetUnresolvedTextureCount() );

            // nothing changed: the second frame replays the kept commands, no widget is painted again
            GUI::CWidget* label = GUCEF_NULL;
            GUI::CGUIContext::TWidgetSet widgets = context->GetOwnedWidgets();
            for ( GUI::CGUIContext::TWidgetSet::iterator i=widgets.begin(); i!=widgets.end(); ++i )
                if ( "Label" == (*i)->GetName() )
                    label = (*i);
            ASSERT_TRUE( GUCEF_NULL != label );
            if ( GUCEF_NULL != label )
            {
                const UInt32 paints = CImGuiWidgetHandler::GetPaintCount( *label );
                ASSERT_TRUE( paints > 0 );
                context->RenderFrame( 16 );
                ASSERT_TRUE( paints == CImGuiWidgetHandler::GetPaintCount( *label ) );
                label->SetProperty( "Text", "Changed" );
                context->RenderFrame( 16 );
                ASSERT_TRUE( paints + 1 == CImGuiWidgetHandler::GetPaintCount( *label ) );
                ASSERT_TRUE( context->CaptureFrame( actual ) );
                ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *context, expected ) );
                ASSERT_TRUE( 0 == CountDifferences( expected, actual, 0, GUI::CRect(), "mixed scene after a change" ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render: at a render scale of 2 pictures and fonts of density 2 have the pixels of the reference, lines follow the logical path" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildDensityScene( context );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( context->CaptureFrame( actual ) );
            ASSERT_TRUE( !actual.IsNULL() && 1280 == actual->GetWidthInPixels() && 960 == actual->GetHeightInPixels() );
            ASSERT_TRUE( GUI::CSoftwareRenderer::RenderContext( *context, expected ) );
            // the line of the draw surface: the reference walks it in pixels of the render scale, the painter in logical pixels
            ASSERT_TRUE( 0 == CountDifferences( expected, actual, 0, GUI::CRect( 240, 20, 80, 80 ), "density scene" ) );
            ASSERT_TRUE( Near( PixelAt( actual, 240, 20 ), GUI::CColour( 0, 255, 0, 255 ), 0 ) );
            ASSERT_TRUE( Near( PixelAt( actual, 241, 21 ), GUI::CColour( 0, 255, 0, 255 ), 0 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render: the same scripted scene in two contexts gives the same draw data, clips within the display, textures that resolve" )
    try
    {
        GUI::TGuiContextPtr firstPtr = CreateImGuiContext();
        GUI::TGuiContextPtr secondPtr = CreateImGuiContext();
        CImGuiContext* first = AsImGui( firstPtr );
        CImGuiContext* second = AsImGui( secondPtr );
        ASSERT_TRUE( GUCEF_NULL != first && GUCEF_NULL != second );
        if ( GUCEF_NULL != first && GUCEF_NULL != second )
        {
            BuildMixedScene( first );
            BuildMixedScene( second );
            CTestFrameBuilder firstBuilder( &BuildScriptedWindow, GUCEF_NULL );
            CTestFrameBuilder secondBuilder( &BuildScriptedWindow, GUCEF_NULL );
            firstBuilder.Attach( firstPtr );
            secondBuilder.Attach( secondPtr );
            for ( UInt32 i=0; i<3; ++i )
            {
                first->InjectMouseMove( 400, 60 + (Int32) i );
                second->InjectMouseMove( 400, 60 + (Int32) i );
                first->RenderFrame( 16 );
                second->RenderFrame( 16 );
            }
            ImDrawData* a = first->GetDrawData();
            ImDrawData* b = second->GetDrawData();
            ASSERT_TRUE( GUCEF_NULL != a && GUCEF_NULL != b );
            if ( GUCEF_NULL != a && GUCEF_NULL != b )
            {
                ASSERT_TRUE( SameDrawData( *a, *b ) );
                ASSERT_TRUE( a->TotalVtxCount > 0 );
                UInt32 outside = 0;
                for ( int n=0; n<a->CmdLists.Size; ++n )
                {
                    const ImDrawList* list = a->CmdLists[ n ];
                    for ( int c=0; c<list->CmdBuffer.Size; ++c )
                    {
                        const ImVec4& clip = list->CmdBuffer[ c ].ClipRect;
                        if ( clip.x < 0.0f || clip.y < 0.0f || clip.z > a->DisplaySize.x || clip.w > a->DisplaySize.y )
                            ++outside;
                    }
                }
                ASSERT_TRUE( 0 == outside );
                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( first->CaptureFrame( pixels ) );
                ASSERT_TRUE( 0 == first->GetSoftwareRenderer().GetUnresolvedTextureCount() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render: a fill of the painter is a rectangle of four vertices with its corners and colour" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Surface", GUCEF_NULL ) );
            surface->SetProperty( "Position", "0,10 0,20" );
            surface->SetProperty( "Size", "0,100 0,100" );
            surface->FillRect( GUI::CRect( 5, 6, 30, 40 ), GUI::CColour( 12, 34, 56, 255 ) );
            surface->Commit();
            context->ShowRoot( surface, false );
            context->RenderFrame( 16 );
            ImDrawData* drawData = context->GetDrawData();
            ASSERT_TRUE( GUCEF_NULL != drawData );
            bool found = false;
            const ImU32 colour = IM_COL32( 12, 34, 56, 255 );
            for ( int n=0; GUCEF_NULL != drawData && n<drawData->CmdLists.Size && !found; ++n )
            {
                const ImDrawList* list = drawData->CmdLists[ n ];
                for ( int v=0; v+3<list->VtxBuffer.Size && !found; ++v )
                {
                    const ImDrawVert* q = list->VtxBuffer.Data + v;
                    found = q[ 0 ].col == colour && q[ 1 ].col == colour && q[ 2 ].col == colour && q[ 3 ].col == colour &&
                            15.0f == q[ 0 ].pos.x && 26.0f == q[ 0 ].pos.y && 45.0f == q[ 1 ].pos.x && 26.0f == q[ 1 ].pos.y &&
                            45.0f == q[ 2 ].pos.x && 66.0f == q[ 2 ].pos.y && 15.0f == q[ 3 ].pos.x && 66.0f == q[ 3 ].pos.y;
                }
            }
            ASSERT_TRUE( found );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render: the software rasterizer follows the fill rule, the scissor, nearest and linear sampling and straight alpha blending" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            IMAGE::TPixelMapPtr ramp = MakeSolidImage( 2, 1, 0, 0, 0, 255 );
            UInt8* data = static_cast< UInt8* >( ramp->GetDataPtr() );
            data[ 4 ] = data[ 5 ] = data[ 6 ] = 255;
            context->RegisterImage( "Ramp", ramp );
            CTestFrameBuilder builder( &BuildRasterizerProbes, GUCEF_NULL );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( context->CaptureFrame( pixels ) );
            if ( !pixels.IsNULL() )
            {
                // fill rule: 10 x 10 pixels from 10,10
                UInt32 white = 0;
                for ( Int32 y=0; y<30; ++y )
                    for ( Int32 x=0; x<30; ++x )
                        if ( White == PixelAt( pixels, x, y ) )
                            ++white;
                ASSERT_TRUE( 100 == white );
                ASSERT_TRUE( White == PixelAt( pixels, 10, 10 ) && White == PixelAt( pixels, 19, 19 ) );
                ASSERT_TRUE( Black == PixelAt( pixels, 9, 10 ) && Black == PixelAt( pixels, 20, 19 ) );

                // the shared edge of two triangles is drawn once: every pixel of the quad has the same colour
                UInt32 even = 0;
                for ( Int32 y=10; y<30; ++y )
                    for ( Int32 x=30; x<50; ++x )
                        if ( GUI::CColour( 128, 0, 0, 255 ) == PixelAt( pixels, x, y ) )
                            ++even;
                ASSERT_TRUE( 400 == even );

                // scissor: only the 10 x 10 pixels of the clip rectangle
                UInt32 green = 0;
                for ( Int32 y=0; y<40; ++y )
                    for ( Int32 x=50; x<90; ++x )
                        if ( GUI::CColour( 0, 255, 0, 255 ) == PixelAt( pixels, x, y ) )
                            ++green;
                ASSERT_TRUE( 100 == green );

                // nearest and linear sampling of a black and a white texel over four pixels
                ASSERT_TRUE( 0 == PixelAt( pixels, 100, 10 ).r && 0 == PixelAt( pixels, 101, 10 ).r && 255 == PixelAt( pixels, 102, 10 ).r && 255 == PixelAt( pixels, 103, 10 ).r );
                ASSERT_TRUE( 0 == PixelAt( pixels, 100, 20 ).r && 64 == PixelAt( pixels, 101, 20 ).r && 191 == PixelAt( pixels, 102, 20 ).r && 255 == PixelAt( pixels, 103, 20 ).r );

                // half red over black
                ASSERT_TRUE( GUI::CColour( 128, 0, 0, 255 ) == PixelAt( pixels, 125, 15 ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render: a capture draws the last frame again without a new frame, also with more vertices than 16-bit indices address" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateImGuiContext();
        CImGuiContext* context = AsImGui( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            IMAGE::TPixelMapPtr none;
            ASSERT_FALSE( context->CaptureFrame( none ) );
            CTestFrameBuilder builder( &BuildManyRects, GUCEF_NULL );
            builder.Attach( contextPtr );
            context->RenderFrame( 16 );
            ImDrawData* drawData = context->GetDrawData();
            ASSERT_TRUE( GUCEF_NULL != drawData && drawData->TotalVtxCount > 65536 );
            const UInt32 frames = context->GetBuiltFrameCount();
            IMAGE::TPixelMapPtr first, second;
            ASSERT_TRUE( context->CaptureFrame( first ) );
            ASSERT_TRUE( context->CaptureFrame( second ) );
            ASSERT_TRUE( frames == context->GetBuiltFrameCount() );
            ASSERT_TRUE( 0 == CountDifferences( first, second, 0, GUI::CRect(), "two captures" ) );
            ASSERT_TRUE( context->GetSoftwareRenderer().GetTriangleCount() >= 40000 );
            // the last rectangle, at 597,297
            ASSERT_TRUE( GUI::CColour( 0, 0, 255, 255 ) == PixelAt( first, 597, 297 ) );
            ASSERT_TRUE( Black == PixelAt( first, 599, 299 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUITEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

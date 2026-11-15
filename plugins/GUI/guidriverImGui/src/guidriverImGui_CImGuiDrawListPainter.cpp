/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#include "guidriverImGui_CImGuiWidgetHandler.h"
#define GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#endif /* GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#include "guidriverImGui_CImGuiTextureCache.h"
#define GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#endif /* GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H ? */

#include "guidriverImGui_CImGuiDrawListPainter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The alpha of a colour times an opacity, rounded as CSoftwareRenderer rounds it */
inline UInt32
ScaledAlpha( UInt32 alpha, Float32 opacity )
{
    if ( opacity >= 1.0f )
        return alpha;
    if ( opacity <= 0.0f )
        return 0;
    return (UInt32) ( (Float32) alpha * opacity + 0.5f );
}

inline ImU32
PackColour( const GUI::CColour& colour, UInt32 alpha )
{
    return IM_COL32( colour.r, colour.g, colour.b, alpha );
}

inline GUI::CRect
ScaledRect( const GUI::CRect& rect, Int32 scale )
{
    return GUI::CRect( rect.left * scale, rect.top * scale, rect.width * scale, rect.height * scale );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiDrawListPainter::CClipScope::CClipScope( ::ImDrawList& drawList, const GUI::CRect& clip )
    : m_drawList( drawList )
{GUCEF_TRACE;

    m_drawList.PushClipRect( ImVec2( (float) clip.left, (float) clip.top ), ImVec2( (float) clip.Right(), (float) clip.Bottom() ), true );
}

/*-------------------------------------------------------------------------*/

CImGuiDrawListPainter::CClipScope::~CClipScope()
{GUCEF_TRACE;

    m_drawList.PopClipRect();
}

/*-------------------------------------------------------------------------*/

CImGuiDrawListPainter::CImGuiDrawListPainter( CImGuiContext& context, ::ImDrawList& drawList )
    : m_context( context )
    , m_drawList( drawList )
    , m_previousFlags( (Int32) drawList.Flags )
    , m_nearest( false )
    , m_quads( 0 )
{GUCEF_TRACE;

    // the pixels of the reference renderer: no anti-aliased edges
    m_drawList.Flags &= ~( ImDrawFlags_AAFill | ImDrawFlags_AALines | ImDrawFlags_AALineEnds );
}

/*-------------------------------------------------------------------------*/

CImGuiDrawListPainter::~CImGuiDrawListPainter()
{GUCEF_TRACE;

    SetNearestSampling( false );
    m_drawList.Flags = (ImDrawFlags) m_previousFlags;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiDrawListPainter::GetQuadCount( void ) const
{GUCEF_TRACE;

    return m_quads;
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::SetNearestSampling( bool nearest )
{GUCEF_TRACE;

    if ( nearest == m_nearest )
        return;
    // the standard callbacks of the renderer: a renderer without them samples as it does by default
    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    ImDrawCallback callback = nearest ? platformIO.DrawCallback_SetSamplerNearest : platformIO.DrawCallback_SetSamplerLinear;
    if ( GUCEF_NULL != callback )
        m_drawList.AddCallback( callback, GUCEF_NULL );
    m_nearest = nearest;
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::FillRect( Int32 left, Int32 top, Int32 width, Int32 height, const GUI::CColour& colour, UInt32 alpha )
{GUCEF_TRACE;

    if ( width <= 0 || height <= 0 || 0 == alpha )
        return;
    m_drawList.PrimReserve( 6, 4 );
    m_drawList.PrimRect( ImVec2( (float) left, (float) top ), ImVec2( (float) ( left + width ), (float) ( top + height ) ), PackColour( colour, alpha ) );
    ++m_quads;
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintLine( Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 thickness, const GUI::CColour& colour, Float32 opacity )
{GUCEF_TRACE;

    const UInt32 alpha = ScaledAlpha( colour.a, opacity );
    if ( 0 == alpha )
        return;
    const Int32 half = thickness > 1 ? thickness / 2 : 0;

    // straight lines are one rectangle, as long as the squares of a thick line would not be blended over each other
    if ( ( y0 == y1 || x0 == x1 ) && ( 0 == half || alpha >= 255 ) )
    {
        const Int32 size = 0 == half ? 1 : thickness;
        const Int32 left = ( x0 < x1 ? x0 : x1 ) - half;
        const Int32 top = ( y0 < y1 ? y0 : y1 ) - half;
        FillRect( left, top, abs( x1 - x0 ) + size, abs( y1 - y0 ) + size, colour, alpha );
        return;
    }

    // the path of CSoftwareRenderer: pixels of the same row next to each other are one rectangle
    Int32 dx = x1 > x0 ? x1 - x0 : x0 - x1;
    Int32 dy = y1 > y0 ? y0 - y1 : y1 - y0;
    Int32 sx = x0 < x1 ? 1 : -1;
    Int32 sy = y0 < y1 ? 1 : -1;
    Int32 error = dx + dy;
    Int32 runStart = x0;
    Int32 runEnd = x0;
    Int32 runY = y0;
    while ( true )
    {
        if ( 0 != half )
        {
            FillRect( x0 - half, y0 - half, thickness, thickness, colour, alpha );
        }
        else
        if ( y0 == runY && ( x0 == runEnd + sx || x0 == runEnd ) )
        {
            runEnd = x0;
        }
        else
        {
            FillRect( runStart < runEnd ? runStart : runEnd, runY, abs( runEnd - runStart ) + 1, 1, colour, alpha );
            runStart = runEnd = x0;
            runY = y0;
        }
        if ( x0 == x1 && y0 == y1 )
            break;
        Int32 e2 = 2 * error;
        if ( e2 >= dy )
        {
            error += dy;
            x0 += sx;
        }
        if ( e2 <= dx )
        {
            error += dx;
            y0 += sy;
        }
    }
    if ( 0 == half )
        FillRect( runStart < runEnd ? runStart : runEnd, runY, abs( runEnd - runStart ) + 1, 1, colour, alpha );
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintImage( const GUI::CDrawSurface::TCommand& command, const GUI::CRect& target, Float32 opacity )
{GUCEF_TRACE;

    const UInt32 alpha = ScaledAlpha( command.colour.a, opacity );
    if ( target.IsEmpty() || 0 == alpha )
        return;
    ::ImTextureData* texture = m_context.GetTextureCache().GetImageTexture( command.image );
    if ( GUCEF_NULL == texture )
        return;

    // the source is in logical pixels of the image: its pixels divided by its density
    const Int32 density = (Int32) m_context.GetImageDensity( command.image );
    GUI::CRect source = command.source.IsEmpty() ? GUI::CRect( 0, 0, texture->Width, texture->Height ) : ScaledRect( command.source, density );
    if ( source.IsEmpty() )
        return;

    const float width = (float) texture->Width;
    const float height = (float) texture->Height;
    SetNearestSampling( true );
    m_drawList.AddImage( texture->GetTexRef(),
                         ImVec2( (float) target.left, (float) target.top ), ImVec2( (float) target.Right(), (float) target.Bottom() ),
                         ImVec2( source.left / width, source.top / height ), ImVec2( source.Right() / width, source.Bottom() / height ),
                         PackColour( command.colour, alpha ) );
    ++m_quads;
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintGlyphs( const GUI::CDrawSurface::TCommand& command, Int32 x, Int32 y, Float32 opacity )
{GUCEF_TRACE;

    const UInt32 alpha = ScaledAlpha( command.colour.a, opacity );
    if ( 0 == alpha || command.text.IsNULLOrEmpty() )
        return;
    GUI::TBitmapFontPtr font = m_context.GetBitmapFont( command.font );
    if ( font.IsNULL() || font->GetAtlas().IsNULL() )
        return;
    ::ImTextureData* texture = m_context.GetTextureCache().GetFontTexture( command.font );
    if ( GUCEF_NULL == texture )
        return;

    GUI::TCodePointVector codePoints;
    GUI::DecodeUtf8( command.text, codePoints );
    const float atlasWidth = (float) texture->Width;
    const float atlasHeight = (float) texture->Height;
    const float density = (float) ( 0 == font->GetDensity() ? 1 : font->GetDensity() );
    const ImU32 colour = PackColour( command.colour, alpha );

    SetNearestSampling( true );
    m_drawList.PushTexture( texture->GetTexRef() );
    Int32 pen = x;
    bool first = true;
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        const GUI::CBitmapFont::TGlyph* glyph = font->ResolveGlyph( codePoints[ i ] );
        if ( GUCEF_NULL == glyph )
            continue;
        if ( !first )
            pen += font->GetSpacing();
        first = false;
        const GUI::CRect& source = glyph->source;
        if ( source.width > 0 && source.height > 0 )
        {
            const float left = (float) ( pen + glyph->offsetX );
            const float top = (float) ( y + glyph->offsetY );
            m_drawList.PrimReserve( 6, 4 );
            m_drawList.PrimRectUV( ImVec2( left, top ), ImVec2( left + source.width / density, top + source.height / density ),
                                   ImVec2( source.left / atlasWidth, source.top / atlasHeight ),
                                   ImVec2( source.Right() / atlasWidth, source.Bottom() / atlasHeight ), colour );
            ++m_quads;
        }
        pen += glyph->advance;
    }
    m_drawList.PopTexture();
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::Execute( const TCommandVector& commands, Int32 offsetX, Int32 offsetY, const GUI::CRect& clip, Float32 opacity )
{GUCEF_TRACE;

    CClipScope scope( m_drawList, clip );
    UInt32 pushedClips = 0;
    for ( UInt32 i=0; i<(UInt32)commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = commands[ i ];
        const GUI::CRect rect( command.rect.left + offsetX, command.rect.top + offsetY, command.rect.width, command.rect.height );
        switch ( command.type )
        {
            case GUI::CDrawSurface::COMMAND_FILLRECT:
            {
                FillRect( rect.left, rect.top, rect.width, rect.height, command.colour, ScaledAlpha( command.colour.a, opacity ) );
                break;
            }
            case GUI::CDrawSurface::COMMAND_STROKERECT:
            {
                const Int32 t = command.thickness < 1 ? 1 : command.thickness;
                const UInt32 alpha = ScaledAlpha( command.colour.a, opacity );
                FillRect( rect.left, rect.top, rect.width, t, command.colour, alpha );
                FillRect( rect.left, rect.Bottom() - t, rect.width, t, command.colour, alpha );
                FillRect( rect.left, rect.top + t, t, rect.height - 2*t, command.colour, alpha );
                FillRect( rect.Right() - t, rect.top + t, t, rect.height - 2*t, command.colour, alpha );
                break;
            }
            case GUI::CDrawSurface::COMMAND_LINE:
            {
                PaintLine( command.x0 + offsetX, command.y0 + offsetY, command.x1 + offsetX, command.y1 + offsetY, command.thickness, command.colour, opacity );
                break;
            }
            case GUI::CDrawSurface::COMMAND_POINTS:
            {
                const UInt32 alpha = ScaledAlpha( command.colour.a, opacity );
                for ( UInt32 p=0; p<(UInt32)command.points.size(); ++p )
                    FillRect( command.points[ p ].x + offsetX, command.points[ p ].y + offsetY, 1, 1, command.colour, alpha );
                break;
            }
            case GUI::CDrawSurface::COMMAND_IMAGE:
            {
                PaintImage( command, rect, opacity );
                break;
            }
            case GUI::CDrawSurface::COMMAND_TEXT:
            {
                PaintGlyphs( command, command.x0 + offsetX, command.y0 + offsetY, opacity );
                break;
            }
            case GUI::CDrawSurface::COMMAND_PUSHCLIP:
            {
                m_drawList.PushClipRect( ImVec2( (float) rect.left, (float) rect.top ), ImVec2( (float) rect.Right(), (float) rect.Bottom() ), true );
                ++pushedClips;
                break;
            }
            case GUI::CDrawSurface::COMMAND_POPCLIP:
            {
                if ( pushedClips > 0 )
                {
                    m_drawList.PopClipRect();
                    --pushedClips;
                }
                break;
            }
        }
    }
    while ( pushedClips > 0 )
    {
        m_drawList.PopClipRect();
        --pushedClips;
    }
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintWidget( const GUI::CWidget& widget, const GUI::CRect& clip, Float32 opacity )
{GUCEF_TRACE;

    if ( !widget.IsVisible() )
        return;
    const GUI::CRect& rect = widget.GetRect();
    const Float32 widgetOpacity = opacity * widget.GetFloatProperty( GUI::EWidgetProperty_Opacity );

    // the widget's own drawing stays within the widget
    if ( GUCEF_NULL != widget.GetImplementationPtr() )
    {
        const TCommandVector& commands = CImGuiWidgetHandler::GetCommands( widget, m_context, m_context.GetImageGeneration() );
        Execute( commands, rect.left, rect.top, clip.Intersection( rect ), widgetOpacity );
    }
    else
    {
        // a widget without the handler of this driver: painted for this frame only
        TCommandVector commands;
        GUI::CWidgetPainter::Paint( widget, m_context, commands );
        Execute( commands, rect.left, rect.top, clip.Intersection( rect ), widgetOpacity );
    }

    // what a collapsed window hides is not drawn
    if ( !widget.AreChildrenShown() )
        return;
    const GUI::CRect childClip = widget.GetBoolProperty( GUI::EWidgetProperty_Clip ) ? clip.Intersection( rect ) : clip;
    const GUI::CWidget::TWidgetVector& children = widget.GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        PaintWidget( *children[ i ], childClip, widgetOpacity );
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintRoots( void )
{GUCEF_TRACE;

    const GUI::CRect screen( 0, 0, (Int32) m_context.GetLogicalWidth(), (Int32) m_context.GetLogicalHeight() );
    const GUI::CGUIContext::TWidgetVector& roots = m_context.GetRootWidgets();
    for ( UInt32 i=0; i<(UInt32)roots.size(); ++i )
        PaintWidget( *roots[ i ], screen, 1.0f );
}

/*-------------------------------------------------------------------------*/

void
CImGuiDrawListPainter::PaintOverlay( void )
{GUCEF_TRACE;

    TCommandVector overlay;
    GUI::CWidgetPainter::PaintOverlay( m_context, overlay );
    if ( overlay.empty() )
        return;
    const GUI::CRect screen( 0, 0, (Int32) m_context.GetLogicalWidth(), (Int32) m_context.GetLogicalHeight() );
    Execute( overlay, 0, 0, screen, 1.0f );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

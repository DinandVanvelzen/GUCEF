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

#ifndef GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H
#define GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

struct ImDrawList;

namespace GUCEF {
namespace GUIDRIVERIMGUI {

class CImGuiContext;

/**
 *  Replays the drawing commands of gucefGUI (CDrawSurface, CWidgetPainter) into a draw list of Dear ImGui, with the pixel
 *  rules of gucefGUI's CSoftwareRenderer, so that a retained widget in an ImGui context looks as it does everywhere:
 *
 *  - the roots are walked as the software renderer walks them: a widget's own drawing is cut off at its rectangle, its
 *    children too when Clip is set, colours are multiplied by the Opacity of the widget and its ancestors;
 *  - fills and borders are rectangles; horizontal and vertical lines are rectangles, other lines are the pixel runs of
 *    the same Bresenham path; points are rectangles of one logical pixel;
 *  - images are textures of the CImGuiTextureCache drawn with their source rectangle and tint, sampled at the nearest
 *    texel (between the DrawCallback_SetSamplerNearest and DrawCallback_SetSamplerLinear callbacks of the platform IO);
 *  - text is one quad per glyph from the atlas of the bitmap font, laid out by the rules of CBitmapFont;
 *  - clipping is PushClipRect() and PopClipRect() of the draw list.
 *
 *  Anti-aliasing is switched off in the draw list for as long as the painter exists. Coordinates are logical pixels.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiDrawListPainter
{
    public:

    typedef GUI::CDrawSurface::TCommandVector TCommandVector;

    CImGuiDrawListPainter( CImGuiContext& context, ::ImDrawList& drawList );

    /** Goes back to linear sampling and restores the flags of the draw list */
    ~CImGuiDrawListPainter();

    /** The shown roots of the context in their order, each with its widgets (their cached drawing commands) */
    void PaintRoots( void );

    /** What gucefGUI draws over everything: the tooltip and the cursor picture (CWidgetPainter::PaintOverlay()) */
    void PaintOverlay( void );

    /** Draws commands moved by an offset, cut off at 'clip', with their alpha multiplied by 'opacity' */
    void Execute( const TCommandVector& commands, Int32 offsetX, Int32 offsetY, const GUI::CRect& clip, Float32 opacity );

    /** The rectangles (fills, glyphs, images, pixel runs) the painter put into the draw list */
    UInt32 GetQuadCount( void ) const;

    private:

    CImGuiDrawListPainter( const CImGuiDrawListPainter& src );
    CImGuiDrawListPainter& operator=( const CImGuiDrawListPainter& src );

    /** A clip rectangle of the draw list for as long as the scope exists */
    class CClipScope
    {
        public:

        CClipScope( ::ImDrawList& drawList, const GUI::CRect& clip );
        ~CClipScope();

        private:

        CClipScope( const CClipScope& src );
        CClipScope& operator=( const CClipScope& src );

        ::ImDrawList& m_drawList;
    };

    void PaintWidget( const GUI::CWidget& widget, const GUI::CRect& clip, Float32 opacity );
    void FillRect( Int32 left, Int32 top, Int32 width, Int32 height, const GUI::CColour& colour, UInt32 alpha );
    void PaintLine( Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 thickness, const GUI::CColour& colour, Float32 opacity );
    void PaintImage( const GUI::CDrawSurface::TCommand& command, const GUI::CRect& target, Float32 opacity );
    void PaintGlyphs( const GUI::CDrawSurface::TCommand& command, Int32 x, Int32 y, Float32 opacity );
    void SetNearestSampling( bool nearest );

    private:

    CImGuiContext& m_context;
    ::ImDrawList& m_drawList;
    Int32 m_previousFlags;
    bool m_nearest;
    UInt32 m_quads;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H ? */

/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CSoftwareRenderer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


namespace {

/** Blends a colour over a pixel; 'alpha' is the coverage 0..255 already including opacity */
inline void
BlendPixel( UInt8* pixel, UInt8 r, UInt8 g, UInt8 b, UInt32 alpha )
{
    if ( 0 == alpha )
        return;
    if ( alpha >= 255 )
    {
        pixel[ 0 ] = r;
        pixel[ 1 ] = g;
        pixel[ 2 ] = b;
        pixel[ 3 ] = 255;
        return;
    }
    UInt32 inverse = 255 - alpha;
    pixel[ 0 ] = (UInt8) ( ( r * alpha + pixel[ 0 ] * inverse + 127 ) / 255 );
    pixel[ 1 ] = (UInt8) ( ( g * alpha + pixel[ 1 ] * inverse + 127 ) / 255 );
    pixel[ 2 ] = (UInt8) ( ( b * alpha + pixel[ 2 ] * inverse + 127 ) / 255 );
    pixel[ 3 ] = (UInt8) ( alpha + ( pixel[ 3 ] * inverse + 127 ) / 255 );
}

inline UInt32
Scaled( UInt32 alpha, Float32 opacity )
{
    if ( opacity >= 1.0f )
        return alpha;
    if ( opacity <= 0.0f )
        return 0;
    return (UInt32) ( (Float32) alpha * opacity + 0.5f );
}

void
FillRect( IMAGE::CPixelMap& target, const CRect& rect, const CRect& clip, const CColour& colour, Float32 opacity )
{
    CRect area = rect.Intersection( clip );
    if ( area.IsEmpty() )
        return;
    UInt32 alpha = Scaled( colour.a, opacity );
    UInt8* data = static_cast< UInt8* >( target.GetDataPtr() );
    UInt32 width = target.GetWidthInPixels();
    for ( Int32 y=area.top; y<area.Bottom(); ++y )
    {
        UInt8* row = data + ( (size_t) y * width + area.left ) * 4;
        for ( Int32 x=0; x<area.width; ++x, row+=4 )
            BlendPixel( row, colour.r, colour.g, colour.b, alpha );
    }
}

void
PutPoint( IMAGE::CPixelMap& target, Int32 x, Int32 y, const CRect& clip, const CColour& colour, UInt32 alpha )
{
    if ( !clip.Contains( x, y ) )
        return;
    UInt8* pixel = static_cast< UInt8* >( target.GetDataPtr() ) + ( (size_t) y * target.GetWidthInPixels() + x ) * 4;
    BlendPixel( pixel, colour.r, colour.g, colour.b, alpha );
}

void
DrawLine( IMAGE::CPixelMap& target, Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 thickness, const CRect& clip, const CColour& colour, Float32 opacity )
{
    UInt32 alpha = Scaled( colour.a, opacity );
    Int32 dx = x1 > x0 ? x1 - x0 : x0 - x1;
    Int32 dy = y1 > y0 ? y0 - y1 : y1 - y0;
    Int32 sx = x0 < x1 ? 1 : -1;
    Int32 sy = y0 < y1 ? 1 : -1;
    Int32 error = dx + dy;
    Int32 half = thickness > 1 ? thickness / 2 : 0;
    while ( true )
    {
        if ( 0 == half )
            PutPoint( target, x0, y0, clip, colour, alpha );
        else
            FillRect( target, CRect( x0 - half, y0 - half, thickness, thickness ), clip, colour, opacity );
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
}

void
DrawImage( IMAGE::CPixelMap& target, const IMAGE::CPixelMap& image, const CRect& destination, const CRect& sourceRect,
           const CRect& clip, const CColour& tint, Float32 opacity )
{
    CRect source = sourceRect.IsEmpty() ? CRect( 0, 0, (Int32) image.GetWidthInPixels(), (Int32) image.GetHeightInPixels() ) : sourceRect;
    CRect area = destination.Intersection( clip );
    if ( area.IsEmpty() || source.IsEmpty() || destination.IsEmpty() )
        return;

    const UInt8* pixels = static_cast< const UInt8* >( image.GetDataPtr() );
    UInt32 imageWidth = image.GetWidthInPixels();
    UInt32 imageHeight = image.GetHeightInPixels();
    UInt8* data = static_cast< UInt8* >( target.GetDataPtr() );
    UInt32 width = target.GetWidthInPixels();
    UInt32 tintAlpha = Scaled( tint.a, opacity );

    for ( Int32 y=area.top; y<area.Bottom(); ++y )
    {
        // the nearest source pixel: the centre of the target pixel mapped into the source
        Int32 sy = source.top + (Int32) ( ( (Int64) ( y - destination.top ) * 2 + 1 ) * source.height / ( (Int64) destination.height * 2 ) );
        if ( sy < 0 || sy >= (Int32) imageHeight )
            continue;
        UInt8* row = data + ( (size_t) y * width + area.left ) * 4;
        for ( Int32 x=area.left; x<area.Right(); ++x, row+=4 )
        {
            Int32 sx = source.left + (Int32) ( ( (Int64) ( x - destination.left ) * 2 + 1 ) * source.width / ( (Int64) destination.width * 2 ) );
            if ( sx < 0 || sx >= (Int32) imageWidth )
                continue;
            const UInt8* in = pixels + ( (size_t) sy * imageWidth + sx ) * 4;
            UInt32 alpha = ( in[ 3 ] * tintAlpha + 127 ) / 255;
            BlendPixel( row, (UInt8) ( ( in[ 0 ] * tint.r + 127 ) / 255 ), (UInt8) ( ( in[ 1 ] * tint.g + 127 ) / 255 ),
                        (UInt8) ( ( in[ 2 ] * tint.b + 127 ) / 255 ), alpha );
        }
    }
}

/** A rectangle in logical pixels in the pixels of a frame drawn at a render scale */
CRect
Scaled( const CRect& rect, Int32 scale )
{
    return CRect( rect.left * scale, rect.top * scale, rect.width * scale, rect.height * scale );
}

/** Text at a logical position; the glyphs cover their source size divided by the density of the font */
void
DrawGlyphs( CGUIContext& context, IMAGE::CPixelMap& target, const CString& font, const CString& text, Int32 x, Int32 y,
          const CRect& clip, const CColour& colour, Float32 opacity, Int32 scale )
{
    TBitmapFontPtr bitmapFont = context.GetBitmapFont( font );
    if ( bitmapFont.IsNULL() || bitmapFont->GetAtlas().IsNULL() )
        return;
    const IMAGE::CPixelMap& atlas = *bitmapFont->GetAtlas();

    TCodePointVector codePoints;
    DecodeUtf8( text, codePoints );
    Int32 pen = x;
    bool first = true;
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        const CBitmapFont::TGlyph* glyph = bitmapFont->ResolveGlyph( codePoints[ i ] );
        if ( GUCEF_NULL == glyph )
            continue;
        if ( !first )
            pen += bitmapFont->GetSpacing();
        first = false;
        const Int32 density = (Int32) bitmapFont->GetDensity();
        CRect destination( ( pen + glyph->offsetX ) * scale, ( y + glyph->offsetY ) * scale,
                           glyph->source.width * scale / density, glyph->source.height * scale / density );
        DrawImage( target, atlas, destination, glyph->source, clip, colour, opacity );
        pen += glyph->advance;
    }
}

}

void
CSoftwareRenderer::Execute( CGUIContext& context          ,
                            const TCommandVector& commands ,
                            Int32 offsetX                  ,
                            Int32 offsetY                  ,
                            const CRect& clip              ,
                            Float32 opacity                ,
                            IMAGE::CPixelMap& target       )
{GUCEF_TRACE;

    // the commands are in logical pixels, the frame is drawn at the render scale
    const Int32 k = (Int32) context.GetRenderScale();
    GUCEF::vector< CRect > clips;
    clips.push_back( Scaled( clip, k ) );

    for ( UInt32 i=0; i<(UInt32)commands.size(); ++i )
    {
        const CDrawSurface::TCommand& command = commands[ i ];
        const CRect& current = clips.back();
        CRect rect( command.rect.left + offsetX, command.rect.top + offsetY, command.rect.width, command.rect.height );
        switch ( command.type )
        {
            case CDrawSurface::COMMAND_FILLRECT:
            {
                FillRect( target, Scaled( rect, k ), current, command.colour, opacity );
                break;
            }
            case CDrawSurface::COMMAND_STROKERECT:
            {
                Int32 t = command.thickness < 1 ? 1 : command.thickness;
                FillRect( target, Scaled( CRect( rect.left, rect.top, rect.width, t ), k ), current, command.colour, opacity );
                FillRect( target, Scaled( CRect( rect.left, rect.Bottom() - t, rect.width, t ), k ), current, command.colour, opacity );
                FillRect( target, Scaled( CRect( rect.left, rect.top + t, t, rect.height - 2*t ), k ), current, command.colour, opacity );
                FillRect( target, Scaled( CRect( rect.Right() - t, rect.top + t, t, rect.height - 2*t ), k ), current, command.colour, opacity );
                break;
            }
            case CDrawSurface::COMMAND_LINE:
            {
                DrawLine( target, ( command.x0 + offsetX ) * k, ( command.y0 + offsetY ) * k, ( command.x1 + offsetX ) * k, ( command.y1 + offsetY ) * k,
                          command.thickness * k, current, command.colour, opacity );
                break;
            }
            case CDrawSurface::COMMAND_POINTS:
            {
                UInt32 alpha = Scaled( command.colour.a, opacity );
                for ( UInt32 p=0; p<(UInt32)command.points.size(); ++p )
                    for ( Int32 py=0; py<k; ++py )
                        for ( Int32 px=0; px<k; ++px )
                            PutPoint( target, ( command.points[ p ].x + offsetX ) * k + px, ( command.points[ p ].y + offsetY ) * k + py, current, command.colour, alpha );
                break;
            }
            case CDrawSurface::COMMAND_IMAGE:
            {
                // the source is in logical pixels of the image: its pixels divided by its density
                IMAGE::TPixelMapPtr pixels;
                UInt32 revision = 0;
                if ( context.ResolveImage( command.image, pixels, revision ) )
                {
                    Int32 density = (Int32) context.GetImageDensity( command.image );
                    CRect source = command.source.IsEmpty() ? command.source : Scaled( command.source, density );
                    DrawImage( target, *pixels, Scaled( rect, k ), source, current, command.colour, opacity );
                }
                break;
            }
            case CDrawSurface::COMMAND_TEXT:
            {
                DrawGlyphs( context, target, command.font, command.text, command.x0 + offsetX, command.y0 + offsetY, current, command.colour, opacity, k );
                break;
            }
            case CDrawSurface::COMMAND_PUSHCLIP:
            {
                clips.push_back( current.Intersection( Scaled( rect, k ) ) );
                break;
            }
            case CDrawSurface::COMMAND_POPCLIP:
            {
                if ( clips.size() > 1 )
                    clips.pop_back();
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSoftwareRenderer::RenderWidget( CGUIContext& context, const CWidget& widget, const CRect& clip, Float32 opacity, IMAGE::CPixelMap& target )
{GUCEF_TRACE;

    if ( !widget.IsVisible() )
        return;
    const CRect& rect = widget.GetRect();
    Float32 widgetOpacity = opacity * widget.GetFloatProperty( EWidgetProperty_Opacity );

    // the widget's own drawing stays within the widget
    TCommandVector commands;
    CWidgetPainter::Paint( widget, context, commands );
    Execute( context, commands, rect.left, rect.top, clip.Intersection( rect ), widgetOpacity, target );

    // what a collapsed window hides is not drawn
    if ( !widget.AreChildrenShown() )
        return;
    CRect childClip = widget.GetBoolProperty( EWidgetProperty_Clip ) ? clip.Intersection( rect ) : clip;
    const CWidget::TWidgetVector& children = widget.GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        RenderWidget( context, *children[ i ], childClip, widgetOpacity, target );
}

/*-------------------------------------------------------------------------*/

bool
CSoftwareRenderer::RenderContext( CGUIContext& context, IMAGE::TPixelMapPtr& target )
{GUCEF_TRACE;

    UInt32 width = context.GetRenderWidth();
    UInt32 height = context.GetRenderHeight();
    if ( 0 == width || 0 == height )
        return false;
    if ( target.IsNULL() || target->GetWidthInPixels() != width || target->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != target->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != target->GetPixelComponentDataType() )
    {
        target = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    }

    // opaque black
    UInt8* data = static_cast< UInt8* >( target->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        data[ i*4 ] = data[ i*4+1 ] = data[ i*4+2 ] = 0;
        data[ i*4+3 ] = 255;
    }

    context.FlushChanges();
    CRect screen( 0, 0, (Int32) context.GetLogicalWidth(), (Int32) context.GetLogicalHeight() );
    const CGUIContext::TWidgetVector& roots = context.GetRootWidgets();
    for ( UInt32 i=0; i<(UInt32)roots.size(); ++i )
        RenderWidget( context, *roots[ i ], screen, 1.0f, *target );

    TCommandVector overlay;
    CWidgetPainter::PaintOverlay( context, overlay );
    Execute( context, overlay, 0, 0, screen, 1.0f, *target );
    return true;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

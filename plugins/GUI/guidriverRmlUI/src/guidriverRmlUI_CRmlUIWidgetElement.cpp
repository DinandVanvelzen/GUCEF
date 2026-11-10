/*
 *  guidriverRmlUI: GUI backend using RmlUi
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

#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/RenderManager.h>

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#include "guidriverRmlUI_CRmlUIContext.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */

#include "guidriverRmlUI_CRmlUIWidgetElement.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
Scaled( UInt32 alpha, Float32 opacity )
{
    if ( opacity >= 1.0f )
        return alpha;
    if ( opacity <= 0.0f )
        return 0;
    return (UInt32) ( (Float32) alpha * opacity + 0.5f );
}

/*-------------------------------------------------------------------------*/

inline Rml::ColourbPremultiplied
Premultiplied( const GUI::CColour& colour, UInt32 alpha )
{
    return Rml::ColourbPremultiplied( (Rml::byte) ( ( colour.r * alpha + 127 ) / 255 ),
                                      (Rml::byte) ( ( colour.g * alpha + 127 ) / 255 ),
                                      (Rml::byte) ( ( colour.b * alpha + 127 ) / 255 ),
                                      (Rml::byte) alpha );
}

/*-------------------------------------------------------------------------*/

bool
SameCommand( const GUI::CDrawSurface::TCommand& a, const GUI::CDrawSurface::TCommand& b )
{
    if ( a.type != b.type || !( a.rect == b.rect ) || !( a.colour == b.colour ) )
        return false;
    switch ( a.type )
    {
        case GUI::CDrawSurface::COMMAND_STROKERECT:
            return a.thickness == b.thickness;
        case GUI::CDrawSurface::COMMAND_LINE:
            return a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1 && a.thickness == b.thickness;
        case GUI::CDrawSurface::COMMAND_IMAGE:
            return a.source == b.source && a.image == b.image;
        case GUI::CDrawSurface::COMMAND_TEXT:
            return a.x0 == b.x0 && a.y0 == b.y0 && a.font == b.font && a.text == b.text;
        case GUI::CDrawSurface::COMMAND_POINTS:
        {
            if ( a.points.size() != b.points.size() )
                return false;
            for ( size_t i=0; i<a.points.size(); ++i )
                if ( a.points[ i ].x != b.points[ i ].x || a.points[ i ].y != b.points[ i ].y )
                    return false;
            return true;
        }
        default:
            return true;
    }
}

}

/*-------------------------------------------------------------------------*/

/**
 *  Turns drawing commands into rectangles of one colour or of a part of a texture, clipped to the
 *  current clip rectangle, in batches that share a texture. The rectangles cover the same pixels
 *  the software renderer of gucefGUI fills, and textures are sampled at the same pixel centres.
 */
class CRmlUIQuadBuilder
{
    public:

    CRmlUIQuadBuilder( CRmlUIContext& context, Rml::RenderManager& renderManager, CRmlUIWidgetElement::TBatchVector& batches )
        : m_context( context )
        , m_renderManager( renderManager )
        , m_batches( batches )
        , m_mesh()
        , m_texture()
        , m_isFont( false )
        , m_textureWidth( 0 )
        , m_textureHeight( 0 )
        , m_density( 1 )
        , m_scale( (Int32) context.GetRenderScale() )
    {
    }

    void UseColour( void )
    {
        Use( CString::Empty, false, 0, 0, 1 );
    }

    void Use( const CString& texture, bool isFont, Int32 width, Int32 height, UInt32 density )
    {
        if ( texture != m_texture || isFont != m_isFont || width != m_textureWidth || height != m_textureHeight || density != m_density )
        {
            Flush();
            m_texture = texture;
            m_isFont = isFont;
            m_textureWidth = width;
            m_textureHeight = height;
            m_density = density;
        }
    }

    /** A rectangle in logical pixels in the pixels of the frame, which is drawn at the render scale */
    GUI::CRect ToFrame( const GUI::CRect& rect ) const
    {
        return GUI::CRect( rect.left * m_scale, rect.top * m_scale, rect.width * m_scale, rect.height * m_scale );
    }

    /** A rectangle of one colour */
    void Fill( const GUI::CRect& rect, const GUI::CRect& clip, Rml::ColourbPremultiplied colour )
    {
        GUI::CRect area = rect.Intersection( clip );
        if ( area.IsEmpty() || 0 == colour.alpha )
            return;
        UseColour();
        AddQuad( (float) area.left, (float) area.top, (float) area.Right(), (float) area.Bottom(), 0.0f, 0.0f, 0.0f, 0.0f, colour );
    }

    /** The 'source' part of the current texture stretched over 'destination' */
    void Image( const GUI::CRect& destination, const GUI::CRect& source, const GUI::CRect& clip, Rml::ColourbPremultiplied colour )
    {
        GUI::CRect area = destination.Intersection( clip );
        if ( area.IsEmpty() || source.IsEmpty() || destination.IsEmpty() || 0 == colour.alpha || m_textureWidth <= 0 || m_textureHeight <= 0 )
            return;

        // the texture coordinate at a pixel edge, so pixel centres land on the texels the software renderer takes
        float scaleX = (float) source.width / (float) destination.width;
        float scaleY = (float) source.height / (float) destination.height;
        float u0 = ( source.left + ( area.left - destination.left ) * scaleX ) / (float) m_textureWidth;
        float u1 = ( source.left + ( area.Right() - destination.left ) * scaleX ) / (float) m_textureWidth;
        float v0 = ( source.top + ( area.top - destination.top ) * scaleY ) / (float) m_textureHeight;
        float v1 = ( source.top + ( area.Bottom() - destination.top ) * scaleY ) / (float) m_textureHeight;
        AddQuad( (float) area.left, (float) area.top, (float) area.Right(), (float) area.Bottom(), u0, v0, u1, v1, colour );
    }

    void Flush( void )
    {
        if ( m_mesh.indices.empty() )
            return;
        CRmlUIWidgetElement::TBatch batch;
        batch.geometry = m_renderManager.MakeGeometry( std::move( m_mesh ) );
        batch.texture = m_texture;
        batch.isFont = m_isFont;
        batch.textureWidth = m_textureWidth;
        batch.textureHeight = m_textureHeight;
        batch.density = m_density;
        m_batches.push_back( std::move( batch ) );
        m_mesh = Rml::Mesh();
    }

    void Execute( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity, Int32 offsetX, Int32 offsetY );

    private:

    void AddQuad( float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, Rml::ColourbPremultiplied colour )
    {
        int base = (int) m_mesh.vertices.size();
        Rml::Vertex vertex;
        vertex.colour = colour;
        vertex.position = Rml::Vector2f( x0, y0 ); vertex.tex_coord = Rml::Vector2f( u0, v0 ); m_mesh.vertices.push_back( vertex );
        vertex.position = Rml::Vector2f( x1, y0 ); vertex.tex_coord = Rml::Vector2f( u1, v0 ); m_mesh.vertices.push_back( vertex );
        vertex.position = Rml::Vector2f( x1, y1 ); vertex.tex_coord = Rml::Vector2f( u1, v1 ); m_mesh.vertices.push_back( vertex );
        vertex.position = Rml::Vector2f( x0, y1 ); vertex.tex_coord = Rml::Vector2f( u0, v1 ); m_mesh.vertices.push_back( vertex );
        m_mesh.indices.push_back( base );
        m_mesh.indices.push_back( base + 1 );
        m_mesh.indices.push_back( base + 2 );
        m_mesh.indices.push_back( base );
        m_mesh.indices.push_back( base + 2 );
        m_mesh.indices.push_back( base + 3 );
    }

    void Line( Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 thickness, const GUI::CRect& clip, Rml::ColourbPremultiplied colour );

    void Glyphs( const CString& font, const CString& text, Int32 x, Int32 y, const GUI::CRect& clip, const GUI::CColour& colour, Float32 opacity );

    private:

    CRmlUIContext& m_context;
    Rml::RenderManager& m_renderManager;
    CRmlUIWidgetElement::TBatchVector& m_batches;
    Rml::Mesh m_mesh;
    CString m_texture;
    bool m_isFont;
    Int32 m_textureWidth;
    Int32 m_textureHeight;
    UInt32 m_density;
    Int32 m_scale;
};

/*-------------------------------------------------------------------------*/

void
CRmlUIQuadBuilder::Line( Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 thickness, const GUI::CRect& clip, Rml::ColourbPremultiplied colour )
{
    // the same steps as the software renderer; single pixel lines are merged into runs along a row
    Int32 dx = x1 > x0 ? x1 - x0 : x0 - x1;
    Int32 dy = y1 > y0 ? y0 - y1 : y1 - y0;
    Int32 sx = x0 < x1 ? 1 : -1;
    Int32 sy = y0 < y1 ? 1 : -1;
    Int32 error = dx + dy;
    Int32 half = thickness > 1 ? thickness / 2 : 0;

    bool runOpen = false;
    Int32 runY = 0, runFrom = 0, runTo = 0;
    while ( true )
    {
        if ( 0 == half )
        {
            if ( runOpen && y0 == runY && ( x0 == runTo + 1 || x0 == runFrom - 1 ) )
            {
                if ( x0 > runTo ) runTo = x0; else runFrom = x0;
            }
            else
            {
                if ( runOpen )
                    Fill( GUI::CRect( runFrom, runY, runTo - runFrom + 1, 1 ), clip, colour );
                runOpen = true;
                runY = y0;
                runFrom = runTo = x0;
            }
        }
        else
        {
            Fill( GUI::CRect( x0 - half, y0 - half, thickness, thickness ), clip, colour );
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
    if ( runOpen )
        Fill( GUI::CRect( runFrom, runY, runTo - runFrom + 1, 1 ), clip, colour );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIQuadBuilder::Glyphs( const CString& font, const CString& text, Int32 x, Int32 y, const GUI::CRect& clip, const GUI::CColour& colour, Float32 opacity )
{
    GUI::TBitmapFontPtr bitmapFont = m_context.GetBitmapFont( font );
    if ( bitmapFont.IsNULL() || bitmapFont->GetAtlas().IsNULL() )
        return;
    Int32 width = 0, height = 0;
    if ( !m_context.HasFontTexture( font, width, height ) )
        return;
    const Int32 density = (Int32) bitmapFont->GetDensity();
    Use( font, true, width, height, (UInt32) density );

    Rml::ColourbPremultiplied tint = Premultiplied( colour, Scaled( colour.a, opacity ) );
    GUI::TCodePointVector codePoints;
    GUI::DecodeUtf8( text, codePoints );
    Int32 pen = x;
    bool first = true;
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        const GUI::CBitmapFont::TGlyph* glyph = bitmapFont->ResolveGlyph( codePoints[ i ] );
        if ( GUCEF_NULL == glyph )
            continue;
        if ( !first )
            pen += bitmapFont->GetSpacing();
        first = false;
        // the glyph covers its source size divided by the density of the font, drawn at the render scale
        GUI::CRect destination( ( pen + glyph->offsetX ) * m_scale, ( y + glyph->offsetY ) * m_scale,
                                glyph->source.width * m_scale / density, glyph->source.height * m_scale / density );
        Image( destination, glyph->source, clip, tint );
        pen += glyph->advance;
    }
}

/*-------------------------------------------------------------------------*/

void
CRmlUIQuadBuilder::Execute( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity, Int32 offsetX, Int32 offsetY )
{
    // the commands are in logical pixels, the frame is drawn at the render scale (as the software renderer does)
    GUCEF::vector< GUI::CRect > clips;
    clips.push_back( ToFrame( clip ) );
    for ( UInt32 i=0; i<(UInt32)commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = commands[ i ];
        const GUI::CRect current = clips.back();
        GUI::CRect rect( command.rect.left + offsetX, command.rect.top + offsetY, command.rect.width, command.rect.height );
        Rml::ColourbPremultiplied colour = Premultiplied( command.colour, Scaled( command.colour.a, opacity ) );
        switch ( command.type )
        {
            case GUI::CDrawSurface::COMMAND_FILLRECT:
            {
                Fill( ToFrame( rect ), current, colour );
                break;
            }
            case GUI::CDrawSurface::COMMAND_STROKERECT:
            {
                Int32 t = command.thickness < 1 ? 1 : command.thickness;
                Fill( ToFrame( GUI::CRect( rect.left, rect.top, rect.width, t ) ), current, colour );
                Fill( ToFrame( GUI::CRect( rect.left, rect.Bottom() - t, rect.width, t ) ), current, colour );
                Fill( ToFrame( GUI::CRect( rect.left, rect.top + t, t, rect.height - 2*t ) ), current, colour );
                Fill( ToFrame( GUI::CRect( rect.Right() - t, rect.top + t, t, rect.height - 2*t ) ), current, colour );
                break;
            }
            case GUI::CDrawSurface::COMMAND_LINE:
            {
                Line( ( command.x0 + offsetX ) * m_scale, ( command.y0 + offsetY ) * m_scale, ( command.x1 + offsetX ) * m_scale, ( command.y1 + offsetY ) * m_scale,
                      command.thickness * m_scale, current, colour );
                break;
            }
            case GUI::CDrawSurface::COMMAND_POINTS:
            {
                for ( UInt32 p=0; p<(UInt32)command.points.size(); ++p )
                    Fill( ToFrame( GUI::CRect( command.points[ p ].x + offsetX, command.points[ p ].y + offsetY, 1, 1 ) ), current, colour );
                break;
            }
            case GUI::CDrawSurface::COMMAND_IMAGE:
            {
                // the source is in logical pixels of the image: its pixels divided by its density
                Int32 width = 0, height = 0;
                if ( m_context.HasImageTexture( command.image, width, height ) )
                {
                    const UInt32 density = m_context.GetImageDensity( command.image );
                    Use( command.image, false, width, height, density );
                    GUI::CRect source = command.source.IsEmpty() ? GUI::CRect( 0, 0, width, height ) :
                        GUI::CRect( command.source.left * (Int32) density, command.source.top * (Int32) density,
                                    command.source.width * (Int32) density, command.source.height * (Int32) density );
                    Image( ToFrame( rect ), source, current, colour );
                }
                break;
            }
            case GUI::CDrawSurface::COMMAND_TEXT:
            {
                Glyphs( command.font, command.text, command.x0 + offsetX, command.y0 + offsetY, current, command.colour, opacity );
                break;
            }
            case GUI::CDrawSurface::COMMAND_PUSHCLIP:
            {
                clips.push_back( current.Intersection( ToFrame( rect ) ) );
                break;
            }
            case GUI::CDrawSurface::COMMAND_POPCLIP:
            {
                if ( clips.size() > 1 )
                    clips.pop_back();
                break;
            }
        }
    }
    Flush();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CRmlUIWidgetElement::TagName = "gucefwidget";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIWidgetElement::CRmlUIWidgetElement( const Rml::String& tag )
    : Rml::Element( tag )
    , m_context( GUCEF_NULL )
    , m_widget( GUCEF_NULL )
    , m_overlay( false )
    , m_valid( false )
    , m_commands()
    , m_clip()
    , m_opacity( 1.0f )
    , m_renderScale( 1 )
    , m_batches()
    , m_buildCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIWidgetElement::~CRmlUIWidgetElement()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetElement::SetWidget( CRmlUIContext* context, GUI::CWidget* widget, bool overlay )
{GUCEF_TRACE;

    m_context = context;
    m_widget = widget;
    m_overlay = overlay;
    Invalidate();
}

/*-------------------------------------------------------------------------*/

GUI::CWidget*
CRmlUIWidgetElement::GetWidget( void ) const
{GUCEF_TRACE;

    return m_widget;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetElement::Invalidate( void )
{GUCEF_TRACE;

    m_valid = false;
    m_batches.clear();
    m_commands.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIWidgetElement::GetBuildCount( void ) const
{GUCEF_TRACE;

    return m_buildCount;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIWidgetElement::IsUpToDate( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity ) const
{GUCEF_TRACE;

    if ( !m_valid || !( clip == m_clip ) || opacity != m_opacity || commands.size() != m_commands.size() || m_context->GetRenderScale() != m_renderScale )
        return false;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( !SameCommand( commands[ i ], m_commands[ i ] ) )
            return false;

    // a picture that changed size moves its texture coordinates
    for ( size_t i=0; i<m_batches.size(); ++i )
    {
        const TBatch& batch = m_batches[ i ];
        if ( batch.texture.IsNULLOrEmpty() )
            continue;
        Int32 width = 0, height = 0;
        bool known = batch.isFont ? m_context->HasFontTexture( batch.texture, width, height ) : m_context->HasImageTexture( batch.texture, width, height );
        if ( !known || width != batch.textureWidth || height != batch.textureHeight )
            return false;
        // a density that changed moves the texture coordinates too
        UInt32 density = 1;
        if ( batch.isFont )
        {
            GUI::TBitmapFontPtr font = m_context->GetBitmapFont( batch.texture );
            density = font.IsNULL() ? 1 : font->GetDensity();
        }
        else
        {
            density = m_context->GetImageDensity( batch.texture );
        }
        if ( density != batch.density )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetElement::Build( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity )
{GUCEF_TRACE;

    m_batches.clear();
    Rml::RenderManager* renderManager = GetRenderManager();
    if ( GUCEF_NULL == renderManager )
        return;

    // the widget's commands are relative to the widget, the overlay's to the context
    Int32 offsetX = 0, offsetY = 0;
    if ( GUCEF_NULL != m_widget )
    {
        offsetX = m_widget->GetRect().left;
        offsetY = m_widget->GetRect().top;
    }
    CRmlUIQuadBuilder builder( *m_context, *renderManager, m_batches );
    builder.Execute( commands, clip, opacity, offsetX, offsetY );

    m_commands = commands;
    m_clip = clip;
    m_opacity = opacity;
    m_renderScale = m_context->GetRenderScale();
    m_valid = true;
    ++m_buildCount;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetElement::OnRender( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || ( GUCEF_NULL == m_widget && !m_overlay ) )
        return;

    GUI::CDrawSurface::TCommandVector commands;
    GUI::CRect clip( 0, 0, (Int32) m_context->GetLogicalWidth(), (Int32) m_context->GetLogicalHeight() );
    Float32 opacity = 1.0f;
    if ( m_overlay )
    {
        GUI::CWidgetPainter::PaintOverlay( *m_context, commands );
    }
    else
    {
        // what the widget may draw in: its own rectangle, within every ancestor that clips its children,
        // at the opacity of the whole chain
        GUCEF::vector< const GUI::CWidget* > chain;
        for ( const GUI::CWidget* w=m_widget; GUCEF_NULL != w; w=w->GetParentWidget() )
        {
            if ( !w->IsVisible() )
                return;
            chain.push_back( w );
        }
        for ( size_t i=chain.size(); i>1; --i )
        {
            const GUI::CWidget* ancestor = chain[ i-1 ];
            if ( !ancestor->AreChildrenShown() )
                return;
            opacity *= ancestor->GetFloatProperty( GUI::EWidgetProperty_Opacity );
            if ( ancestor->GetBoolProperty( GUI::EWidgetProperty_Clip ) )
                clip = clip.Intersection( ancestor->GetRect() );
        }
        opacity *= m_widget->GetFloatProperty( GUI::EWidgetProperty_Opacity );
        clip = clip.Intersection( m_widget->GetRect() );
        if ( clip.IsEmpty() || opacity <= 0.0f )
            return;
        GUI::CWidgetPainter::Paint( *m_widget, *m_context, commands );
    }

    if ( !IsUpToDate( commands, clip, opacity ) )
        Build( commands, clip, opacity );

    for ( size_t i=0; i<m_batches.size(); ++i )
    {
        const TBatch& batch = m_batches[ i ];
        if ( batch.texture.IsNULLOrEmpty() )
        {
            batch.geometry.Render( Rml::Vector2f( 0.0f, 0.0f ) );
        }
        else
        {
            Rml::Texture texture = batch.isFont ? m_context->GetFontTexture( batch.texture ) : m_context->GetImageTexture( batch.texture );
            if ( texture )
                batch.geometry.Render( Rml::Vector2f( 0.0f, 0.0f ), texture );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#include <math.h>
#include <string.h>

#include "guidriverImGui_CImGuiSoftwareRenderer.h"

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

/** The pixels of a texture as the rasterizer reads them */
struct STexture
{
    const unsigned char* pixels;
    Int32 width;
    Int32 height;
    Int32 bytesPerPixel;
};

/** A rectangle of pixels: left and top included, right and bottom not */
struct SPixelRect
{
    Int32 left;
    Int32 top;
    Int32 right;
    Int32 bottom;
};

/** How texels are read */
struct SSampler
{
    bool nearest;
};

bool
ResolveTexture( const ImDrawCmd& command, const ImDrawData& drawData, STexture& texture )
{
    const ImTextureData* data = command.TexRef._TexData;
    if ( GUCEF_NULL == data )
    {
        const ImTextureID id = command.TexRef._TexID;
        if ( ImTextureID_Invalid == id || GUCEF_NULL == drawData.Textures )
            return false;
        for ( int i=0; i<drawData.Textures->Size; ++i )
        {
            const ImTextureData* candidate = (*drawData.Textures)[ i ];
            if ( GUCEF_NULL != candidate && candidate->TexID == id )
            {
                data = candidate;
                break;
            }
        }
    }
    if ( GUCEF_NULL == data || GUCEF_NULL == data->Pixels || data->Width <= 0 || data->Height <= 0 )
        return false;
    texture.pixels = data->Pixels;
    texture.width = data->Width;
    texture.height = data->Height;
    texture.bytesPerPixel = data->BytesPerPixel;
    return 4 == texture.bytesPerPixel || 1 == texture.bytesPerPixel;
}

/** The texel at a place, the edges repeated beyond the texture */
inline void
Texel( const STexture& texture, Int32 x, Int32 y, UInt32* rgba )
{
    if ( x < 0 )
        x = 0;
    else
    if ( x >= texture.width )
        x = texture.width - 1;
    if ( y < 0 )
        y = 0;
    else
    if ( y >= texture.height )
        y = texture.height - 1;
    const unsigned char* texel = texture.pixels + ( (size_t) y * texture.width + x ) * texture.bytesPerPixel;
    if ( 4 == texture.bytesPerPixel )
    {
        rgba[ 0 ] = texel[ 0 ];
        rgba[ 1 ] = texel[ 1 ];
        rgba[ 2 ] = texel[ 2 ];
        rgba[ 3 ] = texel[ 3 ];
    }
    else
    {
        rgba[ 0 ] = rgba[ 1 ] = rgba[ 2 ] = 255;
        rgba[ 3 ] = texel[ 0 ];
    }
}

/** The texel at texel coordinates (s, t): the nearest one, or the four around it weighted */
inline void
Sample( const STexture& texture, const SSampler& sampler, double s, double t, UInt32* rgba )
{
    if ( sampler.nearest )
    {
        // the tiny bias keeps a coordinate that is a whole number on its texel despite the rounding of the arithmetic
        Texel( texture, (Int32) floor( s + 1e-9 ), (Int32) floor( t + 1e-9 ), rgba );
        return;
    }
    double x = s - 0.5;
    double y = t - 0.5;
    double x0 = floor( x );
    double y0 = floor( y );
    double fx = x - x0;
    double fy = y - y0;
    UInt32 a[ 4 ], b[ 4 ], c[ 4 ], d[ 4 ];
    Texel( texture, (Int32) x0, (Int32) y0, a );
    Texel( texture, (Int32) x0 + 1, (Int32) y0, b );
    Texel( texture, (Int32) x0, (Int32) y0 + 1, c );
    Texel( texture, (Int32) x0 + 1, (Int32) y0 + 1, d );
    for ( UInt32 i=0; i<4; ++i )
    {
        double top = a[ i ] + ( (double) b[ i ] - a[ i ] ) * fx;
        double bottom = c[ i ] + ( (double) d[ i ] - c[ i ] ) * fx;
        double value = top + ( bottom - top ) * fy;
        rgba[ i ] = (UInt32) ( value + 0.5 );
    }
}

/** Straight alpha blending of a colour over a pixel, rounded as gucefGUI's CSoftwareRenderer does */
inline void
Blend( UInt8* pixel, UInt32 r, UInt32 g, UInt32 b, UInt32 alpha )
{
    if ( 0 == alpha )
        return;
    if ( alpha >= 255 )
    {
        pixel[ 0 ] = (UInt8) r;
        pixel[ 1 ] = (UInt8) g;
        pixel[ 2 ] = (UInt8) b;
        pixel[ 3 ] = 255;
        return;
    }
    UInt32 inverse = 255 - alpha;
    pixel[ 0 ] = (UInt8) ( ( r * alpha + pixel[ 0 ] * inverse + 127 ) / 255 );
    pixel[ 1 ] = (UInt8) ( ( g * alpha + pixel[ 1 ] * inverse + 127 ) / 255 );
    pixel[ 2 ] = (UInt8) ( ( b * alpha + pixel[ 2 ] * inverse + 127 ) / 255 );
    pixel[ 3 ] = (UInt8) ( alpha + ( pixel[ 3 ] * inverse + 127 ) / 255 );
}

/** The texel times the colour of the vertex, blended over the pixel */
inline void
Shade( UInt8* pixel, const UInt32* texel, const UInt32* colour )
{
    Blend( pixel, ( texel[ 0 ] * colour[ 0 ] + 127 ) / 255, ( texel[ 1 ] * colour[ 1 ] + 127 ) / 255,
           ( texel[ 2 ] * colour[ 2 ] + 127 ) / 255, ( texel[ 3 ] * colour[ 3 ] + 127 ) / 255 );
}

inline void
Unpack( ImU32 colour, UInt32* rgba )
{
    rgba[ 0 ] = ( colour >> IM_COL32_R_SHIFT ) & 0xFF;
    rgba[ 1 ] = ( colour >> IM_COL32_G_SHIFT ) & 0xFF;
    rgba[ 2 ] = ( colour >> IM_COL32_B_SHIFT ) & 0xFF;
    rgba[ 3 ] = ( colour >> IM_COL32_A_SHIFT ) & 0xFF;
}

/** A texture coordinate that is within a thousandth of a texel edge is taken to be on it: the UVs of integer rectangles */
inline double
SnapToTexelEdge( double coordinate )
{
    double edge = floor( coordinate + 0.5 );
    return fabs( coordinate - edge ) < 1e-3 ? edge : coordinate;
}

/** The first pixel row or column whose centre is at or after 'edge' */
inline Int32
FirstCentreFrom( double edge )
{
    return (Int32) ceil( edge - 0.5 );
}

/** Two triangles that are the rectangle PrimRect() and PrimRectUV() make: a, b, c and a, c, d with straight sides */
bool
IsAxisAlignedQuad( const ImDrawVert* vertices, const ImDrawIdx* indices )
{
    if ( indices[ 3 ] != indices[ 0 ] || indices[ 4 ] != indices[ 2 ] )
        return false;
    const ImDrawVert& a = vertices[ indices[ 0 ] ];
    const ImDrawVert& b = vertices[ indices[ 1 ] ];
    const ImDrawVert& c = vertices[ indices[ 2 ] ];
    const ImDrawVert& d = vertices[ indices[ 5 ] ];
    return a.pos.y == b.pos.y && b.pos.x == c.pos.x && c.pos.y == d.pos.y && d.pos.x == a.pos.x &&
           a.uv.y == b.uv.y && b.uv.x == c.uv.x && c.uv.y == d.uv.y && d.uv.x == a.uv.x &&
           a.col == b.col && a.col == c.col && a.col == d.col && a.pos.x != c.pos.x && a.pos.y != c.pos.y;
}

/** A rectangle with straight texture coordinates, from corner a to the opposite corner c, in pixels */
void
DrawQuad( IMAGE::CPixelMap& target, const SPixelRect& scissor, const STexture& texture, const SSampler& sampler,
          double ax, double ay, double cx, double cy, double au, double av, double cu, double cv, ImU32 colour )
{
    UInt32 rgba[ 4 ];
    Unpack( colour, rgba );
    if ( 0 == rgba[ 3 ] )
        return;

    Int32 left = FirstCentreFrom( ax < cx ? ax : cx );
    Int32 right = FirstCentreFrom( ax < cx ? cx : ax );
    Int32 top = FirstCentreFrom( ay < cy ? ay : cy );
    Int32 bottom = FirstCentreFrom( ay < cy ? cy : ay );
    if ( left < scissor.left )
        left = scissor.left;
    if ( right > scissor.right )
        right = scissor.right;
    if ( top < scissor.top )
        top = scissor.top;
    if ( bottom > scissor.bottom )
        bottom = scissor.bottom;
    if ( left >= right || top >= bottom )
        return;

    // texel coordinates of the corners
    double as = SnapToTexelEdge( au * texture.width );
    double at = SnapToTexelEdge( av * texture.height );
    double cs = SnapToTexelEdge( cu * texture.width );
    double ct = SnapToTexelEdge( cv * texture.height );
    double ds = ( cs - as ) / ( cx - ax );
    double dt = ( ct - at ) / ( cy - ay );

    UInt8* data = static_cast< UInt8* >( target.GetDataPtr() );
    const UInt32 width = target.GetWidthInPixels();
    UInt32 texel[ 4 ];
    for ( Int32 y=top; y<bottom; ++y )
    {
        double t = at + ( y + 0.5 - ay ) * dt;
        UInt8* pixel = data + ( (size_t) y * width + left ) * 4;
        for ( Int32 x=left; x<right; ++x, pixel+=4 )
        {
            double s = as + ( x + 0.5 - ax ) * ds;
            Sample( texture, sampler, s, t, texel );
            Shade( pixel, texel, rgba );
        }
    }
}

inline double
Edge( double ax, double ay, double bx, double by, double px, double py )
{
    return ( bx - ax ) * ( py - ay ) - ( by - ay ) * ( px - ax );
}

/** Whether a pixel centre exactly on the edge from a to b belongs to the triangle: top and left edges only */
inline bool
IsTopLeft( double ax, double ay, double bx, double by )
{
    return ( ay == by && bx > ax ) || by < ay;
}

/** Any triangle, with the texture coordinates and the colours interpolated */
void
DrawTriangle( IMAGE::CPixelMap& target, const SPixelRect& scissor, const STexture& texture, const SSampler& sampler,
              const ImDrawVert& v0, const ImDrawVert& v1, const ImDrawVert& v2, const ImVec2& offset, const ImVec2& scale )
{
    const ImDrawVert* p[ 3 ] = { &v0, &v1, &v2 };
    double x[ 3 ], y[ 3 ];
    for ( UInt32 i=0; i<3; ++i )
    {
        x[ i ] = ( p[ i ]->pos.x - offset.x ) * scale.x;
        y[ i ] = ( p[ i ]->pos.y - offset.y ) * scale.y;
    }
    double area = Edge( x[ 0 ], y[ 0 ], x[ 1 ], y[ 1 ], x[ 2 ], y[ 2 ] );
    if ( 0.0 == area )
        return;
    if ( area < 0.0 )
    {
        // the other winding: the same triangle with two corners swapped
        const ImDrawVert* swap = p[ 1 ]; p[ 1 ] = p[ 2 ]; p[ 2 ] = swap;
        double sx = x[ 1 ]; x[ 1 ] = x[ 2 ]; x[ 2 ] = sx;
        double sy = y[ 1 ]; y[ 1 ] = y[ 2 ]; y[ 2 ] = sy;
        area = -area;
    }

    double minX = x[ 0 ], maxX = x[ 0 ], minY = y[ 0 ], maxY = y[ 0 ];
    for ( UInt32 i=1; i<3; ++i )
    {
        if ( x[ i ] < minX ) minX = x[ i ];
        if ( x[ i ] > maxX ) maxX = x[ i ];
        if ( y[ i ] < minY ) minY = y[ i ];
        if ( y[ i ] > maxY ) maxY = y[ i ];
    }
    Int32 left = FirstCentreFrom( minX );
    Int32 right = FirstCentreFrom( maxX ) + 1;
    Int32 top = FirstCentreFrom( minY );
    Int32 bottom = FirstCentreFrom( maxY ) + 1;
    if ( left < scissor.left )
        left = scissor.left;
    if ( right > scissor.right )
        right = scissor.right;
    if ( top < scissor.top )
        top = scissor.top;
    if ( bottom > scissor.bottom )
        bottom = scissor.bottom;
    if ( left >= right || top >= bottom )
        return;

    const bool topLeft0 = IsTopLeft( x[ 1 ], y[ 1 ], x[ 2 ], y[ 2 ] );
    const bool topLeft1 = IsTopLeft( x[ 2 ], y[ 2 ], x[ 0 ], y[ 0 ] );
    const bool topLeft2 = IsTopLeft( x[ 0 ], y[ 0 ], x[ 1 ], y[ 1 ] );

    UInt32 colours[ 3 ][ 4 ];
    for ( UInt32 i=0; i<3; ++i )
        Unpack( p[ i ]->col, colours[ i ] );

    UInt8* data = static_cast< UInt8* >( target.GetDataPtr() );
    const UInt32 width = target.GetWidthInPixels();
    UInt32 texel[ 4 ], colour[ 4 ];
    for ( Int32 py=top; py<bottom; ++py )
    {
        const double cy = py + 0.5;
        UInt8* pixel = data + ( (size_t) py * width + left ) * 4;
        for ( Int32 px=left; px<right; ++px, pixel+=4 )
        {
            const double cx = px + 0.5;
            double w0 = Edge( x[ 1 ], y[ 1 ], x[ 2 ], y[ 2 ], cx, cy );
            double w1 = Edge( x[ 2 ], y[ 2 ], x[ 0 ], y[ 0 ], cx, cy );
            double w2 = Edge( x[ 0 ], y[ 0 ], x[ 1 ], y[ 1 ], cx, cy );
            if ( w0 < 0.0 || w1 < 0.0 || w2 < 0.0 )
                continue;
            if ( ( 0.0 == w0 && !topLeft0 ) || ( 0.0 == w1 && !topLeft1 ) || ( 0.0 == w2 && !topLeft2 ) )
                continue;
            const double l0 = w0 / area, l1 = w1 / area, l2 = w2 / area;
            const double u = l0 * p[ 0 ]->uv.x + l1 * p[ 1 ]->uv.x + l2 * p[ 2 ]->uv.x;
            const double v = l0 * p[ 0 ]->uv.y + l1 * p[ 1 ]->uv.y + l2 * p[ 2 ]->uv.y;
            for ( UInt32 c=0; c<4; ++c )
                colour[ c ] = (UInt32) ( l0 * colours[ 0 ][ c ] + l1 * colours[ 1 ][ c ] + l2 * colours[ 2 ][ c ] + 0.5 );
            if ( 0 == colour[ 3 ] )
                continue;
            Sample( texture, sampler, u * texture.width, v * texture.height, texel );
            Shade( pixel, texel, colour );
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiSoftwareRenderer::CImGuiSoftwareRenderer( void )
    : m_nextTextureId( 0 )
    , m_creates( 0 )
    , m_updates( 0 )
    , m_destroys( 0 )
    , m_unresolved( 0 )
    , m_triangles( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiSoftwareRenderer::~CImGuiSoftwareRenderer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CImGuiSoftwareRenderer::ServeTextureRequests( ImDrawData& drawData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == drawData.Textures )
        return;
    for ( int i=0; i<drawData.Textures->Size; ++i )
    {
        ImTextureData* texture = (*drawData.Textures)[ i ];
        if ( GUCEF_NULL == texture )
            continue;
        switch ( texture->Status )
        {
            case ImTextureStatus_WantCreate:
            {
                // the pixels stay where they are: the id only has to be unique and valid
                texture->SetTexID( (ImTextureID) ++m_nextTextureId );
                texture->SetStatus( ImTextureStatus_OK );
                ++m_creates;
                break;
            }
            case ImTextureStatus_WantUpdates:
            {
                texture->SetStatus( ImTextureStatus_OK );
                ++m_updates;
                break;
            }
            case ImTextureStatus_WantDestroy:
            {
                // as the GPU renderers: only a texture that was not used in the last frame
                if ( texture->UnusedFrames > 0 )
                {
                    texture->SetTexID( ImTextureID_Invalid );
                    texture->SetStatus( ImTextureStatus_Destroyed );
                    ++m_destroys;
                }
                break;
            }
            default:
                break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CImGuiSoftwareRenderer::DestroyTextures( ImVector< ImTextureData* >& textures )
{GUCEF_TRACE;

    for ( int i=0; i<textures.Size; ++i )
    {
        ImTextureData* texture = textures[ i ];
        if ( GUCEF_NULL == texture || ImTextureID_Invalid == texture->TexID || texture->RefCount > 1 )
            continue;
        texture->SetTexID( ImTextureID_Invalid );
        texture->SetStatus( ImTextureStatus_Destroyed );
        ++m_destroys;
    }
}

/*-------------------------------------------------------------------------*/

bool
CImGuiSoftwareRenderer::Render( const ImDrawData& drawData        ,
                                const ImGuiPlatformIO& platformIO ,
                                UInt32 width                      ,
                                UInt32 height                     ,
                                bool clear                        ,
                                IMAGE::TPixelMapPtr& target       )
{GUCEF_TRACE;

    if ( 0 == width || 0 == height )
        return false;
    if ( target.IsNULL() || target->GetWidthInPixels() != width || target->GetHeightInPixels() != height ||
         IMAGE::PSF_RGBA != target->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != target->GetPixelComponentDataType() )
    {
        target = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        clear = true;
    }
    if ( clear )
    {
        UInt8* data = static_cast< UInt8* >( target->GetDataPtr() );
        for ( UInt32 i=0; i<width*height; ++i )
        {
            data[ i*4 ] = data[ i*4+1 ] = data[ i*4+2 ] = 0;
            data[ i*4+3 ] = 255;
        }
    }
    return Render( drawData, platformIO, *target );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiSoftwareRenderer::Render( const ImDrawData& drawData        ,
                                const ImGuiPlatformIO& platformIO ,
                                IMAGE::CPixelMap& target          )
{GUCEF_TRACE;

    m_unresolved = 0;
    m_triangles = 0;
    if ( IMAGE::PSF_RGBA != target.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != target.GetPixelComponentDataType() )
        return false;
    if ( !drawData.Valid )
        return false;

    const Int32 targetWidth = (Int32) target.GetWidthInPixels();
    const Int32 targetHeight = (Int32) target.GetHeightInPixels();
    const ImVec2 offset = drawData.DisplayPos;
    const ImVec2 scale = drawData.FramebufferScale;

    SSampler sampler;
    sampler.nearest = false;
    for ( int n=0; n<drawData.CmdLists.Size; ++n )
    {
        const ImDrawList* list = drawData.CmdLists[ n ];
        const ImDrawVert* vertices = list->VtxBuffer.Data;
        const ImDrawIdx* indices = list->IdxBuffer.Data;
        for ( int c=0; c<list->CmdBuffer.Size; ++c )
        {
            const ImDrawCmd& command = list->CmdBuffer[ c ];
            if ( GUCEF_NULL != command.UserCallback )
            {
                // the standard callbacks; callbacks of an application are for a GPU renderer and are not run here
                if ( command.UserCallback == platformIO.DrawCallback_SetSamplerNearest )
                    sampler.nearest = true;
                else
                if ( command.UserCallback == platformIO.DrawCallback_SetSamplerLinear || command.UserCallback == platformIO.DrawCallback_ResetRenderState )
                    sampler.nearest = false;
                continue;
            }
            if ( 0 == command.ElemCount )
                continue;

            // the scissor as the GL renderer gives it to glScissor()
            const float minX = ( command.ClipRect.x - offset.x ) * scale.x;
            const float minY = ( command.ClipRect.y - offset.y ) * scale.y;
            const float maxX = ( command.ClipRect.z - offset.x ) * scale.x;
            const float maxY = ( command.ClipRect.w - offset.y ) * scale.y;
            if ( maxX <= minX || maxY <= minY )
                continue;
            SPixelRect scissor;
            scissor.left = (Int32) minX;
            scissor.top = (Int32) minY;
            scissor.right = scissor.left + (Int32) ( maxX - minX );
            scissor.bottom = scissor.top + (Int32) ( maxY - minY );
            if ( scissor.left < 0 )
                scissor.left = 0;
            if ( scissor.top < 0 )
                scissor.top = 0;
            if ( scissor.right > targetWidth )
                scissor.right = targetWidth;
            if ( scissor.bottom > targetHeight )
                scissor.bottom = targetHeight;
            if ( scissor.left >= scissor.right || scissor.top >= scissor.bottom )
                continue;

            STexture texture;
            if ( !ResolveTexture( command, drawData, texture ) )
            {
                ++m_unresolved;
                continue;
            }

            const ImDrawVert* base = vertices + command.VtxOffset;
            const ImDrawIdx* index = indices + command.IdxOffset;
            const UInt32 count = command.ElemCount;
            UInt32 i = 0;
            while ( i + 3 <= count )
            {
                if ( i + 6 <= count && IsAxisAlignedQuad( base, index + i ) )
                {
                    const ImDrawVert& a = base[ index[ i ] ];
                    const ImDrawVert& cc = base[ index[ i + 2 ] ];
                    DrawQuad( target, scissor, texture, sampler,
                              ( a.pos.x - offset.x ) * scale.x, ( a.pos.y - offset.y ) * scale.y,
                              ( cc.pos.x - offset.x ) * scale.x, ( cc.pos.y - offset.y ) * scale.y,
                              a.uv.x, a.uv.y, cc.uv.x, cc.uv.y, a.col );
                    m_triangles += 2;
                    i += 6;
                    continue;
                }
                DrawTriangle( target, scissor, texture, sampler, base[ index[ i ] ], base[ index[ i + 1 ] ], base[ index[ i + 2 ] ], offset, scale );
                ++m_triangles;
                i += 3;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiSoftwareRenderer::GetTextureCreateCount( void ) const
{GUCEF_TRACE;

    return m_creates;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiSoftwareRenderer::GetTextureUpdateCount( void ) const
{GUCEF_TRACE;

    return m_updates;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiSoftwareRenderer::GetTextureDestroyCount( void ) const
{GUCEF_TRACE;

    return m_destroys;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiSoftwareRenderer::GetUnresolvedTextureCount( void ) const
{GUCEF_TRACE;

    return m_unresolved;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiSoftwareRenderer::GetTriangleCount( void ) const
{GUCEF_TRACE;

    return m_triangles;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

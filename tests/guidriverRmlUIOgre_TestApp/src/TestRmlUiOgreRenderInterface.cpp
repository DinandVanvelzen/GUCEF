/*
 *  guidriverRmlUIOgre_TestApp: Test application for the GUI driver RmlUiOgre (guidriverRmlUIOgre)
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

#include <RmlUi/Core/Vertex.h>
#include <RmlUi/Core/Matrix4.h>
#include <RmlUi/Core/Rectangle.h>
#include <RmlUi/Core/Span.h>

#include <Ogre.h>
#include <OgreCodec.h>
#include <OgreHardwarePixelBuffer.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#include "guidriverOgre_CIOgreWindowOverlay.h"
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */

#include "guidriverRmlUIOgre_CRmlUIOgreRenderInterface.h"

#include "TestOgreSupport.h"
#include "TestRmlUiOgreRenderInterface.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< Rml::Vertex > TVertexVector;
typedef GUCEF::vector< int > TIndexVector;

/** Adds a quad of one premultiplied colour, texture coordinates over the whole texture */
void
AddQuad( TVertexVector& vertices, TIndexVector& indices, float x, float y, float width, float height, Rml::ColourbPremultiplied colour )
{
    const int first = (int) vertices.size();
    const float xs[ 4 ] = { x, x + width, x + width, x };
    const float ys[ 4 ] = { y, y, y + height, y + height };
    const float us[ 4 ] = { 0.0f, 1.0f, 1.0f, 0.0f };
    const float vs[ 4 ] = { 0.0f, 0.0f, 1.0f, 1.0f };
    for ( int i=0; i<4; ++i )
    {
        Rml::Vertex vertex;
        vertex.position = Rml::Vector2f( xs[ i ], ys[ i ] );
        vertex.colour = colour;
        vertex.tex_coord = Rml::Vector2f( us[ i ], vs[ i ] );
        vertices.push_back( vertex );
    }
    const int quad[ 6 ] = { 0, 1, 2, 0, 2, 3 };
    for ( int i=0; i<6; ++i )
        indices.push_back( first + quad[ i ] );
}

Rml::CompiledGeometryHandle
Compile( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& renderer, const TVertexVector& vertices, const TIndexVector& indices )
{
    return renderer.CompileGeometry( Rml::Span< const Rml::Vertex >( vertices.empty() ? GUCEF_NULL : &vertices[ 0 ], vertices.size() ),
                                     Rml::Span< const int >( indices.empty() ? GUCEF_NULL : &indices[ 0 ], indices.size() ) );
}

/** What a test draws through the render interface into a capture */
class CDrawing
{
    public:
    virtual ~CDrawing() {}
    virtual void Draw( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& renderer ) = 0;
};

/** Lets the render interface draw into a capture of the window */
class CRendererOverlay : public GUIDRIVEROGRE::CIOgreWindowOverlay
{
    public:

    CRendererOverlay( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& renderer, CDrawing& drawing )
        : m_renderer( renderer ), m_drawing( drawing ), drawCalls( 0 ) {}

    virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }

    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem, Ogre::Viewport& viewport, Ogre::SceneManager* sceneManager, bool isCapture ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_renderer.BeginFrame( renderSystem, viewport, 0.0f, 0.0f, 1.0f, 1.0f );
        m_drawing.Draw( m_renderer );
        drawCalls = m_renderer.GetDrawCallCount();
        m_renderer.EndFrame();
    }

    GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& m_renderer;
    CDrawing& m_drawing;
    UInt32 drawCalls;
};

/** Draws the geometry as it is */
class CDrawGeometry : public CDrawing
{
    public:
    CDrawGeometry( Rml::CompiledGeometryHandle geometry, Rml::TextureHandle texture = 0 ) : m_geometry( geometry ), m_texture( texture ), translation( 0.0f, 0.0f ) {}
    virtual void Draw( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& renderer ) GUCEF_VIRTUAL_OVERRIDE
    {
        renderer.RenderGeometry( m_geometry, translation, m_texture );
    }
    Rml::CompiledGeometryHandle m_geometry;
    Rml::TextureHandle m_texture;
    Rml::Vector2f translation;
};

bool
Capture( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& renderer, CDrawing& drawing, IMAGE::TPixelMapPtr& pixels, UInt32& drawCalls )
{
    GUIDRIVEROGRE::COgreWindowContext* window = AsOgreWindow( GetMainTestWindow() );
    if ( GUCEF_NULL == window )
        return false;
    CRendererOverlay overlay( renderer, drawing );
    const bool captured = window->CaptureOverlay( overlay, 64, 48, true, pixels );
    drawCalls = overlay.drawCalls;
    return captured;
}

bool
TextureExists( const Ogre::TexturePtr& texture )
{
    return texture && Ogre::TextureManager::getSingleton().resourceExists( texture->getName(), texture->getGroup() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRmlUiOgreRenderInterfaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverRmlUIOgre.RenderInterface" );

    GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface renderer;
    bool ready = false;
    GUCEF_TESTFW_TESTCASE( "Render interface: it starts with the GUI material of guidriverOgre" )
    try
    {
        ready = renderer.Initialize();
        ASSERT_TRUE( ready );
        ASSERT_TRUE( renderer.IsInitialized() );
        ASSERT_FALSE( renderer.IsDrawing() );
        ASSERT_TRUE( 0 == renderer.GetGeometryCount() && 0 == renderer.GetTextureCount() );
        ASSERT_TRUE( GUIDRIVEROGRE::COgreGuiMaterial::GetWhiteTexture() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    if ( !ready )
        return;

    GUCEF_TESTFW_TESTCASE( "Geometry: compiled into hardware buffers with all vertices and indices, released again" )
    try
    {
        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 0.0f, 0.0f, 10.0f, 10.0f, Rml::ColourbPremultiplied( 255, 0, 0, 255 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );
        ASSERT_TRUE( 0 != geometry && 1 == renderer.GetGeometryCount() );
        UInt32 vertexCount = 0, indexCount = 0;
        ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface::GetGeometryCounts( geometry, vertexCount, indexCount ) );
        ASSERT_TRUE( 4 == vertexCount && 6 == indexCount );

        // empty geometry is a handle that draws nothing
        TVertexVector noVertices;
        TIndexVector noIndices;
        Rml::CompiledGeometryHandle empty = Compile( renderer, noVertices, noIndices );
        ASSERT_TRUE( 0 != empty && 2 == renderer.GetGeometryCount() );
        ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface::GetGeometryCounts( empty, vertexCount, indexCount ) );
        ASSERT_TRUE( 0 == vertexCount && 0 == indexCount );
        CDrawGeometry drawEmpty( empty );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 99;
        ASSERT_TRUE( Capture( renderer, drawEmpty, pixels, drawCalls ) );
        ASSERT_TRUE( 0 == drawCalls );

        renderer.ReleaseGeometry( empty );
        renderer.ReleaseGeometry( geometry );
        ASSERT_TRUE( 0 == renderer.GetGeometryCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Geometry: one compiled geometry of several quads is one draw call, translated by the translation of the call" )
    try
    {
        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 2.0f, 2.0f, 8.0f, 8.0f, Rml::ColourbPremultiplied( 255, 0, 0, 255 ) );
        AddQuad( vertices, indices, 20.0f, 10.0f, 6.0f, 4.0f, Rml::ColourbPremultiplied( 0, 0, 255, 255 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );
        CDrawGeometry draw( geometry );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        ASSERT_TRUE( 1 == drawCalls );
        ASSERT_TRUE( PixelIs( pixels, 2, 2, 255, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 9, 9, 255, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 10, 10, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 22, 12, 0, 0, 255 ) );

        draw.translation = Rml::Vector2f( 30.0f, 20.0f );
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 35, 25, 255, 0, 0 ) );
        renderer.ReleaseGeometry( geometry );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Blending: premultiplied colours are blended with ONE, ONE_MINUS_SOURCE_ALPHA" )
    try
    {
        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 0.0f, 0.0f, 64.0f, 48.0f, Rml::ColourbPremultiplied( 0, 0, 200, 255 ) );
        AddQuad( vertices, indices, 10.0f, 10.0f, 10.0f, 10.0f, Rml::ColourbPremultiplied( 100, 0, 0, 128 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );
        CDrawGeometry draw( geometry );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        // 100 + 0 * 127/255, 0, 0 + 200 * 127/255
        ASSERT_TRUE( PixelIs( pixels, 15, 15, 100, 0, 100 ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 0, 0, 200 ) );
        renderer.ReleaseGeometry( geometry );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Scissor: the region cuts the drawing off, and maps onto the target with the offset and scale of the frame" )
    try
    {
        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 0.0f, 0.0f, 64.0f, 48.0f, Rml::ColourbPremultiplied( 0, 255, 0, 255 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );

        class CScissored : public CDrawing
        {
            public:
            explicit CScissored( Rml::CompiledGeometryHandle g ) : m_geometry( g ) {}
            virtual void Draw( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& r ) GUCEF_VIRTUAL_OVERRIDE
            {
                r.SetScissorRegion( Rml::Rectanglei::FromPositionSize( Rml::Vector2i( 5, 6 ), Rml::Vector2i( 10, 8 ) ) );
                r.EnableScissorRegion( true );
                r.RenderGeometry( m_geometry, Rml::Vector2f( 0.0f, 0.0f ), 0 );
                r.EnableScissorRegion( false );
            }
            Rml::CompiledGeometryHandle m_geometry;
        };
        CScissored draw( geometry );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 6, 0, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 14, 13, 0, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 4, 6, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 15, 13, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 14, 14, 0, 0, 0 ) );
        renderer.ReleaseGeometry( geometry );

        // the region in pixels of the target for a frame drawn at an offset and twice as large
        GUIDRIVEROGRE::COgreWindowContext* window = AsOgreWindow( GetMainTestWindow() );
        Ogre::Viewport* viewport = window->GetOgreRenderWindow()->getNumViewports() > 0 ? window->GetOgreRenderWindow()->getViewport( 0 ) : GUCEF_NULL;
        class CMapping : public GUIDRIVEROGRE::CIOgreWindowOverlay
        {
            public:
            explicit CMapping( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& r ) : m_r( r ), left( -1 ), top( -1 ), right( -1 ), bottom( -1 ), enabled( false ) {}
            virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
            virtual void RenderOverlay( Ogre::RenderSystem& rs, Ogre::Viewport& vp, Ogre::SceneManager*, bool ) GUCEF_VIRTUAL_OVERRIDE
            {
                m_r.BeginFrame( rs, vp, 10.0f, 20.0f, 2.0f, 2.0f );
                m_r.SetScissorRegion( Rml::Rectanglei::FromCorners( Rml::Vector2i( 1, 2 ), Rml::Vector2i( 3, 4 ) ) );
                m_r.EnableScissorRegion( true );
                enabled = m_r.GetTargetScissor( left, top, right, bottom );
                m_r.EndFrame();
            }
            GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& m_r;
            Int32 left, top, right, bottom;
            bool enabled;
        };
        CMapping mapping( renderer );
        ASSERT_TRUE( window->CaptureOverlay( mapping, 64, 48, true, pixels ) );
        ASSERT_TRUE( mapping.enabled );
        ASSERT_TRUE( 12 == mapping.left && 24 == mapping.top && 16 == mapping.right && 28 == mapping.bottom );
        (void) viewport;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Transform: the matrix of RmlUi is applied before the projection, and none means identity" )
    try
    {
        const Rml::Matrix4f translate = Rml::Matrix4f::Translate( 20.0f, 10.0f, 0.0f );
        renderer.SetTransform( &translate );
        ASSERT_TRUE( 20.0f == renderer.GetTransform()[ 0 ][ 3 ] && 10.0f == renderer.GetTransform()[ 1 ][ 3 ] );
        ASSERT_TRUE( 1.0f == renderer.GetTransform()[ 0 ][ 0 ] && 0.0f == renderer.GetTransform()[ 3 ][ 0 ] );
        renderer.SetTransform( GUCEF_NULL );
        ASSERT_TRUE( Ogre::Matrix4::IDENTITY == renderer.GetTransform() );

        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 0.0f, 0.0f, 4.0f, 4.0f, Rml::ColourbPremultiplied( 255, 255, 0, 255 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );
        class CTransformed : public CDrawing
        {
            public:
            explicit CTransformed( Rml::CompiledGeometryHandle g ) : m_geometry( g ) {}
            virtual void Draw( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& r ) GUCEF_VIRTUAL_OVERRIDE
            {
                const Rml::Matrix4f transform = Rml::Matrix4f::Translate( 20.0f, 10.0f, 0.0f ) * Rml::Matrix4f::Scale( 2.0f, 2.0f, 1.0f );
                r.SetTransform( &transform );
                r.RenderGeometry( m_geometry, Rml::Vector2f( 1.0f, 1.0f ), 0 );
                r.SetTransform( GUCEF_NULL );
            }
            Rml::CompiledGeometryHandle m_geometry;
        };
        CTransformed draw( geometry );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        // (0..4 + 1) * 2 + (20, 10): x 22..30, y 12..20
        ASSERT_TRUE( PixelIs( pixels, 22, 12, 255, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 29, 19, 255, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 21, 12, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 30, 19, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 2, 2, 0, 0, 0 ) );
        renderer.ReleaseGeometry( geometry );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Clip mask: drawing is limited to the mask, its inverse, and the intersection of two masks" )
    try
    {
        TVertexVector maskVertices, maskVertices2, fillVertices;
        TIndexVector maskIndices, maskIndices2, fillIndices;
        AddQuad( maskVertices, maskIndices, 10.0f, 10.0f, 20.0f, 20.0f, Rml::ColourbPremultiplied( 255, 255, 255, 255 ) );
        AddQuad( maskVertices2, maskIndices2, 20.0f, 20.0f, 20.0f, 20.0f, Rml::ColourbPremultiplied( 255, 255, 255, 255 ) );
        AddQuad( fillVertices, fillIndices, 0.0f, 0.0f, 64.0f, 48.0f, Rml::ColourbPremultiplied( 255, 0, 255, 255 ) );
        Rml::CompiledGeometryHandle mask = Compile( renderer, maskVertices, maskIndices );
        Rml::CompiledGeometryHandle mask2 = Compile( renderer, maskVertices2, maskIndices2 );
        Rml::CompiledGeometryHandle fill = Compile( renderer, fillVertices, fillIndices );

        class CMasked : public CDrawing
        {
            public:
            CMasked( Rml::CompiledGeometryHandle m, Rml::CompiledGeometryHandle m2, Rml::CompiledGeometryHandle f, Rml::ClipMaskOperation op, bool intersect )
                : m_mask( m ), m_mask2( m2 ), m_fill( f ), m_op( op ), m_intersect( intersect ) {}
            virtual void Draw( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface& r ) GUCEF_VIRTUAL_OVERRIDE
            {
                r.EnableClipMask( true );
                r.RenderToClipMask( m_op, m_mask, Rml::Vector2f( 0.0f, 0.0f ) );
                if ( m_intersect )
                    r.RenderToClipMask( Rml::ClipMaskOperation::Intersect, m_mask2, Rml::Vector2f( 0.0f, 0.0f ) );
                r.RenderGeometry( m_fill, Rml::Vector2f( 0.0f, 0.0f ), 0 );
                r.EnableClipMask( false );
            }
            Rml::CompiledGeometryHandle m_mask, m_mask2, m_fill;
            Rml::ClipMaskOperation m_op;
            bool m_intersect;
        };

        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        CMasked set( mask, mask2, fill, Rml::ClipMaskOperation::Set, false );
        ASSERT_TRUE( Capture( renderer, set, pixels, drawCalls ) );
        ASSERT_TRUE( 1 == drawCalls );
        ASSERT_TRUE( PixelIs( pixels, 15, 15, 255, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 35, 35, 0, 0, 0 ) );

        CMasked inverse( mask, mask2, fill, Rml::ClipMaskOperation::SetInverse, false );
        ASSERT_TRUE( Capture( renderer, inverse, pixels, drawCalls ) );
        ASSERT_TRUE( PixelIs( pixels, 15, 15, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 5, 5, 255, 0, 255 ) );

        CMasked intersect( mask, mask2, fill, Rml::ClipMaskOperation::Set, true );
        ASSERT_TRUE( Capture( renderer, intersect, pixels, drawCalls ) );
        ASSERT_TRUE( PixelIs( pixels, 25, 25, 255, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 15, 15, 0, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 35, 35, 0, 0, 0 ) );

        renderer.ReleaseGeometry( mask );
        renderer.ReleaseGeometry( mask2 );
        renderer.ReleaseGeometry( fill );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: generated from premultiplied pixels into an Ogre texture, drawn with point sampling, removed on release" )
    try
    {
        // 2 x 2: red, green / blue, half transparent white (premultiplied)
        const Rml::byte data[ 16 ] = { 255, 0, 0, 255,   0, 255, 0, 255,
                                       0, 0, 255, 255,   128, 128, 128, 128 };
        Rml::TextureHandle texture = renderer.GenerateTexture( Rml::Span< const Rml::byte >( data, 16 ), Rml::Vector2i( 2, 2 ) );
        ASSERT_TRUE( 0 != texture && 1 == renderer.GetTextureCount() );
        Ogre::TexturePtr ogreTexture = GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface::GetOgreTexture( texture );
        ASSERT_TRUE( TextureExists( ogreTexture ) );
        ASSERT_TRUE( 2 == ogreTexture->getWidth() && 2 == ogreTexture->getHeight() );
        Ogre::uint8 readBack[ 16 ];
        ogreTexture->getBuffer()->blitToMemory( Ogre::PixelBox( 2, 2, 1, Ogre::PF_BYTE_RGBA, readBack ) );
        ASSERT_TRUE( 0 == memcmp( readBack, data, 16 ) );

        // a quad of 20 x 20 shows each texel as a block of 10 x 10 (point sampling, as the OpenGL renderer)
        TVertexVector vertices;
        TIndexVector indices;
        AddQuad( vertices, indices, 0.0f, 0.0f, 20.0f, 20.0f, Rml::ColourbPremultiplied( 255, 255, 255, 255 ) );
        Rml::CompiledGeometryHandle geometry = Compile( renderer, vertices, indices );
        CDrawGeometry draw( geometry, texture );
        IMAGE::TPixelMapPtr pixels;
        UInt32 drawCalls = 0;
        ASSERT_TRUE( Capture( renderer, draw, pixels, drawCalls ) );
        ASSERT_TRUE( PixelIs( pixels, 9, 9, 255, 0, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 10, 9, 0, 255, 0 ) );
        ASSERT_TRUE( PixelIs( pixels, 9, 10, 0, 0, 255 ) );
        ASSERT_TRUE( PixelIs( pixels, 15, 15, 128, 128, 128 ) );
        renderer.ReleaseGeometry( geometry );

        renderer.ReleaseTexture( texture );
        ASSERT_TRUE( 0 == renderer.GetTextureCount() );
        ASSERT_FALSE( TextureExists( ogreTexture ) );

        // a texture of the application: no pixels are copied and the texture stays after the release
        Ogre::TexturePtr own = MakeSolidTexture( "RmlUiOgreTest/Own", 8, 8, 0, 200, 0, 255 );
        renderer.SetNextGeneratedTexture( own );
        Rml::TextureHandle wrapped = renderer.GenerateTexture( Rml::Span< const Rml::byte >(), Rml::Vector2i( 1, 1 ) );
        ASSERT_TRUE( 0 != wrapped && GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface::GetOgreTexture( wrapped ) == own );
        renderer.ReleaseTexture( wrapped );
        ASSERT_TRUE( TextureExists( own ) );
        Ogre::TextureManager::getSingleton().remove( own );

        // bad input gives no texture
        ASSERT_TRUE( 0 == renderer.GenerateTexture( Rml::Span< const Rml::byte >( data, 4 ), Rml::Vector2i( 2, 2 ) ) );
        ASSERT_TRUE( 0 == renderer.GetTextureCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: an image file read through the GucefVFS archive and the codecs of Ogre, premultiplied" )
    if ( !Ogre::ArchiveManager::getSingleton().hasArchiveFactory( "GucefVFS" ) || !Ogre::Codec::isCodecRegistered( "ogwtex" ) )
        GUCEF_TESTFW_SKIP( "OgrePlugin_GucefVFS or OgreCodec_GucefIMAGE is not loaded" );
    try
    {
        const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "guidriverRmlUIOgre_TestApp_Textures" );
        CORE::DeleteDir( dir, true );
        ASSERT_TRUE( CORE::CreateDirs( dir ) );
        {
            CORE::CFileAccess file( CORE::CombinePath( dir, "red.ogwtex" ), "wb" );
            const UInt8 bytes[ 8 ] = { 'O', 'G', 'W', '1', 200, 100, 0, 128 };
            ASSERT_TRUE( file.IsValid() && 1 == file.Write( bytes, 8, 1 ) );
        }
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( dir, "guidriverRmlUIOgreTestTextures", false, false, "/rmlogretest" ) );

        Rml::Vector2i dimensions( 0, 0 );
        Rml::TextureHandle texture = renderer.LoadTexture( dimensions, "/rmlogretest/red.ogwtex" );
        ASSERT_TRUE( 0 != texture );
        ASSERT_TRUE( 4 == dimensions.x && 4 == dimensions.y );
        Ogre::TexturePtr ogreTexture = GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface::GetOgreTexture( texture );
        if ( ogreTexture )
        {
            Ogre::uint8 readBack[ 64 ];
            ogreTexture->getBuffer()->blitToMemory( Ogre::PixelBox( 4, 4, 1, Ogre::PF_BYTE_RGBA, readBack ) );
            // 200 * 128 / 255, 100 * 128 / 255, 0, 128
            ASSERT_TRUE( 100 == readBack[ 0 ] && 50 == readBack[ 1 ] && 0 == readBack[ 2 ] && 128 == readBack[ 3 ] );
        }
        renderer.ReleaseTexture( texture );

        Rml::Vector2i missing( 0, 0 );
        ASSERT_TRUE( 0 == renderer.LoadTexture( missing, "/rmlogretest/missing.ogwtex" ) );
        vfs.UnmountArchiveByName( "guidriverRmlUIOgreTestTextures" );
        CORE::DeleteDir( dir, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

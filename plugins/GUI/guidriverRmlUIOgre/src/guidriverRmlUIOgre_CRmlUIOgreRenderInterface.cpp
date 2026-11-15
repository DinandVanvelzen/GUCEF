/*
 *  guidriverRmlUIOgre: gucefGUI driver drawing RmlUi through Ogre, over the 3D scene of an Ogre window
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

#include <stddef.h>
#include <math.h>

#include <RmlUi/Core/Vertex.h>
#include <RmlUi/Core/Matrix4.h>

#include <Ogre.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwarePixelBuffer.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverRmlUIOgre_CRmlUIOgreRenderInterface.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Compiled geometry: the buffers and the operation that draws them */
struct SGeometry
{
    Ogre::VertexData* vertexData;
    Ogre::IndexData* indexData;
    Ogre::RenderOperation operation;
};

/** A texture handle: an Ogre texture, owned by the render interface or by the application */
struct STexture
{
    Ogre::TexturePtr texture;
    bool owned;
};

/** The archive of OgrePlugin_GucefVFS over a directory of the VFS; Ogre keeps it for the next image of the directory */
Ogre::Archive*
GetVfsArchive( const Ogre::String& directory )
{
    Ogre::ArchiveManager& archives = Ogre::ArchiveManager::getSingleton();
    if ( !archives.hasArchiveFactory( "GucefVFS" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: Images are read through the VFS archive of the Ogre plugin OgrePlugin_GucefVFS, which is not loaded" );
        return GUCEF_NULL;
    }
    return archives.load( directory, "GucefVFS", true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIOgreRenderInterface::CRmlUIOgreRenderInterface( void )
    : Rml::RenderInterface()
    , m_material( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_PREMULTIPLIED )
    , m_renderSystem( GUCEF_NULL )
    , m_viewport( GUCEF_NULL )
    , m_offsetX( 0.0f )
    , m_offsetY( 0.0f )
    , m_scaleX( 1.0f )
    , m_scaleY( 1.0f )
    , m_scissorEnabled( false )
    , m_scissorRegion()
    , m_clipMaskEnabled( false )
    , m_stencilTestValue( 1 )
    , m_transform( Ogre::Matrix4::IDENTITY )
    , m_nextTexture()
    , m_geometryCount( 0 )
    , m_textureCount( 0 )
    , m_drawCalls( 0 )
    , m_textureCounter( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRmlUIOgreRenderInterface::~CRmlUIOgreRenderInterface()
{GUCEF_TRACE;

    if ( 0 != m_geometryCount || 0 != m_textureCount )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: Destroyed while RmlUi still has " << m_geometryCount
            << " geometries and " << m_textureCount << " textures of it" );
    }
    m_material.Release();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderInterface::Initialize( void )
{GUCEF_TRACE;

    return m_material.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderInterface::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_material.IsInitialized();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::BeginFrame( Ogre::RenderSystem& renderSystem ,
                                       Ogre::Viewport& viewport         ,
                                       float offsetX                    ,
                                       float offsetY                    ,
                                       float scaleX                     ,
                                       float scaleY                     )
{GUCEF_TRACE;

    m_renderSystem = &renderSystem;
    m_viewport = &viewport;
    m_offsetX = offsetX;
    m_offsetY = offsetY;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_scissorEnabled = false;
    m_clipMaskEnabled = false;
    m_stencilTestValue = 1;
    m_transform = Ogre::Matrix4::IDENTITY;
    m_drawCalls = 0;

    const bool flip = viewport.getTarget()->requiresTextureFlipping();
    const Ogre::Matrix4 projection = GUIDRIVEROGRE::COgreGuiMaterial::MakeProjection( offsetX, offsetY, scaleX, scaleY,
                                                                                      (UInt32) viewport.getActualWidth(),
                                                                                      (UInt32) viewport.getActualHeight(), flip );
    m_material.BeginDrawing( renderSystem, projection );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::EndFrame( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_renderSystem )
    {
        m_renderSystem->setScissorTest( false );
        m_renderSystem->setStencilState( Ogre::StencilState() );
        m_material.SetColourWriteEnabled( *m_renderSystem, true );
    }
    m_renderSystem = GUCEF_NULL;
    m_viewport = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderInterface::IsDrawing( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_renderSystem;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::SetNextGeneratedTexture( const Ogre::TexturePtr& texture )
{GUCEF_TRACE;

    m_nextTexture = texture;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CRmlUIOgreRenderInterface::GetOgreTexture( Rml::TextureHandle texture )
{GUCEF_TRACE;

    if ( 0 == texture )
        return Ogre::TexturePtr();
    return reinterpret_cast< STexture* >( texture )->texture;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIOgreRenderInterface::GetGeometryCount( void ) const
{GUCEF_TRACE;

    return m_geometryCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIOgreRenderInterface::GetTextureCount( void ) const
{GUCEF_TRACE;

    return m_textureCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIOgreRenderInterface::GetDrawCallCount( void ) const
{GUCEF_TRACE;

    return m_drawCalls;
}

/*-------------------------------------------------------------------------*/

const Ogre::Matrix4&
CRmlUIOgreRenderInterface::GetTransform( void ) const
{GUCEF_TRACE;

    return m_transform;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderInterface::GetTargetScissor( Int32& left, Int32& top, Int32& right, Int32& bottom ) const
{GUCEF_TRACE;

    // the region in pixels of the target, out to the pixels the scaled region touches
    left = (Int32) floorf( m_offsetX + m_scissorRegion.Left() * m_scaleX );
    top = (Int32) floorf( m_offsetY + m_scissorRegion.Top() * m_scaleY );
    right = (Int32) ceilf( m_offsetX + m_scissorRegion.Right() * m_scaleX );
    bottom = (Int32) ceilf( m_offsetY + m_scissorRegion.Bottom() * m_scaleY );
    if ( GUCEF_NULL != m_viewport )
    {
        const Int32 width = (Int32) m_viewport->getActualWidth();
        const Int32 height = (Int32) m_viewport->getActualHeight();
        left = left < 0 ? 0 : ( left > width ? width : left );
        right = right < left ? left : ( right > width ? width : right );
        top = top < 0 ? 0 : ( top > height ? height : top );
        bottom = bottom < top ? top : ( bottom > height ? height : bottom );
    }
    return m_scissorEnabled;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderInterface::GetGeometryCounts( Rml::CompiledGeometryHandle geometry, UInt32& vertexCount, UInt32& indexCount )
{GUCEF_TRACE;

    if ( 0 == geometry )
        return false;
    const SGeometry* compiled = reinterpret_cast< const SGeometry* >( geometry );
    vertexCount = GUCEF_NULL != compiled->vertexData ? (UInt32) compiled->vertexData->vertexCount : 0;
    indexCount = GUCEF_NULL != compiled->indexData ? (UInt32) compiled->indexData->indexCount : 0;
    return true;
}

/*-------------------------------------------------------------------------*/

Rml::CompiledGeometryHandle
CRmlUIOgreRenderInterface::CompileGeometry( Rml::Span< const Rml::Vertex > vertices, Rml::Span< const int > indices )
{GUCEF_TRACE;

    SGeometry* geometry = GUCEF_NEW SGeometry();
    geometry->vertexData = GUCEF_NULL;
    geometry->indexData = GUCEF_NULL;
    ++m_geometryCount;
    if ( vertices.empty() || indices.empty() )
        return reinterpret_cast< Rml::CompiledGeometryHandle >( geometry );

    try
    {
        Ogre::HardwareBufferManager& buffers = Ogre::HardwareBufferManager::getSingleton();

        geometry->vertexData = OGRE_NEW Ogre::VertexData();
        geometry->vertexData->vertexStart = 0;
        geometry->vertexData->vertexCount = vertices.size();
        Ogre::VertexDeclaration* declaration = geometry->vertexData->vertexDeclaration;
        declaration->addElement( 0, offsetof( Rml::Vertex, position ), Ogre::VET_FLOAT2, Ogre::VES_POSITION );
        declaration->addElement( 0, offsetof( Rml::Vertex, colour ), Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE );
        declaration->addElement( 0, offsetof( Rml::Vertex, tex_coord ), Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0 );
        Ogre::HardwareVertexBufferSharedPtr vertexBuffer = buffers.createVertexBuffer( sizeof( Rml::Vertex ), vertices.size(), Ogre::HBU_GPU_ONLY );
        vertexBuffer->writeData( 0, vertices.size() * sizeof( Rml::Vertex ), vertices.data(), true );
        geometry->vertexData->vertexBufferBinding->setBinding( 0, vertexBuffer );

        geometry->indexData = OGRE_NEW Ogre::IndexData();
        geometry->indexData->indexStart = 0;
        geometry->indexData->indexCount = indices.size();
        geometry->indexData->indexBuffer = buffers.createIndexBuffer( Ogre::HardwareIndexBuffer::IT_32BIT, indices.size(), Ogre::HBU_GPU_ONLY );
        geometry->indexData->indexBuffer->writeData( 0, indices.size() * sizeof( int ), indices.data(), true );

        geometry->operation.vertexData = geometry->vertexData;
        geometry->operation.indexData = geometry->indexData;
        geometry->operation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        geometry->operation.useIndexes = true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: Geometry could not be compiled: " << e.getFullDescription() );
        OGRE_DELETE geometry->vertexData;
        OGRE_DELETE geometry->indexData;
        geometry->vertexData = GUCEF_NULL;
        geometry->indexData = GUCEF_NULL;
    }
    return reinterpret_cast< Rml::CompiledGeometryHandle >( geometry );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::RenderGeometry( Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderSystem || 0 == geometry )
        return;
    const SGeometry* compiled = reinterpret_cast< const SGeometry* >( geometry );
    if ( GUCEF_NULL == compiled->vertexData || GUCEF_NULL == compiled->indexData )
        return;
    m_material.SetTranslation( translation.x, translation.y );
    m_material.SetTexture( *m_renderSystem, GetOgreTexture( texture ) );
    m_material.Render( *m_renderSystem, compiled->operation );
    ++m_drawCalls;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::ReleaseGeometry( Rml::CompiledGeometryHandle geometry )
{GUCEF_TRACE;

    if ( 0 == geometry )
        return;
    SGeometry* compiled = reinterpret_cast< SGeometry* >( geometry );
    OGRE_DELETE compiled->vertexData;
    OGRE_DELETE compiled->indexData;
    GUCEF_DELETE compiled;
    if ( m_geometryCount > 0 )
        --m_geometryCount;
}

/*-------------------------------------------------------------------------*/

Rml::TextureHandle
CRmlUIOgreRenderInterface::MakeTexture( const Ogre::TexturePtr& texture, bool owned )
{GUCEF_TRACE;

    STexture* entry = GUCEF_NEW STexture();
    entry->texture = texture;
    entry->owned = owned;
    ++m_textureCount;
    return reinterpret_cast< Rml::TextureHandle >( entry );
}

/*-------------------------------------------------------------------------*/

Rml::TextureHandle
CRmlUIOgreRenderInterface::LoadTexture( Rml::Vector2i& textureDimensions, const Rml::String& source )
{GUCEF_TRACE;

    try
    {
        // the file in the VFS, read through the archive of OgrePlugin_GucefVFS over its directory (a listing of the
        // VFS root does not show what is mounted below it), decoded by the codecs of Ogre for its extension
        Ogre::String path = source;
        while ( !path.empty() && ( '/' == path[ 0 ] || '\\' == path[ 0 ] ) )
            path.erase( 0, 1 );
        Ogre::String directory, fileName;
        Ogre::StringUtil::splitFilename( path, fileName, directory );
        Ogre::String baseName, extension;
        Ogre::StringUtil::splitBaseFilename( fileName, baseName, extension );
        Ogre::Archive* archive = GetVfsArchive( "/" + directory );
        if ( GUCEF_NULL == archive )
            return 0;
        Ogre::DataStreamPtr stream = archive->open( fileName );
        if ( !stream )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: The image \"" << source << "\" is not in the VFS" );
            return 0;
        }
        Ogre::Image image;
        image.load( stream, extension );

        // RmlUi draws with premultiplied alpha
        const Ogre::uint32 width = image.getWidth();
        const Ogre::uint32 height = image.getHeight();
        Ogre::Image rgba( Ogre::PF_BYTE_RGBA, width, height );
        Ogre::PixelUtil::bulkPixelConversion( image.getPixelBox(), rgba.getPixelBox() );
        Ogre::uchar* data = rgba.getData();
        const size_t count = (size_t) width * height;
        for ( size_t i=0; i<count; ++i, data+=4 )
        {
            const UInt32 a = data[ 3 ];
            data[ 0 ] = (Ogre::uchar) ( ( data[ 0 ] * a + 127 ) / 255 );
            data[ 1 ] = (Ogre::uchar) ( ( data[ 1 ] * a + 127 ) / 255 );
            data[ 2 ] = (Ogre::uchar) ( ( data[ 2 ] * a + 127 ) / 255 );
        }

        const Ogre::String name = "RmlUiOgre/Texture/" + Ogre::StringConverter::toString( ++m_textureCounter ) + "/" + path;
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual( name, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                                    width, height, 0, Ogre::PF_BYTE_RGBA );
        texture->getBuffer()->blitFromMemory( rgba.getPixelBox() );
        textureDimensions = Rml::Vector2i( (int) width, (int) height );
        return MakeTexture( texture, true );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: The image \"" << source << "\" cannot be loaded: " << e.getFullDescription() );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Rml::TextureHandle
CRmlUIOgreRenderInterface::GenerateTexture( Rml::Span< const Rml::byte > source, Rml::Vector2i sourceDimensions )
{GUCEF_TRACE;

    if ( m_nextTexture )
    {
        // a texture of the application, shown as it is
        Ogre::TexturePtr texture = m_nextTexture;
        m_nextTexture.reset();
        return MakeTexture( texture, false );
    }

    if ( sourceDimensions.x <= 0 || sourceDimensions.y <= 0 || source.size() < (size_t) sourceDimensions.x * sourceDimensions.y * 4 )
        return 0;
    try
    {
        const Ogre::String name = "RmlUiOgre/Texture/" + Ogre::StringConverter::toString( ++m_textureCounter );
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual( name, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                                    (Ogre::uint) sourceDimensions.x, (Ogre::uint) sourceDimensions.y,
                                                                                    0, Ogre::PF_BYTE_RGBA );
        Ogre::PixelBox pixels( (Ogre::uint32) sourceDimensions.x, (Ogre::uint32) sourceDimensions.y, 1, Ogre::PF_BYTE_RGBA,
                               const_cast< Rml::byte* >( source.data() ) );
        texture->getBuffer()->blitFromMemory( pixels );
        return MakeTexture( texture, true );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIOgreRenderInterface: A texture cannot be made: " << e.getFullDescription() );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::ReleaseTexture( Rml::TextureHandle texture )
{GUCEF_TRACE;

    if ( 0 == texture )
        return;
    STexture* entry = reinterpret_cast< STexture* >( texture );
    if ( entry->owned && entry->texture && GUCEF_NULL != Ogre::TextureManager::getSingletonPtr() )
        Ogre::TextureManager::getSingleton().remove( entry->texture );
    GUCEF_DELETE entry;
    if ( m_textureCount > 0 )
        --m_textureCount;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::EnableScissorRegion( bool enable )
{GUCEF_TRACE;

    m_scissorEnabled = enable;
    ApplyScissor();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::SetScissorRegion( Rml::Rectanglei region )
{GUCEF_TRACE;

    m_scissorRegion = region;
    ApplyScissor();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::ApplyScissor( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderSystem )
        return;
    Int32 left = 0, top = 0, right = 0, bottom = 0;
    if ( GetTargetScissor( left, top, right, bottom ) )
        m_renderSystem->setScissorTest( true, Ogre::Rect( left, top, right, bottom ) );
    else
        m_renderSystem->setScissorTest( false );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::EnableClipMask( bool enable )
{GUCEF_TRACE;

    m_clipMaskEnabled = enable;
    ApplyStencil();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::ApplyStencil( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderSystem )
        return;
    Ogre::StencilState stencil;
    if ( m_clipMaskEnabled )
    {
        // only where the mask has the value of the clip region
        stencil.enabled = true;
        stencil.compareOp = Ogre::CMPF_EQUAL;
        stencil.referenceValue = m_stencilTestValue;
        stencil.compareMask = 0xFFFFFFFF;
        stencil.writeMask = 0;
    }
    m_renderSystem->setStencilState( stencil );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::RenderToClipMask( Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderSystem )
        return;

    // as RmlUi's OpenGL 3 renderer: Set makes the geometry the mask, SetInverse everything but the geometry,
    // Intersect keeps the part of the mask that the geometry covers
    UInt32 writeValue = 1;
    UInt32 testValue = 1;
    Ogre::StencilOperation passOperation = Ogre::SOP_REPLACE;
    switch ( operation )
    {
        case Rml::ClipMaskOperation::Set:
        {
            m_renderSystem->clearFrameBuffer( Ogre::FBT_STENCIL, Ogre::ColourValue::Black, 1.0f, 0 );
            break;
        }
        case Rml::ClipMaskOperation::SetInverse:
        {
            m_renderSystem->clearFrameBuffer( Ogre::FBT_STENCIL, Ogre::ColourValue::Black, 1.0f, 1 );
            writeValue = 0;
            break;
        }
        case Rml::ClipMaskOperation::Intersect:
        {
            passOperation = Ogre::SOP_INCREMENT;
            testValue = m_stencilTestValue + 1;
            break;
        }
    }

    Ogre::StencilState stencil;
    stencil.enabled = true;
    stencil.compareOp = Ogre::CMPF_ALWAYS_PASS;
    stencil.referenceValue = writeValue;
    stencil.compareMask = 0xFFFFFFFF;
    stencil.writeMask = 0xFFFFFFFF;
    stencil.stencilFailOp = Ogre::SOP_KEEP;
    stencil.depthFailOp = Ogre::SOP_KEEP;
    stencil.depthStencilPassOp = passOperation;
    m_renderSystem->setStencilState( stencil );
    m_material.SetColourWriteEnabled( *m_renderSystem, false );

    const UInt32 drawCalls = m_drawCalls;
    RenderGeometry( geometry, translation, 0 );
    m_drawCalls = drawCalls;

    m_material.SetColourWriteEnabled( *m_renderSystem, true );
    m_stencilTestValue = testValue;
    m_clipMaskEnabled = true;
    ApplyStencil();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreRenderInterface::SetTransform( const Rml::Matrix4f* transform )
{GUCEF_TRACE;

    if ( GUCEF_NULL == transform )
    {
        m_transform = Ogre::Matrix4::IDENTITY;
    }
    else
    {
        // the same mathematical matrix, whatever the storage order RmlUi was built with
        for ( int row=0; row<4; ++row )
            for ( int column=0; column<4; ++column )
                m_transform[ row ][ column ] = transform->GetRow( row )[ column ];
    }
    m_material.SetTransform( &m_transform );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

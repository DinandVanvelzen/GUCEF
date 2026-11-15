/*
 *  guidriverImGuiOgre: gucefGUI driver ImGuiOgre: Dear ImGui drawn through Ogre over the 3D scene of an Ogre window
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
#include <string.h>
#include <math.h>

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverImGuiOgre_CImGuiOgreRenderer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 MinimumVertexCapacity = 5000;
const UInt32 MinimumIndexCapacity = 10000;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiOgreRenderer::CImGuiOgreRenderer( void )
    : m_material( GUIDRIVEROGRE::COgreGuiMaterial::BLENDMODE_STRAIGHT )
    , m_vertexData( GUCEF_NULL )
    , m_indexData( GUCEF_NULL )
    , m_vertexBuffer()
    , m_indexBuffer()
    , m_vertexCapacity( 0 )
    , m_indexCapacity( 0 )
    , m_textureCounter( 0 )
    , m_texturesCreated( 0 )
    , m_texturesUpdated( 0 )
    , m_texturesDestroyed( 0 )
    , m_drawCalls( 0 )
    , m_totalDrawCalls( 0 )
    , m_lastRenderFailed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiOgreRenderer::~CImGuiOgreRenderer()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreRenderer::Initialize( void )
{GUCEF_TRACE;

    if ( m_material.IsInitialized() )
        return true;
    if ( !m_material.Initialize() )
        return false;
    m_material.SetNearestFiltering( false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreRenderer::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_material.IsInitialized();
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::Release( void )
{GUCEF_TRACE;

    ReleaseBuffers();
    m_material.Release();
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::ReleaseBuffers( void )
{GUCEF_TRACE;

    // the vertex data lets go of its buffer through its binding
    if ( GUCEF_NULL != m_vertexData )
    {
        OGRE_DELETE m_vertexData;
        m_vertexData = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_indexData )
    {
        OGRE_DELETE m_indexData;
        m_indexData = GUCEF_NULL;
    }
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_vertexCapacity = 0;
    m_indexCapacity = 0;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreRenderer::EnsureBuffers( UInt32 vertexCount, UInt32 indexCount )
{GUCEF_TRACE;

    Ogre::HardwareBufferManager& buffers = Ogre::HardwareBufferManager::getSingleton();

    if ( GUCEF_NULL == m_vertexData )
    {
        // the layout of ImDrawVert: position, texture coordinate, colour
        m_vertexData = OGRE_NEW Ogre::VertexData();
        m_vertexData->vertexStart = 0;
        m_vertexData->vertexCount = 0;
        Ogre::VertexDeclaration* declaration = m_vertexData->vertexDeclaration;
        declaration->addElement( 0, offsetof( ImDrawVert, pos ), Ogre::VET_FLOAT2, Ogre::VES_POSITION );
        declaration->addElement( 0, offsetof( ImDrawVert, uv ), Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0 );
        declaration->addElement( 0, offsetof( ImDrawVert, col ), Ogre::VET_UBYTE4_NORM, Ogre::VES_DIFFUSE );
    }
    if ( GUCEF_NULL == m_indexData )
    {
        m_indexData = OGRE_NEW Ogre::IndexData();
        m_indexData->indexStart = 0;
        m_indexData->indexCount = 0;
    }

    if ( vertexCount > m_vertexCapacity )
    {
        UInt32 capacity = m_vertexCapacity + m_vertexCapacity / 2;
        if ( capacity < vertexCount )
            capacity = vertexCount;
        if ( capacity < MinimumVertexCapacity )
            capacity = MinimumVertexCapacity;
        m_vertexBuffer = buffers.createVertexBuffer( sizeof( ImDrawVert ), capacity, Ogre::HBU_CPU_TO_GPU );
        m_vertexData->vertexBufferBinding->setBinding( 0, m_vertexBuffer );
        m_vertexCapacity = capacity;
    }
    if ( indexCount > m_indexCapacity )
    {
        UInt32 capacity = m_indexCapacity + m_indexCapacity / 2;
        if ( capacity < indexCount )
            capacity = indexCount;
        if ( capacity < MinimumIndexCapacity )
            capacity = MinimumIndexCapacity;
        m_indexBuffer = buffers.createIndexBuffer( sizeof( ImDrawIdx ) == 4 ? Ogre::HardwareIndexBuffer::IT_32BIT : Ogre::HardwareIndexBuffer::IT_16BIT,
                                                   capacity, Ogre::HBU_CPU_TO_GPU );
        m_indexData->indexBuffer = m_indexBuffer;
        m_indexCapacity = capacity;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CImGuiOgreRenderer::GetOgreTexture( ImTextureID textureId )
{GUCEF_TRACE;

    if ( ImTextureID_Invalid == textureId || GUCEF_NULL == Ogre::TextureManager::getSingletonPtr() )
        return Ogre::TexturePtr();
    return Ogre::static_pointer_cast< Ogre::Texture >( Ogre::TextureManager::getSingleton().getByHandle( (Ogre::ResourceHandle) textureId ) );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreRenderer::ServeTextureRequests( ImDrawData& drawData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == drawData.Textures || GUCEF_NULL == Ogre::TextureManager::getSingletonPtr() )
        return true;

    bool success = true;
    for ( int i=0; i<drawData.Textures->Size; ++i )
    {
        ImTextureData* texture = (*drawData.Textures)[ i ];
        if ( GUCEF_NULL == texture || ImTextureStatus_OK == texture->Status || ImTextureStatus_Destroyed == texture->Status )
            continue;

        try
        {
            if ( ImTextureStatus_WantCreate == texture->Status )
            {
                if ( ImTextureFormat_RGBA32 != texture->Format )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreRenderer: A texture of Dear ImGui has a format other than RGBA32, it is not shown" );
                    texture->SetStatus( ImTextureStatus_OK );
                    success = false;
                    continue;
                }

                // a resource of Ogre; its handle is the texture id
                const Ogre::String name = "ImGuiOgre/Texture/" + Ogre::StringConverter::toString( ++m_textureCounter );
                Ogre::TexturePtr ogreTexture = Ogre::TextureManager::getSingleton().createManual( name, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                                                (Ogre::uint) texture->Width, (Ogre::uint) texture->Height,
                                                                                                0, Ogre::PF_BYTE_RGBA );
                Ogre::PixelBox pixels( (Ogre::uint32) texture->Width, (Ogre::uint32) texture->Height, 1, Ogre::PF_BYTE_RGBA, texture->GetPixels() );
                ogreTexture->getBuffer()->blitFromMemory( pixels );
                texture->SetTexID( (ImTextureID) ogreTexture->getHandle() );
                texture->SetStatus( ImTextureStatus_OK );
                ++m_texturesCreated;
            }
            else if ( ImTextureStatus_WantUpdates == texture->Status )
            {
                Ogre::TexturePtr ogreTexture = GetOgreTexture( texture->TexID );
                if ( ogreTexture )
                {
                    // the rectangles that changed, from the pixels of the Dear ImGui texture (its rows are as wide as the texture)
                    for ( int r=0; r<texture->Updates.Size; ++r )
                    {
                        const ImTextureRect& rect = texture->Updates[ r ];
                        Ogre::PixelBox pixels( (Ogre::uint32) rect.w, (Ogre::uint32) rect.h, 1, Ogre::PF_BYTE_RGBA, texture->GetPixelsAt( rect.x, rect.y ) );
                        pixels.rowPitch = (size_t) texture->Width;
                        ogreTexture->getBuffer()->blitFromMemory( pixels, Ogre::Box( rect.x, rect.y, rect.x + rect.w, rect.y + rect.h ) );
                    }
                    ++m_texturesUpdated;
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreRenderer: The Ogre texture of a texture of Dear ImGui to update is gone" );
                    success = false;
                }
                texture->SetStatus( ImTextureStatus_OK );
            }
            else if ( ImTextureStatus_WantDestroy == texture->Status && texture->UnusedFrames > 0 )
            {
                if ( ImTextureID_Invalid != texture->TexID )
                    Ogre::TextureManager::getSingleton().remove( (Ogre::ResourceHandle) texture->TexID );
                texture->SetTexID( ImTextureID_Invalid );
                texture->SetStatus( ImTextureStatus_Destroyed );
                ++m_texturesDestroyed;
            }
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreRenderer: A texture request of Dear ImGui failed: " << e.getFullDescription() );
            success = false;
            if ( ImTextureStatus_WantCreate == texture->Status )
                texture->SetStatus( ImTextureStatus_Destroyed );
            else
                texture->SetStatus( ImTextureStatus_OK );
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::DestroyTextures( ImVector< ImTextureData* >& textures )
{GUCEF_TRACE;

    const bool haveManager = GUCEF_NULL != Ogre::TextureManager::getSingletonPtr();
    for ( int i=0; i<textures.Size; ++i )
    {
        ImTextureData* texture = textures[ i ];
        if ( GUCEF_NULL == texture || ImTextureID_Invalid == texture->TexID )
            continue;
        try
        {
            if ( haveManager )
                Ogre::TextureManager::getSingleton().remove( (Ogre::ResourceHandle) texture->TexID );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreRenderer: A texture cannot be destroyed: " << e.getFullDescription() );
        }
        texture->SetTexID( ImTextureID_Invalid );
        texture->SetStatus( ImTextureStatus_Destroyed );
        ++m_texturesDestroyed;
    }
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreRenderer::Render( Ogre::RenderSystem& renderSystem, Ogre::Viewport& viewport, const ImDrawData& drawData )
{GUCEF_TRACE;

    m_drawCalls = 0;
    m_lastRenderFailed = false;
    if ( !m_material.IsInitialized() )
        return false;
    const Int32 targetWidth = (Int32) viewport.getActualWidth();
    const Int32 targetHeight = (Int32) viewport.getActualHeight();
    if ( drawData.CmdLists.Size <= 0 || drawData.TotalVtxCount <= 0 || drawData.TotalIdxCount <= 0 || targetWidth <= 0 || targetHeight <= 0 )
        return true;

    try
    {
        EnsureBuffers( (UInt32) drawData.TotalVtxCount, (UInt32) drawData.TotalIdxCount );

        // all the lists into the two buffers, one after the other
        {
            ImDrawVert* vertices = static_cast< ImDrawVert* >( m_vertexBuffer->lock( Ogre::HardwareBuffer::HBL_DISCARD ) );
            ImDrawIdx* indices = static_cast< ImDrawIdx* >( m_indexBuffer->lock( Ogre::HardwareBuffer::HBL_DISCARD ) );
            for ( int i=0; i<drawData.CmdLists.Size; ++i )
            {
                const ImDrawList* drawList = drawData.CmdLists[ i ];
                memcpy( vertices, drawList->VtxBuffer.Data, (size_t) drawList->VtxBuffer.Size * sizeof( ImDrawVert ) );
                memcpy( indices, drawList->IdxBuffer.Data, (size_t) drawList->IdxBuffer.Size * sizeof( ImDrawIdx ) );
                vertices += drawList->VtxBuffer.Size;
                indices += drawList->IdxBuffer.Size;
            }
            m_vertexBuffer->unlock();
            m_indexBuffer->unlock();
        }

        // the logical picture where the mapping of the draw data puts it on the target
        const float scaleX = drawData.FramebufferScale.x;
        const float scaleY = drawData.FramebufferScale.y;
        const float offsetX = -drawData.DisplayPos.x * scaleX;
        const float offsetY = -drawData.DisplayPos.y * scaleY;
        const bool flip = viewport.getTarget()->requiresTextureFlipping();
        const Ogre::Matrix4 projection = GUIDRIVEROGRE::COgreGuiMaterial::MakeProjection( offsetX, offsetY, scaleX, scaleY,
                                                                                         (UInt32) targetWidth, (UInt32) targetHeight, flip );
        m_material.BeginDrawing( renderSystem, projection );

        Ogre::RenderOperation operation;
        operation.vertexData = m_vertexData;
        operation.indexData = m_indexData;
        operation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        operation.useIndexes = true;
        m_vertexData->vertexCount = (size_t) drawData.TotalVtxCount;

        bool nearest = false;
        m_material.SetNearestFiltering( nearest );
        ImTextureID boundId = ImTextureID_Invalid;
        Ogre::TexturePtr boundTexture;
        UInt32 vertexBase = 0, indexBase = 0;

        for ( int i=0; i<drawData.CmdLists.Size; ++i )
        {
            const ImDrawList* drawList = drawData.CmdLists[ i ];
            for ( int c=0; c<drawList->CmdBuffer.Size; ++c )
            {
                const ImDrawCmd& command = drawList->CmdBuffer[ c ];
                if ( GUCEF_NULL != command.UserCallback )
                {
                    if ( command.UserCallback == &ResetRenderStateCallback )
                    {
                        m_material.BeginDrawing( renderSystem, projection );
                        m_material.SetNearestFiltering( nearest );
                        boundId = ImTextureID_Invalid;
                        boundTexture.reset();
                    }
                    else if ( command.UserCallback == &SetSamplerNearestCallback )
                    {
                        nearest = true;
                        m_material.SetNearestFiltering( true );
                    }
                    else if ( command.UserCallback == &SetSamplerLinearCallback )
                    {
                        nearest = false;
                        m_material.SetNearestFiltering( false );
                    }
                    else
                    {
                        command.UserCallback( drawList, &command );
                    }
                    continue;
                }
                if ( 0 == command.ElemCount )
                    continue;

                // the clip rectangle in pixels of the target, to the pixels whose centre is inside it (left and top edges
                // included, right and bottom edges not: the rule of the fills) and cut to the target
                float left = ( command.ClipRect.x - drawData.DisplayPos.x ) * scaleX;
                float top = ( command.ClipRect.y - drawData.DisplayPos.y ) * scaleY;
                float right = ( command.ClipRect.z - drawData.DisplayPos.x ) * scaleX;
                float bottom = ( command.ClipRect.w - drawData.DisplayPos.y ) * scaleY;
                long scissorLeft = (long) ceilf( left - 0.5f );
                long scissorTop = (long) ceilf( top - 0.5f );
                long scissorRight = (long) ceilf( right - 0.5f );
                long scissorBottom = (long) ceilf( bottom - 0.5f );
                if ( scissorLeft < 0 ) scissorLeft = 0;
                if ( scissorTop < 0 ) scissorTop = 0;
                if ( scissorRight > (long) targetWidth ) scissorRight = (long) targetWidth;
                if ( scissorBottom > (long) targetHeight ) scissorBottom = (long) targetHeight;
                if ( scissorRight <= scissorLeft || scissorBottom <= scissorTop )
                    continue;
                renderSystem.setScissorTest( true, Ogre::Rect( scissorLeft, scissorTop, scissorRight, scissorBottom ) );

                // the texture of the command: by resource handle
                const ImTextureID textureId = command.GetTexID();
                if ( textureId != boundId )
                {
                    boundTexture = GetOgreTexture( textureId );
                    boundId = textureId;
                }
                if ( ImTextureID_Invalid != textureId && !boundTexture )
                    continue;   // a texture of the application that is gone
                m_material.SetTexture( renderSystem, boundTexture );

                m_vertexData->vertexStart = vertexBase + command.VtxOffset;
                m_indexData->indexStart = indexBase + command.IdxOffset;
                m_indexData->indexCount = command.ElemCount;
                m_material.Render( renderSystem, operation );
                ++m_drawCalls;
                ++m_totalDrawCalls;
            }
            vertexBase += (UInt32) drawList->VtxBuffer.Size;
            indexBase += (UInt32) drawList->IdxBuffer.Size;
        }
        renderSystem.setScissorTest( false );
        m_material.SetNearestFiltering( false );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreRenderer: Drawing failed: " << e.getFullDescription() );
        m_lastRenderFailed = true;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::SetSamplerLinearCallback( const ImDrawList* drawList, const ImDrawCmd* command )
{GUCEF_TRACE;

    // an identifier: Render() recognizes the callback
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::SetSamplerNearestCallback( const ImDrawList* drawList, const ImDrawCmd* command )
{GUCEF_TRACE;

    // an identifier: Render() recognizes the callback
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreRenderer::ResetRenderStateCallback( const ImDrawList* drawList, const ImDrawCmd* command )
{GUCEF_TRACE;

    // an identifier: Render() recognizes the callback
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiOgreRenderer::GetTexturesCreated( void ) const
{GUCEF_TRACE;

    return m_texturesCreated;
}

UInt32
CImGuiOgreRenderer::GetTexturesUpdated( void ) const
{GUCEF_TRACE;

    return m_texturesUpdated;
}

UInt32
CImGuiOgreRenderer::GetTexturesDestroyed( void ) const
{GUCEF_TRACE;

    return m_texturesDestroyed;
}

UInt32
CImGuiOgreRenderer::GetDrawCallCount( void ) const
{GUCEF_TRACE;

    return m_drawCalls;
}

UInt32
CImGuiOgreRenderer::GetTotalDrawCallCount( void ) const
{GUCEF_TRACE;

    return m_totalDrawCalls;
}

UInt32
CImGuiOgreRenderer::GetVertexBufferCapacity( void ) const
{GUCEF_TRACE;

    return m_vertexCapacity;
}

UInt32
CImGuiOgreRenderer::GetIndexBufferCapacity( void ) const
{GUCEF_TRACE;

    return m_indexCapacity;
}

bool
CImGuiOgreRenderer::GetLastRenderFailed( void ) const
{GUCEF_TRACE;

    return m_lastRenderFailed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

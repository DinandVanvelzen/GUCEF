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

#ifndef GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERER_H
#define GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreRenderOperation.h>
#include <OgreTexture.h>

#include "imgui.h"

#ifndef GUIDRIVEROGRE_COGREGUIMATERIAL_H
#include "guidriverOgre_COgreGuiMaterial.h"
#define GUIDRIVEROGRE_COGREGUIMATERIAL_H
#endif /* GUIDRIVEROGRE_COGREGUIMATERIAL_H ? */

#ifndef GUIDRIVERIMGUIOGRE_MACROS_H
#include "guidriverImGuiOgre_macros.h"
#define GUIDRIVERIMGUIOGRE_MACROS_H
#endif /* GUIDRIVERIMGUIOGRE_MACROS_H ? */

namespace Ogre { class RenderSystem; class Viewport; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Draws the draw lists of Dear ImGui through Ogre, with batches of its own (no Overlay component of Ogre).
 *
 *  Vertices and indices of a frame go into one dynamic vertex buffer and one dynamic index buffer (HBU_CPU_TO_GPU,
 *  grown to what a frame needs, 32-bit indices), laid out as ImDrawVert: position float2, texture coordinate float2,
 *  diffuse colour four normalised bytes. Each draw command of Dear ImGui is one draw call of the GUI material of
 *  guidriverOgre with straight alpha blending, with the scissor rectangle of its clip rectangle. The textures of Dear
 *  ImGui are Ogre textures (PF_BYTE_RGBA, resource group RGN_INTERNAL) whose resource handle is the ImTextureID.
 *
 *  Texture requests (ImTextureData: create, update, destroy) are served by ServeTextureRequests() at any time; the
 *  drawing happens in Render(), in the overlay pass of a window.
 */
class GUIDRIVERIMGUIOGRE_PUBLIC_CPP CImGuiOgreRenderer
{
    public:

    CImGuiOgreRenderer( void );

    ~CImGuiOgreRenderer();

    /** Makes the GUI material (straight alpha) of guidriverOgre; needs an initialised Root with a render window */
    bool Initialize( void );

    bool IsInitialized( void ) const;

    /** Lets go of the buffers and the material. The textures are not touched: DestroyTextures() first */
    void Release( void );

    /**
     *  Serves the texture requests of the draw data: creates, updates and destroys (a texture that is not used for a frame)
     *  Ogre textures. False when one request failed (see the log)
     */
    bool ServeTextureRequests( ImDrawData& drawData );

    /** Destroys the Ogre textures of the Dear ImGui textures of the list that have one, whatever their state */
    void DestroyTextures( ImVector< ImTextureData* >& textures );

    /**
     *  Draws the draw data onto the viewport. The mapping of the draw data (DisplayPos, FramebufferScale) says where the
     *  logical picture lies on the target, and the viewport size is the size of the target. The render state is the one
     *  of the GUI material, and what Dear ImGui changes (scissor) is back to normal at the end
     */
    bool Render( Ogre::RenderSystem& renderSystem, Ogre::Viewport& viewport, const ImDrawData& drawData );

    /** The Ogre texture that an ImTextureID stands for, NULL when it is not (or no longer) there */
    static Ogre::TexturePtr GetOgreTexture( ImTextureID textureId );

    /** The standard draw callbacks of the platform IO of a context drawn by this renderer */
    static void SetSamplerLinearCallback( const ImDrawList* drawList, const ImDrawCmd* command );
    static void SetSamplerNearestCallback( const ImDrawList* drawList, const ImDrawCmd* command );
    static void ResetRenderStateCallback( const ImDrawList* drawList, const ImDrawCmd* command );

    UInt32 GetTexturesCreated( void ) const;
    UInt32 GetTexturesUpdated( void ) const;
    UInt32 GetTexturesDestroyed( void ) const;
    UInt32 GetDrawCallCount( void ) const;      /**< draw calls of the last Render() */
    UInt32 GetTotalDrawCallCount( void ) const;
    UInt32 GetVertexBufferCapacity( void ) const;
    UInt32 GetIndexBufferCapacity( void ) const;
    bool GetLastRenderFailed( void ) const;

    private:

    CImGuiOgreRenderer( const CImGuiOgreRenderer& src );
    CImGuiOgreRenderer& operator=( const CImGuiOgreRenderer& src );

    bool EnsureBuffers( UInt32 vertexCount, UInt32 indexCount );
    void ReleaseBuffers( void );

    private:

    GUIDRIVEROGRE::COgreGuiMaterial m_material;
    Ogre::VertexData* m_vertexData;
    Ogre::IndexData* m_indexData;
    Ogre::HardwareVertexBufferSharedPtr m_vertexBuffer;
    Ogre::HardwareIndexBufferSharedPtr m_indexBuffer;
    UInt32 m_vertexCapacity;
    UInt32 m_indexCapacity;
    UInt32 m_textureCounter;
    UInt32 m_texturesCreated;
    UInt32 m_texturesUpdated;
    UInt32 m_texturesDestroyed;
    UInt32 m_drawCalls;
    UInt32 m_totalDrawCalls;
    bool m_lastRenderFailed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERER_H ? */

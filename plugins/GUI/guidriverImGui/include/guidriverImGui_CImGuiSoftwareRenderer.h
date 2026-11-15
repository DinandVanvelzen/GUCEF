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

#ifndef GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H
#define GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "imgui.h"

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Draws the draw data of Dear ImGui on the CPU into an RGBA picture, with the rules of the GPU renderers:
 *
 *  - the projection and the scissor come from the draw data: a point p lands at ( p - DisplayPos ) * FramebufferScale,
 *    the scissor of a command is its clip rectangle mapped the same way and cut to whole pixels as glScissor() gets it;
 *  - triangles cover the pixels whose centre lies inside them, with the top-left rule for centres on an edge;
 *  - textures are sampled linearly by default, at the nearest texel after the DrawCallback_SetSamplerNearest callback
 *    of the platform IO (DrawCallback_SetSamplerLinear and DrawCallback_ResetRenderState go back to linear), with clamped
 *    edges; Alpha8 textures are white with the alpha of their texels;
 *  - the texel is multiplied by the colour of the vertex and blended straight: SRC_ALPHA, ONE_MINUS_SRC_ALPHA for the
 *    colour, ONE, ONE_MINUS_SRC_ALPHA for the alpha, rounded as gucefGUI's CSoftwareRenderer rounds, so a retained scene
 *    drawn through Dear ImGui has the pixels of the reference renderer.
 *
 *  Texture references are resolved without a GPU: a reference to an ImTextureData reads its pixels, a bare texture id is
 *  looked up in the texture list of the draw data. Textures of other renderers (a GL texture name) cannot be read and
 *  their commands are skipped and counted.
 *
 *  It is also a renderer of textures in memory: ServeTextureRequests() honours the texture requests of Dear ImGui for
 *  textures that keep their pixels in the ImTextureData, which is what an ImGui context without a GPU renderer uses.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiSoftwareRenderer
{
    public:

    CImGuiSoftwareRenderer( void );

    ~CImGuiSoftwareRenderer();

    /** Honours the texture requests (create, update, destroy) of the texture list of the draw data in memory */
    void ServeTextureRequests( ImDrawData& drawData );

    /** Destroys the textures of the list it created that only one context uses, as a renderer does when it shuts down */
    void DestroyTextures( ImVector< ImTextureData* >& textures );

    /**
     *  Draws the draw data into 'target', which is made or resized to width x height RGBA pixels and, with 'clear', filled
     *  with opaque black first
     */
    bool Render( const ImDrawData& drawData        ,
                 const ImGuiPlatformIO& platformIO ,
                 UInt32 width                      ,
                 UInt32 height                     ,
                 bool clear                        ,
                 IMAGE::TPixelMapPtr& target       );

    /** Draws the draw data over the RGBA pixels (8 bits per channel) of 'target' */
    bool Render( const ImDrawData& drawData        ,
                 const ImGuiPlatformIO& platformIO ,
                 IMAGE::CPixelMap& target          );

    /** How many textures ServeTextureRequests() created, updated and destroyed */
    UInt32 GetTextureCreateCount( void ) const;
    UInt32 GetTextureUpdateCount( void ) const;
    UInt32 GetTextureDestroyCount( void ) const;

    /** The draw commands of the last Render() whose texture could not be read */
    UInt32 GetUnresolvedTextureCount( void ) const;

    /** The triangles the last Render() drew */
    UInt32 GetTriangleCount( void ) const;

    private:

    CImGuiSoftwareRenderer( const CImGuiSoftwareRenderer& src );
    CImGuiSoftwareRenderer& operator=( const CImGuiSoftwareRenderer& src );

    private:

    UInt64 m_nextTextureId;
    UInt32 m_creates;
    UInt32 m_updates;
    UInt32 m_destroys;
    UInt32 m_unresolved;
    UInt32 m_triangles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H ? */

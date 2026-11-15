/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUIDRIVEROGRE_COGREGUIMATERIAL_H
#define GUIDRIVEROGRE_COGREGUIMATERIAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreGpuProgram.h>
#include <OgreGpuProgramParams.h>
#include <OgreMaterial.h>
#include <OgreMatrix4.h>
#include <OgreTexture.h>
#include <OgreTextureUnitState.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUIDRIVEROGRE_MACROS_H
#include "guidriverOgre_macros.h"
#define GUIDRIVEROGRE_MACROS_H
#endif /* GUIDRIVEROGRE_MACROS_H ? */

namespace Ogre { class Pass; class RenderOperation; class RenderSystem; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The material the GUI drivers on Ogre draw with: a vertex colour times a texture, in pixels of the GUI.
 *
 *  The programs exist once per process, per render system: GLSL 330 (OpenGL 3+), GLSL ES 300 (OpenGL ES 3, WebGL 2)
 *  and HLSL shader model 4 (Direct3D 11). Their sources are the data files of this plugin (data/shaders, mounted
 *  in the VFS as "guidriverOgre"). They read the vertex position, the diffuse colour (four bytes, normalised) and the
 *  first texture coordinate by semantic, so every vertex layout with those three works: a driver describes its own
 *  in its Ogre::VertexDeclaration. The programs take three parameters: "projection" (pixels to clip space, set by
 *  BeginDrawing()), "transform" (a transform of the geometry before the projection, identity by default) and
 *  "translation" (pixels added to every position).
 *
 *  The material (one Ogre::Pass) has culling, depth check, depth write, lighting and fog off and one texture unit
 *  with clamped addressing; the blending is chosen per material:
 *      premultiplied   SBF_ONE, SBF_ONE_MINUS_SOURCE_ALPHA (RmlUi 6)
 *      straight        SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA for the colour, SBF_ONE,
 *                      SBF_ONE_MINUS_SOURCE_ALPHA for the alpha (Dear ImGui)
 *
 *  Drivers draw outside the render queue of a scene manager (in the overlay of a window, CIOgreWindowOverlay):
 *  BeginDrawing() sets the whole render state of the pass on the render system, then per draw call SetTexture(),
 *  SetTranslation() / SetTransform() and Render(). Everything is used on the render thread.
 */
class GUIDRIVEROGRE_PUBLIC_CPP COgreGuiMaterial
{
    public:

    enum EBlendMode
    {
        BLENDMODE_PREMULTIPLIED = 0 ,
        BLENDMODE_STRAIGHT
    };
    typedef enum EBlendMode TBlendMode;

    /** "guidriverOgre": the name the data of this plugin is mounted under in the VFS */
    static const CORE::CString DataModuleName;

    explicit COgreGuiMaterial( TBlendMode blendMode = BLENDMODE_PREMULTIPLIED );

    ~COgreGuiMaterial();

    /**
     *  Makes the programs for the render system of the Root (once per process) and the material. Needs an initialised
     *  Root with a render window. False, with the reason in the log, when the render system has none of the shading
     *  languages or the shader files cannot be read
     */
    bool Initialize( void );

    /** Lets go of the material; the programs go when the last material lets go of them */
    void Release( void );

    bool IsInitialized( void ) const;

    TBlendMode GetBlendMode( void ) const;

    Ogre::MaterialPtr GetMaterial( void ) const;

    Ogre::Pass* GetPass( void ) const;

    /** Point sampling (the default: pixel art stays sharp) or bilinear sampling of the textures */
    void SetNearestFiltering( bool nearest );

    bool GetNearestFiltering( void ) const;

    /**
     *  Sets the render state of the pass on the render system: the programs, the blending, no depth test or write, no
     *  culling, no stencil test, no scissor test, no alpha rejection, solid polygons, and the projection
     */
    void BeginDrawing( Ogre::RenderSystem& renderSystem, const Ogre::Matrix4& projection );

    /** Writes the colour channels or not (drawing only into the stencil buffer for a clip mask); BeginDrawing() turns them on */
    void SetColourWriteEnabled( Ogre::RenderSystem& renderSystem, bool enabled );

    /** Changes the projection between draw calls (for example for a capture) */
    void SetProjection( const Ogre::Matrix4& projection );

    /** The transform applied before the projection; identity when 'transform' is NULL */
    void SetTransform( const Ogre::Matrix4* transform );

    void SetTranslation( float x, float y );

    /** Binds the texture to unit 0 with the sampler of the material; a NULL texture binds a white texture */
    void SetTexture( Ogre::RenderSystem& renderSystem, const Ogre::TexturePtr& texture );

    /** Binds the program parameters and draws */
    void Render( Ogre::RenderSystem& renderSystem, const Ogre::RenderOperation& operation );

    /**
     *  A projection for drawing in pixels of the GUI (x right, y down) on a render target of the given size: GUI pixel
     *  (x, y) lands on target pixel ( offsetX + x * scaleX, offsetY + y * scaleY ). 'flipY' is the
     *  requiresTextureFlipping() of the render target (OpenGL render textures)
     */
    static Ogre::Matrix4 MakeProjection( float offsetX       ,
                                         float offsetY       ,
                                         float scaleX        ,
                                         float scaleY        ,
                                         UInt32 targetWidth  ,
                                         UInt32 targetHeight ,
                                         bool flipY          );

    /** "glsl", "glsles" or "hlsl": the shading language of the programs for the render system, empty when there is none */
    static CORE::CString GetShadingLanguage( void );

    /** The white texture bound for geometry without a texture; NULL before the first Initialize() */
    static Ogre::TexturePtr GetWhiteTexture( void );

    /** Releases the programs and textures that the materials share, when no material uses them anymore */
    static void ReleaseSharedResources( void );

    private:

    COgreGuiMaterial( const COgreGuiMaterial& src );              /**< not implemented */
    COgreGuiMaterial& operator=( const COgreGuiMaterial& src );   /**< not implemented */

    private:

    TBlendMode m_blendMode;
    bool m_initialized;
    bool m_nearestFiltering;
    Ogre::MaterialPtr m_material;
    Ogre::GpuProgramParametersPtr m_vertexParameters;
    Ogre::GpuProgramParametersPtr m_fragmentParameters;
    Ogre::SamplerPtr m_sampler;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_COGREGUIMATERIAL_H ? */

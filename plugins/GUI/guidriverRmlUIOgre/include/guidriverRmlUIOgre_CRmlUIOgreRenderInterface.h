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

#ifndef GUIDRIVERRMLUIOGRE_CRMLUIOGRERENDERINTERFACE_H
#define GUIDRIVERRMLUIOGRE_CRMLUIOGRERENDERINTERFACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUi/Core/RenderInterface.h>

#include <OgreMatrix4.h>
#include <OgreTexture.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUIDRIVEROGRE_COGREGUIMATERIAL_H
#include "guidriverOgre_COgreGuiMaterial.h"
#define GUIDRIVEROGRE_COGREGUIMATERIAL_H
#endif /* GUIDRIVEROGRE_COGREGUIMATERIAL_H ? */

#ifndef GUIDRIVERRMLUIOGRE_MACROS_H
#include "guidriverRmlUIOgre_macros.h"
#define GUIDRIVERRMLUIOGRE_MACROS_H
#endif /* GUIDRIVERRMLUIOGRE_MACROS_H ? */

namespace Ogre { class RenderSystem; class Viewport; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The render interface of RmlUi 6 on Ogre 1.x.
 *
 *  - Geometry: every CompileGeometry() becomes static hardware vertex and index buffers (32-bit indices) in an
 *    Ogre::RenderOperation, with a vertex declaration over Rml::Vertex: position float2, diffuse colour as four
 *    normalised bytes (premultiplied), texture coordinate float2. RenderGeometry() draws it with one
 *    RenderSystem::_render() call.
 *  - Textures: GenerateTexture() makes a manual PF_BYTE_RGBA texture from the premultiplied pixels;
 *    LoadTexture() reads an image file of the VFS through the archive of OgrePlugin_GucefVFS (type GucefVFS) over its
 *    directory, decodes it with the Ogre codec of its extension (OgreCodec_GucefIMAGE for what gucefIMAGE decodes)
 *    and premultiplies it. A texture of the application can be shown without a copy (SetNextGeneratedTexture()).
 *  - Drawing state: the premultiplied GUI material of guidriverOgre (COgreGuiMaterial), point sampling as the
 *    OpenGL renderer of RmlUi in gucefGUI. Translation and SetTransform() are program parameters; the scissor
 *    region becomes RenderSystem::setScissorTest(); the clip mask is drawn into the stencil buffer.
 *  - Layers, filters and shaders of RmlUi are not supported: the defaults of Rml::RenderInterface apply.
 *
 *  Drawing happens between BeginFrame() and EndFrame(), on the render thread, with the viewport of the render target
 *  set on the render system. RmlUi works in pixels of the frame of the GUI context; BeginFrame() says where those
 *  land on the target (offset and scale of the present rectangle).
 */
class GUIDRIVERRMLUIOGRE_PUBLIC_CPP CRmlUIOgreRenderInterface : public Rml::RenderInterface
{
    public:

    CRmlUIOgreRenderInterface( void );

    virtual ~CRmlUIOgreRenderInterface();

    /** Makes the GUI material; needs an initialised Ogre Root with a render window */
    bool Initialize( void );

    bool IsInitialized( void ) const;

    /**
     *  Starts drawing in the viewport the render system has set: GUI pixel (x, y) lands on target pixel
     *  ( offsetX + x * scaleX, offsetY + y * scaleY )
     */
    void BeginFrame( Ogre::RenderSystem& renderSystem ,
                     Ogre::Viewport& viewport         ,
                     float offsetX                    ,
                     float offsetY                    ,
                     float scaleX                     ,
                     float scaleY                     );

    void EndFrame( void );

    bool IsDrawing( void ) const;

    /** The next GenerateTexture() call gives a texture handle for this texture instead of making one from pixels */
    void SetNextGeneratedTexture( const Ogre::TexturePtr& texture );

    /** The Ogre texture of a texture handle, NULL for none */
    static Ogre::TexturePtr GetOgreTexture( Rml::TextureHandle texture );

    /** For tests: the compiled geometry and the textures that exist, the draw calls since BeginFrame() */
    UInt32 GetGeometryCount( void ) const;
    UInt32 GetTextureCount( void ) const;
    UInt32 GetDrawCallCount( void ) const;

    /** For tests: the scissor rectangle on the render target, and whether it is on */
    bool GetTargetScissor( Int32& left, Int32& top, Int32& right, Int32& bottom ) const;

    /** For tests: the transform of SetTransform(), identity when there is none */
    const Ogre::Matrix4& GetTransform( void ) const;

    /** For tests: the vertex and index count of compiled geometry */
    static bool GetGeometryCounts( Rml::CompiledGeometryHandle geometry, UInt32& vertexCount, UInt32& indexCount );

    virtual Rml::CompiledGeometryHandle CompileGeometry( Rml::Span< const Rml::Vertex > vertices, Rml::Span< const int > indices ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void RenderGeometry( Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ReleaseGeometry( Rml::CompiledGeometryHandle geometry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Rml::TextureHandle LoadTexture( Rml::Vector2i& textureDimensions, const Rml::String& source ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Rml::TextureHandle GenerateTexture( Rml::Span< const Rml::byte > source, Rml::Vector2i sourceDimensions ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void ReleaseTexture( Rml::TextureHandle texture ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void EnableScissorRegion( bool enable ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetScissorRegion( Rml::Rectanglei region ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void EnableClipMask( bool enable ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void RenderToClipMask( Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetTransform( const Rml::Matrix4f* transform ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void ApplyScissor( void );
    void ApplyStencil( void );
    Rml::TextureHandle MakeTexture( const Ogre::TexturePtr& texture, bool owned );

    private:

    GUIDRIVEROGRE::COgreGuiMaterial m_material;
    Ogre::RenderSystem* m_renderSystem;
    Ogre::Viewport* m_viewport;
    float m_offsetX;
    float m_offsetY;
    float m_scaleX;
    float m_scaleY;
    bool m_scissorEnabled;
    Rml::Rectanglei m_scissorRegion;
    bool m_clipMaskEnabled;
    UInt32 m_stencilTestValue;
    Ogre::Matrix4 m_transform;
    Ogre::TexturePtr m_nextTexture;
    UInt32 m_geometryCount;
    UInt32 m_textureCount;
    UInt32 m_drawCalls;
    UInt32 m_textureCounter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERRMLUIOGRE_CRMLUIOGRERENDERINTERFACE_H ? */

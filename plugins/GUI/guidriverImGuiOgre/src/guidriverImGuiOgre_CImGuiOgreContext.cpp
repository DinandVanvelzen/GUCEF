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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#include "guidriverImGui_CImGuiContextScope.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H ? */

#include "guidriverImGuiOgre_CImGuiOgreRenderer.h"
#include "guidriverImGuiOgre_CImGuiOgreContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CImGuiOgreContext::ClassTypeName = "GUCEF::GUIDRIVERIMGUIOGRE::CImGuiOgreContext";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiOgreContext::CImGuiOgreContext( GUIDRIVERIMGUI::CImGuiDriver* driver, GUI::TWindowContextPtr windowContext )
    : GUIDRIVERIMGUI::CImGuiContext( driver, windowContext )
    , GUIDRIVEROGRE::CIOgreWindowOverlay()
    , m_renderer( GUCEF_NULL )
    , m_drawData( GUCEF_NULL )
    , m_overlayDrawCount( 0 )
    , m_registered( false )
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( !IsReady() || GUCEF_NULL == window )
        return;

    m_renderer = GUCEF_NEW CImGuiOgreRenderer();
    if ( !m_renderer->Initialize() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGuiOgreContext: The GUI material of Ogre could not be made" );
        GUCEF_DELETE m_renderer;
        m_renderer = GUCEF_NULL;
        return;
    }

    // the standard draw callbacks of this renderer: the retained painter adds them to its draw lists, Render() recognizes them
    GUIDRIVERIMGUI::CImGuiContextScope scope( GetImGuiContext() );
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "gucefGUI Ogre";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    platformIO.DrawCallback_SetSamplerLinear = &CImGuiOgreRenderer::SetSamplerLinearCallback;
    platformIO.DrawCallback_SetSamplerNearest = &CImGuiOgreRenderer::SetSamplerNearestCallback;
    platformIO.DrawCallback_ResetRenderState = &CImGuiOgreRenderer::ResetRenderStateCallback;

    window->RegisterOverlay( this );
    m_registered = true;
}

/*-------------------------------------------------------------------------*/

CImGuiOgreContext::~CImGuiOgreContext()
{GUCEF_TRACE;

    // no more drawing in the window, then the widgets, the textures and the buffers, then the base releases the rest
    if ( m_registered )
    {
        GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
        if ( GUCEF_NULL != window )
            window->UnregisterOverlay( this );
        m_registered = false;
    }
    m_drawData = GUCEF_NULL;
    DestroyAllWidgets();
    ShutdownRenderer();
    m_ogreTextures.clear();
    GUCEF_DELETE m_renderer;
    m_renderer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImGuiOgreContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::IsRendererReady( void ) const
{GUCEF_TRACE;

    return m_registered && GUCEF_NULL != m_renderer && m_renderer->IsInitialized();
}

/*-------------------------------------------------------------------------*/

CImGuiOgreRenderer*
CImGuiOgreContext::GetRenderer( void ) const
{GUCEF_TRACE;

    return m_renderer;
}

/*-------------------------------------------------------------------------*/

GUIDRIVEROGRE::COgreWindowContext*
CImGuiOgreContext::GetOgreWindowContext( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != window->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< GUIDRIVEROGRE::COgreWindowContext* >( window.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

ImTextureRef
CImGuiOgreContext::GetTextureRef( const Ogre::TexturePtr& texture ) const
{GUCEF_TRACE;

    if ( !texture )
        return ImTextureRef();
    return ImTextureRef( (ImTextureID) texture->getHandle() );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::RegisterOgreTexture( const CORE::CString& alias, const Ogre::TexturePtr& texture )
{GUCEF_TRACE;

    if ( alias.IsNULLOrEmpty() || !texture )
        return false;
    const bool known = m_ogreTextures.find( alias ) != m_ogreTextures.end();
    m_ogreTextures[ alias ] = texture;
    if ( known )
        return UpdateImage( alias );

    // the image the drawing commands name: one pixel, so the whole texture is its source rectangle (0, 0, 1, 1)
    IMAGE::TPixelMapPtr placeholder( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( placeholder->GetDataPtr() );
    data[ 0 ] = data[ 1 ] = data[ 2 ] = 0;
    data[ 3 ] = 255;
    if ( !RegisterImage( alias, placeholder ) )
    {
        m_ogreTextures.erase( alias );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::UnregisterOgreTexture( const CORE::CString& alias )
{GUCEF_TRACE;

    TOgreTextureMap::iterator i = m_ogreTextures.find( alias );
    if ( i == m_ogreTextures.end() )
        return false;
    m_ogreTextures.erase( i );
    return UnregisterImage( alias );
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CImGuiOgreContext::GetRegisteredOgreTexture( const CORE::CString& alias ) const
{GUCEF_TRACE;

    TOgreTextureMap::const_iterator i = m_ogreTextures.find( alias );
    if ( i == m_ogreTextures.end() )
        return Ogre::TexturePtr();
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::GetBackendImageTextureRef( const CORE::CString& reference, ImTextureRef& textureRef ) const
{GUCEF_TRACE;

    if ( m_ogreTextures.empty() )
        return false;
    TOgreTextureMap::const_iterator i = m_ogreTextures.find( reference );
    if ( i == m_ogreTextures.end() )
        return false;
    textureRef = GetTextureRef( (*i).second );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiOgreContext::GetOverlayDrawCount( void ) const
{GUCEF_TRACE;

    return m_overlayDrawCount;
}

/*-------------------------------------------------------------------------*/

Int32
CImGuiOgreContext::GetOverlayOrder( void ) const
{GUCEF_TRACE;

    return GetLayer();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::OnRendererNewFrame( void )
{GUCEF_TRACE;

    // the draw data of the last frame goes with the new frame
    m_drawData = GUCEF_NULL;
    return IsRendererReady();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::RenderDrawData( ImDrawData& drawData )
{GUCEF_TRACE;

    if ( !IsRendererReady() )
        return false;

    // the textures now, the drawing when the window draws its overlays; the logical picture goes to the present rectangle
    const bool served = m_renderer->ServeTextureRequests( drawData );
    SetDrawDataMapping( drawData, GetPresentMapping() );
    m_drawData = &drawData;
    return served;
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreContext::RenderOverlay( Ogre::RenderSystem& renderSystem ,
                                  Ogre::Viewport& viewport         ,
                                  Ogre::SceneManager* sceneManager ,
                                  bool isCapture                   )
{GUCEF_TRACE;

    if ( !IsRendererReady() || GUCEF_NULL == m_drawData || !m_drawData->Valid )
        return;
    m_renderer->Render( renderSystem, viewport, *m_drawData );
    if ( !isCapture )
        ++m_overlayDrawCount;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( !IsRendererReady() || GUCEF_NULL == window )
        return false;

    // this draw data alone into a render texture, on opaque black; the mapping to the target is set
    const ImDrawData* previous = m_drawData;
    m_drawData = &drawData;
    const bool captured = window->CaptureOverlay( *this, width, height, true, pixels );
    m_drawData = previous;
    return captured;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiOgreContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( GUCEF_NULL == window )
        return false;
    return window->RenderFrame( &pixels );
}

/*-------------------------------------------------------------------------*/

void
CImGuiOgreContext::ReleaseRenderer( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderer )
        return;
    m_drawData = GUCEF_NULL;
    m_renderer->DestroyTextures( ImGui::GetPlatformIO().Textures );
    m_renderer->Release();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

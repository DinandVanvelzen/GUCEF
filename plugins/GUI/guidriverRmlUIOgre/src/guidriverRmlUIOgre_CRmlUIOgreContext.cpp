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

#include <RmlUi/Core/Core.h>

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#include "guidriverRmlUIOgre_CRmlUIOgreRenderInterface.h"
#include "guidriverRmlUIOgre_CRmlUIOgreContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CRmlUIOgreContext::ClassTypeName = "GUCEF::GUIDRIVERRMLUIOGRE::CRmlUIOgreContext";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIOgreContext::CRmlUIOgreContext( GUI::CGUIDriver* driver                 ,
                                      GUI::TWindowContextPtr windowContext    ,
                                      CRmlUIOgreRenderInterface* renderer     )
    : guidriverRmlUI::CRmlUIContext( driver, windowContext, renderer )
    , GUIDRIVEROGRE::CIOgreWindowOverlay()
    , m_renderer( renderer )
    , m_ogreTextures()
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( GUCEF_NULL != window )
        window->RegisterOverlay( this );
}

/*-------------------------------------------------------------------------*/

CRmlUIOgreContext::~CRmlUIOgreContext()
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( GUCEF_NULL != window )
        window->UnregisterOverlay( this );

    // RmlUi lets go of its geometry and textures through the render interface, which goes after it
    ReleaseRml();
    m_ogreTextures.clear();
    Rml::ReleaseRenderManagers();
    GUCEF_DELETE m_renderer;
    m_renderer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRmlUIOgreContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CRmlUIOgreRenderInterface*
CRmlUIOgreContext::GetRenderInterface( void ) const
{GUCEF_TRACE;

    return m_renderer;
}

/*-------------------------------------------------------------------------*/

GUIDRIVEROGRE::COgreWindowContext*
CRmlUIOgreContext::GetOgreWindowContext( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() || GUIDRIVEROGRE::COgreWindowContext::ClassTypeName != window->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< GUIDRIVEROGRE::COgreWindowContext* >( window.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

Int32
CRmlUIOgreContext::GetOverlayOrder( void ) const
{GUCEF_TRACE;

    return GetLayer();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreContext::OnRender( void )
{GUCEF_TRACE;

    // the window draws the overlay after its viewports; here RmlUi only catches up with the widgets
    PrepareRml();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIOgreContext::RenderOverlay( Ogre::RenderSystem& renderSystem ,
                                  Ogre::Viewport& viewport         ,
                                  Ogre::SceneManager* sceneManager ,
                                  bool isCapture                   )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_renderer || !m_renderer->IsInitialized() )
        return;
    const UInt32 renderWidth = GetRenderWidth();
    const UInt32 renderHeight = GetRenderHeight();
    if ( 0 == renderWidth || 0 == renderHeight )
        return;

    float offsetX = 0.0f, offsetY = 0.0f, scaleX = 1.0f, scaleY = 1.0f;
    if ( !isCapture )
    {
        // the frame of the context onto the present rectangle of the window
        const GUI::CRect present = GetPresentRect();
        offsetX = (float) present.left;
        offsetY = (float) present.top;
        scaleX = (float) present.width / (float) renderWidth;
        scaleY = (float) present.height / (float) renderHeight;
    }
    m_renderer->BeginFrame( renderSystem, viewport, offsetX, offsetY, scaleX, scaleY );
    RenderRml();
    m_renderer->EndFrame();
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( GUCEF_NULL == window )
        return false;
    // the picture as it is now, without moving time on
    FlushChanges();
    PrepareRml();
    return window->CaptureOverlay( *this, GetRenderWidth(), GetRenderHeight(), IsBottomLayer(), pixels );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreContext::CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    GUIDRIVEROGRE::COgreWindowContext* window = GetOgreWindowContext();
    if ( GUCEF_NULL == window )
        return false;
    return window->RenderFrame( &pixels );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreContext::RegisterOgreTexture( const CORE::CString& alias, const Ogre::TexturePtr& texture )
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
CRmlUIOgreContext::UnregisterOgreTexture( const CORE::CString& alias )
{GUCEF_TRACE;

    TOgreTextureMap::iterator i = m_ogreTextures.find( alias );
    if ( i == m_ogreTextures.end() )
        return false;
    m_ogreTextures.erase( i );
    return UnregisterImage( alias );
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CRmlUIOgreContext::GetRegisteredOgreTexture( const CORE::CString& alias ) const
{GUCEF_TRACE;

    TOgreTextureMap::const_iterator i = m_ogreTextures.find( alias );
    if ( i == m_ogreTextures.end() )
        return Ogre::TexturePtr();
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreContext::GenerateImageTexture( const CORE::CString& reference                             ,
                                         bool isFont                                          ,
                                         const IMAGE::CPixelMap& pixels                       ,
                                         const Rml::CallbackTextureInterface& textureInterface )
{GUCEF_TRACE;

    if ( !isFont && GUCEF_NULL != m_renderer )
    {
        TOgreTextureMap::const_iterator i = m_ogreTextures.find( reference );
        if ( i != m_ogreTextures.end() )
        {
            // the texture of the application itself, no pixels are copied; the size RmlUi gets is the one of the image
            m_renderer->SetNextGeneratedTexture( (*i).second );
            return textureInterface.GenerateTexture( Rml::Span< const Rml::byte >(), Rml::Vector2i( (int) pixels.GetWidthInPixels(), (int) pixels.GetHeightInPixels() ) );
        }
    }
    return guidriverRmlUI::CRmlUIContext::GenerateImageTexture( reference, isFont, pixels, textureInterface );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

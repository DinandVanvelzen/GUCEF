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

#ifndef GUIDRIVERRMLUIOGRE_CRMLUIOGRECONTEXT_H
#define GUIDRIVERRMLUIOGRE_CRMLUIOGRECONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreTexture.h>

#include "guidriverRmlUI_CRmlUIContext.h"

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#include "guidriverOgre_CIOgreWindowOverlay.h"
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */

#ifndef GUIDRIVERRMLUIOGRE_MACROS_H
#include "guidriverRmlUIOgre_macros.h"
#define GUIDRIVERRMLUIOGRE_MACROS_H
#endif /* GUIDRIVERRMLUIOGRE_MACROS_H ? */

namespace GUCEF { namespace GUIDRIVEROGRE { class COgreWindowContext; } }

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

class CRmlUIOgreRenderInterface;

/**
 *  An RmlUi context drawn through Ogre in an Ogre window (window backend "Ogre"), over the viewports of the
 *  application.
 *
 *  It is an overlay of its window (CIOgreWindowOverlay) with its GUI layer as the overlay order. In a frame of the
 *  window the redraw event lets it update and prepare (OnRender() calls PrepareRml() only); it draws when the window
 *  draws its overlays, after the viewports. The frame of the context (the logical resolution times the render scale)
 *  is drawn directly in the window, scaled onto the present rectangle; no picture of its own is made and the window
 *  is not cleared by it.
 *
 *  CaptureFrame() draws the context alone into a render texture of the render size (opaque black below the bottom
 *  layer, transparent below a higher one) and reads it back. CapturePresentedFrame() draws a frame of the window and
 *  reads the window back before it is presented.
 *
 *  Ogre textures of the application are shown in DrawSurface widgets (RenderContext) through
 *  CRmlUIOgreRenderContext.
 */
class GUIDRIVERRMLUIOGRE_PUBLIC_CPP CRmlUIOgreContext : public guidriverRmlUI::CRmlUIContext ,
                                                         public GUIDRIVEROGRE::CIOgreWindowOverlay
{
    public:

    static const CORE::CString ClassTypeName;

    /** Takes the render interface, which must be initialised, and registers as an overlay of the Ogre window */
    CRmlUIOgreContext( GUI::CGUIDriver* driver                 ,
                       GUI::TWindowContextPtr windowContext    ,
                       CRmlUIOgreRenderInterface* renderer     );

    virtual ~CRmlUIOgreContext();

    CRmlUIOgreRenderInterface* GetRenderInterface( void ) const;

    GUIDRIVEROGRE::COgreWindowContext* GetOgreWindowContext( void ) const;

    /**
     *  Makes the image 'alias' stand for the Ogre texture: drawing commands that draw the image draw the texture,
     *  without a copy. The image has the size 1 x 1, so its whole area is the source rectangle (0, 0, 1, 1). The
     *  context keeps the texture until the alias is unregistered. Assigning another texture replaces it
     */
    bool RegisterOgreTexture( const CORE::CString& alias, const Ogre::TexturePtr& texture );

    bool UnregisterOgreTexture( const CORE::CString& alias );

    Ogre::TexturePtr GetRegisteredOgreTexture( const CORE::CString& alias ) const;

    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem ,
                                Ogre::Viewport& viewport         ,
                                Ogre::SceneManager* sceneManager ,
                                bool isCapture                   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GenerateImageTexture( const CORE::CString& reference                             ,
                                       bool isFont                                          ,
                                       const IMAGE::CPixelMap& pixels                       ,
                                       const Rml::CallbackTextureInterface& textureInterface ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, Ogre::TexturePtr > TOgreTextureMap;

    CRmlUIOgreContext( const CRmlUIOgreContext& src );
    CRmlUIOgreContext& operator=( const CRmlUIOgreContext& src );

    private:

    CRmlUIOgreRenderInterface* m_renderer;
    TOgreTextureMap m_ogreTextures;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERRMLUIOGRE_CRMLUIOGRECONTEXT_H ? */

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

#ifndef GUIDRIVERIMGUIOGRE_CIMGUIOGRECONTEXT_H
#define GUIDRIVERIMGUIOGRE_CIMGUIOGRECONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreTexture.h>

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#include "guidriverImGui_CImGuiContext.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */

#ifndef GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#include "guidriverOgre_CIOgreWindowOverlay.h"
#define GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H
#endif /* GUIDRIVEROGRE_CIOGREWINDOWOVERLAY_H ? */

#ifndef GUIDRIVERIMGUIOGRE_MACROS_H
#include "guidriverImGuiOgre_macros.h"
#define GUIDRIVERIMGUIOGRE_MACROS_H
#endif /* GUIDRIVERIMGUIOGRE_MACROS_H ? */

namespace GUCEF { namespace GUIDRIVEROGRE { class COgreWindowContext; } }

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

class CImGuiOgreRenderer;

/**
 *  A Dear ImGui context drawn through Ogre in an Ogre window (window backend "Ogre"), over the viewports of the
 *  application.
 *
 *  It is an overlay of its window (CIOgreWindowOverlay) with its GUI layer as the overlay order. In a frame of the window
 *  the redraw event lets it build its ImGui frame (OnRender(), BuildFrameEvent); RenderDrawData() serves the texture
 *  requests of the frame at once and keeps the draw data, which the window lets it draw when it draws its overlays, after
 *  the viewports (RenderOverlay()). The draw data is the one of the ImGui context and is valid until the next frame.
 *  The logical picture is drawn on the present rectangle of the window at the present scale, as for the OpenGL driver;
 *  the window is not cleared by it.
 *
 *  CaptureFrame() draws the last draw data again, alone, into a render texture of the render size on opaque black and
 *  reads it back. CapturePresentedFrame() draws a frame of the window and reads the window back before it is presented.
 *
 *  Ogre textures of the application are shown with ImGui::Image( GetTextureRef( texture ), ... ); the application keeps
 *  them alive while they are drawn.
 */
class GUIDRIVERIMGUIOGRE_PUBLIC_CPP CImGuiOgreContext : public GUIDRIVERIMGUI::CImGuiContext ,
                                                         public GUIDRIVEROGRE::CIOgreWindowOverlay
{
    public:

    static const CORE::CString ClassTypeName;

    /** Makes the context for an Ogre window and registers it as an overlay of the window; IsRendererReady() tells whether it worked */
    CImGuiOgreContext( GUIDRIVERIMGUI::CImGuiDriver* driver, GUI::TWindowContextPtr windowContext );

    virtual ~CImGuiOgreContext();

    /** The renderer and the GUI material are made and the context is an overlay of the window: it can draw */
    bool IsRendererReady( void ) const;

    CImGuiOgreRenderer* GetRenderer( void ) const;

    GUIDRIVEROGRE::COgreWindowContext* GetOgreWindowContext( void ) const;

    /** An Ogre texture of the application as a texture reference for ImGui::Image(): its resource handle is the ImTextureID */
    ImTextureRef GetTextureRef( const Ogre::TexturePtr& texture ) const;

    /** How often the window let this context draw its overlay (not the captures) */
    UInt32 GetOverlayDrawCount( void ) const;

    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetOverlayOrder( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void RenderOverlay( Ogre::RenderSystem& renderSystem ,
                                Ogre::Viewport& viewport         ,
                                Ogre::SceneManager* sceneManager ,
                                bool isCapture                   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnRendererNewFrame( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RenderDrawData( ImDrawData& drawData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReleaseRenderer( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CImGuiOgreContext( const CImGuiOgreContext& src );
    CImGuiOgreContext& operator=( const CImGuiOgreContext& src );

    private:

    CImGuiOgreRenderer* m_renderer;
    const ImDrawData* m_drawData;       /**< what the overlay draws: of the last frame, valid until the next one */
    UInt32 m_overlayDrawCount;
    bool m_registered;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_CIMGUIOGRECONTEXT_H ? */

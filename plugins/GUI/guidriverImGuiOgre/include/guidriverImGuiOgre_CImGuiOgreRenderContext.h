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

#ifndef GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H
#define GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreTexture.h>

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUIDRIVERIMGUIOGRE_MACROS_H
#include "guidriverImGuiOgre_macros.h"
#define GUIDRIVERIMGUIOGRE_MACROS_H
#endif /* GUIDRIVERIMGUIOGRE_MACROS_H ? */

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
 *  Shows an Ogre texture of the application (a render texture of its own camera, a video frame on the GPU) in a
 *  RenderContext or DrawSurface widget of a context of the driver ImGuiOgre, without a copy: the texture is registered
 *  with the context (CImGuiOgreContext::RegisterOgreTexture()) as an image that the drawing of the widget names, and the
 *  context draws that image through the texture reference of the texture (CImGuiOgreContext::GetTextureRef()).
 *  Rendering into the texture is the application's Ogre code; the widget shows what the texture holds when the window
 *  draws, and the context keeps the texture alive until the widget is cleared or the context goes.
 */
class GUIDRIVERIMGUIOGRE_PUBLIC_CPP CImGuiOgreRenderContext
{
    public:

    /** The name of the image that stands for the texture of the widget */
    static CORE::CString GetImageAlias( const GUI::CWidget& widget );

    /**
     *  The widget shows the texture in 'target' (widget coordinates in logical pixels): its drawing is replaced by
     *  the whole texture scaled into the rectangle. False when the widget is not in a context of ImGuiOgre or the
     *  texture is NULL
     */
    static bool SetOgreTexture( GUI::CDrawSurface& widget        ,
                                const Ogre::TexturePtr& texture  ,
                                const GUI::CRect& target         );

    /** The widget no longer shows a texture: its drawing is cleared and the texture let go */
    static bool ClearOgreTexture( GUI::CDrawSurface& widget );

    /** The texture the widget shows, NULL for none */
    static Ogre::TexturePtr GetOgreTexture( const GUI::CDrawSurface& widget );

    private:

    CImGuiOgreRenderContext( void );                                            /**< not implemented: static functions only */
    CImGuiOgreRenderContext( const CImGuiOgreRenderContext& src );              /**< not implemented */
    CImGuiOgreRenderContext& operator=( const CImGuiOgreRenderContext& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_CIMGUIOGRERENDERCONTEXT_H ? */

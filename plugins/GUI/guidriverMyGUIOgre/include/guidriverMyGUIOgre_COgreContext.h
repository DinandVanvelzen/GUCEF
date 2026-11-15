/*
 *  guidriverMyGUIOgre: GUI backend using MyGUI with Ogre
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

#ifndef GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H
#define GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#include "guidriverMyGUI_CMyGUIContext.h"
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */

#ifndef GUCEF_guidriverMyGUIOgre_MACROS_H
#include "guidriverMyGUIOgre_macros.h"
#define GUCEF_guidriverMyGUIOgre_MACROS_H
#endif /* GUCEF_guidriverMyGUIOgre_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Ogre { class RenderWindow; class SceneManager; }
namespace MyGUI { class OgreRenderManager; }

namespace GUCEF {
namespace guidriverMyGUIOgre {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A MyGUI context that draws inside an Ogre render window. The Ogre platform of MyGUI listens to the render
 *  queue of the scene manager and draws the widgets in the overlay queue of the first viewport of the window,
 *  once per Ogre frame: the application renders the window (through the frame order of guidriverOgre), the
 *  context only brings the widgets up to date. The picture is the size of the window, one logical pixel for one
 *  pixel of the window.
 */
class GUCEF_guidriverMyGUIOgre_PUBLIC_CPP COgreContext : public guidriverMyGUI::CMyGUIContext
{
    public:

    COgreContext( GUI::CGUIDriver* driver              ,
                  GUI::TWindowContextPtr windowContext ,
                  Ogre::RenderWindow* renderWindow     ,
                  Ogre::SceneManager* sceneManager     );

    virtual ~COgreContext();

    bool IsValid( void ) const;

    /** The Ogre platform of MyGUI uses PF_A8R8G8B8 for its 32 bit textures */
    virtual bool IsBgraTextureMemory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The render window as it is now, without the next frame: what the user sees (the scene and the widgets) */
    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgreContext( const COgreContext& src );
    COgreContext& operator=( const COgreContext& src );

    private:

    Ogre::RenderWindow* m_renderWindow;
    MyGUI::OgreRenderManager* m_renderManager;
    bool m_isValid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOgre */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOgre_COGRECONTEXT_H ? */

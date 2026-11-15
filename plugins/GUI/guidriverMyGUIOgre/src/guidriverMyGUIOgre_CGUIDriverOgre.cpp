/*
 *  guidriverMyGUIOgre: glue module for the MyGUI GUI backend using OpenGL
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_MYGUIOGRE_CGUICONTEXTOGRE_H
#include "guidriverMyGUIOgre_CGUIContextOgre.h"
#define GUCEF_MYGUIOGRE_CGUICONTEXTOGRE_H
#endif /* GUCEF_MYGUIOGRE_CGUICONTEXTOGRE_H ? */

#include "guidriverMyGUIOgre_CGUIDriverOgre.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIDriverOgre::CGUIDriverOgre( void )
    : MYGUI::CMyGUIDriver()       ,
      m_contextSet()              ,
      m_fontTypes()               ,
      m_myGUI()                   ,
      m_renderManager()           ,
      //m_imageLoader()             ,
      m_logAdapter()              ,
      m_dataManager()             ,
      m_myGuiInitialized( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUIDriverOgre::~CGUIDriverOgre()
{GUCEF_TRACE;

    m_contextSet.clear();
    m_myGUI.shutdown();
}

/*-------------------------------------------------------------------------*/

MyGUI::Gui*
CGUIDriverOgre::GetMyGui( void )
{
    return &m_myGUI;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CGUIDriverOgre::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    // Lazy initialize if needed
    if ( !m_myGuiInitialized )
    {
        Ogre::RenderWindow* renderWindow = nullptr;
        CORE::CString renderWindowPtrStr = windowContext->GetProperty( "Ogre::RenderWindow" );
        if ( !renderWindowPtrStr.IsNULLOrEmpty() )
        {
            renderWindow = static_cast< Ogre::RenderWindow* >( CORE::StringToPointer( renderWindowPtrStr ) );
        }
        Ogre::SceneManager* sceneManager = nullptr;
        CORE::CString sceneManagerPtrStr = windowContext->GetProperty( "Ogre::SceneManager" );
        if ( !sceneManagerPtrStr.IsNULLOrEmpty() )
        {
            sceneManager = static_cast< Ogre::SceneManager* >( CORE::StringToPointer( sceneManagerPtrStr ) );
        }

        m_renderManager.initialise( renderWindow, sceneManager );
        
        Ogre::Viewport* viewport = nullptr;
        if ( renderWindow->getNumViewports() > 0 )
            viewport = renderWindow->getViewport( 0 );
        if ( nullptr == viewport )
        {
            Ogre::Camera* camera = sceneManager->createCamera( "MyGUI" );
            viewport = renderWindow->addViewport( camera );
        }
        
        // MyGUI uses Overlays, ensure that feature is enabled
        viewport->setOverlaysEnabled( true );

        m_myGUI.initialise();        

        m_myGuiInitialized = true;
    }
    
    // Create an input context using the default driver and set it for this GUI context so that we can interact
    // with the GUI based on input events. Note that in order to correctly tie in the input system we have to pass the
    // correct parameters for the O/S
    CORE::CValueList inputContextParams;
    inputContextParams.Set( "WINDOW", windowContext->GetProperty( "WINDOW" ) );
    INPUT::CInputContext* inputContext = INPUT::CInputGlobal::Instance()->GetInputController().CreateContext( inputContextParams );

    // Add a reference to the context in our set
    GUI::TGuiContextPtr guiContextPtr( new CGUIContextOgre( *this            ,
                                                            &m_renderManager ,
                                                            windowContext    ,
                                                            inputContext     ) );
    m_contextSet.insert( guiContextPtr );

    return guiContextPtr;
}

/*-------------------------------------------------------------------------*/

GUI::CGUIDriver::TGUIContextSet
CGUIDriverOgre::GetContextList( void )
{GUCEF_TRACE;

    return m_contextSet;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CGUIDriverOgre::GetContextCount( void )
{GUCEF_TRACE;

    return (GUI::UInt32) m_contextSet.size();
}

/*-------------------------------------------------------------------------*/

GUI::CString
CGUIDriverOgre::GetDriverName( void )
{GUCEF_TRACE;

    return "MyGUIOgre";
}

/*-------------------------------------------------------------------------*/

GUI::CGUIDriver::TStringSet
CGUIDriverOgre::GetAvailableFonts( void )
{GUCEF_TRACE;

    return m_fontTypes;
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverOgre::LoadFontFromAsset( const GUI::CString& assetPath )
{GUCEF_TRACE;

    //// Simple pass on the load request to Rocket, all the callback inerfaces will take
    //// care of integrating with GUCEF
    //if ( Rocket::Core::FontDatabase::LoadFontFace( assetPath.C_String() ) )
    //{
    //    // @TODO: fix somehow
    //    // This is not correct but at present Rocket cant give you the name of the font back
    //    m_fontTypes.insert( assetPath );
    //    return true;
    //}
    return false;
}

/*-------------------------------------------------------------------------*/

const GUI::CString&
CGUIDriverOgre::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static GUI::CString classTypeName = "GUCEF::MYGUIOGRE::CGUIDriverOgre";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CGUIDriverOgre::GetDriverProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    return GUI::CString();
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverOgre::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverOgre::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

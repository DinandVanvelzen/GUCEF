/*
 *  guidriverCEGUIOpenGL: glue module for the MyGUI GUI backend using OpenGL
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

#ifndef __RenderTarget_H__
#include "OgreRenderTarget.h"
#define __RenderTarget_H__
#endif /* __RenderTarget_H__ ? */

#ifndef __SceneManager_H__
#include "OgreSceneManager.h"
#define __SceneManager_H__
#endif /* __SceneManager_H__ ? */

#ifndef __Viewport_H__
#include "OgreViewport.h"
#define __Viewport_H__
#endif /* __Viewport_H__ ? */

#ifndef __Camera_H__
#include "OgreCamera.h"
#define __Camera_H__
#endif /* __Camera_H__ ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_INPUT_CINPUTGLOBAL_H
#include "gucefINPUT_CInputGlobal.h"
#define GUCEF_INPUT_CINPUTGLOBAL_H
#endif /* GUCEF_INPUT_CINPUTGLOBAL_H ? */

#ifndef GUCEF_INPUT_CINPUTCONTROLLER_H
#include "CInputController.h"
#define GUCEF_INPUT_CINPUTCONTROLLER_H
#endif /* GUCEF_INPUT_CINPUTCONTROLLER_H ? */

#ifndef GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H
#include "guidriverCEGUIOgre_CGUIContextOgre.h"
#define GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H
#endif /* GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H ? */

#include "guidriverCEGUIOgre_CGUIDriverOgre.h"

#ifndef GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H
#include "guidriverCEGUI_CLogAdapter.h"
#define GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H
#endif /* GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H ? */

#ifndef _CEGUIOgreRenderer_h_
#include "CEGUI/RendererModules/Ogre/Renderer.h"
#define _CEGUIOgreRenderer_h_
#endif /* _CEGUIOgreRenderer_h_ */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIDriverOgre::CGUIDriverOgre( void )
    : GUIDRIVERCEGUI::CCEGUIDriver()       ,
      m_contextSet()              ,
      m_logAdapter()              ,
      m_ceGuiInitialized( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUIDriverOgre::~CGUIDriverOgre()
{GUCEF_TRACE;

    m_contextSet.clear();

    if ( GUCEF_NULL != m_guiSystem )
        m_guiSystem->destroy();
}

/*-------------------------------------------------------------------------*/

CEGUI::System*
CGUIDriverOgre::GetCEGui( void )
{
    return m_guiSystem;
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverOgre::Init( GUI::TWindowContextPtr windowContext )
{
    if ( !m_ceGuiInitialized )
    {
        try
        {
            Ogre::RenderTarget* renderTarget = GUCEF_NULL;
            CORE::CString renderTargetPtrStr = windowContext->GetProperty( "Ogre::RenderTarget" );
            if ( !renderTargetPtrStr.IsNULLOrEmpty() )
            {
                renderTarget = static_cast< Ogre::RenderTarget* >( CORE::StringToPointer( renderTargetPtrStr ) );
                if ( GUCEF_NULL == renderTarget )
                    return false;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CGUIDriverOgre:Init: No \"Ogre::RenderTarget\" has NOT been set on the Window context, unable to proceed" );
                return false;
            }
            Ogre::SceneManager* sceneManager = GUCEF_NULL;
            CORE::CString sceneManagerPtrStr = windowContext->GetProperty( "Ogre::SceneManager" );
            if ( !sceneManagerPtrStr.IsNULLOrEmpty() )
            {
                sceneManager = static_cast< Ogre::SceneManager* >( CORE::StringToPointer( sceneManagerPtrStr ) );
                if ( GUCEF_NULL == sceneManager )
                    return false;
            }
            
            // Auto-create a viewport here if none exists yet
            unsigned short viewportCount = renderTarget->getNumViewports();
            if ( 0 == viewportCount )
            {
                Ogre::Camera* camera = sceneManager->createCamera( "CEGUI" );
                camera->setPosition( Ogre::Vector3( 0, 0, 500 ) );
                camera->lookAt( Ogre::Vector3( 0, 0, -300 ) );
                camera->setNearClipDistance( 5 );

                // Create a viewport covering whole window
                Ogre::Viewport* viewport = renderTarget->addViewport( camera );
                viewport->setBackgroundColour( Ogre::ColourValue( 0.0f, 0.0f, 0.0f, 0.0f ) );
                viewport->setOverlaysEnabled( true );
                                                  
                // Update the camera aspect ratio to that of the viewport
                camera->setAspectRatio( Ogre::Real( viewport->getActualWidth() ) / Ogre::Real( viewport->getActualHeight() ) );                
            }

            CEGUI::Sizef displaySize( (float) windowContext->GetWidth(), (float) windowContext->GetHeight() );
            m_guiRenderer = &CEGUI::OgreRenderer::create( *renderTarget );// displaySize );//, CEGUI::OpenGLRenderer::TTT_AUTO );
            m_guiRenderer->setDefaultRootRenderTarget( *renderTarget );
            m_guiSystem = &CEGUI::System::create( *m_guiRenderer, &m_vfsResourceProvider, &m_xmlParserAdapter, m_imageCodecAdapter );

            // setup default group for validation schemas
            CEGUI::XMLParser* parser = m_guiSystem->getXMLParser();
            if ( GUCEF_NULL != parser && parser->isPropertyPresent( "SchemaDefaultResourceGroup" )  )
                parser->setProperty( "SchemaDefaultResourceGroup", m_schemasResourceGroup );

            // Load the fonts
            CEGUI::FontManager::getSingleton().createAll( m_defaultFont, CEGUI::Font::getDefaultResourceGroup() );
        
            // Load the scheme
            try
            {
                CEGUI::SchemeManager::getSingleton().createFromFile( "Generic.scheme" );
            }
            catch ( CEGUI::Exception& e )
            {
                CORE::CString info = e.getMessage() + " - at - " + e.getFileName() + ":" + e.getFunctionName() + ":" + CORE::UInt32ToString( e.getLine() ).STL_String();
                GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "Unhandled exception during CEGUI initialization: " + info );
            }
            CEGUI::SchemeManager::getSingleton().createFromFile( m_schemeToUse );
        
            // Set the defaults
            CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont( m_defaultFont );
            CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage( m_defaultCursorImage );
            CEGUI::Window* rootWindow = CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", "root" );
            CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow( rootWindow );
            
            // clearing this queue actually makes sure it's created(!)
            CEGUI::System::getSingleton().getDefaultGUIContext().clearGeometry( CEGUI::RQ_OVERLAY );                                             
        
            m_ceGuiInitialized = true;
        }
        catch ( CEGUI::Exception& e )
        {
            CORE::CString info = e.getMessage() + " - at - " + e.getFileName() + ":" + e.getFunctionName() + ":" + CORE::UInt32ToString( e.getLine() ).STL_String();
            GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "Unhandled exception during CEGUI initialization: " + info );

            m_ceGuiInitialized = false;
        }
    }
    return m_ceGuiInitialized;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CGUIDriverOgre::CreateGUIContext( GUI::TWindowContextPtr windowContext )
{GUCEF_TRACE;

    // Lazy initialize if needed
    if ( !Init( windowContext ) )
    {
        return GUI::TGuiContextPtr();
    }

    // Create an input context using the default driver and set it for this GUI context so that we can interact
    // with the GUI based on input events. Note that in order to correctly tie in the input system we have to pass the
    // correct parameters for the O/S
    CORE::CValueList inputContextParams;
    inputContextParams.Set( "WINDOW", windowContext->GetProperty( "WINDOW" ) );
    INPUT::CInputContext* inputContext = INPUT::CInputGlobal::Instance()->GetInputController().CreateContext( inputContextParams );

    //CEGUI::RenderTarget* renderTarget = m_guiRenderer->getActiveRenderTarget();
    //if ( GUCEF_NULL == renderTarget )
    //{
    //    renderTarget = &m_guiRenderer->getDefaultRenderTarget();
    //}
    CEGUI::RenderTarget* renderTarget = &m_guiRenderer->getDefaultRenderTarget();
    
    CEGUI::GUIContext* ceGuiContext = &m_guiSystem->createGUIContext( *renderTarget );
    ceGuiContext->setRootWindow( CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow() );
    GUI::TGuiContextPtr guiContextPtr = GUI::TGuiContextPtr( new CGUIContextOgre( *this         ,
                                                                                  windowContext ,
                                                                                  inputContext  ,
                                                                                  ceGuiContext  ) );
    // Add a reference to the context in our set
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

    return m_contextSet.size();
}

/*-------------------------------------------------------------------------*/

GUI::CString
CGUIDriverOgre::GetDriverName( void )
{GUCEF_TRACE;

    return "CEGUIOgre";
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

    return false;
}

/*-------------------------------------------------------------------------*/

const GUI::CString&
CGUIDriverOgre::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static GUI::CString classTypeName = "GUCEF::GUIDRIVERCEGUIOgre::CGUIDriverOgre";
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

    return CCEGUIDriver::SaveConfig( tree );
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverOgre::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return CCEGUIDriver::LoadConfig( treeroot );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

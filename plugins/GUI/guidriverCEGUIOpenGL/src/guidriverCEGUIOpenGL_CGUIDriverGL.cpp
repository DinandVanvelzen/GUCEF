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

#ifndef GUCEF_GUIDRIVERCEGUIGL_CGUICONTEXTGL_H
#include "guidriverCEGUIOpenGL_CGUIContextGL.h"
#define GUCEF_GUIDRIVERCEGUIGL_CGUICONTEXTGL_H
#endif /* GUCEF_GUIDRIVERCEGUIGL_CGUICONTEXTGL_H ? */

#include "guidriverCEGUIOpenGL_CGUIDriverGL.h"

#ifndef GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H
#include "guidriverCEGUI_CLogAdapter.h"
#define GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H
#endif /* GUCEF_GUIDRIVERCEGUI_CLOGADAPTER_H ? */

#ifndef _CEGUIOpenGLRenderer_h_
#include "CEGUI/RendererModules/OpenGL/GLRenderer.h"
#define _CEGUIOpenGLRenderer_h_
#endif /* _CEGUIOpenGLRenderer_h_ */

#ifndef _CEGUIOpenGL3Renderer_h_
#include "CEGUI/RendererModules/OpenGL/GL3Renderer.h"
#define _CEGUIOpenGL3Renderer_h_
#endif /* _CEGUIOpenGL3Renderer_h_ */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIDriverGL::CGUIDriverGL( void )
    : GUIDRIVERCEGUI::CCEGUIDriver()       ,
      m_contextSet()              ,
      m_logAdapter()              ,
      m_ceGuiInitialized( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGUIDriverGL::~CGUIDriverGL()
{GUCEF_TRACE;

    m_contextSet.clear();

    if ( GUCEF_NULL != m_guiSystem )
        m_guiSystem->destroy();
}

/*-------------------------------------------------------------------------*/

CEGUI::System*
CGUIDriverGL::GetCEGui( void )
{
    return m_guiSystem;
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverGL::Init( GUI::TWindowContextPtr windowContext )
{
    if ( !m_ceGuiInitialized )
    {
        try
        {
            CEGUI::Sizef displaySize( (float) windowContext->GetWidth(), (float) windowContext->GetHeight() );
            m_guiRenderer = &CEGUI::OpenGL3Renderer::create( displaySize );//, CEGUI::OpenGLRenderer::TTT_AUTO );
            m_guiSystem = &CEGUI::System::create( *m_guiRenderer, &m_vfsResourceProvider, &m_xmlParserAdapter, m_imageCodecAdapter );

            // setup default group for validation schemas
            CEGUI::XMLParser* parser = m_guiSystem->getXMLParser();
            if ( nullptr != parser && parser->isPropertyPresent( "SchemaDefaultResourceGroup" )  )
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
CGUIDriverGL::CreateGUIContext( GUI::TWindowContextPtr windowContext )
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

    CEGUI::RenderTarget* renderTarget = m_guiRenderer->getActiveRenderTarget();
    if ( nullptr == renderTarget )
    {
        renderTarget = &m_guiRenderer->getDefaultRenderTarget();
    }
    
    CEGUI::GUIContext* ceGuiContext = &m_guiSystem->createGUIContext( *renderTarget );
    ceGuiContext->setRootWindow( CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow() );
    GUI::TGuiContextPtr guiContextPtr = GUI::TGuiContextPtr( new CGUIContextGL( *this         ,
                                                                                windowContext ,
                                                                                inputContext  ,
                                                                                ceGuiContext  ) );
    // Add a reference to the context in our set
    m_contextSet.insert( guiContextPtr );

    return guiContextPtr;
}

/*-------------------------------------------------------------------------*/

GUI::CGUIDriver::TGUIContextSet
CGUIDriverGL::GetContextList( void )
{GUCEF_TRACE;

    return m_contextSet;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CGUIDriverGL::GetContextCount( void )
{GUCEF_TRACE;

    return m_contextSet.size();
}

/*-------------------------------------------------------------------------*/

GUI::CString
CGUIDriverGL::GetDriverName( void )
{GUCEF_TRACE;

    return "CEGUIOpenGL";
}

/*-------------------------------------------------------------------------*/

GUI::CGUIDriver::TStringSet
CGUIDriverGL::GetAvailableFonts( void )
{GUCEF_TRACE;

    return m_fontTypes;
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverGL::LoadFontFromAsset( const GUI::CString& assetPath )
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
CGUIDriverGL::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static GUI::CString classTypeName = "GUCEF::GUIDRIVERCEGUIGL::CGUIDriverGL";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CGUIDriverGL::GetDriverProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    return GUI::CString();
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverGL::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return CCEGUIDriver::SaveConfig( tree );
}

/*-------------------------------------------------------------------------*/

bool
CGUIDriverGL::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    return CCEGUIDriver::LoadConfig( treeroot );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

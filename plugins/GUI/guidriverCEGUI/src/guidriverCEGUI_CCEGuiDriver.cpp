/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_MACROS_H
#include "guidriverCEGUI_macros.h" 
#define GUCEF_GUIDRIVERCEGUI_MACROS_H
#endif /* GUCEF_GUIDRIVERCEGUI_MACROS_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_CFORMBACKENDIMP_H
#include "guidriverCEGUI_CFormBackendImp.h"
#define GUCEF_GUIDRIVERCEGUI_CFORMBACKENDIMP_H
#endif /* GUCEF_GUIDRIVERCEGUI_CFORMBACKENDIMP_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_WIDGETS_H
#include "guidriverCEGUI_widgets.h"
#define GUCEF_GUIDRIVERCEGUI_WIDGETS_H
#endif /* GUCEF_GUIDRIVERCEGUI_WIDGETS_H ? */

#include "guidriverCEGUI_CCEGuiDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CCEGUIDriver::CCEGUIDriver( void )
    : GUI::CGUIDriver() 
    , m_vfsResourceProvider()            
    , m_logAdapter()                     
    , m_imageCodecAdapter( 0 )           
    , m_xmlParserAdapter()               
    , m_schemeToUse()                    
    , m_defaultFont()                    
    , m_defaultCursorImage()             
    , m_schemasResourceGroup()
{GUCEF_TRACE;

    m_imageCodecAdapter = new ImageCodecAdapter( &m_vfsResourceProvider );
}

/*-------------------------------------------------------------------------*/

CCEGUIDriver::~CCEGUIDriver()
{GUCEF_TRACE;

    delete m_imageCodecAdapter;
    m_imageCodecAdapter = 0;
}

/*-------------------------------------------------------------------------*/

CCEGUIDriver::TStringSet
CCEGUIDriver::GetAvailableFormTypes( void )
{GUCEF_TRACE;

    TStringSet formsAvailable;

    formsAvailable.insert( "Form" );
    formsAvailable.insert( "FormEx" );

    return formsAvailable;
}

/*-------------------------------------------------------------------------*/

CCEGUIDriver::TStringSet
CCEGUIDriver::GetAvailableWidgetTypes( void )
{GUCEF_TRACE;

    TStringSet widgetsAvailable;

    widgetsAvailable.insert( "Widget" );
    widgetsAvailable.insert( "Window" );
    widgetsAvailable.insert( "Button" );

    return widgetsAvailable;
}

/*-------------------------------------------------------------------------*/

const GUI::CString&
CCEGUIDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static GUI::CString classTypeName = "GUCEF::GUIDRIVERCEGUI::CCEGUIDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CWidget*
CCEGUIDriver::CreateWidget( const GUI::CString& widgetName )
{GUCEF_TRACE;

    //if ( "Widget" == widgetName )
    //{
    //    MyGUI::Widget* widget = new MyGUI::Widget();
    //    TBasicWidgetImp* widgetWrapper = new TBasicWidgetImp();
    //    widgetWrapper->Hook( widget );
    //    return widgetWrapper;
    //}
    //else
    //if ( "Window" == widgetName )
    //{
    //    MyGUI::Window* widget = new MyGUI::Window();
    //    CWindowImp* widgetWrapper = new CWindowImp();
    //    widgetWrapper->Hook( widget );
    //    return widgetWrapper;
    //}
    //else
    //if ( "Button" == widgetName )
    //{
    //    MyGUI::Button* widget = new MyGUI::Button();
    //    CButtonImp* widgetWrapper = new CButtonImp();
    //    widgetWrapper->Hook( widget );
    //    return widgetWrapper;
    //}

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCEGUIDriver::DestroyWidget( GUI::CWidget* widget )
{GUCEF_TRACE;

    delete widget;
}

/*-------------------------------------------------------------------------*/

GUCEF::GUI::CFormBackend*
CCEGUIDriver::CreateFormBackend( void )
{GUCEF_TRACE;

    return new CFormBackendImp();
}

/*-------------------------------------------------------------------------*/

void
CCEGUIDriver::DestroyFormBackend( GUI::CFormBackend* formBackend )
{GUCEF_TRACE;

    delete static_cast< CFormBackendImp* >( formBackend );
}

/*-------------------------------------------------------------------------*/

bool
CCEGUIDriver::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCEGUIDriver::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* ceguiConfig = treeroot.Find( "CEGUI" );
    if ( GUCEF_NULL == ceguiConfig )
    {   
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CGUIDriver:LoadConfig: Cannot find CEGUI config and as such will not be able to initialize" );
        return false;
    }

    m_schemeToUse = ceguiConfig->GetAttributeValueOrChildValueByName( "Scheme" );
    m_defaultFont = ceguiConfig->GetAttributeValueOrChildValueByName( "DefaultFont" );
    m_defaultCursorImage = ceguiConfig->GetAttributeValueOrChildValueByName( "DefaultCursorImage" );
     
    // the default is the fallback resource group if none is passed to the adapter
    CORE::CString defaultResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "DefaultResourceGroup" );
    if ( defaultResourceGroup.IsNULLOrEmpty() )
        defaultResourceGroup = "CEGUI";
    m_vfsResourceProvider.setDefaultResourceGroup( defaultResourceGroup );

    CORE::CString imageSetsResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "ImageSetsResourceGroup" );
    CORE::CString fontsResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "FontsResourceGroup" );
    CORE::CString schemesResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "SchemesResourceGroup" );
    CORE::CString lookNFeelResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "LookNFeelsResourceGroup" );
    CORE::CString layoutsResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "LayoutsResourceGroup" );
    CORE::CString animationsResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "AnimationsResourceGroup" );
    CORE::CString scriptsResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "ScriptsResourceGroup" );
    m_schemasResourceGroup = ceguiConfig->GetAttributeValueOrChildValueByName( "SchemasResourceGroup" );

    // set the default resource groups to be used
    CEGUI::ImageManager::setImagesetDefaultResourceGroup( imageSetsResourceGroup );
    CEGUI::Font::setDefaultResourceGroup( fontsResourceGroup );
    CEGUI::Scheme::setDefaultResourceGroup( schemesResourceGroup );
    CEGUI::WidgetLookManager::setDefaultResourceGroup( lookNFeelResourceGroup );
    CEGUI::WindowManager::setDefaultResourceGroup( layoutsResourceGroup );
    CEGUI::AnimationManager::setDefaultResourceGroup( animationsResourceGroup );
    CEGUI::ScriptModule::setDefaultResourceGroup( scriptsResourceGroup );

    // setup default group for validation schemas
    CEGUI::System* ceguiSystem = CEGUI::System::getSingletonPtr();
    if ( GUCEF_NULL != ceguiSystem )
    {
        CEGUI::XMLParser* parser = ceguiSystem->getXMLParser();
        if ( GUCEF_NULL != parser && parser->isPropertyPresent( "SchemaDefaultResourceGroup" )  )
            parser->setProperty( "SchemaDefaultResourceGroup", m_schemasResourceGroup );
    }

    const CORE::CDataNode* ceguiVfsConfig = treeroot.Search( "CEGUI/VFSAdapter", '/', false );
    if ( GUCEF_NULL != ceguiVfsConfig )
    {   
        CORE::CDataNode::TConstDataNodeSet grpNodes = ceguiVfsConfig->FindChildrenOfType( "ResourceGroup" );
        CORE::CDataNode::TConstDataNodeSet::iterator i = grpNodes.begin();
        while ( i != grpNodes.end() )
        {
            CORE::CString groupName = (*i)->GetAttributeValueOrChildValueByName( "Name" );
            CORE::CString vfsPath = (*i)->GetAttributeValueOrChildValueByName( "Path" );

            m_vfsResourceProvider.setResourceGroupDirectory( groupName, vfsPath );

            ++i;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

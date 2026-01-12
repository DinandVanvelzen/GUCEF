///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU Lesser General Public
// *  License along with this library; if not, write to the Free Software
// *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
// */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUIPropertyHelper_h_
//#include "CEGUIPropertyHelper.h"
//#define _CEGUIPropertyHelper_h_
//#endif /* _CEGUIPropertyHelper_h_ ? */
//
//#ifndef _CEGUIImagesetManager_h_
//#include "CEGUIImagesetManager.h"
//#define _CEGUIImagesetManager_h_
//#endif /* _CEGUIImagesetManager_h_ ? */
//
//#ifndef GUCE_GUI_OGRECEGUITEXTURE_H
//#include "guceGUI_OgreCEGUITexture.h"
//#define GUCE_GUI_OGRECEGUITEXTURE_H
//#endif /* GUCE_GUI_OGRECEGUITEXTURE_H ? */
//
//#ifndef GUCE_GUI_CGUIMANAGER_H
//#include "CGUIManager.h"
//#define GUCE_GUI_CGUIMANAGER_H
//#endif /* GUCE_GUI_CGUIMANAGER_H ? */
//
//#ifndef GUCE_GUI_OGRECEGUIRENDERER_H
//#include "guceGUI_OgreCEGUIRenderer.h"
//#define GUCE_GUI_OGRECEGUIRENDERER_H
//#endif /* GUCE_GUI_OGRECEGUIRENDERER_H ? */
//
//#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
//#include "dvcppstringutils.h"
//#define GUCEF_CORE_DVCPPSTRINGUTILS_H
//#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
//
//#include "guceMyGUIOgre_CRenderContextImp.h"
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCE {
//namespace MYGUIOGRE {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CRenderContextImp::CRenderContextImp( void )
//    : CWidgetImp< GUI::COgreGUIRenderContext >() ,
//      m_imageWindow( NULL )                      ,
//      m_rttImageSet( NULL )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//    
//CRenderContextImp::~CRenderContextImp()
//{GUCE_TRACE;
//
//    delete static_cast< GUI::OgreCEGUITexture* >( m_renderTexture );
//    m_renderTexture = NULL;
//    
//    CEGUI::ImagesetManager::getSingleton().destroyImageset( m_rttImageSet );
//    m_rttImageSet = NULL;
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CRenderContextImp::Refresh( void )
//{GUCE_TRACE;
//
//    m_imageWindow->requestRedraw();
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CRenderContextImp::SetRenderTexture( Ogre::TexturePtr renderTexture )
//{GUCE_TRACE;
//
//    if ( COgreGUIRenderContext::SetRenderTexture( renderTexture ) )
//    {
//        Initialize();
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CRenderContextImp::Hook( CEGUI::Window* imageWindow )
//{GUCE_TRACE;
//
//    CWidgetImp< GUI::COgreGUIRenderContext >::Hook( imageWindow );
//    m_imageWindow = imageWindow;
//        
//    Initialize();
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CRenderContextImp::Initialize( void )
//{GUCE_TRACE;
//
//    Ogre::TexturePtr ogreRenderTexture = GetRenderTexture();
//    
//    if ( ( NULL != m_imageWindow )     &&
//         ( !ogreRenderTexture.isNull() ) )
//    {       
//        // Create a CEGUI texture using the Ogre texture
//        m_renderTexture = GUI::CGUIManager::Instance()->GetRenderer()->createTexture( (CEGUI::utf8*) ogreRenderTexture->getName().c_str(), "GUI" );
//
//        // Create the name prefix we will use to make the resources unique
//        CString uniqueID = GUCEF::CORE::PointerToString( this );
//            
//        // Create an image set as a placeholder for our texture
//        m_rttImageSet = CEGUI::ImagesetManager::getSingleton().createImageset( (CEGUI::utf8*) CString( uniqueID + "RttImageset" ).C_String(), m_renderTexture );
//        
//        // define a single image in the image set that covers everything
//        m_rttImageSet->defineImage( (CEGUI::utf8*) "RttImage"                          , 
//                                    CEGUI::Point( 0.0f, 0.0f )                         ,
//                                    CEGUI::Size( m_renderTexture->getWidth()  , 
//                                                 m_renderTexture->getHeight() )        ,
//                                    CEGUI::Point( 0.0f, 0.0f )                         );        
//    
//        // Make sure the image property is set to use our rendering texture
//        m_imageWindow->setProperty( "Image", CEGUI::PropertyHelper::imageToString( &m_rttImageSet->getImage( (CEGUI::utf8*) "RttImage" ) ) );
//    }
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace MYGUIOGRE */
//}; /* namespace GUCE */
//
///*-------------------------------------------------------------------------*/


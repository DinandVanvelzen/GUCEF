///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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
//#ifndef _Texture_H__
//#include "OgreTexture.h"
//#define _Texture_H__
//#endif /* _Texture_H__ ? */
//
//#ifndef _CEGUIImagesetManager_h_
//#include "CEGUIImageSetManager.h"
//#define _CEGUIImagesetManager_h_
//#endif /* _CEGUIImagesetManager_h_ ? */
//
//#ifndef _CEGUIImageset_h_
//#include "CEGUIImageSet.h"
//#define _CEGUIImageset_h_
//#endif /* _CEGUIImageset_h_ ? */
//
//#ifndef _CEGUIPropertyHelper_h_
//#include "CEGUIPropertyHelper.h"
//#define _CEGUIPropertyHelper_h_
//#endif /* _CEGUIPropertyHelper_h_ ? */
//
//#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
//#include "dvcppstringutils.h"
//#define GUCEF_CORE_DVCPPSTRINGUTILS_H
//#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
//
//#ifndef GUCE_GUI_CGUIMANAGER_H
//#include "CGUIManager.h"
//#define GUCE_GUI_CGUIMANAGER_H
//#endif /* GUCE_GUI_CGUIMANAGER_H ? */
//
//#ifndef GUCE_CORE_MISCUTILS_H
//#include "miscutils.h"
//#define GUCE_CORE_MISCUTILS_H
//#endif /* GUCE_CORE_MISCUTILS_H ? */
//
//#ifndef GUCE_GUI_OGRECEGUITEXTURE_H
//#include "guceGUI_OgreCEGUITexture.h"
//#define GUCE_GUI_OGRECEGUITEXTURE_H
//#endif /* GUCE_GUI_OGRECEGUITEXTURE_H ? */
//
//#include "guceMyGUIOgre_CImageFrameImp.h"
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
//CImageFrameImp::CImageFrameImp( void )
//    : CWidgetImp< GUCEF::GUI::CImageFrame >() ,
//      m_imageSet( NULL )                      ,
//      m_texture( NULL )
//{GUCE_TRACE;
//
//    m_texture = new GUI::OgreCEGUITexture( GUI::CGUIManager::Instance()->GetRenderer() );
//}
//
///*-------------------------------------------------------------------------*/
//    
//CImageFrameImp::~CImageFrameImp()
//{GUCE_TRACE;
//
//    // Cleanup any existing imageSet
//    if ( NULL != m_imageSet )
//    {
//        CEGUI::ImagesetManager::getSingletonPtr()->destroyImageset( m_imageSet );
//        m_imageSet = NULL;
//    }
//
//    delete m_texture;
//    m_texture = NULL;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CImageFrameImp::SetImage( const GUCEF::IMAGE::CImage& srcImage )
//{GUCE_TRACE;
//
//    CEGUI::Window* window = GetCEGUIWindow();
//    if ( NULL != window )
//    {
//        // First we convert the image into an Ogre texture
//        CString imageName( "GUCE::CEGUIOGRE::CImageFrameImp::" + GUCEF::CORE::PointerToString( this ) );
//        Ogre::TexturePtr ogreTexture = CORE::ImageToTexture( imageName ,
//                                                             "GUI"     ,
//                                                             srcImage  );
//
//        if ( !ogreTexture.isNull() )
//        {
//            // Now that we have the Ogre texture we can provide a CEGUI texture to the window
//            m_texture->setOgreTexture( ogreTexture );
//
//            // Cleanup any existing imageset
//            CEGUI::ImagesetManager* imagesetManager = CEGUI::ImagesetManager::getSingletonPtr();
//            if ( NULL != m_imageSet )
//            {
//                imagesetManager->destroyImageset( m_imageSet );
//                m_imageSet = NULL;
//            }
//
//            // Create an image in the image set that covers the entire texture
//            m_imageSet = imagesetManager->createImageset( imageName.C_String(), m_texture );
//            m_imageSet->defineImage( imageName.C_String(), CEGUI::Rect( 0.0, 0.0, 1.0, 1.0 ), CEGUI::Point( 0.0, 0.0 ) ); 
//            
//            // Set the image property, this will cause the image to be displayed
//            window->setProperty( "Image", CEGUI::PropertyHelper::imageToString( &m_imageSet->getImage( (CEGUI::utf8*) imageName.C_String() ) ) );
//
//            return true;
//        }
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CImageFrameImp::Hook( CEGUI::Window* window )
//{GUCE_TRACE;
//
//    CWidgetImp< GUCEF::GUI::CImageFrame >::Hook( window );
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//} /* namespace MYGUIOGRE */
//} /* namespace GUCE */
//
///*-------------------------------------------------------------------------*/


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
//#ifndef GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H
//#define GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUIImageset_h_
//#include "CEGUIImageset.h"
//#define _CEGUIImageset_h_
//#endif /* _CEGUIImageset_h_ ? */
//
//#ifndef _CEGUITexture_h_
//#include "CEGUITexture.h"
//#define _CEGUITexture_h_
//#endif /* _CEGUITexture_h_ ? */
//
//#ifndef GUCE_GUI_COGREGUIRENDERCONTEXT_H
//#include "guceGUI_COgreGUIRenderContext.h"
//#define GUCE_GUI_COGREGUIRENDERCONTEXT_H
//#endif /* GUCE_GUI_COGREGUIRENDERCONTEXT_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
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
////      CLASSES                                                            //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//class GUCE_MYGUIOGRE_EXPORT_CPP CRenderContextImp : public CWidgetImp< GUI::COgreGUIRenderContext >
//{    
//    public:
//    
//    CRenderContextImp( void );
//    
//    virtual ~CRenderContextImp();
//    
//    virtual void Refresh( void );
//
//    virtual bool SetRenderTexture( Ogre::TexturePtr renderTexture );
//    
//    virtual void Hook( CEGUI::Window* imageWindow );
//    
//    private:
//    
//    CRenderContextImp( const CRenderContextImp& src );
//    CRenderContextImp& operator=( const CRenderContextImp& src );
//
//    void Initialize( void );
//    
//    private:
//    
//    CEGUI::Window* m_imageWindow;
//    CEGUI::Texture* m_renderTexture;
//    CEGUI::Imageset *m_rttImageSet;
//};
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
//          
//#endif /* GUCE_MYGUIOGRE_CRENDERCONTEXTIMP_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      Info & Changes                                                     //
////                                                                         //
////-------------------------------------------------------------------------//
//
//- 18-08-2007 :
//        - Dinand: Initial implementation
//
//-----------------------------------------------------------------------------*/


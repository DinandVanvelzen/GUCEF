///*
// *  guidriverRmlUI: GUI backend using RmlUI
// *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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
//#ifndef GUCE_MYGUIOGRE_CBUTTONIMP_H
//#define GUCE_MYGUIOGRE_CBUTTONIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CBUTTON_H
//#include "gucefGUI_CButton.h"
//#define GUCEF_GUI_CBUTTON_H
//#endif /* GUCEF_GUI_CBUTTON_H ? */
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
//class GUCE_MYGUIOGRE_EXPORT_CPP CButtonImp : public CWidgetImp< GUCEF::GUI::CButton >
//{
//    public:
//       
//    CButtonImp( void );
//    
//    virtual ~CButtonImp();
//    
//    virtual bool SetButtonText( const CString& newText );
//    
//    virtual bool GetButtonText( CString& text ) const;
//    
//    virtual void Hook( MyGUI::Button* button );  
//
//    protected:
//
//    virtual void OnMouseButtonPressed( MyGUI::Widget* sender     , 
//                                       int left                  ,
//                                       int top                   ,
//                                       MyGUI::MouseButton button );
//                                       
//    virtual void OnMouseButtonReleased( MyGUI::Widget* sender     , 
//                                        int left                  ,
//                                        int top                   ,
//                                        MyGUI::MouseButton button );
//    
//    virtual void OnMouseClick( MyGUI::Widget* sender );
//    
//    private:
//    
//    CButtonImp( const CButtonImp& src );
//    CButtonImp& operator=( const CButtonImp& src );
//    
//    private:
//    
//    MyGUI::Button* m_button;
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
//#endif /* GUCE_MYGUIOGRE_CBUTTONIMP_H ? */
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
//---------------------------------------------------------------------------*/


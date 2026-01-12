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
//#ifndef GUCE_MYGUIOGRE_CWINDOWIMP_H
//#define GUCE_MYGUIOGRE_CWINDOWIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef __MYGUI_WINDOW_H__
//#include "MyGUI_Window.h"
//#define __MYGUI_WINDOW_H__
//#endif /* __MYGUI_WINDOW_H__ ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CWINDOW_H
//#include "gucefGUI_CWindow.h"
//#define GUCEF_GUI_CWINDOW_H
//#endif /* GUCEF_GUI_CWINDOW_H ? */
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
//class GUCE_MYGUIOGRE_EXPORT_CPP CWindowImp : public CWidgetImp< GUCEF::GUI::CWindow >
//{
//    public:
//    
//    CWindowImp( void );
//    
//    virtual ~CWindowImp();
//    
//    virtual bool ShowModal( void );
//    
//    virtual bool Hide( void );
//    
//    virtual bool SetTitleBarText( const CString& title );
//    
//    virtual bool GetTitleBarText( CString& title ) const;
//    
//    virtual bool SetTitleBarVisible( const bool isVisible );
//    
//    virtual bool GetTitleBarVisible( bool& isVisible );
//    
//    void Hook( MyGUI::Window* window );
//    
//    private:
//
//    CWindowImp( const CWindowImp& src );
//    CWindowImp& operator=( const CWindowImp& src );
//    
//    private:
//    
//    MyGUI::Window* m_window;
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
//#endif /* GUCE_MYGUIOGRE_CWINDOWIMP_H ? */
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


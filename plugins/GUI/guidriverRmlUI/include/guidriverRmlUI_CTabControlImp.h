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
//#ifndef GUCE_MYGUIOGRE_CTABCONTROLIMP_H
//#define GUCE_MYGUIOGRE_CTABCONTROLIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUITabControl_h_
//#include "elements/CEGUITabControl.h"
//#define _CEGUITabControl_h_
//#endif /* _CEGUITabControl_h_ ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CTABCONTROL_H
//#include "gucefGUI_CTabControl.h"
//#define GUCEF_GUI_CTABCONTROL_H
//#endif /* GUCEF_GUI_CTABCONTROL_H ? */
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
//class GUCE_MYGUIOGRE_EXPORT_CPP CTabControlImp : public CWidgetImp< GUCEF::GUI::CTabControl >
//{
//    public:
//       
//    CTabControlImp( void );
//    
//    virtual ~CTabControlImp();
//    
//    virtual GUCEF::GUI::CTabContentPane* GetTabContentPane( const UInt32 tabIndex );
//    
//    virtual bool SetActiveTab( const UInt32 tabIndex );
//    
//    virtual Int32 GetActiveTab( void ) const;
//    
//    virtual const CString& GetClassTypeName( void ) const;
//
//    virtual void Hook( CEGUI::TabControl* tabControl );   
//    
//    private:
//    
//    CTabControlImp( const CTabControlImp& src );
//    CTabControlImp& operator=( const CTabControlImp& src );
//    
//    private:
//    
//    CEGUI::TabControl* m_tabControl;
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
//#endif /* GUCE_MYGUIOGRE_CTABCONTROLIMP_H ? */
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


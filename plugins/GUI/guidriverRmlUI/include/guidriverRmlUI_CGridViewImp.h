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
//#ifndef GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//#define GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUIMultiColumnList_h_
//#include "elements/CEGUIMultiColumnList.h"
//#define _CEGUIMultiColumnList_h_
//#endif /* _CEGUIMultiColumnList_h_ ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CGRIDVIEW_H
//#include "gucefGUI_CGridView.h"
//#define GUCEF_GUI_CGRIDVIEW_H
//#endif /* GUCEF_GUI_CGRIDVIEW_H ? */
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
//class GUCE_MYGUIOGRE_EXPORT_CPP CGridViewImp : public CWidgetImp< GUCEF::GUI::CGridView >
//{
//    public:
//       
//    CGridViewImp( void );
//    
//    virtual ~CGridViewImp();
//    
//    virtual void Hook( CEGUI::MultiColumnList* grid );
//
//    virtual void SetText( const UInt32 columnIndex ,
//                          const UInt32 rowIndex    ,
//                          const CString& text      );
//
//    virtual CString GetText( const UInt32 columnIndex ,
//                             const UInt32 rowIndex    ) const;
//
//    virtual void AppendText( const UInt32 columnIndex ,
//                             const CString& text      );
//
//    virtual void AppendText( const CString& text );
//    
//    virtual void Clear( void );
//    
//    virtual void GetSelectedItems( TGridIndexVector& gridIndexList );
//    
//    virtual const CString& GetClassTypeName( void ) const;   
//    
//    bool IsThereRoomInColumn( const UInt32 columnIndex ) const;
//
//    bool TestForItemHit( const Float32 posX  ,
//                         const Float32 posY  ,
//                         UInt32& columnIndex ,
//                         UInt32& rowIndex    ) const;
//                         
//    Int32 GetFirstEmptyRowInColumn( const UInt32 columnIndex ) const;
//
//    CEGUI::MultiColumnList* GetImplementationWidget( void );
//    
//    private:
//    
//    CGridViewImp( const CGridViewImp& src );
//    CGridViewImp& operator=( const CGridViewImp& src );
//    
//    private:
//    
//    CEGUI::MultiColumnList* m_grid;
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
//#endif /* GUCE_MYGUIOGRE_CGRIDVIEWIMP_H ? */
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


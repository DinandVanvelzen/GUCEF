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
//#ifndef GUCE_MYGUIOGRE_CLISTBOXIMP_H
//#define GUCE_MYGUIOGRE_CLISTBOXIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUIListbox_h_
//#include "elements/CEGUIListbox.h"
//#define _CEGUIListbox_h_
//#endif /* _CEGUIListbox_h_ ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CLISTBOX_H
//#include "gucefGUI_CListbox.h"
//#define GUCEF_GUI_CLISTBOX_H
//#endif /* GUCEF_GUI_CLISTBOX_H ? */
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
//class CListboxImp : public CWidgetImp< GUCEF::GUI::CListbox >
//{
//    public:
//    
//    CListboxImp( void );
//    
//    virtual ~CListboxImp();
//    
//    virtual const CString& GetClassTypeName( void ) const;
//    
//    virtual bool SetAllowMultiSelect( bool allowMultiselect );
//    
//    virtual bool GetAllowMultiSelect( bool allowMultiselect );
//    
//    virtual UInt32 GetLineCount( void );
//    
//    virtual bool Clear( void );
//    
//    virtual bool GetLineAtIndex( const UInt32 lineIndex ,
//                                 CString& lineText      ) const;
//    
//    virtual bool InsertLineAtIndex( const UInt32 lineIndex ,
//                                    const CString& text    );
//    
//    virtual bool AppendLine( const CString& text );
//    
//    virtual bool GetText( CString& text ) const;
//    
//    virtual bool SetLineSelection( const UInt32 lineIndex ,
//                                   const bool isSelected  );
//
//    virtual bool GetLineSelection( const UInt32 lineIndex ,
//                                   bool& isSelected       ) const;
//                                       
//    virtual bool GetFirstSelectedLine( CString& text ) const;    
//    
//    virtual bool GetLastSelectedLine( CString& text ) const;    
//    
//    virtual bool GetSelectedLines( TStringVector& selectedLines ) const;
//    
//    virtual void Hook( CEGUI::Listbox* listbox );
//    
//    private:
//    
//    CListboxImp( const CListboxImp& src );
//    CListboxImp& operator=( const CListboxImp& src );
//    
//    private:
//    
//    CEGUI::Listbox* m_listbox;
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
//#endif /* GUCE_MYGUIOGRE_CLISTBOXIMP_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      Info & Changes                                                     //
////                                                                         //
////-------------------------------------------------------------------------//
//
//- 10-02-2008 :
//        - Dinand: Initial implementation
//
//-----------------------------------------------------------------------------*/


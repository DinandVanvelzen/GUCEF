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
//#ifndef _CEGUIListboxTextItem_h_
//#include "elements/CEGUIListboxTextItem.h"
//#define _CEGUIListboxTextItem_h_
//#endif /* _CEGUIListboxTextItem_h_ ? */
//
//#ifndef GUCEF_CORE_CTRACER_H
//#include "CTracer.h"
//#define GUCEF_CORE_CTRACER_H
//#endif /* GUCEF_CORE_CTRACER_H ? */
//
//#include "guceMyGUIOgre_CListboxImp.h"
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
//class ListItem : public CEGUI::ListboxTextItem
//{
//    public:
//    
//	ListItem( const CEGUI::String& text ) 
//	        : ListboxTextItem( text )
//	{GUCE_TRACE;
//		setSelectionBrushImage( (CEGUI::utf8*) "TaharezLook"             , 
//		                        (CEGUI::utf8*) "MultiListSelectionBrush" );
//	}
//	
//	virtual ~ListItem()
//	{GUCE_TRACE;
//	}
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CListboxImp::CListboxImp( void )
//    : CWidgetImp< GUCEF::GUI::CListbox >() ,
//      m_listbox( NULL )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CListboxImp::~CListboxImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//const CString&
//CListboxImp::GetClassTypeName( void ) const
//{GUCE_TRACE;
//
//    static CString typeName = "GUCE::CEGUIOGRE::CListboxImp";
//    return typeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CListboxImp::Hook( CEGUI::Listbox* listbox )
//{GUCE_TRACE;
//
//    m_listbox = listbox;
//    CWidgetImp< GUCEF::GUI::CListbox >::Hook( m_listbox );
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CListboxImp::SetAllowMultiSelect( bool allowMultiselect )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::GetAllowMultiSelect( bool allowMultiselect )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//UInt32
//CListboxImp::GetLineCount( void )
//{GUCE_TRACE;
//
//    return (UInt32) m_listbox->getItemCount();
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::Clear( void )
//{GUCE_TRACE;
//
//    m_listbox->resetList();
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::GetLineAtIndex( const UInt32 lineIndex ,
//                             CString& lineText      ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::InsertLineAtIndex( const UInt32 lineIndex ,
//                                const CString& text    )
//{GUCE_TRACE;
//
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::AppendLine( const CString& text )
//{GUCE_TRACE;
//
//    m_listbox->addItem( new ListItem( text.C_String() ) );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::GetText( CString& text ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CListboxImp::SetLineSelection( const UInt32 lineIndex ,
//                            const bool isSelected  )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CListboxImp::GetLineSelection( const UInt32 lineIndex ,
//                               bool& isSelected       ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CListboxImp::GetFirstSelectedLine( CString& text ) const
//{GUCE_TRACE;
//
//    CEGUI::ListboxItem* item = m_listbox->getFirstSelectedItem();
//    if ( NULL != item )
//    {
//        text = item->getText().c_str();
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CListboxImp::GetLastSelectedLine( CString& text ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CListboxImp::GetSelectedLines( TStringVector& selectedLines ) const
//{GUCE_TRACE;
//
//    return false;
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


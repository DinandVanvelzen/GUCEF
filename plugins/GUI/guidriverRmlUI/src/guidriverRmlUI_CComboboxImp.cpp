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
//#include "guceMyGUIOgre_CComboboxImp.h"
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
//CComboboxImp::CComboboxImp( void )
//    : CWidgetImp< GUCEF::GUI::CCombobox >() ,
//      m_combobox( NULL )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CComboboxImp::~CComboboxImp()
//{GUCE_TRACE;
//
//    m_combobox = NULL;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CComboboxImp::SetText( const CString& text )
//{GUCE_TRACE;
//
//    if ( NULL != m_combobox )
//    {
//        m_combobox->setText( text.C_String() );
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CComboboxImp::GetText( CString& text ) const
//{GUCE_TRACE;
//
//    if ( NULL != m_combobox )
//    {
//        text = m_combobox->getText().c_str();
//        return true;
//    }
//    return false;
//}
//    
///*-------------------------------------------------------------------------*/
//
//bool
//CComboboxImp::SetListItems( const TStringVector& items )
//{GUCE_TRACE;
//
//    if ( NULL != m_combobox )
//    {
//        m_combobox->resetList();
//        for ( UInt32 i=0; i<items.size(); ++i )
//        {
//            m_combobox->addItem( new ListItem( items[ i ].C_String() ) );
//        }
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CComboboxImp::GetListItems( TStringVector& items ) const
//{GUCE_TRACE;
//
//    if ( NULL != m_combobox )
//    {
//        CEGUI::ListboxItem* item = NULL;
//        size_t itemCount = m_combobox->getItemCount();
//        for ( size_t i=0; i<itemCount; ++i )
//        {
//            item = m_combobox->getListboxItemFromIndex( i );
//            items.push_back( item->getText().c_str() );
//        }
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CComboboxImp::Hook( CEGUI::Combobox* combobox )
//{GUCE_TRACE;
//
//    m_combobox = combobox;
//    CWidgetImp< GUCEF::GUI::CCombobox >::Hook( m_combobox );
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


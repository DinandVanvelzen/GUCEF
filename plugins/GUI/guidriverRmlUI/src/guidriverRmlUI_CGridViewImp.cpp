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
//#include "elements/CEGUIListboxTextItem.h"
//
//#ifndef GUCEF_CORE_CTRACER_H
//#include "CTracer.h"
//#define GUCEF_CORE_CTRACER_H
//#endif /* GUCEF_CORE_CTRACER_H ? */
//
//#include "guceMyGUIOgre_CGridViewImp.h"
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
//class CDummyForHitTestAccess : public CEGUI::MultiColumnList
//{
//    public:
//    
//    CEGUI::ListboxItem* DoGetItemAtPoint( const CEGUI::Point& pt ) const
//    {
//        return getItemAtPoint( pt );
//    }
//};
//
///*-------------------------------------------------------------------------*/
//
//class CIconListTextItem : public CEGUI::ListboxTextItem
//{
//    private:
//    
//    const CEGUI::Image* m_iconImage;
//    
//    public:
//
//    CIconListTextItem( const CEGUI::String& text ,
//                       const CEGUI::Image* icon  ) 
//        : ListboxTextItem( text ) ,
//          m_iconImage( icon )
//    {GUCE_TRACE;
//
//        setSelectionBrushImage( (CEGUI::utf8*) "TaharezLook"             , 
//                                (CEGUI::utf8*) "MultiListSelectionBrush" );
//    }
//    
//    virtual ~CIconListTextItem()
//    {GUCE_TRACE;
//    
//    }
//
///*        
//    virtual void draw( const CEGUI::Vector3& position , 
//                       float alpha                    , 
//                       const CEGUI::Rect& clipper     ) const
//    {GUCE_TRACE;
//    
//        if ( NULL == m_iconImage )
//        {
//            CEGUI::ListboxTextItem::draw( position ,
//                                          alpha    ,
//                                          clipper  );
//        }
//        else
//        {
//            
//        }
//    }
//*/
//                       
//    virtual void draw( CEGUI::RenderCache& cache     , 
//                       const CEGUI::Rect& targetRect , 
//                       float zBase                   ,
//                       float alpha                   ,
//                       const CEGUI::Rect* clipper    ) const
//    {GUCE_TRACE;
//
//        if ( NULL == m_iconImage )
//        {
//            CEGUI::ListboxTextItem::draw( cache      ,
//                                          targetRect ,
//                                          zBase      ,
//                                          alpha      ,
//                                          clipper    );
//        }
//        else
//        {
//            // Derived from the TreeItem class
//
//            CEGUI::Rect finalRect(targetRect);
//
//            if ( m_iconImage != NULL)
//            {
//                //      Size iconSize = d_iconImage->getSize();
//                CEGUI::Rect finalPos( finalRect );
//                finalPos.setWidth( targetRect.getHeight() );
//                finalPos.setHeight( targetRect.getHeight() );
//                cache.cacheImage( *m_iconImage, finalPos, zBase, CEGUI::ColourRect(CEGUI::colour(1,1,1,alpha)), clipper);
//                finalRect.d_left += targetRect.getHeight();
//            }
//
//            if ( d_selected && d_selectBrush != 0 )
//            {
//                cache.cacheImage(*d_selectBrush, finalRect, zBase, getModulateAlphaColourRect(d_selectCols, alpha), clipper);
//            }
//
//            CEGUI::Font* font = getFont();
//
//            if (font)
//            {
//                CEGUI::Rect finalPos(finalRect);
//                finalPos.d_top -= (font->getLineSpacing() - font->getBaseline()) * 0.5f;
//                cache.cacheText(d_itemText, font, CEGUI::LeftAligned, finalPos, zBase, getModulateAlphaColourRect(d_textCols, alpha), clipper);
//            }       
//        }    
//    }
//    
//    void SetIconImage( const CEGUI::Image* icon )
//    {
//        m_iconImage = icon;
//    }
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CGridViewImp::CGridViewImp( void )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CGridViewImp::~CGridViewImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::SetText( const UInt32 columnIndex ,
//                       const UInt32 rowIndex    ,
//                       const CString& text      )
//{GUCE_TRACE;
//
//    CIconListTextItem* item = NULL;
//    try
//    {
//        item = new CIconListTextItem( (CEGUI::utf8*) text.C_String(), NULL );
//        m_grid->setItem( item, columnIndex, rowIndex );
//    }
//    catch ( CEGUI::InvalidRequestException& )
//    {
//        delete item;
//        item = NULL;
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//CString
//CGridViewImp::GetText( const UInt32 columnIndex ,
//                       const UInt32 rowIndex    ) const
//{GUCE_TRACE;
//
//    try
//    {
//        return m_grid->getItemAtGridReference( CEGUI::MCLGridRef( rowIndex, columnIndex ) )->getText().c_str();
//    }
//    catch ( CEGUI::InvalidRequestException& )
//    {
//        return CString();
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//CEGUI::MultiColumnList*
//CGridViewImp::GetImplementationWidget( void )
//{GUCE_TRACE;
//
//    return m_grid;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::AppendText( const UInt32 columnIndex ,
//                          const CString& text      )
//{GUCE_TRACE;
//
//    // First check if we have to add a new column
//    if ( m_grid->getColumnCount() < columnIndex+1 )
//    {
//        m_grid->addColumn( "", columnIndex, CEGUI::UDim( 0.33f, 0.0f ) );
//        CEGUI::ListHeaderSegment& listHeaderSegment = m_grid->getHeaderSegmentForColumn( columnIndex );
//        
//        // New columns should not have a header
//        listHeaderSegment.setClickable( false );
//        //listHeaderSegment.setHeight( CEGUI::UDim( 0.0, 0.0 ) );
//        //listHeaderSegment.hide();
//    }
//    
//    Int32 rowIndex = GetFirstEmptyRowInColumn( columnIndex );
//    if ( -1 == rowIndex )
//    {    
//        // Now we can add the row to the list's column
//        m_grid->addRow( new CIconListTextItem( (CEGUI::utf8*) text.C_String(), NULL ), columnIndex );
//    }
//    else
//    {
//        m_grid->setItem( new CIconListTextItem( (CEGUI::utf8*) text.C_String(), NULL ), columnIndex, rowIndex );
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//Int32
//CGridViewImp::GetFirstEmptyRowInColumn( const UInt32 columnIndex ) const
//{GUCE_TRACE;
//    
//    if ( m_grid->getColumnCount() < columnIndex+1 )
//    {
//        CEGUI::ListboxItem* listItem = NULL;
//        for ( UInt32 i=0; i<m_grid->getRowCount(); ++i )
//        {
//            listItem = m_grid->getItemAtGridReference( CEGUI::MCLGridRef( i, columnIndex ) );
//            if ( NULL == listItem )
//            {
//                return (Int32)i;
//            }
//            else
//            {
//                if ( NULL == listItem->getText().c_str() )
//                {
//                    return (Int32)i;
//                }
//            }
//        }
//    }
//    return -1;    
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::AppendText( const CString& text )
//{GUCE_TRACE;
//
//    UInt32 column = m_grid->getColumnCount();
//    
//    // In case the list is empty,..
//    if ( column == 0 )
//    {
//        AppendText( 0, text );
//        return;
//    }
//    
//    // Check if we can append to the last column or whether
//    // we should append to a new column
//    --column;
//    if ( IsThereRoomInColumn( column ) )
//    {
//        AppendText( column, text );
//    }
//    else
//    {
//        AppendText( column+1, text );
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::Clear( void )
//{GUCE_TRACE;
//
//    m_grid->resetList();
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CGridViewImp::IsThereRoomInColumn( const UInt32 columnIndex ) const
//{GUCE_TRACE;
//
//    // Check initial conditions
//    if ( ( m_grid->getRowCount() == 0 )               ||
//         ( m_grid->getColumnCount() < columnIndex+1 )  ) return true;
//    
//    // Determine the maximum number of rows possible without scrollbars
//    float avgRowHeightInPixels = m_grid->getTotalRowsHeight() / m_grid->getRowCount();
//    float maxColumnRows = m_grid->getPixelRect().getHeight() / avgRowHeightInPixels;
//    
//    // Check if the row count has reached the max. 
//    // Note that this is for all columns combined
//    if ( m_grid->getRowCount() < maxColumnRows )
//    {
//        return true;
//    }
//    
//    // If we get here then the grid has been set to max capacity
//    // The only way to determine if there is room now is to check for empty spots directly
//    return true; //@FIXME GetFirstEmptyRowInColumn( columnIndex ) > -1;
//}
//
///*-------------------------------------------------------------------------*/
//
//const CString&
//CGridViewImp::GetClassTypeName( void ) const
//{GUCE_TRACE;
//
//    static CString typeName = "GUCE::CEGUIOGRE::CGridViewImp";
//    return typeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::Hook( CEGUI::MultiColumnList* grid )
//{GUCE_TRACE;
//
//    m_grid = grid;
//    CWidgetImp< GUCEF::GUI::CGridView >::Hook( grid );
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CGridViewImp::TestForItemHit( const Float32 posX  ,
//                              const Float32 posY  ,
//                              UInt32& columnIndex ,
//                              UInt32& rowIndex    ) const
//{GUCE_TRACE;
//
//    // Convert to local space pixel coordinates    
//    CEGUI::Rect listArea( m_grid->getPixelRect() );
//    float y = posY - listArea.d_top;
//    float x = posX - listArea.d_left;
//    
//    // Hack to the test function
//    CDummyForHitTestAccess* mcList = static_cast< CDummyForHitTestAccess* >( m_grid );
//    CEGUI::ListboxItem* listItem = mcList->DoGetItemAtPoint( CEGUI::Point( x, y ) );
//    if ( NULL != listItem )
//    {
//        // We have a hit !!!
//        CEGUI::MCLGridRef gridRef = m_grid->getItemGridReference( listItem );
//        columnIndex = gridRef.column;
//        rowIndex = gridRef.row;
//        return true;
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CGridViewImp::GetSelectedItems( TGridIndexVector& gridIndexList )
//{GUCE_TRACE;
//
//    if ( NULL != m_grid )
//    {
//        CEGUI::ListboxItem* listItem = m_grid->getFirstSelectedItem();
//        while ( NULL != listItem )
//        {
//            TGridIndex gridIndex;
//            CEGUI::MCLGridRef gridRef = m_grid->getItemGridReference( listItem );
//            gridIndex.column = gridRef.column;
//            gridIndex.row = gridRef.row;
//            gridIndexList.push_back( gridIndex );
//            
//            listItem = m_grid->getNextSelected( listItem );
//        }
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
//}; /* namespace GUCEF */
//
///*-------------------------------------------------------------------------*/


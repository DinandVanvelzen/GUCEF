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
//#ifndef GUCEF_GUI_CIFILESYSTEMINFOPROVIDER_H
//#include "gucefGUI_CIFileSystemInfoProvider.h"
//#define GUCEF_GUI_CIFILESYSTEMINFOPROVIDER_H
//#endif /* GUCEF_GUI_CIFILESYSTEMINFOPROVIDER_H ? */
//
//#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
//#include "dvcppstringutils.h"
//#define GUCEF_CORE_DVCPPSTRINGUTILS_H
//#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
//
//#include "CEGUIWindowManager.h"
//#include "CEGUIEventArgs.h"
//#include "CEGUIInputEvent.h"
//#include "CEGUIImagesetManager.h"
//#include "CEGUIImageset.h"
//#include "elements/CEGUIMultiColumnList.h"
//#include "elements/CEGUIFrameWindow.h"
//#include "elements/CEGUITitlebar.h"
//#include "elements/CEGUIListboxTextItem.h"
//#include "elements/CEGUIPushButton.h"
//#include "elements/CEGUIEditbox.h"
//#include "elements/CEGUICombobox.h"
//
//#ifndef GUCE_MYGUIOGRE_CVFSINFOPROVIDER_H
//#include "guceMyGUIOgre_CVFSInfoProvider.h"
//#define GUCE_MYGUIOGRE_CVFSINFOPROVIDER_H
//#endif /* GUCE_MYGUIOGRE_CVFSINFOPROVIDER_H ? */
//
//#ifndef GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//#include "guceMyGUIOgre_CGridViewImp.h"
//#define GUCE_MYGUIOGRE_CGRIDVIEWIMP_H
//#endif /* GUCE_MYGUIOGRE_CGRIDVIEWIMP_H ? */
//
//#include "guceMyGUIOgre_CFileSystemDialogImp.h"
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
//CFileSystemDialogImp::CFileSystemDialogImp( void )
//    : CFileSystemDialog()         ,
//      m_iconsImageSet( NULL )     ,
//      m_showDirs( true )          ,
//      m_showFiles( true )         ,
//      m_allowMultiSelect( false ) ,
//      m_selectedItems()           ,
//      m_fsInfoProvider( NULL )    ,
//      m_currentPath()             ,
//      m_fileItems()               ,
//      m_dirItems()                ,
//      m_archiveItems()            ,
//      m_allowDirSelect( false )   ,
//      m_allowFileSelect( true )
//{GUCE_TRACE;
//
//    SetFileSystemInfoProvider( new CVFSInfoProvider() );
//}
//
///*-------------------------------------------------------------------------*/
//    
//CFileSystemDialogImp::~CFileSystemDialogImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFileSystemDialogImp::OnPostLayoutLoad( void )
//{GUCE_TRACE;
//
//    GUCEF::GUI::CFileSystemDialog::OnPostLayoutLoad();
//    
//    // Try and link the icon imageset
//    CEGUI::ImagesetManager* imgSetManager = CEGUI::ImagesetManager::getSingletonPtr();
//    if ( imgSetManager->isImagesetPresent( "Icons" ) )
//    {
//        m_iconsImageSet = imgSetManager->getImageset( "Icons" );
//    }
//
//    // Hook up the event handlers
//    CEGUI::FrameWindow* window = static_cast< CEGUI::FrameWindow* >( GetWindow()->GetImplementationPtr() );
//    window->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked                                ,
//                            CEGUI::Event::Subscriber( &CFileSystemDialogImp::OnCancelButtonClick , 
//                                                      this                                       ) );    
//
//    CEGUI::MultiColumnList* fsViewWidget = static_cast< CEGUI::MultiColumnList* >( GetFileSystemGridView()->GetImplementationPtr() );
//    fsViewWidget->subscribeEvent( CEGUI::MultiColumnList::EventSelectionChanged                   ,
//                                  CEGUI::Event::Subscriber( &CFileSystemDialogImp::OnItemSelected , 
//                                                            this                                  ) );    
//    fsViewWidget->subscribeEvent( CEGUI::MultiColumnList::EventMouseDoubleClick                     ,
//                                  CEGUI::Event::Subscriber( &CFileSystemDialogImp::OnItemDblClicked , 
//                                                            this                                    ) );  
//    CEGUI::PushButton* okButton = static_cast< CEGUI::PushButton* >( GetOkButton()->GetImplementationPtr() );
//    okButton->subscribeEvent( CEGUI::PushButton::EventClicked                                  ,
//                              CEGUI::Event::Subscriber( &CFileSystemDialogImp::OnOkButtonClick , 
//                                                        this                                   ) );
//    CEGUI::PushButton* cancelButton = static_cast< CEGUI::PushButton* >( GetCancelButton()->GetImplementationPtr() );
//    cancelButton->subscribeEvent( CEGUI::PushButton::EventClicked                                      ,
//                                  CEGUI::Event::Subscriber( &CFileSystemDialogImp::OnCancelButtonClick , 
//                                                            this                                       ) );
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFileSystemDialogImp::SetFileSystemInfoProvider( GUCEF::GUI::CIFileSystemInfoProvider* provider )
//{GUCE_TRACE;
//
//    m_fsInfoProvider = provider;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFileSystemDialogImp::SetCurrentPath( const CString& path )
//{GUCE_TRACE;
//
//    m_currentPath = path;
//}
//
///*-------------------------------------------------------------------------*/
//    
//const CString&
//CFileSystemDialogImp::GetCurrentPath( void ) const
//{GUCE_TRACE;
//
//    return m_currentPath;
//}
//
///*-------------------------------------------------------------------------*/
//
//const CEGUI::Image*
//CFileSystemDialogImp::GetIcon( const CString& iconName )
//{GUCE_TRACE;
//
//    if ( NULL != m_iconsImageSet )
//    {
//        CEGUI::String iName = iconName.C_String();
//        if ( m_iconsImageSet->isImageDefined( iName ) )
//        {
//            return &m_iconsImageSet->getImage( iName );
//        }
//    }
//    return NULL;    
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::OnItemDblClicked( const CEGUI::EventArgs& e )
//{GUCE_TRACE;
//            
//    // Test if this was a left mouse click
//    const CEGUI::MouseEventArgs& eData = static_cast< const CEGUI::MouseEventArgs& >( e );    
//    if ( eData.button == CEGUI::LeftButton )
//    {
//        // Get some easy access to data
//        CGridViewImp* fsView = static_cast< CGridViewImp* >( GetFileSystemGridView() );
//        CEGUI::MultiColumnList* fsViewWidget = fsView->GetImplementationWidget();        
//
//        UInt32 columnIndex = 0;
//        UInt32 rowIndex = 0;
//        if ( fsView->TestForItemHit( eData.position.d_x , 
//                                     eData.position.d_y ,
//                                     columnIndex        ,
//                                     rowIndex           ) )
//        {        
//            CEGUI::ListboxItem* listItem = fsViewWidget->getItemAtGridReference( CEGUI::MCLGridRef( rowIndex, columnIndex ) );
//            if ( NULL != listItem )
//            {
//                if ( listItem->getText() == ".." )
//                {
//                    // ".." means go up one dir
//                    m_currentPath = GUCEF::CORE::StripLastSubDir( m_currentPath );
//                    RefreshView();
//                }
//                else
//                if ( IsItemADirectory( listItem->getText().c_str() ) )
//                {
//                    GUCEF::CORE::AppendToPath( m_currentPath, listItem->getText().c_str() );
//                    RefreshView();
//                }
//                else
//                if ( IsItemAnArchive( listItem->getText().c_str() ) )
//                {
//                    CString itemName( listItem->getText().c_str() );
//                    CString realName = itemName.CutChars( 3, true );
//                    GUCEF::CORE::AppendToPath( m_currentPath, realName );
//                    RefreshView();
//                }
//            }
//        }
//    }
//    
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::OnOkButtonClick( const CEGUI::EventArgs& e )
//{GUCE_TRACE;
//
//    Hide();
//    NotifyObservers( OkPressedEvent );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::OnCancelButtonClick( const CEGUI::EventArgs& e )
//{GUCE_TRACE;
//
//    Hide();
//    NotifyObservers( CancelPressedEvent );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::IsItemADirectory( const CString& name ) const
//{GUCE_TRACE;
//
//    return ( name == ".." ) || ( m_dirItems.find( name ) != m_dirItems.end() );
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::IsItemAnArchive( const CString& name ) const
//{GUCE_TRACE;
//
//    // Check if the item is an archive
//    if ( 0 == name.HasSubstr( "<A>", true ) )
//    {
//        CString realName = name.CutChars( 3, true );
//        return m_archiveItems.find( realName ) != m_archiveItems.end();
//    }
//    
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::IsItemAllowedToBeSelected( const CString& itemText )
//{GUCE_TRACE;
//
//    if ( IsItemADirectory( itemText ) || IsItemAnArchive( itemText ) )
//    {
//        return m_allowDirSelect;
//    }
//    else
//    {
//        return m_allowFileSelect;
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFileSystemDialogImp::UpdateSelectionIndicator( void )
//{GUCE_TRACE;
//    
//    // Clear the current selection string
//    GetSelectionEditbox()->Clear();
//    m_selectedItems.clear();
//    
//    // get a list of all selected items
//    GUCEF::GUI::CGridView::TGridIndexVector selectedGridItems;
//    GUCEF::GUI::CGridView* gridView = GetFileSystemGridView();
//    gridView->GetSelectedItems( selectedGridItems );
//
//    CString selItemText;
//    UInt32 selectedCount = 0;
//    GUCEF::GUI::CGridView::TGridIndexVector::iterator i = selectedGridItems.begin();
//    while ( i != selectedGridItems.end() )
//    {
//        GUCEF::GUI::CGridView::TGridIndex& gridIndex = (*i);
//        CString itemText = gridView->GetText( gridIndex.column, gridIndex.row );
//        
//        if ( IsItemAllowedToBeSelected( itemText ) )
//        {
//            // We add the item to the list of selected items            
//            m_selectedItems.push_back( itemText );
//            
//            // modify the selected items string for user feedback
//            if ( selectedCount > 0 )
//            {
//                selItemText += " \"" + itemText + "\"";
//            }
//            else
//            {
//                selItemText = "\"" + itemText + "\"";
//            }
//
//            ++selectedCount;
//        }
//        ++i;
//    }
//    
//    // Update the selection string with the new string
//    GetSelectionEditbox()->SetText( selItemText );
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CFileSystemDialogImp::SetAllowedSelection( const bool allowDirSelect  ,
//                                           const bool allowFileSelect )
//{GUCE_TRACE;
//
//    m_allowDirSelect = allowDirSelect;
//    m_allowFileSelect = allowFileSelect;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::OnItemSelected( const CEGUI::EventArgs& e )
//{GUCE_TRACE;
//
//    UpdateSelectionIndicator();
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::ShowModal( void )
//{GUCE_TRACE;
//
//    if ( NULL != GetWindow() )
//    {
//        // Are we hidden right now?
//        if ( !GetWindow()->IsVisible() )
//        {
//            RefreshView();
//            GetSelectionEditbox()->Clear();
//        }
//        return CForm::ShowModal();
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CFileSystemDialogImp::Show( void )
//{GUCE_TRACE;
//
//    if ( NULL != GetWindow() )
//    {
//        // Are we hidden right now?
//        if ( !GetWindow()->IsVisible() )
//        {
//            RefreshView();
//            GetSelectionEditbox()->Clear();
//        }
//        return CForm::Show();
//    }
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//CEGUI::Window*
//CFileSystemDialogImp::GetWindowImp( void )
//{GUCE_TRACE;
//
//    return static_cast< CEGUI::Window* >( GetWindow()->GetImplementationPtr() );
//}
//
///*-------------------------------------------------------------------------*/
//    
//CEGUI::PushButton*
//CFileSystemDialogImp::GetOkButtonImp( void )
//{GUCE_TRACE;
//
//    return static_cast< CEGUI::PushButton* >( GetOkButton()->GetImplementationPtr() );
//}
//
///*-------------------------------------------------------------------------*/
//    
//CEGUI::PushButton*
//CFileSystemDialogImp::GetCancelButtonImp( void )
//{GUCE_TRACE;
//
//    return static_cast< CEGUI::PushButton* >( GetCancelButton()->GetImplementationPtr() );
//}
//   
///*-------------------------------------------------------------------------*/
//    
//void
//CFileSystemDialogImp::RefreshView( void )
//{GUCE_TRACE;
//
//    if ( NULL != m_fsInfoProvider )
//    {       
//        // Try and link the icon imageset
//        if ( NULL == m_iconsImageSet )
//        {
//            CEGUI::ImagesetManager* imgSetManager = CEGUI::ImagesetManager::getSingletonPtr();
//            if ( imgSetManager->isImagesetPresent( "Icons" ) )
//            {
//                m_iconsImageSet = imgSetManager->getImageset( "Icons" );
//            }
//        }
//    
//        GUCEF::GUI::CGridView* fsView = GetFileSystemGridView();
//        
//        fsView->Clear();
//        CString filter = GetFilterCombobox()->GetText();
//        
//        m_dirItems.clear();
//        if ( m_showDirs )
//        {
//            const CEGUI::Image* icon = GetIcon( "Folder" );            
//            if ( m_currentPath.Length() != 0 )
//            {
//                fsView->AppendText( ".." );
//            }
//            
//            m_fsInfoProvider->GetDirectoryList( m_currentPath ,
//                                                filter        ,
//                                                m_dirItems    );
//
//            TStringSet::iterator i = m_dirItems.begin();
//            while ( i != m_dirItems.end() )
//            {
//                fsView->AppendText( (*i) );
//                ++i;
//            }
//        }
//        
//        m_archiveItems.clear();
//        if ( m_showFiles )
//        {
//            m_fsInfoProvider->GetArchiveList( m_currentPath  ,
//                                              filter         ,
//                                              m_archiveItems );
//            const CEGUI::Image* icon = GetIcon( "Folder" );
//            TStringSet::iterator i = m_archiveItems.begin();
//            while ( i != m_archiveItems.end() )
//            {
//                fsView->AppendText( "<A>" + (*i) );
//                ++i;
//            }        
//        }        
//        
//        m_fileItems.clear();
//        if ( m_showFiles )
//        {
//            m_fsInfoProvider->GetFileList( m_currentPath ,
//                                           filter        ,
//                                           m_fileItems   );
//            const CEGUI::Image* icon = GetIcon( "Unknown" );
//            TStringSet::iterator i = m_fileItems.begin();
//            while ( i != m_fileItems.end() )
//            {
//                fsView->AppendText( (*i) );
//                ++i;
//            }        
//        }               
//    }
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CFileSystemDialogImp::SetAllowMultiSelect( const bool allow )
//{GUCE_TRACE;
//
//    m_allowMultiSelect = allow;
//}
//
///*-------------------------------------------------------------------------*/
//    
//bool
//CFileSystemDialogImp::GetAllowMultiSelect( void ) const
//{GUCE_TRACE;
//
//    return m_allowMultiSelect;
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CFileSystemDialogImp::SetSelectedItems( const TStringVector& items )
//{GUCE_TRACE;
//
//    m_selectedItems = items;
//    UpdateSelectionIndicator();
//}
//
///*-------------------------------------------------------------------------*/
//    
//const CFileSystemDialogImp::TStringVector&
//CFileSystemDialogImp::GetSelectedItems( void ) const
//{GUCE_TRACE;
//
//    return m_selectedItems;
//}
//
///*-------------------------------------------------------------------------*/
//    
//void
//CFileSystemDialogImp::SetItemDisplayTypes( const bool showDirs  ,
//                                           const bool showFiles )
//{GUCE_TRACE;
//
//    m_showDirs = showDirs;
//    m_showFiles = showFiles;
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


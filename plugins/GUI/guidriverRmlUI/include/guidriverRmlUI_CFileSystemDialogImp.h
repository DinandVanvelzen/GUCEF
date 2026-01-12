///*
// *  guceGUI: GUCE module providing GUI functionality
// *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU Lesser General Public
// *  License along with this library; if not, write to the Free Software
// *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
// */
//
//#ifndef GUCE_MYGUIOGRE_CFILESYSTEMDIALOGIMP_H
//#define GUCE_MYGUIOGRE_CFILESYSTEMDIALOGIMP_H 
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef GUCEF_GUI_CFILESYSTEMDIALOG_H
//#include "gucefGUI_CFileSystemDialog.h"
//#define GUCEF_GUI_CFILESYSTEMDIALOG_H
//#endif /* GUCEF_GUI_CFILESYSTEMDIALOG_H ? */
//
//#ifndef GUCE_MYGUIOGRE_MACROS_H
//#include "guceMyGUIOgre_macros.h"     /* often used guceCEGUIOGRE macros */
//#define GUCE_MYGUIOGRE_MACROS_H
//#endif /* GUCE_MYGUIOGRE_MACROS_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      CLASSES                                                            //
////                                                                         //
////-------------------------------------------------------------------------*/
//
///*
// *      Forward declarations of classes used here 
// */
// 
//namespace CEGUI{ class FrameWindow; 
//                 class PushButton;
//                 class Imageset;
//               }
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
//class GUCE_MYGUIOGRE_EXPORT_CPP CFileSystemDialogImp : public GUCEF::GUI::CFileSystemDialog
//{   
//
//    public:
//    
//    typedef std::vector< CString >  TStringVector;
//    
//    CFileSystemDialogImp( void );
//    
//    virtual ~CFileSystemDialogImp();
//    
//    virtual bool ShowModal( void );
//
//    virtual bool Show( void );
//    
//    virtual void RefreshView( void );
//    
//    virtual void SetAllowMultiSelect( const bool allow );
//    
//    virtual bool GetAllowMultiSelect( void ) const;
//    
//    virtual void SetSelectedItems( const TStringVector& item );
//    
//    virtual const TStringVector& GetSelectedItems( void ) const;
//    
//    virtual void SetItemDisplayTypes( const bool showDirs  ,
//                                      const bool showFiles );
//
//    virtual void SetAllowedSelection( const bool allowDirSelect  ,
//                                      const bool allowFileSelect );
//
//    virtual void SetFileSystemInfoProvider( GUCEF::GUI::CIFileSystemInfoProvider* provider );
//
//    virtual void SetCurrentPath( const CString& path );
//    
//    virtual const CString& GetCurrentPath( void ) const;
//    
//    virtual bool IsItemADirectory( const CString& name ) const;
//    
//    virtual bool IsItemAnArchive( const CString& name ) const;
//
//    protected:
//    
//    virtual bool OnOkButtonClick( const CEGUI::EventArgs& e );
//
//    virtual bool OnCancelButtonClick( const CEGUI::EventArgs& e );
//    
//    virtual bool OnItemSelected( const CEGUI::EventArgs& e );
//    
//    virtual bool OnItemDblClicked( const CEGUI::EventArgs& e );
//
//    CEGUI::Window* GetWindowImp( void );
//    
//    CEGUI::PushButton* GetOkButtonImp( void );
//    
//    CEGUI::PushButton* GetCancelButtonImp( void );
//    
//    const CEGUI::Image* GetIcon( const CString& iconName );
//
//    protected:
//    
//    virtual void OnPostLayoutLoad( void );
//        
//    private:
//    
//    CFileSystemDialogImp( const CFileSystemDialogImp& src );
//    CFileSystemDialogImp& operator=( const CFileSystemDialogImp& src );
//    
//    void UpdateSelectionIndicator( void );
//    
//    bool IsItemAllowedToBeSelected( const CString& itemText );
//    
//    private:
//    
//    typedef std::set< CString > TStringSet;
//    
//    CEGUI::Imageset* m_iconsImageSet;
//    bool m_showDirs;
//    bool m_showFiles;
//    bool m_allowMultiSelect;
//    TStringVector m_selectedItems;
//    GUCEF::GUI::CIFileSystemInfoProvider* m_fsInfoProvider;
//    CString m_currentPath;
//    TStringSet m_fileItems;
//    TStringSet m_dirItems;
//    TStringSet m_archiveItems;
//    bool m_allowDirSelect;
//    bool m_allowFileSelect;
//};
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
//
//#endif /* GUCE_MYGUIOGRE_CFILESYSTEMDIALOGIMP_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      Info & Changes                                                     //
////                                                                         //
////-------------------------------------------------------------------------//
//
//- 05-09-2007 :
//        - Designed and implemented this class.
//
//-----------------------------------------------------------------------------*/


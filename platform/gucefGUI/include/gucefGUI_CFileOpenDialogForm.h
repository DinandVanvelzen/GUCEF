/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_GUI_CFILEOPENDIALOGFORM_H
#define GUCEF_GUI_CFILEOPENDIALOGFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#include "gucefGUI_CFileSystemDialogForm.h"
#define GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESYSTEMDIALOGFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Asks the user for one or more files that are there: a list of folders and files with the app's file types, "Open".
 *  With GetOptions().acceptDirectoriesToo a separate "Open folder" button accepts a folder as well.
 *  See CFileSystemDialogForm for everything the file system dialogs share.
 */
class GUCEF_GUI_PUBLIC_CPP CFileOpenDialogForm : public CFileSystemDialogForm
{
    public:

    CFileOpenDialogForm( CIGUIContext* context );

    virtual ~CFileOpenDialogForm();

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFileOpenDialogForm( const CFileOpenDialogForm& src );
    CFileOpenDialogForm& operator=( const CFileOpenDialogForm& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFILEOPENDIALOGFORM_H ? */

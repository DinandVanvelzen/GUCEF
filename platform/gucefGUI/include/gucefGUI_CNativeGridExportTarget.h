/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H
#define GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIGRIDEXPORTTARGET_H
#include "gucefGUI_CIGridExportTarget.h"
#define GUCEF_GUI_CIGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CIGRIDEXPORTTARGET_H ? */

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
 *  The file system of the operating system as a place to export to: the dialog browses it with a
 *  CORE::CNativeFileSystemAccess, a file is written as a temporary file next to the chosen path, synced to storage and
 *  moved over the path with CORE::MoveFileAtomically().
 */
class GUCEF_GUI_PUBLIC_CPP CNativeGridExportTarget : public CIGridExportTarget
{
    public:

    static const CString DefaultDisplayName;

    /** An empty directory starts the dialog in the home directory of the user */
    explicit CNativeGridExportTarget( const CString& displayName = DefaultDisplayName, const CString& initialDirectory = CString::Empty );

    virtual ~CNativeGridExportTarget() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessPtr GetFileSystemAccess( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetInitialDirectory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenForWriting( const CString& path, CORE::IOAccessPtr& stream, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Commit( const CString& path, CORE::IOAccessPtr& stream, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abandon( const CString& path, CORE::IOAccessPtr& stream ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CNativeGridExportTarget( const CNativeGridExportTarget& src );
    CNativeGridExportTarget& operator=( const CNativeGridExportTarget& src );

    private:

    CString m_displayName;
    CString m_initialDirectory;
    CORE::TFileSystemAccessPtr m_access;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H ? */

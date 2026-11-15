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

#ifndef GUCEF_GUI_CVFSGRIDEXPORTTARGET_H
#define GUCEF_GUI_CVFSGRIDEXPORTTARGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

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
 *  The VFS of CVfsGlobal as a place to export to: the dialog browses it with a VFS::CVfsFileSystemAccess, a file is
 *  written through CVFS::GetFile() as a temporary file next to the chosen path in a writeable mount and moved over the
 *  path with CVFS::MoveFile(). A path only read only mounts cover cannot be written.
 */
class GUCEF_GUI_PUBLIC_CPP CVfsGridExportTarget : public CIGridExportTarget
{
    public:

    static const CString DefaultDisplayName;

    explicit CVfsGridExportTarget( const CString& displayName = DefaultDisplayName, const CString& initialDirectory = "/" );

    virtual ~CVfsGridExportTarget() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TFileSystemAccessPtr GetFileSystemAccess( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetInitialDirectory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenForWriting( const CString& path, CORE::IOAccessPtr& stream, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Commit( const CString& path, CORE::IOAccessPtr& stream, CString& error ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abandon( const CString& path, CORE::IOAccessPtr& stream ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void Close( const CString& temporary, CORE::IOAccessPtr& stream );

    CVfsGridExportTarget( const CVfsGridExportTarget& src );
    CVfsGridExportTarget& operator=( const CVfsGridExportTarget& src );

    private:

    typedef GUCEF::map< CString, VFS::TBasicVfsResourcePtr > TOpenFileMap;

    CString m_displayName;
    CString m_initialDirectory;
    CORE::TFileSystemAccessPtr m_access;
    TOpenFileMap m_openFiles;       /**< the VFS resources of the temporary files being written, by path */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVFSGRIDEXPORTTARGET_H ? */

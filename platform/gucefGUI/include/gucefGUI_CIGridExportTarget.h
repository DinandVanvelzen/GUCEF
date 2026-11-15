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

#ifndef GUCEF_GUI_CIGRIDEXPORTTARGET_H
#define GUCEF_GUI_CIGRIDEXPORTTARGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
 *  A place the user can save an export into: a root of the file save dialog (a name, a file system to browse, a
 *  directory to start in) and the way a file is written there.
 *
 *  A file is written in three steps so that a file the user chose is only replaced by a complete one:
 *  OpenForWriting() opens a temporary file next to the path, the export writes to it, Commit() closes it and moves it
 *  over the path in one step (or fails and removes it), Abandon() closes and removes it. The path is the one the file
 *  save dialog gave for this root, which has asked the user before an existing file is replaced. Made with GUCEF_NEW and
 *  handed around as TGridExportTargetPtr.
 */
class GUCEF_GUI_PUBLIC_CPP CIGridExportTarget : public CORE::CTSharedPtrCreator< CIGridExportTarget, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CIGridExportTarget, MT::CMutex > TGridExportTargetPtr;

    CIGridExportTarget( void );

    virtual ~CIGridExportTarget();

    /** The name of the root in the dialog */
    virtual CString GetDisplayName( void ) const = 0;

    /** The file system the dialog browses for this root */
    virtual CORE::TFileSystemAccessPtr GetFileSystemAccess( void ) const = 0;

    /** Where the dialog starts in this root when there is no last export location */
    virtual CString GetInitialDirectory( void ) const = 0;

    /** Opens a temporary file next to the path for writing */
    virtual bool OpenForWriting( const CString& path, CORE::IOAccessPtr& stream, CString& error ) = 0;

    /** Closes the stream and moves the temporary file over the path; on failure the temporary file is removed */
    virtual bool Commit( const CString& path, CORE::IOAccessPtr& stream, CString& error ) = 0;

    /** Closes the stream and removes the temporary file; the path is left as it was */
    virtual void Abandon( const CString& path, CORE::IOAccessPtr& stream ) = 0;

    /** The temporary file of a path: next to it, with the process in its name */
    static CString GetTemporaryPath( const CString& path );

    private:

    CIGridExportTarget( const CIGridExportTarget& src );
    CIGridExportTarget& operator=( const CIGridExportTarget& src );
};

/*-------------------------------------------------------------------------*/

typedef CIGridExportTarget::TGridExportTargetPtr TGridExportTargetPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIGRIDEXPORTTARGET_H ? */

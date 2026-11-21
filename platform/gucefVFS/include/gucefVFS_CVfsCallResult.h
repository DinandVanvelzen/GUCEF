/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#define GUCEF_VFS_CVFSCALLRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#include "gucefCORE_EFileSystemAccessResult.h"
#define GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#endif /* GUCEF_CORE_EFILESYSTEMACCESSRESULT_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Why a call on a remote or slow archive did what it did: the bool, size and empty string answers of CArchive carry no
 *  reason. The result value says what happened in the terms of the file system access layer, the message says it in words for a
 *  log or a person (for a place that cannot be reached it names the cause), and fetchStarted tells that a background
 *  fetch was started, or was already under way, for what the call asked for.
 *
 *  An archive that has a result of its own keeps it and converts to this one (the DRN bridge does so with CDrnBridgeResult).
 */
class GUCEF_VFS_PUBLIC_CPP CVfsCallResult
{
    public:

    CORE::TFileSystemAccessResult result;
    CORE::CString message;
    bool fetchStarted;

    /** A success */
    CVfsCallResult( void );

    CVfsCallResult( CORE::TFileSystemAccessResult callResult, const CORE::CString& callMessage = CORE::CString::Empty, bool callFetchStarted = false );

    CVfsCallResult( const CVfsCallResult& src );

    CVfsCallResult& operator=( const CVfsCallResult& src );

    /** Whether the call went well: the result is FILESYSTEMACCESS_SUCCESS */
    bool IsSuccess( void ) const;

    /** Whether the data is not here now but is on its way: a retry or the arrival event of the path has the answer (NOT_YET_AVAILABLE, CONNECTION_PENDING, CAPLIMIT_RETRY_LATER) */
    bool IsTemporary( void ) const;

    /** The result value as its name, with the message when there is one, for a log line */
    CORE::CString ToString( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

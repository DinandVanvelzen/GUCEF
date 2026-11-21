/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H

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

#ifndef GUCEF_VFS_CVFSCALLRESULT_H
#include "gucefVFS_CVfsCallResult.h"
#define GUCEF_VFS_CVFSCALLRESULT_H
#endif /* GUCEF_VFS_CVFSCALLRESULT_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACERESULT_H
#include "gucefDRN_CDRNNamespaceResult.h"
#define GUCEF_DRN_CDRNNAMESPACERESULT_H
#endif /* GUCEF_DRN_CDRNNAMESPACERESULT_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#include "gucefDRN_CDRNResourceTransferResult.h"
#define GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H
#endif /* GUCEF_DRN_CDRNRESOURCETRANSFERRESULT_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Why a call of the bridge archive did what it did. The bool, size and empty string answers of VFS::CArchive carry no
 *  reason; the archive keeps this result of the last call per calling thread (CDrnBridgeArchive::GetLastResult()).
 *
 *  Three views of one outcome:
 *    - fileSystemResult: the mapping onto CORE::TFileSystemAccessResult of the gucefVFS bridge design (D9), the form
 *      CORE::IFileSystemAccess users know. The remote cases have values of their own in that enumeration
 *      (REMOTE_CONNECTION_PENDING, REMOTE_RESOURCE_CHANGED, REMOTE_CAPLIMIT_RETRY_LATER, RESOURCE_CORRUPT,
 *      REMOTE_PROTOCOL_ERROR; MapToFileSystemResult() uses them), and code has the fine grained answer.
 *    - code: the result code of the namespace client (gucefDRN), which does have UNAVAILABLE, CHANGED, INTEGRITY, ...
 *    - status: the facility status the code was made from, when there was one
 *  message is words for a log or a person; for a share that cannot be reached it says so.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeResult
{
    public:

    CORE::TFileSystemAccessResult fileSystemResult;
    DRN::TDRNNamespaceResultCode code;
    DRN::TDRNResourceStatus status;
    CORE::CString message;

    /** A success */
    CDrnBridgeResult( void );

    CDrnBridgeResult( DRN::TDRNNamespaceResultCode resultCode, DRN::TDRNResourceStatus resultStatus, const CORE::CString& text );

    /** OK and UNCHANGED are successes */
    bool IsOk( void ) const;

    /** The share could not be reached: no usable link, the link was lost, no answer in time, or the call came from the thread of the node. Not true for the result of a call that was answered "not yet available" (the link may be fine) */
    bool IsUnreachable( void ) const;

    /** The data is not held locally, the calling thread must not wait for it and a background fetch was started (FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE) */
    bool IsNotYetAvailable( void ) const;

    /** The file changed on the serving side since it was opened */
    bool IsChanged( void ) const;

    /** "<code>: <message>" */
    CORE::CString ToString( void ) const;

    static CDrnBridgeResult FromNamespaceResult( const DRN::CDRNNamespaceResult& result );

    static CDrnBridgeResult FromTransferResult( const DRN::CDRNResourceTransferResult& result );

    /** The result of a call that a thread which must not wait made on data that is not held locally: fileSystemResult is set directly, code stays UNAVAILABLE and IsUnreachable() is false */
    static CDrnBridgeResult NotYetAvailable( const CORE::CString& text );

    /** What the generic remote layer says of a call (VFS::CVfsCallResult), in the form of the bridge */
    static CDrnBridgeResult FromCallResult( const VFS::CVfsCallResult& result );

    /** The same outcome in the form of the generic remote layer */
    VFS::CVfsCallResult ToCallResult( void ) const;

    /** The mapping of the design (D9), see the class description */
    static CORE::TFileSystemAccessResult MapToFileSystemResult( DRN::TDRNNamespaceResultCode code, DRN::TDRNResourceStatus status );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGERESULT_H ? */

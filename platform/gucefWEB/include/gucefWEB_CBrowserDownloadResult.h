/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CBROWSERDOWNLOADRESULT_H
#define GUCEF_WEB_CBROWSERDOWNLOADRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One download a CBrowserDownloadCapture saw: where it came from (as a template, no query), what the browser called
 *  it, where it ended up and how it was checked.
 */
class GUCEF_WEB_PUBLIC_CPP CBrowserDownloadResult
{
    public:

    enum EState
    {
        STATE_IN_PROGRESS   ,   /**< "inProgress" */
        STATE_COMPLETED     ,   /**< "completed": checked and moved to finalPath */
        STATE_REFUSED       ,   /**< "refused": a check failed, the file was deleted, see failureReason */
        STATE_CANCELED      ,   /**< "canceled": by the browser, the page or End() */
        STATE_FAILED            /**< "failed": the file went missing or could not be moved */
    };

    static const CORE::CString ReasonTooLarge;              /**< "too-large" */
    static const CORE::CString ReasonTooSmall;              /**< "too-small" */
    static const CORE::CString ReasonExtensionNotAllowed;   /**< "extension-not-allowed" */
    static const CORE::CString ReasonMimeTypeNotAllowed;    /**< "mime-type-not-allowed" */
    static const CORE::CString ReasonHtmlInsteadOfFile;     /**< "html-instead-of-file" */
    static const CORE::CString ReasonCanceled;              /**< "canceled" */
    static const CORE::CString ReasonFileMissing;           /**< "file-missing" */
    static const CORE::CString ReasonMoveFailed;            /**< "move-failed" */

    CORE::CString guid;                     /**< the browser's id of the download */
    CORE::CString suggestedFilename;        /**< as the browser named it (Content-Disposition, the URL) */
    CORE::CString finalPath;                /**< the sanitised file in the run folder, only when completed */
    UInt64 bytes;                           /**< received so far, the file size when completed */
    CORE::CString mimeType;                 /**< the server's content type when it was seen, otherwise the sniffed one */
    bool mimeTypeFromServer;
    CORE::CString sniffedMimeType;          /**< from the first bytes and the extension */
    CORE::CString sha256;                   /**< lower case hex of the completed file */
    CORE::CString urlTemplate;              /**< scheme://host/path-template, the scheme only for blob: and data: */
    CORE::CString frameId;
    EState state;
    CORE::CString failureReason;

    CBrowserDownloadResult( void );

    static CORE::CString StateToString( EState state );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CBrowserDownloadResult >             TBrowserDownloadResultVector;
typedef CORE::CTCloneableObj< CBrowserDownloadResult >      TBrowserDownloadResultEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CBROWSERDOWNLOADRESULT_H ? */

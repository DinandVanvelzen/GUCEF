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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefWEB_CBrowserDownloadResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CBrowserDownloadResult::ReasonTooLarge = "too-large";
const CORE::CString CBrowserDownloadResult::ReasonTooSmall = "too-small";
const CORE::CString CBrowserDownloadResult::ReasonExtensionNotAllowed = "extension-not-allowed";
const CORE::CString CBrowserDownloadResult::ReasonMimeTypeNotAllowed = "mime-type-not-allowed";
const CORE::CString CBrowserDownloadResult::ReasonHtmlInsteadOfFile = "html-instead-of-file";
const CORE::CString CBrowserDownloadResult::ReasonCanceled = "canceled";
const CORE::CString CBrowserDownloadResult::ReasonFileMissing = "file-missing";
const CORE::CString CBrowserDownloadResult::ReasonMoveFailed = "move-failed";

/*-------------------------------------------------------------------------*/

CBrowserDownloadResult::CBrowserDownloadResult( void )
    : guid()
    , suggestedFilename()
    , finalPath()
    , bytes( 0 )
    , mimeType()
    , mimeTypeFromServer( false )
    , sniffedMimeType()
    , sha256()
    , urlTemplate()
    , frameId()
    , state( STATE_IN_PROGRESS )
    , failureReason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CBrowserDownloadResult::StateToString( EState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case STATE_IN_PROGRESS: return "inProgress";
        case STATE_COMPLETED: return "completed";
        case STATE_REFUSED: return "refused";
        case STATE_CANCELED: return "canceled";
        case STATE_FAILED: return "failed";
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

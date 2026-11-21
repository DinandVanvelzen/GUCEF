/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNAMESPACERESULT_H
#define GUCEF_DRN_CDRNNAMESPACERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a call of CDRNNamespaceClient ended, in the small set a file system layer maps onto its own results. The suggested
 *  mapping onto CORE::TFileSystemAccessResult (gucefVFS bridge design D9):
 *
 *    OK, UNCHANGED         SUCCESS
 *    NOT_FOUND, NO_SUCH_SHARE  NOT_FOUND
 *    NOT_A_DIRECTORY       NOT_A_DIRECTORY
 *    ACCESS_DENIED         ACCESS_DENIED
 *    INVALID_NAME          INVALID_PATH
 *    READ_ONLY             READ_ONLY
 *    BUSY                  IN_USE (retry later)
 *    TIMEOUT               TIMEOUT
 *    UNSUPPORTED           NOT_SUPPORTED
 *    TOO_LARGE             TOO_LONG
 *    UNAVAILABLE, CHANGED, INTEGRITY, CURSOR_EXPIRED, WRONG_THREAD, IO_ERROR   FAILED (the proposed UNAVAILABLE, CHANGED,
 *                          INTEGRITY values when the VFS gets them)
 */
enum TDRNNamespaceResultCode
{
    DRN_NSRESULT_OK                 = 0,
    DRN_NSRESULT_UNCHANGED          = 1,    /**< success: the revision the caller holds is the current one, nothing was sent */
    DRN_NSRESULT_NOT_FOUND          = 2,    /**< the path does not exist, or the member may not see it */
    DRN_NSRESULT_NOT_A_DIRECTORY    = 3,
    DRN_NSRESULT_ACCESS_DENIED      = 4,    /**< the member may not see this (roles, policy) or the link is not protected */
    DRN_NSRESULT_INVALID_NAME       = 5,    /**< a name that is not a safe relative name (CDRNResourceName) */
    DRN_NSRESULT_BUSY               = 6,    /**< a limit of the serving side: try again later */
    DRN_NSRESULT_TIMEOUT            = 7,    /**< no answer in time */
    DRN_NSRESULT_UNAVAILABLE        = 8,    /**< no usable link to the member, or it was lost: the share cannot be reached now */
    DRN_NSRESULT_UNSUPPORTED        = 9,    /**< the member's node or the share has no namespace operations */
    DRN_NSRESULT_NO_SUCH_SHARE      = 10,
    DRN_NSRESULT_TOO_LARGE          = 11,   /**< the request goes beyond the limits of the serving side (paths in a STAT, entries in a page) */
    DRN_NSRESULT_CURSOR_EXPIRED     = 12,   /**< a listing was forgotten by the serving side: list again from the start */
    DRN_NSRESULT_CHANGED            = 13,   /**< a listing changed while it was read (CDRNNamespaceClient::ListAll() gave up restarting) */
    DRN_NSRESULT_INTEGRITY          = 14,   /**< the serving side did not follow the protocol, or an answer did not make sense */
    DRN_NSRESULT_WRONG_THREAD       = 15,   /**< a blocking call on a thread that works on the node: it would never be answered */
    DRN_NSRESULT_READ_ONLY          = 16,
    DRN_NSRESULT_IO_ERROR           = 17    /**< the serving side could not read its file system */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The outcome of a call of CDRNNamespaceClient: a code, the status the serving side gave (if one did) and a message */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceResult
{
    public:

    TDRNNamespaceResultCode code;
    TDRNResourceStatus status;      /**< the status of the answer or of the local refusal the code was made from */
    CORE::CString message;

    CDRNNamespaceResult( void );
    CDRNNamespaceResult( TDRNNamespaceResultCode resultCode, TDRNResourceStatus resultStatus, const CORE::CString& text );

    /** OK and UNCHANGED are successes */
    bool IsOk( void ) const;

    bool IsUnchanged( void ) const;

    /** BUSY, TIMEOUT and UNAVAILABLE may work later */
    bool IsRetryable( void ) const;

    static TDRNNamespaceResultCode CodeFromStatus( TDRNResourceStatus status );

    static const char* CodeToString( TDRNNamespaceResultCode code );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACERESULT_H ? */

/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The detail codes of the remote service kit: the first word of the message of a refusal and the "code" member of
 *  its status details, so that programs tell refusals apart without parsing texts
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceDetailCodes
{
    public:

    static const char* const SessionUnknown;          /**< "SESSION_UNKNOWN" */
    static const char* const SessionLimit;            /**< "SESSION_LIMIT" */
    static const char* const SessionRequired;         /**< "SESSION_REQUIRED" */
    static const char* const ChannelLimit;            /**< "CHANNEL_LIMIT" */
    static const char* const Draining;                /**< "DRAINING" */
    static const char* const RoleRequired;            /**< "ROLE_REQUIRED" */
    static const char* const DeviceKind;              /**< "DEVICE_KIND" */
    static const char* const DeviceNotActive;         /**< "DEVICE_NOT_ACTIVE" */
    static const char* const DeviceUnknown;           /**< "DEVICE_UNKNOWN" */
    static const char* const NoRule;                  /**< "NO_RULE" */
    static const char* const CommandIdReused;         /**< "COMMAND_ID_REUSED" */
    static const char* const BadCommandId;            /**< "BAD_COMMAND_ID" */
    static const char* const PairingClosed;           /**< "PAIRING_CLOSED" */
    static const char* const PairingDeclined;         /**< "PAIRING_DECLINED" */
    static const char* const BadProof;                /**< "BAD_PROOF" */
    static const char* const TokenRenewTooEarly;      /**< "TOKEN_RENEW_TOO_EARLY" */
    static const char* const UnsafePath;              /**< "UNSAFE_PATH" */
    static const char* const QuotaExceeded;           /**< "QUOTA_EXCEEDED" */
    static const char* const StagingFull;             /**< "STAGING_FULL" */
    static const char* const UploadLimit;             /**< "UPLOAD_LIMIT" */
    static const char* const UploadUnknown;           /**< "UPLOAD_UNKNOWN" */
    static const char* const UploadIncomplete;        /**< "UPLOAD_INCOMPLETE" */
    static const char* const UploadDigestMismatch;    /**< "UPLOAD_DIGEST_MISMATCH" */
    static const char* const OutOfRange;              /**< "OUT_OF_RANGE" */

    /** Status details with the code: { "code": <code> } */
    static void SetDetails( CORE::CDataNode& details, const char* code );

    /** The code of status details, empty when there is none */
    static CORE::CString GetCode( const CORE::CDataNode& details );

    private:

    CRemoteServiceDetailCodes( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

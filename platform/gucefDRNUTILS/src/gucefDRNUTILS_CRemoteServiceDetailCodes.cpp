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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CRemoteServiceDetailCodes::SessionUnknown = "SESSION_UNKNOWN";
const char* const CRemoteServiceDetailCodes::SessionLimit = "SESSION_LIMIT";
const char* const CRemoteServiceDetailCodes::SessionRequired = "SESSION_REQUIRED";
const char* const CRemoteServiceDetailCodes::ChannelLimit = "CHANNEL_LIMIT";
const char* const CRemoteServiceDetailCodes::Draining = "DRAINING";
const char* const CRemoteServiceDetailCodes::RoleRequired = "ROLE_REQUIRED";
const char* const CRemoteServiceDetailCodes::DeviceKind = "DEVICE_KIND";
const char* const CRemoteServiceDetailCodes::DeviceNotActive = "DEVICE_NOT_ACTIVE";
const char* const CRemoteServiceDetailCodes::DeviceUnknown = "DEVICE_UNKNOWN";
const char* const CRemoteServiceDetailCodes::NoRule = "NO_RULE";
const char* const CRemoteServiceDetailCodes::CommandIdReused = "COMMAND_ID_REUSED";
const char* const CRemoteServiceDetailCodes::BadCommandId = "BAD_COMMAND_ID";
const char* const CRemoteServiceDetailCodes::PairingClosed = "PAIRING_CLOSED";
const char* const CRemoteServiceDetailCodes::PairingDeclined = "PAIRING_DECLINED";
const char* const CRemoteServiceDetailCodes::BadProof = "BAD_PROOF";
const char* const CRemoteServiceDetailCodes::TokenRenewTooEarly = "TOKEN_RENEW_TOO_EARLY";
const char* const CRemoteServiceDetailCodes::UnsafePath = "UNSAFE_PATH";
const char* const CRemoteServiceDetailCodes::QuotaExceeded = "QUOTA_EXCEEDED";
const char* const CRemoteServiceDetailCodes::StagingFull = "STAGING_FULL";
const char* const CRemoteServiceDetailCodes::UploadLimit = "UPLOAD_LIMIT";
const char* const CRemoteServiceDetailCodes::UploadUnknown = "UPLOAD_UNKNOWN";
const char* const CRemoteServiceDetailCodes::UploadIncomplete = "UPLOAD_INCOMPLETE";
const char* const CRemoteServiceDetailCodes::UploadDigestMismatch = "UPLOAD_DIGEST_MISMATCH";
const char* const CRemoteServiceDetailCodes::OutOfRange = "OUT_OF_RANGE";

/*-------------------------------------------------------------------------*/

void
CRemoteServiceDetailCodes::SetDetails( CORE::CDataNode& details, const char* code )
{GUCEF_TRACE;

    details.Clear();
    details.SetNodeType( GUCEF_DATATYPE_OBJECT );
    details.SetName( "details" );
    details.SetAttribute( "code", CORE::CString( GUCEF_NULL != code ? code : "" ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteServiceDetailCodes::GetCode( const CORE::CDataNode& details )
{GUCEF_TRACE;

    return details.GetAttributeValueOrChildValueByName( "code" ).AsString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

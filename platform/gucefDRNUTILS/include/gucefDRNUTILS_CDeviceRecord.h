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

#ifndef GUCEF_DRNUTILS_CDEVICERECORD_H
#define GUCEF_DRNUTILS_CDEVICERECORD_H

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

#ifndef GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#include "gucefDRNUTILS_RemoteServiceTypes.h"
#define GUCEF_DRNUTILS_REMOTESERVICETYPES_H
#endif /* GUCEF_DRNUTILS_REMOTESERVICETYPES_H ? */

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
 *  One paired device in the device registry of a remote service (CDeviceRegistry). Instants are UTC milliseconds since
 *  the Unix epoch, 0 for "never". The public key is the device's Ed25519 key as 64 lower case hexadecimal characters.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceRecord
{
    public:

    CORE::CString deviceId;                 /**< "d-" and 26 Crockford base32 characters (CDevicePairingCrypto::GenerateDeviceId()) */
    CORE::CString userId;                   /**< the person the device belongs to; one owner in version 1 */
    CORE::CString name;
    CORE::CString kind;                     /**< the device kind the pairing window granted: "viewer", "worker", "relay", "cli", ... (service defined) */
    CORE::CString platform;
    CORE::CString appVersion;
    CORE::CString::StringSet roles;
    TDeviceStatus status;
    CORE::CString publicKey;
    CORE::CString fingerprint;              /**< CDevicePairingCrypto::Fingerprint() of the key */
    UInt64 pairedUtcMs;
    CORE::CString pairedBy;                 /**< "local-admin" or the device id of the admin device */
    UInt64 tokenExpiresUtcMs;               /**< of the last token issued */
    UInt64 lastSeenUtcMs;
    CORE::CString lastAddress;              /**< host of the last join, without the port */
    UInt64 revokedUtcMs;
    CORE::CString revokedBy;
    CORE::CString revokedReason;

    CDeviceRecord( void );

    bool IsActive( void ) const;
    bool HasRole( const CORE::CString& role ) const;
    bool HasAnyRole( const CORE::CString::StringSet& wanted ) const;

    /** The roles as one comma separated text, sorted */
    CORE::CString GetRolesText( void ) const;

    /** The JSON form of one device (a member of the registry's "devices" object) */
    void ToDataNode( CORE::CDataNode& node ) const;

    /** Reads one device; false with the reason when a required value (deviceId, status, publicKey) is missing or malformed */
    bool FromDataNode( const CORE::CDataNode& node, CORE::CString& error );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICERECORD_H ? */

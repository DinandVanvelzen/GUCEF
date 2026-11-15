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

#ifndef GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#define GUCEF_DRNUTILS_CDEVICEREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICERECORD_H
#include "gucefDRNUTILS_CDeviceRecord.h"
#define GUCEF_DRNUTILS_CDEVICERECORD_H
#endif /* GUCEF_DRNUTILS_CDEVICERECORD_H ? */

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
 *  The device registry of a remote service: "<state directory>/devices.json", document format
 *  "gucef.drnutils.DeviceRegistry" version 1, the devices as the members of the object "devices" keyed by device id.
 *  The registry is authoritative for the roles and the state of a device: the access policies read it at every join and
 *  every call, so a revocation or a role change applies at once.
 *  Save() replaces the file atomically (WriteFileAtomically: a reader or a crash sees the old or the new file, never a
 *  part), owner only (0600), and keeps the file it replaces as "devices.json.prev"; Load() reads devices.json and, when
 *  that cannot be read or parsed, the .prev file (logged). Needs the JSON codec of the data store codec registry
 *  (dstorepluginJSONPARSER). Thread safe: one mutex, held for copies only.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceRegistry
{
    public:

    static const char* const FileName;      /**< "devices.json" */
    static const char* const Format;        /**< "gucef.drnutils.DeviceRegistry" */
    static const UInt32 FormatVersion = 1;

    typedef GUCEF::vector< CDeviceRecord > TDeviceRecordVector;

    CDeviceRegistry( void );
    ~CDeviceRegistry();

    /** Reads the registry of the state directory (made when missing); a missing file is an empty registry. False with the reason when neither file can be read */
    bool Load( const CORE::CString& stateDirectory, CORE::CString& error );

    /** Writes the registry to the file of the state directory given at Load(); false with the reason */
    bool Save( CORE::CString& error );

    /** In memory only (tests, a service without a state directory): Save() does nothing and succeeds */
    void SetInMemory( void );

    bool Find( const CORE::CString& deviceId, CDeviceRecord& record ) const;

    /** The device of a public key (hexadecimal, any case), also a revoked one */
    bool FindByPublicKey( const CORE::CString& publicKeyHex, CDeviceRecord& record ) const;

    /** Adds or replaces the record of its device id */
    void Put( const CDeviceRecord& record );

    bool Remove( const CORE::CString& deviceId );

    void GetAll( TDeviceRecordVector& records ) const;
    UInt32 GetCount( void ) const;
    UInt32 GetActiveCount( void ) const;

    /** Notes a join: last seen and address; persisted with the next save */
    void NoteSeen( const CORE::CString& deviceId, UInt64 nowUtcMs, const CORE::CString& address );

    /** Changes since the last save */
    bool IsDirty( void ) const;

    /** Grows with every change; for tests and telemetry */
    UInt64 GetRevision( void ) const;

    CORE::CString GetFilePath( void ) const;

    private:

    typedef GUCEF::map< CORE::CString, CDeviceRecord > TDeviceMap;

    CDeviceRegistry( const CDeviceRegistry& src );              /**< not implemented */
    CDeviceRegistry& operator=( const CDeviceRegistry& src );   /**< not implemented */

    bool LoadFile( const CORE::CString& path, TDeviceMap& devices, CORE::CString& error ) const;

    mutable MT::CMutex m_lock;
    TDeviceMap m_devices;
    CORE::CString m_filePath;
    bool m_inMemory;
    bool m_dirty;
    UInt64 m_revision;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEREGISTRY_H ? */

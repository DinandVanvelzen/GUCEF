/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The values of settings that a client or an administrator set while the service ran, and the audit of those changes
 *  (gucefASSETSTORE_RemoteService_Design.md 18.4). With a VFS folder (the state folder of the service) both are kept in it:
 *  OverridesFileName (the key and the value of every override) and AuditFileName (the newest MaxAuditEntries entries), each
 *  written to a temporary file that is renamed in place (VFS::CVFS::MoveFileAtomically()), so a reader never sees half a file
 *  and a failed write leaves the previous one. Without a folder they live in memory. The keys and values are not checked here:
 *  the service checks them against its registry. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingsOverrideStore
{
    public:

    static const CORE::CString OverridesFileName;       /**< "settings-overrides.json" */
    static const CORE::CString AuditFileName;           /**< "settings-audit.json" */
    static const UInt32 MaxAuditEntries = 500;

    CAssetSettingsOverrideStore( void );
    virtual ~CAssetSettingsOverrideStore();

    /** The VFS folder of the files (a mount of the state folder); empty: in memory only. Forgets what was held */
    void SetFolder( const CORE::CString& vfsFolder );

    const CORE::CString& GetFolder( void ) const;

    bool IsPersistent( void ) const;

    /** Reads the overrides and the audit of the folder; a file that is not there is no overrides or no audit. False with the reason for a file that cannot be read */
    bool Load( CORE::CString& problem );

    /** The overrides by key */
    CORE::CString::StringMap GetOverrides( void ) const;

    /** The override of the key; false when it has none */
    bool GetOverride( const CORE::CString& key, CORE::CString& value ) const;

    /** Replaces all overrides and saves them; false with the reason when they could not be saved (they are held in memory then) */
    bool SetOverrides( const CORE::CString::StringMap& overrides, CORE::CString& problem );

    /** Adds an entry to the audit (the oldest goes beyond MaxAuditEntries) and saves the audit; false when it could not be saved */
    bool AddAuditEntry( const CORE::CDataNode& entry );

    UInt32 GetAuditCount( void ) const;

    /** The entries of the audit from the position, oldest first, at most maxEntries */
    void GetAuditEntries( UInt32 offset, UInt32 maxEntries, GUCEF::vector< CORE::CDataNode >& entries ) const;

    private:

    bool StoreAtomically( const CORE::CString& fileName, const CORE::CDataNode& root, CORE::CString& problem ) const;

    CAssetSettingsOverrideStore( const CAssetSettingsOverrideStore& src );              /**< not implemented */
    CAssetSettingsOverrideStore& operator=( const CAssetSettingsOverrideStore& src );   /**< not implemented */

    private:

    CORE::CString m_folder;
    CORE::CString::StringMap m_overrides;
    GUCEF::vector< CORE::CDataNode > m_audit;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSOVERRIDESTORE_H ? */

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

#ifndef GUCEF_DRN_CDRNBANLIST_H
#define GUCEF_DRN_CDRNBANLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_CIDRNBANLIST_H
#include "gucefDRN_CIDRNBanList.h"
#define GUCEF_DRN_CIDRNBANLIST_H
#endif /* GUCEF_DRN_CIDRNBANLIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A ban list kept in memory and in a file.
 *
 *  The file is UTF-8 text, one ban per line, fields separated by a tab:
 *      kind (user, key, address, account) TAB expiry (Unix seconds, 0 = never) TAB value TAB reason
 *  A tab, line break or backslash in the value or the reason is written as \t, \n, \r or \\. Lines that start
 *  with # are comments. With a file path set (SetFilePath()) every change is saved right away.
 *  As a CORE::CIConfigurable it also reads and writes a CDataNode (children "Ban" with the same four fields).
 *  Thread-safe.
 */
class GUCEF_DRN_EXPORT_CPP CDRNBanList : public CIDRNBanList ,
                                         public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CDRNBanList( void );

    virtual ~CDRNBanList();

    virtual bool IsBanned( TDRNBanKind kind, const CORE::CString& value, CDRNBanEntry* entry = GUCEF_NULL ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Adds the ban or replaces the ban of the same kind and value */
    bool Ban( TDRNBanKind kind, const CORE::CString& value, UInt64 expiryUnixSeconds = 0, const CORE::CString& reason = CORE::CString::Empty );

    bool Ban( const CDRNBanEntry& entry );

    /** @return true if there was such a ban */
    bool Unban( TDRNBanKind kind, const CORE::CString& value );

    void Clear( void );

    /** Bans that have not expired */
    UInt32 GetCount( void ) const;

    void GetEntries( TDRNBanEntryVector& entries ) const;

    /** Removes the bans that expired, returns how many */
    UInt32 PurgeExpired( void );

    /** The file every change is saved to, empty = none */
    void SetFilePath( const CORE::CString& filePath );

    const CORE::CString& GetFilePath( void ) const;

    /** Replaces the bans with those in the file; a file that does not exist is an empty list */
    bool LoadFromFile( const CORE::CString& filePath );

    /** Writes the bans that have not expired */
    bool SaveToFile( const CORE::CString& filePath ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Now in Unix seconds, the time base of the expiry */
    static UInt64 GetUnixTimeNow( void );

    private:

    CDRNBanList( const CDRNBanList& src );
    CDRNBanList& operator=( const CDRNBanList& src );

    Int32 Find( TDRNBanKind kind, const CORE::CString& value ) const;
    bool SaveIfFileSet( void ) const;

    TDRNBanEntryVector m_entries;
    CORE::CString m_filePath;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNBANLIST_H ? */

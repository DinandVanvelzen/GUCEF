/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_WEBUSERS_H
#define GUCEF_LABLINK_WEBUSERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "gucefLabLink_Store.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The roles of the users of the website */
extern const char* WebRoleAdmin;            /**< everything, the management of users included */
extern const char* WebRoleViewer;           /**< reads the status of the lab */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A user of the website */
struct SWebUser
{
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString role;                  /**< WebRole... */
    bool disabled;
    GUCEF::UInt64 createdInUnixSecs;
    GUCEF::CORE::CString record;                /**< the record of the password, see CPasswordHash; never the password */

    SWebUser( void ) : name(), role(), disabled( false ), createdInUnixSecs( 0 ), record() {}
};

/**
 *  The users of the website, in the file "users" of the state directory (CStore: readable by its owner only, replaced
 *  atomically), one user per line: name, role, "enabled" or "disabled", created (unix seconds) and the record of the
 *  password, tab separated. The command line changes the file while the service runs: the service reads it again
 *  (Refresh) and learns which users changed, whose sessions then end. Every change reads the file first, so that a
 *  change of the command line and one of the website do not undo each other.
 */
class CWebUserStore
{
    public:

    explicit CWebUserStore( CStore& store );

    /** Reads the file now; a missing file is no users */
    bool Load( GUCEF::CORE::CString& problem );

    /**
     *  Reads the file again when it was read longer than intervalInMs ago, or at once when forced. Users that were
     *  removed, disabled, given another role or another password since the last read, by the file or by a change
     *  through this store, are added to 'changed'.
     *  @return false when the file cannot be read; the users stay as they were then
     */
    bool Refresh( GUCEF::UInt64 nowInMs, GUCEF::UInt32 intervalInMs, GUCEF::CORE::CString::StringVector& changed );

    bool Find( const GUCEF::CORE::CString& name, SWebUser& user ) const;

    void GetAll( GUCEF::vector< SWebUser >& users ) const;

    GUCEF::UInt32 GetCount( void ) const;

    /** Adds a user; fails when there is one of that name */
    bool Add( const SWebUser& user, GUCEF::CORE::CString& problem );

    /**
     *  A new password record, which ends the sessions of the user at the next Refresh.
     *  @param upgradeOf  for a record of the same password with other parameters: the record it replaces. Nothing is
     *                    written when the file holds another record by now, and the sessions stay
     */
    bool SetRecord( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& record, GUCEF::CORE::CString& problem,
                    const GUCEF::CORE::CString* upgradeOf = GUCEF_NULL );

    bool SetDisabled( const GUCEF::CORE::CString& name, bool disabled, GUCEF::CORE::CString& problem );

    bool Remove( const GUCEF::CORE::CString& name, GUCEF::CORE::CString& problem );

    /** 1 to 64 of the letters, digits, '.', '_', '-' and '@' */
    static bool IsValidName( const GUCEF::CORE::CString& name );

    static bool IsValidRole( const GUCEF::CORE::CString& role );

    static GUCEF::CORE::CString Serialize( const GUCEF::vector< SWebUser >& users );

    /** Lines that are not a user are skipped */
    static void Parse( const GUCEF::CORE::CString& content, GUCEF::vector< SWebUser >& users );

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, SWebUser > TUserMap;

    CWebUserStore( const CWebUserStore& src );
    CWebUserStore& operator=( const CWebUserStore& src );

    bool ReadLocked( TUserMap& users, GUCEF::CORE::CString& content, GUCEF::CORE::CString& problem ) const;
    bool WriteLocked( const TUserMap& users, GUCEF::CORE::CString& problem );
    void Adopt( const TUserMap& users, const GUCEF::CORE::CString& content, GUCEF::CORE::CString::StringVector* changed );

    CStore& m_store;
    TUserMap m_users;
    GUCEF::CORE::CString m_digest;              /**< of the text last read or written */
    GUCEF::CORE::CString::StringVector m_pendingChanged;    /**< users a change of this store changed, for the next Refresh */
    GUCEF::UInt64 m_lastReadInMs;
    bool m_read;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBUSERS_H ? */

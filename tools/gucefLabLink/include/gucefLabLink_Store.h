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

#ifndef GUCEF_LABLINK_STORE_H
#define GUCEF_LABLINK_STORE_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A machine that asked the cluster for access */
struct SApproval
{
    GUCEF::CORE::CString state;         /**< AccessPending, AccessApproved or AccessRefused */
    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString address;
    GUCEF::CORE::CString os;
    GUCEF::CORE::CString token;         /**< the token of an approved machine */
};

/** What a machine keeps about itself and the cluster it was approved by */
struct SIdentity
{
    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString token;         /**< from the approval, empty while there is none */
    GUCEF::CORE::CString clusterKey;    /**< the public key of the lab endpoint of the cluster that approved it */
    GUCEF::CORE::CString cluster;
};

/**
 *  The files of gucefLabLink in its state directory, each readable by its owner only: "secret" (the lab secret),
 *  "identity" (SIdentity as key=value lines), "approvals" (one tab separated SApproval per line). Files are replaced
 *  atomically; a file that others can read is refused.
 */
class CStore
{
    public:

    explicit CStore( const GUCEF::CORE::CString& stateDir );

    const GUCEF::CORE::CString& GetStateDir( void ) const;

    /** @return false when there is none or it cannot be read; problem says which */
    bool ReadSecret( GUCEF::CORE::CString& secret, GUCEF::CORE::CString& problem ) const;
    bool WriteSecret( const GUCEF::CORE::CString& secret, GUCEF::CORE::CString& problem ) const;

    /** Reads the identity; a missing file gives a new one, with a new machine id, which is written */
    bool LoadIdentity( SIdentity& identity, GUCEF::CORE::CString& problem ) const;
    bool SaveIdentity( const SIdentity& identity, GUCEF::CORE::CString& problem ) const;

    /** The approvals, read again on every call: the command line changes them while the service runs */
    bool LoadApprovals( GUCEF::vector< SApproval >& approvals, GUCEF::CORE::CString& problem ) const;
    bool SaveApprovals( const GUCEF::vector< SApproval >& approvals, GUCEF::CORE::CString& problem ) const;

    /** A file of the state directory */
    GUCEF::CORE::CString GetPath( const char* fileName ) const;

    private:

    bool ReadFile( const char* fileName, GUCEF::CORE::CString& content, bool& missing, GUCEF::CORE::CString& problem ) const;
    bool WriteFile( const char* fileName, const GUCEF::CORE::CString& content, GUCEF::CORE::CString& problem ) const;

    GUCEF::CORE::CString m_stateDir;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_STORE_H ? */

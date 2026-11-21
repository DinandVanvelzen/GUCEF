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

#ifndef GUCEF_LABLINK_HUBSTORE_H
#define GUCEF_LABLINK_HUBSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

#include "gucefLabLink_Settings.h"
#include "gucefLabLink_Registry.h"

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

/**
 *  What the hub keeps of its nodes, in tables of the database abstraction of GUCEF (gucefCOMCORE CIDatabaseClient):
 *  "nodes" (one row per machine id), "node_events" (the history of online, offline, state and instance changes) and
 *  "schema_version". The backend is chosen by the settings: "vfstables", tables as Parquet files in a directory that is
 *  mounted in the VFS, or "postgresql". Both come from plugins that Open() loads next to the program. Credentials and
 *  trust (the lab secret, approvals, the key of the lab endpoint) are not kept here: they stay in CStore.
 *  Thread safe.
 */
class CHubStore
{
    public:

    explicit CHubStore( const SSettings& settings );

    ~CHubStore();

    /** Loads the plugins of the backend, opens the database and makes the tables. @return false when that is not possible; problem says why */
    bool Open( GUCEF::CORE::CString& problem );

    void Close( void );

    bool IsOpen( void ) const;

    /** All nodes that are stored; liveness is that of the last time they were written */
    bool LoadNodes( GUCEF::vector< SNodeEntry >& nodes, GUCEF::CORE::CString& problem );

    /** Writes the nodes: a node that is stored already is replaced */
    bool SaveNodes( const GUCEF::vector< SNodeEntry >& nodes, GUCEF::CORE::CString& problem );

    bool DeleteNodes( const GUCEF::CORE::CString::StringVector& machineIds, GUCEF::CORE::CString& problem );

    bool AppendEvents( const GUCEF::vector< SNodeEvent >& events, GUCEF::CORE::CString& problem );

    /** The events of one node, oldest first */
    bool LoadEvents( const GUCEF::CORE::CString& machineId, GUCEF::vector< SNodeEvent >& events, GUCEF::CORE::CString& problem );

    /** Removes the events older than the retention, and the oldest of what is left when there are more than the limit */
    bool PurgeEvents( GUCEF::UInt64 unixNowInSecs, GUCEF::UInt32 retentionInDays, GUCEF::UInt32 maxEvents, GUCEF::CORE::CString& problem );

    private:

    CHubStore( const CHubStore& src );
    CHubStore& operator=( const CHubStore& src );

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CVariant > TRow;
    typedef GUCEF::vector< TRow > TRowVector;

    bool SetUpBackend( GUCEF::COMCORE::CDatabaseClientSettings& databaseSettings, GUCEF::CORE::CString& problem );
    bool EnsureTables( GUCEF::CORE::CString& problem );
    bool WriteRows( GUCEF::CORE::CIDataTablePtr table, const TRowVector& rows, GUCEF::CORE::CString& problem );
    bool ReadRows( GUCEF::CORE::CIDataTablePtr table, const GUCEF::CORE::CDataTableQuery& query, TRowVector& rows, GUCEF::CORE::CString& problem );
    bool DeleteWhere( GUCEF::CORE::CIDataTablePtr table, const GUCEF::CORE::CDataTableQuery& query, GUCEF::CORE::CString& problem );
    void CloseUnlocked( void );

    SSettings m_settings;
    GUCEF::COMCORE::CIDatabaseClientPtr m_client;
    GUCEF::CORE::CIDataTablePtr m_nodes;
    GUCEF::CORE::CIDataTablePtr m_events;
    GUCEF::CORE::CIDataTablePtr m_version;
    GUCEF::CORE::CString m_mountName;
    GUCEF::UInt64 m_eventCounter;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_HUBSTORE_H ? */

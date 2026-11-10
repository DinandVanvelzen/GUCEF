/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CDATALAYOUT_H
#define ANCESTRYBACKUP_CDATALAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where everything the tool stores for one tree lives:
 *
 *    <dataDir>/<treeId>/  browser-profile/  baseline.ged  endpoints.ini  discovery/
 *                         raw/  media/  state/  export/
 *
 *  Only computes paths, creating the directories is up to the commands that need them.
 */
class CDataLayout
{
    public:

    CDataLayout( void );

    CDataLayout( const CORE::CString& dataDir, const CORE::CString& treeId );

    /**
     *  A tree id becomes a directory name so it may only contain letters, digits, '-', '_' and '.'
     *  and may not be '.' or '..'
     */
    static bool IsValidTreeId( const CORE::CString& treeId );

    bool IsValid( void ) const;

    const CORE::CString& GetDataDir( void ) const;

    const CORE::CString& GetTreeId( void ) const;

    CORE::CString GetTreeDir( void ) const;

    CORE::CString GetBrowserProfileDir( void ) const;

    CORE::CString GetBaselineGedcomPath( void ) const;

    /** Every export that was taken in is kept here under its own name, next to the latest one in baseline.ged */
    CORE::CString GetBaselineSnapshotsDir( void ) const;

    CORE::CString GetEndpointsConfigPath( void ) const;

    CORE::CString GetDiscoveryDir( void ) const;

    CORE::CString GetDiscoveryBodiesDir( void ) const;

    /** The journal of everything observed during discovery, one JSON document per line */
    CORE::CString GetDiscoveryJournalPath( void ) const;

    /** The human readable summary of the endpoints that were observed */
    CORE::CString GetEndpointsReportPath( void ) const;

    CORE::CString GetRawDir( void ) const;

    CORE::CString GetMediaDir( void ) const;

    CORE::CString GetStateDir( void ) const;

    CORE::CString GetExportDir( void ) const;

    /** Creates the directories discovery writes to, including the browser profile */
    bool CreateDiscoveryDirs( void ) const;

    private:

    CORE::CString m_dataDir;
    CORE::CString m_treeId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CDATALAYOUT_H ? */

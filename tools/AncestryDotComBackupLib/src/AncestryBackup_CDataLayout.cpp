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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AncestryBackup_CDataLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataLayout::CDataLayout( void )
    : m_dataDir()
    , m_treeId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataLayout::CDataLayout( const CORE::CString& dataDir, const CORE::CString& treeId )
    : m_dataDir( dataDir )
    , m_treeId( treeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataLayout::IsValidTreeId( const CORE::CString& treeId )
{GUCEF_TRACE;

    if ( treeId.IsNULLOrEmpty() || treeId == "." || treeId == ".." )
        return false;

    const char* id = treeId.C_String();
    for ( UInt32 i=0; id[ i ] != '\0'; ++i )
    {
        const char c = id[ i ];
        const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
                        c == '-' || c == '_' || c == '.';
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataLayout::IsValid( void ) const
{GUCEF_TRACE;

    return !m_dataDir.IsNULLOrEmpty() && IsValidTreeId( m_treeId );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDataLayout::GetDataDir( void ) const
{GUCEF_TRACE;

    return m_dataDir;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDataLayout::GetTreeId( void ) const
{GUCEF_TRACE;

    return m_treeId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetTreeDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_dataDir, m_treeId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetBrowserProfileDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "browser-profile" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetBaselineGedcomPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "baseline.ged" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetBaselineSnapshotsDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "baselines" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetEndpointsConfigPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "endpoints.ini" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetDiscoveryDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "discovery" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetDiscoveryBodiesDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetDiscoveryDir(), "bodies" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetDiscoveryJournalPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetDiscoveryDir(), "requests.jsonl" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetEndpointsReportPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetDiscoveryDir(), "endpoints.md" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetRawDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "raw" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetMediaDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "media" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetStateDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "state" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDataLayout::GetExportDir( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( GetTreeDir(), "export" );
}

/*-------------------------------------------------------------------------*/

bool
CDataLayout::CreateDiscoveryDirs( void ) const
{GUCEF_TRACE;

    if ( !IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Data directory or tree id is not valid: \"" + m_dataDir + "\" / \"" + m_treeId + "\"" );
        return false;
    }
    return CORE::CreateDirs( GetBrowserProfileDir() ) && CORE::CreateDirs( GetDiscoveryBodiesDir() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  AssetConverterLib: the import activity log of an intermediate folder
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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CImportActivityLog.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImportActivityLog::CImportActivityLog( void )
    : m_runs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CImportActivityLog::TRunVector&
CImportActivityLog::GetRuns( void ) const
{GUCEF_TRACE;

    return m_runs;
}

/*-------------------------------------------------------------------------*/

void
CImportActivityLog::AddRun( const SRun& run )
{GUCEF_TRACE;

    m_runs.push_back( run );
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityLog::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* runs = node.AddChild( "runs", GUCEF_DATATYPE_ARRAY );
    for ( size_t r=0; r<m_runs.size(); ++r )
    {
        CORE::CDataNode* run = runs->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        run->SetAttribute( "time", m_runs[ r ].time );
        run->SetAttribute( "pack", m_runs[ r ].packId );
        CORE::CDataNode* entries = run->AddChild( "entries", GUCEF_DATATYPE_ARRAY );
        for ( size_t e=0; e<m_runs[ r ].entries.size(); ++e )
            m_runs[ r ].entries[ e ].Serialize( *entries->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityLog::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    m_runs.clear();
    const CORE::CDataNode* runs = node.FindChild( "runs" );
    if ( GUCEF_NULL == runs )
        return false;
    CORE::CDataNode::const_iterator r = runs->ConstBegin();
    while ( r != runs->ConstEnd() )
    {
        SRun run;
        run.time = (*r)->GetAttributeValueOrChildValueByName( "time" ).AsString();
        run.packId = (*r)->GetAttributeValueOrChildValueByName( "pack" ).AsString();
        const CORE::CDataNode* entries = (*r)->FindChild( "entries" );
        if ( GUCEF_NULL != entries )
        {
            CORE::CDataNode::const_iterator e = entries->ConstBegin();
            while ( e != entries->ConstEnd() )
            {
                CImportActivityEntry entry;
                if ( entry.Deserialize( *(*e) ) )
                    run.entries.push_back( entry );
                ++e;
            }
        }
        m_runs.push_back( run );
        ++r;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityLog::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityLog::Save( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "activity", GUCEF_DATATYPE_OBJECT );
    Serialize( node );
    return CCanonicalJson::Store( vfsPath, node );
}

/*-------------------------------------------------------------------------*/

bool
CImportActivityLog::AppendRun( const CORE::CString& vfsPath, const SRun& run )
{GUCEF_TRACE;

    CImportActivityLog log;
    if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) )
        log.Load( vfsPath );
    log.AddRun( run );
    return log.Save( vfsPath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImportActivityLog::GetLogPath( const CORE::CString& intermediateDir )
{GUCEF_TRACE;

    return CVfsPaths::Join( intermediateDir, "activity.json" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImportActivityLog::GetUtcTimeText( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToIso8601DateTimeString( true, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */

#include "gucefASSETSTORE_CAssetConversionReportSummary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const g_statusNames[] = { "converted", "approximated", "skipped", "failed", "cached", "refused" };
const UInt32 g_statusCount = 6;

bool
LoadJson( const CORE::CString& vfsPath, CORE::CDataNode& root )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer bytes;
    if ( vfsPath.IsNULLOrEmpty() || !vfs.FileExists( vfsPath ) || !vfs.LoadFile( bytes, vfsPath, "rb" ) )
        return false;
    const CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    root.Clear();
    return CAssetStoreJson::FromText( text, root ) && GUCEF_DATATYPE_OBJECT == root.GetNodeType();
}

void
AddChange( CORE::CDataNode& list, UInt32 maxListed, const CORE::CString& asset, const CORE::CString& before, const CORE::CString& after )
{GUCEF_TRACE;

    if ( list.GetNrOfChildNodes() >= maxListed )
        return;
    CORE::CDataNode* entry = list.AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
    entry->SetAttribute( "asset", asset );
    entry->SetAttribute( "before", before.IsNULLOrEmpty() ? CORE::CString( "none" ) : before );
    entry->SetAttribute( "after", after.IsNULLOrEmpty() ? CORE::CString( "none" ) : after );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const*
CAssetConversionReportSummary::GetStatusNames( UInt32& count )
{GUCEF_TRACE;

    count = g_statusCount;
    return g_statusNames;
}

/*-------------------------------------------------------------------------*/

CAssetConversionReportSummary::CAssetConversionReportSummary( void )
    : found( false )
    , reportPath()
    , assets()
    , counts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::Clear( void )
{GUCEF_TRACE;

    found = false;
    reportPath.Clear();
    assets.clear();
    counts.clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::CountAssets( void )
{GUCEF_TRACE;

    counts.clear();
    for ( TStatusMap::const_iterator a = assets.begin(); a != assets.end(); ++a )
        ++counts[ (*a).second ];
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionReportSummary::Load( const CORE::CString& reportVfsPath )
{GUCEF_TRACE;

    Clear();
    reportPath = reportVfsPath;
    CORE::CDataNode root;
    if ( !LoadJson( reportVfsPath, root ) )
        return false;
    found = true;
    const CORE::CDataNode* list = root.FindChild( "assets" );
    if ( GUCEF_NULL != list && list->GetNrOfChildNodes() > 0 )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        for ( ; i != list->ConstEnd(); ++i )
        {
            const CORE::CString asset = (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString( CORE::CString::Empty, true );
            const CORE::CString status = (*i)->GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true );
            if ( !asset.IsNULLOrEmpty() )
                assets[ asset ] = status;
        }
        CountAssets();
        return true;
    }
    const CORE::CDataNode* countNode = root.FindChild( "counts" );
    for ( UInt32 s=0; s<g_statusCount; ++s )
    {
        const UInt32 count = GUCEF_NULL != countNode ? countNode->GetAttributeValueOrChildValueByName( g_statusNames[ s ] ).AsUInt32( 0, true )
                                                     : root.GetAttributeValueOrChildValueByName( g_statusNames[ s ] ).AsUInt32( 0, true );
        if ( count > 0 )
            counts[ g_statusNames[ s ] ] = count;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetConversionReportSummary::GetCount( const CORE::CString& status ) const
{GUCEF_TRACE;

    TCountMap::const_iterator i = counts.find( status );
    return i == counts.end() ? 0 : (*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetConversionReportSummary::GetTotal( void ) const
{GUCEF_TRACE;

    UInt32 total = 0;
    for ( TCountMap::const_iterator i = counts.begin(); i != counts.end(); ++i )
        total += (*i).second;
    return total;
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::Merge( const CAssetConversionReportSummary& newer )
{GUCEF_TRACE;

    if ( !newer.found )
        return;
    for ( TStatusMap::const_iterator a = newer.assets.begin(); a != newer.assets.end(); ++a )
        assets[ (*a).first ] = (*a).second;
    found = true;
    CountAssets();
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::GetAssetsWithStatus( const CORE::CString::StringSet& statuses, CORE::CString::StringVector& paths ) const
{GUCEF_TRACE;

    paths.clear();
    for ( TStatusMap::const_iterator a = assets.begin(); a != assets.end(); ++a )
        if ( statuses.end() != statuses.find( (*a).second ) )
            paths.push_back( (*a).first );
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::SerializeCounts( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "found", found );
    for ( UInt32 s=0; s<g_statusCount; ++s )
        node.SetAttribute( g_statusNames[ s ], GetCount( g_statusNames[ s ] ) );
    node.SetAttribute( "total", GetTotal() );
}

/*-------------------------------------------------------------------------*/

Int32
CAssetConversionReportSummary::Rank( const CORE::CString& status )
{GUCEF_TRACE;

    if ( "converted" == status || "cached" == status )
        return 3;
    if ( "approximated" == status )
        return 2;
    if ( "skipped" == status )
        return 1;
    if ( "failed" == status || "refused" == status )
        return 0;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CAssetConversionReportSummary::Compare( const CAssetConversionReportSummary& before, const CAssetConversionReportSummary& after, UInt32 maxListed,
                                        CORE::CDataNode& changes, UInt32& improved, UInt32& regressed )
{GUCEF_TRACE;

    improved = 0;
    regressed = 0;
    UInt32 stillFailing = 0;
    UInt32 unchanged = 0;
    CORE::CDataNode* improvedList = changes.AddChild( "improved", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* regressedList = changes.AddChild( "regressed", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNode* failingList = changes.AddChild( "stillFailing", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet paths;
    for ( TStatusMap::const_iterator a = before.assets.begin(); a != before.assets.end(); ++a )
        paths.insert( (*a).first );
    for ( TStatusMap::const_iterator a = after.assets.begin(); a != after.assets.end(); ++a )
        paths.insert( (*a).first );
    for ( CORE::CString::StringSet::const_iterator p = paths.begin(); p != paths.end(); ++p )
    {
        TStatusMap::const_iterator b = before.assets.find( *p );
        TStatusMap::const_iterator f = after.assets.find( *p );
        const CORE::CString was = b == before.assets.end() ? CORE::CString::Empty : (*b).second;
        const CORE::CString now = f == after.assets.end() ? CORE::CString::Empty : (*f).second;
        const Int32 wasRank = Rank( was );
        const Int32 nowRank = Rank( now );
        if ( nowRank > wasRank && !( wasRank < 0 && nowRank <= 0 ) )
        {
            ++improved;
            AddChange( *improvedList, maxListed, *p, was, now );
        }
        else
        if ( nowRank < wasRank && !( nowRank < 0 && after.assets.empty() ) )
        {
            ++regressed;
            AddChange( *regressedList, maxListed, *p, was, now );
        }
        else
        if ( 0 == nowRank )
        {
            ++stillFailing;
            AddChange( *failingList, maxListed, *p, was, now );
        }
        else
            ++unchanged;
    }
    changes.SetAttribute( "improvedCount", improved );
    changes.SetAttribute( "regressedCount", regressed );
    changes.SetAttribute( "stillFailingCount", stillFailing );
    changes.SetAttribute( "unchangedCount", unchanged );
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionReportSummary::MergeReportFiles( const CORE::CString& olderVfsPath, const CORE::CString& newerVfsPath, const CORE::CString& targetVfsPath )
{GUCEF_TRACE;

    CORE::CDataNode newer;
    if ( !LoadJson( newerVfsPath, newer ) )
        return false;
    CORE::CDataNode older;
    CORE::CDataNode merged( newer );
    const CORE::CDataNode* newerAssets = newer.FindChild( "assets" );
    if ( LoadJson( olderVfsPath, older ) && GUCEF_NULL != older.FindChild( "assets" ) && GUCEF_NULL != newerAssets )
    {
        // the entries of the older report, those of the newer one in their place, in the order of the older one; new ones at the end
        GUCEF::map< CORE::CString, const CORE::CDataNode* > fresh;
        CORE::CDataNode::const_iterator i = newerAssets->ConstBegin();
        for ( ; i != newerAssets->ConstEnd(); ++i )
            fresh[ (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString( CORE::CString::Empty, true ) ] = (*i);
        CORE::CDataNode list( "assets", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringSet written;
        const CORE::CDataNode* olderAssets = older.FindChild( "assets" );
        for ( i = olderAssets->ConstBegin(); i != olderAssets->ConstEnd(); ++i )
        {
            const CORE::CString asset = (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString( CORE::CString::Empty, true );
            GUCEF::map< CORE::CString, const CORE::CDataNode* >::const_iterator n = fresh.find( asset );
            list.AddChild( n == fresh.end() ? *(*i) : *(*n).second );
            written.insert( asset );
        }
        for ( i = newerAssets->ConstBegin(); i != newerAssets->ConstEnd(); ++i )
            if ( written.end() == written.find( (*i)->GetAttributeValueOrChildValueByName( "asset" ).AsString( CORE::CString::Empty, true ) ) )
                list.AddChild( *(*i) );
        merged.DelChild( "assets" );
        // the counts of the merged list
        TCountMap mergedCounts;
        for ( i = list.ConstBegin(); i != list.ConstEnd(); ++i )
            ++mergedCounts[ (*i)->GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true ) ];
        CORE::CDataNode* countNode = merged.FindChild( "counts" );
        if ( GUCEF_NULL == countNode )
            countNode = merged.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        for ( UInt32 s=0; s<g_statusCount; ++s )
        {
            TCountMap::const_iterator c = mergedCounts.find( g_statusNames[ s ] );
            countNode->SetAttribute( g_statusNames[ s ], c == mergedCounts.end() ? (UInt32) 0 : (*c).second );
        }
        merged.AddChild( list );
    }
    CORE::CString text;
    if ( !CAssetStoreJson::ToText( merged, text ) )
        return false;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer bytes;
    bytes.Append( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    return vfs.StoreAsFile( targetVfsPath, bytes, 0, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchEntry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchEntry::CMatchEntry( void )
    : CORE::CIDataNodeSerializable()
    , preview()
    , info()
    , hasInfo( false )
    , providerName()
    , lastSeenRevision( 0 )
    , unseen( false )
    , muted( false )
    , pinned( false )
    , archived( false )
    , cachedPackagePath()
    , yourTurnSince( 0 )
    , notifiedRevision( 0 )
    , cursor()
    , localTitle()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchEntry::CMatchEntry( const CMatchEntry& src )
    : CORE::CIDataNodeSerializable( src )
    , preview( src.preview )
    , info( src.info )
    , hasInfo( src.hasInfo )
    , providerName( src.providerName )
    , lastSeenRevision( src.lastSeenRevision )
    , unseen( src.unseen )
    , muted( src.muted )
    , pinned( src.pinned )
    , archived( src.archived )
    , cachedPackagePath( src.cachedPackagePath )
    , yourTurnSince( src.yourTurnSince )
    , notifiedRevision( src.notifiedRevision )
    , cursor( src.cursor )
    , localTitle( src.localTitle )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchEntry::~CMatchEntry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchEntry&
CMatchEntry::operator=( const CMatchEntry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        preview = src.preview;
        info = src.info;
        hasInfo = src.hasInfo;
        providerName = src.providerName;
        lastSeenRevision = src.lastSeenRevision;
        unseen = src.unseen;
        muted = src.muted;
        pinned = src.pinned;
        archived = src.archived;
        cachedPackagePath = src.cachedPackagePath;
        yourTurnSince = src.yourTurnSince;
        notifiedRevision = src.notifiedRevision;
        cursor = src.cursor;
        localTitle = src.localTitle;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchEntry::GetMatchId( void ) const
{GUCEF_TRACE;

    return preview.matchId;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchEntry::GetDisplayTitle( void ) const
{GUCEF_TRACE;

    if ( !localTitle.IsNULLOrEmpty() )
        return localTitle;
    if ( hasInfo && !info.title.IsNULLOrEmpty() )
        return info.title;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CMatchEntry::Serialize( CORE::CDataNode& domRootNode                        ,
                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "providerName", providerName );
    domRootNode.SetAttribute( "lastSeenRevision", lastSeenRevision );
    domRootNode.SetAttribute( "unseen", unseen );
    domRootNode.SetAttribute( "muted", muted );
    domRootNode.SetAttribute( "pinned", pinned );
    domRootNode.SetAttribute( "archived", archived );
    domRootNode.SetAttribute( "cachedPackagePath", cachedPackagePath );
    domRootNode.SetAttribute( "yourTurnSince", yourTurnSince );
    domRootNode.SetAttribute( "notifiedRevision", notifiedRevision );
    domRootNode.SetAttribute( "cursor", cursor );
    domRootNode.SetAttribute( "localTitle", localTitle );
    preview.Serialize( *domRootNode.AddChild( "preview", GUCEF_DATATYPE_OBJECT ), settings );
    if ( hasInfo )
        info.Serialize( *domRootNode.AddChild( "info", GUCEF_DATATYPE_OBJECT ), settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchEntry::Deserialize( const CORE::CDataNode& domRootNode                  ,
                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    providerName = domRootNode.GetAttributeValueOrChildValueByName( "providerName" ).AsString( CORE::CString::Empty, true );
    lastSeenRevision = domRootNode.GetAttributeValueOrChildValueByName( "lastSeenRevision" ).AsUInt32( 0, true );
    unseen = domRootNode.GetAttributeValueOrChildValueByName( "unseen" ).AsBool( false, true );
    muted = domRootNode.GetAttributeValueOrChildValueByName( "muted" ).AsBool( false, true );
    pinned = domRootNode.GetAttributeValueOrChildValueByName( "pinned" ).AsBool( false, true );
    archived = domRootNode.GetAttributeValueOrChildValueByName( "archived" ).AsBool( false, true );
    cachedPackagePath = domRootNode.GetAttributeValueOrChildValueByName( "cachedPackagePath" ).AsString( CORE::CString::Empty, true );
    yourTurnSince = domRootNode.GetAttributeValueOrChildValueByName( "yourTurnSince" ).AsInt64( 0, true );
    notifiedRevision = domRootNode.GetAttributeValueOrChildValueByName( "notifiedRevision" ).AsUInt32( 0, true );
    cursor = domRootNode.GetAttributeValueOrChildValueByName( "cursor" ).AsString( CORE::CString::Empty, true );
    localTitle = domRootNode.GetAttributeValueOrChildValueByName( "localTitle" ).AsString( CORE::CString::Empty, true );

    preview = CMatchPreview();
    const CORE::CDataNode* previewNode = domRootNode.FindChild( "preview" );
    if ( GUCEF_NULL == previewNode || !preview.Deserialize( *previewNode, settings ) )
        return false;

    info = CMatchInfo();
    hasInfo = false;
    const CORE::CDataNode* infoNode = domRootNode.FindChild( "info" );
    if ( GUCEF_NULL != infoNode )
    {
        hasInfo = info.Deserialize( *infoNode, settings ) && info.matchId == preview.matchId;
        if ( !hasInfo )
            info = CMatchInfo();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchEntry::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchEntry( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchEntry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchEntry";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchEntry::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchEntry::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchEntry";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

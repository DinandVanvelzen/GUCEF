/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "matchprovidermail_CEMailMatchState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 g_maxList = 4096;
static const UInt32 g_maxRevisions = 200000;

/*-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
JoinList( const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<list.size(); ++i )
        joined += ( i > 0 ? " " : "" ) + list[ i ];
    return joined;
}

/*-------------------------------------------------------------------------*/

static void
SaveTimeMap( CORE::CDataNode& parent, const char* name, const GUCEF::map< CORE::CString, Int64 >& values )
{GUCEF_TRACE;

    CORE::CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_OBJECT );
    GUCEF::map< CORE::CString, Int64 >::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        node->SetAttribute( (*i).first, (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

static void
LoadTimeMap( const CORE::CDataNode& parent, const char* name, GUCEF::map< CORE::CString, Int64 >& values )
{GUCEF_TRACE;

    values.clear();
    const CORE::CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    CORE::CDataNode::TAttributeMap::const_iterator a = node->AttributeBegin();
    while ( a != node->AttributeEnd() && values.size() < g_maxList )
    {
        values[ (*a).first ] = (*a).second.AsInt64( 0, true );
        ++a;
    }
}

/*-------------------------------------------------------------------------*/

template< class TItem >
static void
SaveList( CORE::CDataNode& parent, const char* name, const char* itemName, const GUCEF::vector< TItem >& items, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CORE::CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<items.size(); ++i )
        items[ i ].Serialize( *node->AddChild( itemName, GUCEF_DATATYPE_OBJECT ), settings );
}

/*-------------------------------------------------------------------------*/

template< class TItem >
static void
LoadList( const CORE::CDataNode& parent, const char* name, GUCEF::vector< TItem >& items, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    items.clear();
    const CORE::CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    CORE::CDataNode::const_iterator i = node->ConstBegin();
    while ( i != node->ConstEnd() && items.size() < g_maxList )
    {
        TItem item;
        if ( item.Deserialize( *(*i), settings ) )
            items.push_back( item );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CEMailMatchState::CEMailMatchState( void )
    : CORE::CIDataNodeSerializable(), record(), hasRecord( false ), info(), acceptances(), resigns(), proposals(), votes(), appliedDecisions(), trustedRevision( 0 ), shaByRevision(), refusals(), equivocations()
    , lastMessageId(), references(), nudgesSent(), nudgesReceived(), resendRequested(), resendAnswered(), deliveredRevisions(), lastActivity( 0 ), cursor() {}
CEMailMatchState::CEMailMatchState( const CEMailMatchState& src )
    : CORE::CIDataNodeSerializable( src ), record( src.record ), hasRecord( src.hasRecord ), info( src.info ), acceptances( src.acceptances ), resigns( src.resigns ), proposals( src.proposals ), votes( src.votes ), appliedDecisions( src.appliedDecisions )
    , trustedRevision( src.trustedRevision ), shaByRevision( src.shaByRevision ), refusals( src.refusals ), equivocations( src.equivocations ), lastMessageId( src.lastMessageId ), references( src.references ), nudgesSent( src.nudgesSent )
    , nudgesReceived( src.nudgesReceived ), resendRequested( src.resendRequested ), resendAnswered( src.resendAnswered ), deliveredRevisions( src.deliveredRevisions ), lastActivity( src.lastActivity ), cursor( src.cursor ) {}
CEMailMatchState::~CEMailMatchState() {}
CEMailMatchState& CEMailMatchState::operator=( const CEMailMatchState& src )
{
    if ( &src != this )
    {
        record = src.record; hasRecord = src.hasRecord; info = src.info; acceptances = src.acceptances; resigns = src.resigns; proposals = src.proposals; votes = src.votes; appliedDecisions = src.appliedDecisions;
        trustedRevision = src.trustedRevision; shaByRevision = src.shaByRevision; refusals = src.refusals; equivocations = src.equivocations; lastMessageId = src.lastMessageId; references = src.references;
        nudgesSent = src.nudgesSent; nudgesReceived = src.nudgesReceived; resendRequested = src.resendRequested; resendAnswered = src.resendAnswered; deliveredRevisions = src.deliveredRevisions; lastActivity = src.lastActivity; cursor = src.cursor;
    }
    return *this;
}
bool
CEMailMatchState::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "hasRecord", hasRecord );
    if ( hasRecord )
        record.Serialize( *domRootNode.AddChild( "record", GUCEF_DATATYPE_OBJECT ), settings );
    info.Serialize( *domRootNode.AddChild( "info", GUCEF_DATATYPE_OBJECT ), settings );
    SaveList( domRootNode, "acceptances", "acceptance", acceptances, settings );
    SaveList( domRootNode, "resigns", "resign", resigns, settings );
    SaveList( domRootNode, "proposals", "proposal", proposals, settings );
    SaveList( domRootNode, "votes", "vote", votes, settings );
    SaveList( domRootNode, "refusals", "refusal", refusals, settings );
    SaveList( domRootNode, "equivocations", "equivocation", equivocations, settings );
    domRootNode.SetAttribute( "appliedDecisions", JoinList( appliedDecisions ) );
    domRootNode.SetAttribute( "trustedRevision", trustedRevision );
    CORE::CDataNode* shaNode = domRootNode.AddChild( "shaByRevision", GUCEF_DATATYPE_OBJECT );
    GUCEF::map< UInt32, CORE::CString >::const_iterator s = shaByRevision.begin();
    while ( s != shaByRevision.end() )
    {
        shaNode->SetAttribute( CORE::UInt32ToString( (*s).first ), (*s).second );
        ++s;
    }
    domRootNode.SetAttribute( "lastMessageId", lastMessageId );
    domRootNode.SetAttribute( "references", JoinList( references ) );
    SaveTimeMap( domRootNode, "nudgesSent", nudgesSent );
    SaveTimeMap( domRootNode, "nudgesReceived", nudgesReceived );
    SaveTimeMap( domRootNode, "resendAnswered", resendAnswered );
    CORE::CDataNode* requestedNode = domRootNode.AddChild( "resendRequested", GUCEF_DATATYPE_OBJECT );
    GUCEF::map< UInt32, Int64 >::const_iterator r = resendRequested.begin();
    while ( r != resendRequested.end() )
    {
        requestedNode->SetAttribute( CORE::UInt32ToString( (*r).first ), (*r).second );
        ++r;
    }
    CORE::CString delivered;
    GUCEF::set< UInt32 >::const_iterator d = deliveredRevisions.begin();
    while ( d != deliveredRevisions.end() )
    {
        delivered += ( delivered.IsNULLOrEmpty() ? "" : " " ) + CORE::UInt32ToString( *d );
        ++d;
    }
    domRootNode.SetAttribute( "deliveredRevisions", delivered );
    domRootNode.SetAttribute( "lastActivity", lastActivity );
    domRootNode.SetAttribute( "cursor", cursor );
    return true;
}
bool
CEMailMatchState::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    // the file is not trusted more than a mail: every list is bounded
    CEMailMatchState loaded;
    const CORE::CDataNode* infoNode = domRootNode.FindChild( "info" );
    if ( GUCEF_NULL == infoNode || !loaded.info.Deserialize( *infoNode, settings ) )
        return false;
    const CORE::CDataNode* recordNode = domRootNode.FindChild( "record" );
    loaded.hasRecord = domRootNode.GetAttributeValueOrChildValueByName( "hasRecord" ).AsBool( false, true ) && GUCEF_NULL != recordNode && loaded.record.Deserialize( *recordNode, settings );
    LoadList( domRootNode, "acceptances", loaded.acceptances, settings );
    LoadList( domRootNode, "resigns", loaded.resigns, settings );
    LoadList( domRootNode, "proposals", loaded.proposals, settings );
    LoadList( domRootNode, "votes", loaded.votes, settings );
    LoadList( domRootNode, "refusals", loaded.refusals, settings );
    LoadList( domRootNode, "equivocations", loaded.equivocations, settings );
    loaded.appliedDecisions = GetText( domRootNode, "appliedDecisions" ).ParseElements( ' ', false );
    loaded.trustedRevision = domRootNode.GetAttributeValueOrChildValueByName( "trustedRevision" ).AsUInt32( 0, true );
    const CORE::CDataNode* shaNode = domRootNode.FindChild( "shaByRevision" );
    if ( GUCEF_NULL != shaNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = shaNode->AttributeBegin();
        while ( a != shaNode->AttributeEnd() && loaded.shaByRevision.size() < g_maxRevisions )
        {
            loaded.shaByRevision[ (UInt32) atol( (*a).first.C_String() ) ] = (*a).second.AsString( CORE::CString::Empty, true );
            ++a;
        }
    }
    loaded.lastMessageId = GetText( domRootNode, "lastMessageId" );
    loaded.references = GetText( domRootNode, "references" ).ParseElements( ' ', false );
    if ( loaded.references.size() > g_maxList )
        loaded.references.resize( g_maxList );
    LoadTimeMap( domRootNode, "nudgesSent", loaded.nudgesSent );
    LoadTimeMap( domRootNode, "nudgesReceived", loaded.nudgesReceived );
    LoadTimeMap( domRootNode, "resendAnswered", loaded.resendAnswered );
    const CORE::CDataNode* requestedNode = domRootNode.FindChild( "resendRequested" );
    if ( GUCEF_NULL != requestedNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = requestedNode->AttributeBegin();
        while ( a != requestedNode->AttributeEnd() && loaded.resendRequested.size() < g_maxList )
        {
            loaded.resendRequested[ (UInt32) atol( (*a).first.C_String() ) ] = (*a).second.AsInt64( 0, true );
            ++a;
        }
    }
    CORE::CString::StringVector delivered = GetText( domRootNode, "deliveredRevisions" ).ParseElements( ' ', false );
    for ( size_t i=0; i<delivered.size() && i<g_maxRevisions; ++i )
        loaded.deliveredRevisions.insert( (UInt32) atol( delivered[ i ].C_String() ) );
    loaded.lastActivity = domRootNode.GetAttributeValueOrChildValueByName( "lastActivity" ).AsInt64( 0, true );
    loaded.cursor = GetText( domRootNode, "cursor" );
    *this = loaded;
    return true;
}
CORE::CICloneable* CEMailMatchState::Clone( void ) const { return GUCEF_NEW CEMailMatchState( *this ); }
const CORE::CString& CEMailMatchState::GetClassTypeName( void ) const { static const CORE::CString n = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchState"; return n; }
const char* CEMailMatchState::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CEMailMatchState::CloneableTypeName( void ) { return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailMatchState"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

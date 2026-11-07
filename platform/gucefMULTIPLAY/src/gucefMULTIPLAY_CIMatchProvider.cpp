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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMULTIPLAY_CIMatchProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIMatchProvider::MatchAddedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchAddedEvent";
const CORE::CEvent CIMatchProvider::MatchUpdatedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchUpdatedEvent";
const CORE::CEvent CIMatchProvider::MatchRemovedEvent = "GUCEF::MULTIPLAY::CIMatchProvider::MatchRemovedEvent";
const CORE::CEvent CIMatchProvider::SubmitResultEvent = "GUCEF::MULTIPLAY::CIMatchProvider::SubmitResultEvent";
const CORE::CEvent CIMatchProvider::ProviderStateEvent = "GUCEF::MULTIPLAY::CIMatchProvider::ProviderStateEvent";
const CORE::CEvent CIMatchProvider::ProviderErrorEvent = "GUCEF::MULTIPLAY::CIMatchProvider::ProviderErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MatchOperationStatusToString( TMatchOperationStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case MATCHOPERATION_OK:         return "OK";
        case MATCHOPERATION_STALE:      return "STALE";
        case MATCHOPERATION_REJECTED:   return "REJECTED";
        case MATCHOPERATION_NOT_FOUND:  return "NOT_FOUND";
        default:                        return "OK";
    }
}

/*-------------------------------------------------------------------------*/

CMatchOperationResult::CMatchOperationResult( void )
    : CORE::CICloneable()
    , status( MATCHOPERATION_OK )
    , matchId()
    , reason()
    , preview()
    , hasPreview( false )
    , package()
    , hasPackage( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchOperationResult::CMatchOperationResult( const CMatchOperationResult& src )
    : CORE::CICloneable( src )
    , status( src.status )
    , matchId( src.matchId )
    , reason( src.reason )
    , preview( src.preview )
    , hasPreview( src.hasPreview )
    , package( src.package )
    , hasPackage( src.hasPackage )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchOperationResult::~CMatchOperationResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchOperationResult&
CMatchOperationResult::operator=( const CMatchOperationResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        status = src.status;
        matchId = src.matchId;
        reason = src.reason;
        preview = src.preview;
        hasPreview = src.hasPreview;
        package = src.package;
        hasPackage = src.hasPackage;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchOperationResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchOperationResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchOperationResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchOperationResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchOperationResult";
}

/*-------------------------------------------------------------------------*/

bool
CMatchOperationResult::TryGetFromFuture( const CORE::CFutureResult& future, CMatchOperationResult& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;
    const CMatchOperationResult* data = CORE::CICloneable::CloneableAs< CMatchOperationResult >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    result = *data;
    return true;
}

/*-------------------------------------------------------------------------*/

CMatchUpdate::CMatchUpdate( void )
    : CORE::CIDataNodeSerializable()
    , preview()
    , info()
    , hasInfo( false )
    , cursor()
    , providerName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate::CMatchUpdate( const CMatchUpdate& src )
    : CORE::CIDataNodeSerializable( src )
    , preview( src.preview )
    , info( src.info )
    , hasInfo( src.hasInfo )
    , cursor( src.cursor )
    , providerName( src.providerName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate::~CMatchUpdate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate&
CMatchUpdate::operator=( const CMatchUpdate& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        preview = src.preview;
        info = src.info;
        hasInfo = src.hasInfo;
        cursor = src.cursor;
        providerName = src.providerName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchUpdate::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "cursor", cursor );
    domRootNode.SetAttribute( "providerName", providerName );
    preview.Serialize( *domRootNode.AddChild( "preview", GUCEF_DATATYPE_OBJECT ), settings );
    if ( hasInfo )
        info.Serialize( *domRootNode.AddChild( "info", GUCEF_DATATYPE_OBJECT ), settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchUpdate::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    cursor = domRootNode.GetAttributeValueOrChildValueByName( "cursor" ).AsString( CORE::CString::Empty, true );
    providerName = domRootNode.GetAttributeValueOrChildValueByName( "providerName" ).AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* previewNode = domRootNode.FindChild( "preview" );
    if ( GUCEF_NULL == previewNode || !preview.Deserialize( *previewNode, settings ) )
        return false;
    const CORE::CDataNode* infoNode = domRootNode.FindChild( "info" );
    hasInfo = GUCEF_NULL != infoNode && info.Deserialize( *infoNode, settings );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchUpdate::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchUpdate( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchUpdate::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchUpdate";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchUpdate::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchUpdate::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchUpdate";
}

/*-------------------------------------------------------------------------*/

CMatchProviderState::CMatchProviderState( void )
    : CORE::CICloneable()
    , online( false )
    , providerName()
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProviderState::CMatchProviderState( const CMatchProviderState& src )
    : CORE::CICloneable( src )
    , online( src.online )
    , providerName( src.providerName )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProviderState::~CMatchProviderState()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchProviderState&
CMatchProviderState::operator=( const CMatchProviderState& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        online = src.online;
        providerName = src.providerName;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchProviderState::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchProviderState( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchProviderState::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchProviderState::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchProviderState";
}

/*-------------------------------------------------------------------------*/

void
CIMatchProvider::RegisterEvents( void )
{GUCEF_TRACE;

    MatchAddedEvent.Initialize();
    MatchUpdatedEvent.Initialize();
    MatchRemovedEvent.Initialize();
    SubmitResultEvent.Initialize();
    ProviderStateEvent.Initialize();
    ProviderErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIMatchProvider::CIMatchProvider( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIMatchProvider::~CIMatchProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::HasCapability( TMatchCapabilities capability ) const
{GUCEF_TRACE;

    return 0 != ( GetCapabilities() & (UInt32) capability );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Invite( const CORE::CString& matchId, const CMatchParticipant& participant, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Respond( const CORE::CString& matchId, const CMatchAcceptance& acceptance, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Resign( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Remove( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Rematch( const CORE::CString& matchId, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Propose( const CORE::CString& matchId, const CMatchProposal& proposal, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::Vote( const CORE::CString& matchId, const CMatchVote& vote, CORE::CFutureResult& future )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchAdded( const CMatchUpdate& update )
{GUCEF_TRACE;

    CMatchUpdate eData( update );
    eData.providerName = GetProviderName();
    return NotifyObservers( MatchAddedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchUpdated( const CMatchUpdate& update )
{GUCEF_TRACE;

    CMatchUpdate eData( update );
    eData.providerName = GetProviderName();
    return NotifyObservers( MatchUpdatedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportMatchRemoved( const CORE::CString& matchId )
{GUCEF_TRACE;

    TMatchRemovedEventData eData( matchId );
    return NotifyObservers( MatchRemovedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportSubmitResult( const CMatchOperationResult& result )
{GUCEF_TRACE;

    CMatchOperationResult eData( result );
    return NotifyObservers( SubmitResultEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportProviderState( bool online, const CORE::CString& description )
{GUCEF_TRACE;

    CMatchProviderState eData;
    eData.online = online;
    eData.providerName = GetProviderName();
    eData.description = description;
    return NotifyObservers( ProviderStateEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CIMatchProvider::ReportProviderError( const CORE::CString& description )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MatchProvider(" << GetProviderName() << "): " << description );
    TProviderErrorEventData eData( description );
    return NotifyObservers( ProviderErrorEvent, &eData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

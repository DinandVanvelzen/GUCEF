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

#include "gucefMULTIPLAY_CMatchOperationResult.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

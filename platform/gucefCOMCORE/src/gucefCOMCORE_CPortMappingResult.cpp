/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#include "gucefCOMCORE_CPortMappingResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPortMappingResult::CPortMappingResult( void )
    : CORE::CICloneable()
    , protocol( PROTOCOL_UDP )
    , internalPort( 0 )
    , externalAddress()
    , externalPort( 0 )
    , leaseInSecs( 0 )
    , mapperName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPortMappingResult::CPortMappingResult( const CPortMappingResult& src )
    : CORE::CICloneable( src )
    , protocol( src.protocol )
    , internalPort( src.internalPort )
    , externalAddress( src.externalAddress )
    , externalPort( src.externalPort )
    , leaseInSecs( src.leaseInSecs )
    , mapperName( src.mapperName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPortMappingResult::~CPortMappingResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPortMappingResult&
CPortMappingResult::operator=( const CPortMappingResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        protocol = src.protocol;
        internalPort = src.internalPort;
        externalAddress = src.externalAddress;
        externalPort = src.externalPort;
        leaseInSecs = src.leaseInSecs;
        mapperName = src.mapperName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPortMappingResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPortMappingResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CPortMappingResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CPortMappingResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COMCORE::CPortMappingResult";
}

/*-------------------------------------------------------------------------*/

bool
CPortMappingResult::TryGetFromFuture( const CORE::CFutureResult& future, CPortMappingResult& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;

    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;

    const CPortMappingResult* data = CORE::CICloneable::CloneableAs< CPortMappingResult >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;

    result = *data;
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
CPortMappingResult::ProtocolToString( TProtocol protocol )
{GUCEF_TRACE;

    return PROTOCOL_TCP == protocol ? "TCP" : "UDP";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

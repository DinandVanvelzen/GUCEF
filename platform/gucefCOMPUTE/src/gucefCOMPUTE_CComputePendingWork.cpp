/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputePendingWork.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CComputePendingWork::CComputePendingWork( UInt64 fenceValue, const CComputeCommandList& commands )
    : m_fenceValue( fenceValue )
    , m_commands( commands )
    , m_source()
    , m_hasFuture( false )
    , m_finished( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputePendingWork::CComputePendingWork( UInt64 fenceValue, const CComputeCommandList& commands, const CORE::CFutureResultSource& source )
    : m_fenceValue( fenceValue )
    , m_commands( commands )
    , m_source( source )
    , m_hasFuture( true )
    , m_finished( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputePendingWork::~CComputePendingWork()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CComputePendingWork::GetFenceValue( void ) const
{GUCEF_TRACE;

    return m_fenceValue;
}

/*-------------------------------------------------------------------------*/

const CComputeCommandList&
CComputePendingWork::GetCommands( void ) const
{GUCEF_TRACE;

    return m_commands;
}

/*-------------------------------------------------------------------------*/

bool
CComputePendingWork::HasFuture( void ) const
{GUCEF_TRACE;

    return m_hasFuture;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CComputePendingWork::GetFuture( void ) const
{GUCEF_TRACE;

    return m_source.GetFuture();
}

/*-------------------------------------------------------------------------*/

void
CComputePendingWork::Finish( TComputeResult result, const CString& errorText )
{GUCEF_TRACE;

    if ( m_finished )
        return;
    m_finished = true;
    if ( !m_hasFuture )
        return;
    if ( COMPUTERESULT_SUCCESS == result )
        m_source.Complete();
    else
        m_source.Fail( CORE::TASKSTATUS_STARTUP_FAILED, ComputeResultToFutureInfo( result, errorText ) );
}

/*-------------------------------------------------------------------------*/

bool
CComputePendingWork::IsFinished( void ) const
{GUCEF_TRACE;

    return m_finished;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

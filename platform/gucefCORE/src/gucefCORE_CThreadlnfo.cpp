/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CThreadlnfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

const CString CThreadInfo::ClassTypeName = "GUCEF::CORE::CThreadInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CThreadInfo::CThreadInfo( void )
    : CIDataNodeSerializable()
    , m_threadId( 0 )
    , m_threadStatus( MT::THREADSTATUS_UNDEFINED )
    , m_affinityMask( 0 )
    , m_threadCpuId( 0 )
    , m_threadName()
    , m_isExternal( false )
    , m_threadPoolName()
    , m_currentTaskId( 0 )
    , m_currentTaskTypeName()
    , m_busySinceMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CThreadInfo::CThreadInfo( const CThreadInfo& src )
    : CIDataNodeSerializable( src )
    , m_threadId( src.m_threadId )
    , m_threadStatus( src.m_threadStatus )
    , m_affinityMask( src.m_affinityMask )
    , m_threadCpuId( src.m_threadCpuId )
    , m_threadName( src.m_threadName )
    , m_isExternal( src.m_isExternal )
    , m_threadPoolName( src.m_threadPoolName )
    , m_currentTaskId( src.m_currentTaskId )
    , m_currentTaskTypeName( src.m_currentTaskTypeName )
    , m_busySinceMs( src.m_busySinceMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CThreadInfo::~CThreadInfo()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::Clear( void )
{GUCEF_TRACE;

    m_threadId = 0;
    m_threadStatus = MT::THREADSTATUS_UNDEFINED;
    m_affinityMask = 0;
    m_threadCpuId = 0;
    m_threadName.Clear();
    m_isExternal = false;
    m_threadPoolName = CString();
    m_currentTaskId = 0;
    m_currentTaskTypeName = CString();
    m_busySinceMs = 0;
}

/*-------------------------------------------------------------------------*/

bool 
CThreadInfo::Serialize( CDataNode& domRootNode                        ,
                        const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
        
    totalSuccess = domRootNode.SetAttribute( "threadId", m_threadId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "threadStatusId", (UInt16) m_threadStatus ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "affinityMask", m_affinityMask ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "threadCpuId", m_threadCpuId ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "isExternal", m_isExternal ) && totalSuccess;
    if ( !m_threadName.IsNULLOrEmpty() )
        totalSuccess = domRootNode.SetAttribute( "threadName", m_threadName ) && totalSuccess;
    if ( !m_threadPoolName.IsNULLOrEmpty() )
        totalSuccess = domRootNode.SetAttribute( "threadPoolName", m_threadPoolName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "currentTaskId", m_currentTaskId ) && totalSuccess;
    if ( !m_currentTaskTypeName.IsNULLOrEmpty() )
        totalSuccess = domRootNode.SetAttribute( "currentTaskTypeName", m_currentTaskTypeName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "busySinceMs", m_busySinceMs ) && totalSuccess;

    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails < settings.levelOfDetail )
    {
        totalSuccess = domRootNode.SetAttribute( "threadStatus", MT::ThreadStatusToThreadStatusString( m_threadStatus ) ) && totalSuccess;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CThreadInfo::Deserialize( const CDataNode& domRootNode                  ,
                          const CDataNodeSerializableSettings& settings ) 
{GUCEF_TRACE;

    Clear();
    
    m_threadId = (TSystemThreadId) domRootNode.GetAttributeValueOrChildValueByName( "threadId" ).AsUInt64( m_threadId, true );
    m_threadStatus = (TThreadStatus) domRootNode.GetAttributeValueOrChildValueByName( "threadStatusId" ).AsUInt8( (UInt8) m_threadStatus, true );
    m_affinityMask = domRootNode.GetAttributeValueOrChildValueByName( "affinityMask" ).AsUInt64( m_affinityMask, true );
    m_threadCpuId = domRootNode.GetAttributeValueOrChildValueByName( "threadCpuId" ).AsUInt32( m_threadCpuId, true );
    m_threadName = domRootNode.GetAttributeValueOrChildValueByName( "threadName" ).AsString( m_threadName, true );
    m_isExternal = domRootNode.GetAttributeValueOrChildValueByName( "isExternal" ).AsBool( m_isExternal, true );
    m_threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName, true );
    m_currentTaskId = domRootNode.GetAttributeValueOrChildValueByName( "currentTaskId" ).AsUInt32( m_currentTaskId, true );
    m_currentTaskTypeName = domRootNode.GetAttributeValueOrChildValueByName( "currentTaskTypeName" ).AsString( m_currentTaskTypeName, true );
    m_busySinceMs = domRootNode.GetAttributeValueOrChildValueByName( "busySinceMs" ).AsUInt64( m_busySinceMs, true );

    return true;
}

/*-------------------------------------------------------------------------*/

void 
CThreadInfo::SetThreadId( TSystemThreadId threadId )
{GUCEF_TRACE;

    m_threadId = threadId;
}

/*-------------------------------------------------------------------------*/

TSystemThreadId
CThreadInfo::GetThreadId( void ) const
{GUCEF_TRACE;

    return m_threadId;
}

/*-------------------------------------------------------------------------*/

void 
CThreadInfo::SetThreadStatus( TThreadStatus threadStatus )
{GUCEF_TRACE;

    m_threadStatus = threadStatus;
}

/*-------------------------------------------------------------------------*/

TThreadStatus
CThreadInfo::GetThreadStatus( void ) const
{GUCEF_TRACE;

    return m_threadStatus;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetThreadName( const CString& threadName )
{GUCEF_TRACE;

    m_threadName = threadName;
}

/*-------------------------------------------------------------------------*/

const CString&
CThreadInfo::GetThreadName( void ) const
{GUCEF_TRACE;

    return m_threadName;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetIsExternal( bool isExternal )
{GUCEF_TRACE;

    m_isExternal = isExternal;
}

/*-------------------------------------------------------------------------*/

bool
CThreadInfo::GetIsExternal( void ) const
{GUCEF_TRACE;

    return m_isExternal;
}

/*-------------------------------------------------------------------------*/

const CString& 
CThreadInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CThreadInfo::Clone( void ) const
{GUCEF_TRACE;

    return new CThreadInfo( *this );
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetThreadAffinityMask( UInt64 affinityMask )
{GUCEF_TRACE;

    m_affinityMask = affinityMask;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadInfo::GetThreadAffinityMask( void ) const
{GUCEF_TRACE;

    return m_affinityMask;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetThreadAffinityByCpuId( UInt32 cpuId )
{GUCEF_TRACE;

    m_threadCpuId = cpuId;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadInfo::GetThreadAffinityByCpuId( void ) const
{GUCEF_TRACE;

    return m_threadCpuId;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetThreadPoolName( const CString& threadPoolName )
{GUCEF_TRACE;

    m_threadPoolName = threadPoolName;
}

/*-------------------------------------------------------------------------*/

const CString&
CThreadInfo::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetCurrentTaskId( UInt32 currentTaskId )
{GUCEF_TRACE;

    m_currentTaskId = currentTaskId;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadInfo::GetCurrentTaskId( void ) const
{GUCEF_TRACE;

    return m_currentTaskId;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetCurrentTaskTypeName( const CString& currentTaskTypeName )
{GUCEF_TRACE;

    m_currentTaskTypeName = currentTaskTypeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CThreadInfo::GetCurrentTaskTypeName( void ) const
{GUCEF_TRACE;

    return m_currentTaskTypeName;
}

/*-------------------------------------------------------------------------*/

void
CThreadInfo::SetBusySinceMs( UInt64 busySinceMs )
{GUCEF_TRACE;

    m_busySinceMs = busySinceMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CThreadInfo::GetBusySinceMs( void ) const
{GUCEF_TRACE;

    return m_busySinceMs;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

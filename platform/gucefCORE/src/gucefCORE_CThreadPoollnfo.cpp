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

#include "gucefCORE_CThreadPoollnfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CThreadPoolInfo::ClassTypeName = "GUCEF::CORE::CThreadPoolInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CThreadPoolInfo::CThreadPoolInfo( void )
    : CIDataNodeSerializable()
    , m_threadPoolName()
    , m_desiredMaxNrOfThreads( -1 )
    , m_desiredMinNrOfWorkerThreads( 0 )
    , m_activeNrOfDedicatedThreads( 0 )
    , m_activeNrOfWorkerThreads( 0 )
    , m_taskConsumerFactoryTypes()
    , m_taskDataFactoryTypes()
    , m_allowAppThreadToWork( false )
    , m_queuedTaskCount( 0 )
    , m_taskIds()
    , m_counters()
    , m_runningTaskCount( 0 )
    , m_scheduledJobCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CThreadPoolInfo::CThreadPoolInfo( const CThreadPoolInfo& src )
    : CIDataNodeSerializable( src )
    , m_threadPoolName( src.m_threadPoolName )
    , m_desiredMaxNrOfThreads( src.m_desiredMaxNrOfThreads )
    , m_desiredMinNrOfWorkerThreads( src.m_desiredMinNrOfWorkerThreads )
    , m_activeNrOfDedicatedThreads( src.m_activeNrOfDedicatedThreads )
    , m_activeNrOfWorkerThreads( src.m_activeNrOfWorkerThreads )
    , m_taskConsumerFactoryTypes( src.m_taskConsumerFactoryTypes )
    , m_taskDataFactoryTypes( src.m_taskDataFactoryTypes )
    , m_allowAppThreadToWork( src.m_allowAppThreadToWork )
    , m_queuedTaskCount( src.m_queuedTaskCount )
    , m_taskIds( src.m_taskIds )
    , m_counters( src.m_counters )
    , m_runningTaskCount( src.m_runningTaskCount )
    , m_scheduledJobCount( src.m_scheduledJobCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CThreadPoolInfo::~CThreadPoolInfo()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::Clear( void )
{GUCEF_TRACE;

    m_threadPoolName.Clear();
    m_desiredMaxNrOfThreads = -1;
    m_desiredMinNrOfWorkerThreads = 0;
    m_activeNrOfDedicatedThreads = 0;
    m_activeNrOfWorkerThreads = 0;
    m_taskConsumerFactoryTypes.clear();
    m_taskDataFactoryTypes.clear();
    m_allowAppThreadToWork = false;
    m_queuedTaskCount = 0;
    m_counters = CThreadPoolCounters();
    m_runningTaskCount = 0;
    m_scheduledJobCount = 0;
}

/*-------------------------------------------------------------------------*/

bool 
CThreadPoolInfo::Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    
    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly == settings.levelOfDetail )
    {
        domRootNode.SetNodeType( GUCEF_DATATYPE_ARRAY );
        totalSuccess = domRootNode.AddAllValuesAsChildren( m_taskIds ) && totalSuccess;
        return totalSuccess;
    }
    
    totalSuccess = domRootNode.SetAttribute( "threadPoolName", m_threadPoolName ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "desiredMinNrOfWorkerThreads", m_desiredMinNrOfWorkerThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "desiredMaxNrOfThreads", m_desiredMaxNrOfThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "activeNrOfDedicatedThreads", m_activeNrOfDedicatedThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "activeNrOfWorkerThreads", m_activeNrOfWorkerThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "allowAppThreadToWork", m_allowAppThreadToWork ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "queuedTaskCount", m_queuedTaskCount ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "runningTaskCount", m_runningTaskCount ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "scheduledJobCount", m_scheduledJobCount ) && totalSuccess;
    CDataNode* countersNode = domRootNode.AddChild( "counters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != countersNode )
    {
        totalSuccess = countersNode->SetAttribute( "queuedTotal", m_counters.GetQueuedTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "startedTotal", m_counters.GetStartedTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "finishedTotal", m_counters.GetFinishedTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "failedTotal", m_counters.GetFailedTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "cancelledTotal", m_counters.GetCancelledTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "killedTotal", m_counters.GetKilledTotal() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "cumulativeQueueWaitMs", m_counters.GetCumulativeQueueWaitMs() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "cumulativeRunMs", m_counters.GetCumulativeRunMs() ) && totalSuccess;
        totalSuccess = countersNode->SetAttribute( "sampleMonotonicMs", m_counters.GetSampleMonotonicMs() ) && totalSuccess;
    }
    else
        totalSuccess = false;
    
    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails < settings.levelOfDetail )
    {
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "taskIds", m_taskIds, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "taskConsumerFactoryTypes", m_taskConsumerFactoryTypes, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "taskDataFactoryTypes", m_taskDataFactoryTypes, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
    }    

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPoolInfo::Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) 
{GUCEF_TRACE;

    Clear();
    
    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly == settings.levelOfDetail )
    {
        m_taskIds = ToStringSet( domRootNode.GetChildrenValues() );
        return true;
    }

    m_threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_threadPoolName, true );
    m_desiredMaxNrOfThreads = domRootNode.GetAttributeValueOrChildValueByName( "desiredMaxNrOfThreads" ).AsInt32( m_desiredMaxNrOfThreads, true );
    m_desiredMinNrOfWorkerThreads = domRootNode.GetAttributeValueOrChildValueByName( "desiredMinNrOfWorkerThreads" ).AsUInt32( m_desiredMinNrOfWorkerThreads, true );
    m_activeNrOfDedicatedThreads = domRootNode.GetAttributeValueOrChildValueByName( "activeNrOfDedicatedThreads" ).AsUInt32( m_activeNrOfDedicatedThreads, true );
    m_activeNrOfWorkerThreads = domRootNode.GetAttributeValueOrChildValueByName( "activeNrOfWorkerThreads" ).AsUInt32( m_activeNrOfWorkerThreads, true );
    m_allowAppThreadToWork = domRootNode.GetAttributeValueOrChildValueByName( "allowAppThreadToWork" ).AsBool( m_allowAppThreadToWork, true );
    m_queuedTaskCount = domRootNode.GetAttributeValueOrChildValueByName( "queuedTaskCount" ).AsUInt32( m_queuedTaskCount, true );

    m_runningTaskCount = domRootNode.GetAttributeValueOrChildValueByName( "runningTaskCount" ).AsUInt32( m_runningTaskCount, true );
    m_scheduledJobCount = domRootNode.GetAttributeValueOrChildValueByName( "scheduledJobCount" ).AsUInt32( m_scheduledJobCount, true );
    const CDataNode* countersNode = domRootNode.FindChild( "counters" );
    if ( GUCEF_NULL != countersNode )
    {
        m_counters.SetThreadPoolName( m_threadPoolName );
        m_counters.SetQueuedTotal( countersNode->GetAttributeValueOrChildValueByName( "queuedTotal" ).AsUInt64( 0, true ) );
        m_counters.SetStartedTotal( countersNode->GetAttributeValueOrChildValueByName( "startedTotal" ).AsUInt64( 0, true ) );
        m_counters.SetFinishedTotal( countersNode->GetAttributeValueOrChildValueByName( "finishedTotal" ).AsUInt64( 0, true ) );
        m_counters.SetFailedTotal( countersNode->GetAttributeValueOrChildValueByName( "failedTotal" ).AsUInt64( 0, true ) );
        m_counters.SetCancelledTotal( countersNode->GetAttributeValueOrChildValueByName( "cancelledTotal" ).AsUInt64( 0, true ) );
        m_counters.SetKilledTotal( countersNode->GetAttributeValueOrChildValueByName( "killedTotal" ).AsUInt64( 0, true ) );
        m_counters.SetCumulativeQueueWaitMs( countersNode->GetAttributeValueOrChildValueByName( "cumulativeQueueWaitMs" ).AsUInt64( 0, true ) );
        m_counters.SetCumulativeRunMs( countersNode->GetAttributeValueOrChildValueByName( "cumulativeRunMs" ).AsUInt64( 0, true ) );
        m_counters.SetSampleMonotonicMs( countersNode->GetAttributeValueOrChildValueByName( "sampleMonotonicMs" ).AsUInt64( 0, true ) );
    }
    domRootNode.GetValuesOfChildByName( "taskIds", m_taskIds );
    domRootNode.GetValuesOfChildByName( "taskConsumerFactoryTypes", m_taskConsumerFactoryTypes );
    domRootNode.GetValuesOfChildByName( "taskDataFactoryTypes", m_taskDataFactoryTypes );

    return true;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetThreadPoolName( const CString& poolName )
{GUCEF_TRACE;

    m_threadPoolName = poolName;
}

/*-------------------------------------------------------------------------*/

const CString& 
CThreadPoolInfo::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void 
CThreadPoolInfo::SetActiveNrOfDedicatedThreads( UInt32 nrOfThreads )
{GUCEF_TRACE;

    m_activeNrOfDedicatedThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetActiveNrOfDedicatedThreads( void ) const
{GUCEF_TRACE;

    return m_activeNrOfDedicatedThreads;
}

/*-------------------------------------------------------------------------*/

void 
CThreadPoolInfo::SetActiveNrOfWorkerThreads( UInt32 nrOfThreads )
{GUCEF_TRACE;

    m_activeNrOfWorkerThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetActiveNrOfWorkerThreads( void ) const
{GUCEF_TRACE;

    return m_activeNrOfWorkerThreads;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetDesiredMaxNrOfThreads( Int32 nrOfThreads )
{GUCEF_TRACE;

    m_desiredMaxNrOfThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

Int32
CThreadPoolInfo::GetDesiredMaxNrOfThreads( void ) const
{GUCEF_TRACE;

    return m_desiredMaxNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetDesiredMinNrOfWorkerThreads( UInt32 nrOfThreads )
{GUCEF_TRACE;

    m_desiredMinNrOfWorkerThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetDesiredMinNrOfWorkerThreads( void ) const
{GUCEF_TRACE;

    return m_desiredMinNrOfWorkerThreads;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetTaskConsumerFactoryTypes( const CString::StringSet& factoryTypes )
{GUCEF_TRACE;

    m_taskConsumerFactoryTypes = factoryTypes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet& 
CThreadPoolInfo::GetTaskConsumerFactoryTypes( void ) const
{GUCEF_TRACE;

    return m_taskConsumerFactoryTypes;
}

/*-------------------------------------------------------------------------*/

CString::StringSet& 
CThreadPoolInfo::GetTaskConsumerFactoryTypes( void )
{GUCEF_TRACE;

    return m_taskConsumerFactoryTypes;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetTaskDataFactoryTypes( const CString::StringSet& factoryTypes )
{GUCEF_TRACE;

    m_taskDataFactoryTypes = factoryTypes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet& 
CThreadPoolInfo::GetTaskDataFactoryTypes( void ) const
{GUCEF_TRACE;

    return m_taskDataFactoryTypes;
}

/*-------------------------------------------------------------------------*/

CString::StringSet& 
CThreadPoolInfo::GetTaskDataFactoryTypes( void )
{GUCEF_TRACE;

    return m_taskDataFactoryTypes;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetAllowMainApplicationThreadToPickUpQueuedTasks( bool allowAppThreadToWork )
{GUCEF_TRACE;

    m_allowAppThreadToWork = allowAppThreadToWork;
}

/*-------------------------------------------------------------------------*/

bool
CThreadPoolInfo::GetAllowMainApplicationThreadToPickUpQueuedTasks( void ) const
{GUCEF_TRACE;

    return m_allowAppThreadToWork;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetQueuedTaskCount( UInt32 nrOfTasks )
{GUCEF_TRACE;

    m_queuedTaskCount = nrOfTasks;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetQueuedTaskCount( void ) const
{GUCEF_TRACE;

    return m_queuedTaskCount;
}

/*-------------------------------------------------------------------------*/

const CString& 
CThreadPoolInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CThreadPoolInfo::Clone( void ) const
{GUCEF_TRACE;

    return new CThreadPoolInfo( *this );
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetCounters( const CThreadPoolCounters& counters )
{GUCEF_TRACE;

    m_counters = counters;
}

/*-------------------------------------------------------------------------*/

const CThreadPoolCounters&
CThreadPoolInfo::GetCounters( void ) const
{GUCEF_TRACE;

    return m_counters;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetRunningTaskCount( UInt32 runningTaskCount )
{GUCEF_TRACE;

    m_runningTaskCount = runningTaskCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetRunningTaskCount( void ) const
{GUCEF_TRACE;

    return m_runningTaskCount;
}

/*-------------------------------------------------------------------------*/

void
CThreadPoolInfo::SetScheduledJobCount( UInt32 scheduledJobCount )
{GUCEF_TRACE;

    m_scheduledJobCount = scheduledJobCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CThreadPoolInfo::GetScheduledJobCount( void ) const
{GUCEF_TRACE;

    return m_scheduledJobCount;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

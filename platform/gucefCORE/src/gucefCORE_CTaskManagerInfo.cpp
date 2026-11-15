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

#include "gucefCORE_CTaskManagerInfo.h"

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

const CString CTaskManagerInfo::ClassTypeName = "GUCEF::CORE::CTaskManagerInfo";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTaskManagerInfo::CTaskManagerInfo( void )
    : CIDataNodeSerializable()
    , m_threadPoolNames()
    , m_desiredGlobalNrOfThreads( 0 )
    , m_activeGlobalNrOfThreads( 0 )
    , m_taskConsumerFactoryTypes()
    , m_taskDataFactoryTypes()
    , m_allowAppThreadToWork( false )
    , m_globalNrOfQueuedTasks( 0 )
    , m_jobSchedulerCount( 0 )
    , m_scheduledJobCount( 0 )
    , m_mainThreadScheduledJobCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskManagerInfo::CTaskManagerInfo( const CTaskManagerInfo& src )
    : CIDataNodeSerializable( src )
    , m_threadPoolNames( src.m_threadPoolNames )
    , m_desiredGlobalNrOfThreads( src.m_desiredGlobalNrOfThreads )
    , m_activeGlobalNrOfThreads( src.m_activeGlobalNrOfThreads )
    , m_taskConsumerFactoryTypes( src.m_taskConsumerFactoryTypes )
    , m_taskDataFactoryTypes( src.m_taskDataFactoryTypes )
    , m_allowAppThreadToWork( src.m_allowAppThreadToWork )
    , m_globalNrOfQueuedTasks( src.m_globalNrOfQueuedTasks )
    , m_jobSchedulerCount( src.m_jobSchedulerCount )
    , m_scheduledJobCount( src.m_scheduledJobCount )
    , m_mainThreadScheduledJobCount( src.m_mainThreadScheduledJobCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskManagerInfo::~CTaskManagerInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool 
CTaskManagerInfo::Serialize( CDataNode& domRootNode                        ,
                             const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    
    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly == settings.levelOfDetail )
    {
        domRootNode.SetNodeType( GUCEF_DATATYPE_ARRAY );
        totalSuccess = domRootNode.AddAllValuesAsChildren( m_threadPoolNames ) && totalSuccess;
        return totalSuccess;
    }
    
    totalSuccess = domRootNode.SetAttribute( "desiredGlobalNrOfThreads", m_desiredGlobalNrOfThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "activeGlobalNrOfThreads", m_activeGlobalNrOfThreads ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "allowAppThreadToWork", m_allowAppThreadToWork ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "globalNrOfQueuedTasks", m_globalNrOfQueuedTasks ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "jobSchedulerCount", m_jobSchedulerCount ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "scheduledJobCount", m_scheduledJobCount ) && totalSuccess;
    totalSuccess = domRootNode.SetAttribute( "mainThreadScheduledJobCount", m_mainThreadScheduledJobCount ) && totalSuccess;
    
    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails < settings.levelOfDetail )
    {
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "threadPoolNames", m_threadPoolNames, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "taskConsumerFactoryTypes", m_taskConsumerFactoryTypes, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
        totalSuccess = domRootNode.AddAllValuesAsChildrenOfChild( "taskDataFactoryTypes", m_taskDataFactoryTypes, GUCEF_DATATYPE_ARRAY ) && totalSuccess;
    }    

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerInfo::Deserialize( const CDataNode& domRootNode                  ,
                               const CDataNodeSerializableSettings& settings ) 
{GUCEF_TRACE;

    if ( CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly == settings.levelOfDetail )
    {
        m_threadPoolNames = ToStringSet( domRootNode.GetChildrenValues() );
        return true;
    }

    m_desiredGlobalNrOfThreads = domRootNode.GetAttributeValueOrChildValueByName( "desiredGlobalNrOfThreads" ).AsUInt32( m_desiredGlobalNrOfThreads, true );
    m_activeGlobalNrOfThreads = domRootNode.GetAttributeValueOrChildValueByName( "activeGlobalNrOfThreads" ).AsInt32( m_activeGlobalNrOfThreads, true );
    m_allowAppThreadToWork = domRootNode.GetAttributeValueOrChildValueByName( "allowAppThreadToWork" ).AsBool( m_allowAppThreadToWork, true );
    m_globalNrOfQueuedTasks = domRootNode.GetAttributeValueOrChildValueByName( "globalNrOfQueuedTasks" ).AsUInt32( m_globalNrOfQueuedTasks, true );
    m_jobSchedulerCount = domRootNode.GetAttributeValueOrChildValueByName( "jobSchedulerCount" ).AsUInt32( m_jobSchedulerCount, true );
    m_scheduledJobCount = domRootNode.GetAttributeValueOrChildValueByName( "scheduledJobCount" ).AsUInt32( m_scheduledJobCount, true );
    m_mainThreadScheduledJobCount = domRootNode.GetAttributeValueOrChildValueByName( "mainThreadScheduledJobCount" ).AsUInt32( m_mainThreadScheduledJobCount, true );

    domRootNode.GetValuesOfChildByName( "threadPoolNames", m_threadPoolNames );
    domRootNode.GetValuesOfChildByName( "taskConsumerFactoryTypes", m_taskConsumerFactoryTypes );
    domRootNode.GetValuesOfChildByName( "taskDataFactoryTypes", m_taskDataFactoryTypes );

    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetThreadPoolNames( const CString::StringSet& pools )
{GUCEF_TRACE;

    m_threadPoolNames = pools;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet& 
CTaskManagerInfo::GetThreadPoolNames( void ) const
{GUCEF_TRACE;

    return m_threadPoolNames;
}

/*-------------------------------------------------------------------------*/

CString::StringSet& 
CTaskManagerInfo::GetThreadPoolNames( void )
{GUCEF_TRACE;

    return m_threadPoolNames;
}

/*-------------------------------------------------------------------------*/

void 
CTaskManagerInfo::SetActiveGlobalNrOfThreads( UInt32 nrOfThreads )
{GUCEF_TRACE;

    m_activeGlobalNrOfThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetActiveGlobalNrOfThreads( void ) const
{GUCEF_TRACE;

    return m_activeGlobalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetDesiredGlobalNrOfThreads( UInt32 nrOfThreads )
{GUCEF_TRACE;

    m_desiredGlobalNrOfThreads = nrOfThreads;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetDesiredGlobalNrOfThreads( void ) const
{GUCEF_TRACE;

    return m_desiredGlobalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetTaskConsumerFactoryTypes( const CString::StringSet& factoryTypes )
{GUCEF_TRACE;

    m_taskConsumerFactoryTypes = factoryTypes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet& 
CTaskManagerInfo::GetTaskConsumerFactoryTypes( void ) const
{GUCEF_TRACE;

    return m_taskConsumerFactoryTypes;
}

/*-------------------------------------------------------------------------*/

CString::StringSet& 
CTaskManagerInfo::GetTaskConsumerFactoryTypes( void )
{GUCEF_TRACE;

    return m_taskConsumerFactoryTypes;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetTaskDataFactoryTypes( const CString::StringSet& factoryTypes )
{GUCEF_TRACE;

    m_taskDataFactoryTypes = factoryTypes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet& 
CTaskManagerInfo::GetTaskDataFactoryTypes( void ) const
{GUCEF_TRACE;

    return m_taskDataFactoryTypes;
}

/*-------------------------------------------------------------------------*/

CString::StringSet& 
CTaskManagerInfo::GetTaskDataFactoryTypes( void )
{GUCEF_TRACE;

    return m_taskDataFactoryTypes;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetGlobalNrOfQueuedTasks( UInt32 nrOfTasks )
{GUCEF_TRACE;

    m_globalNrOfQueuedTasks = nrOfTasks;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetGlobalNrOfQueuedTasks( void ) const
{GUCEF_TRACE;

    return m_globalNrOfQueuedTasks;
}

/*-------------------------------------------------------------------------*/

const CString& 
CTaskManagerInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CTaskManagerInfo::Clone( void ) const
{GUCEF_TRACE;

    return new CTaskManagerInfo( *this );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetJobSchedulerCount( UInt32 jobSchedulerCount )
{GUCEF_TRACE;

    m_jobSchedulerCount = jobSchedulerCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetJobSchedulerCount( void ) const
{GUCEF_TRACE;

    return m_jobSchedulerCount;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetScheduledJobCount( UInt32 scheduledJobCount )
{GUCEF_TRACE;

    m_scheduledJobCount = scheduledJobCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetScheduledJobCount( void ) const
{GUCEF_TRACE;

    return m_scheduledJobCount;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerInfo::SetMainThreadScheduledJobCount( UInt32 mainThreadScheduledJobCount )
{GUCEF_TRACE;

    m_mainThreadScheduledJobCount = mainThreadScheduledJobCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManagerInfo::GetMainThreadScheduledJobCount( void ) const
{GUCEF_TRACE;

    return m_mainThreadScheduledJobCount;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#define GUCEF_CORE_CTHREADPOOLINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLCOUNTERS_H
#include "gucefCORE_CThreadPoolCounters.h"
#define GUCEF_CORE_CTHREADPOOLCOUNTERS_H
#endif /* GUCEF_CORE_CTHREADPOOLCOUNTERS_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CTaskDelegator;

/*-------------------------------------------------------------------------*/

/**
 *  Class for representing a snapshot of information obtained from the threadpool class 
 */
class GUCEF_CORE_PUBLIC_CPP CThreadPoolInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CThreadPoolInfo( void );

    CThreadPoolInfo( const CThreadPoolInfo& src );

    virtual ~CThreadPoolInfo() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successfull.
     */
    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    void SetThreadPoolName( const CString& poolName );
    
    const CString& GetThreadPoolName( void ) const;
    
    void SetActiveNrOfDedicatedThreads( UInt32 nrOfThreads );
    
    UInt32 GetActiveNrOfDedicatedThreads( void ) const;

    void SetActiveNrOfWorkerThreads( UInt32 nrOfThreads );
    
    UInt32 GetActiveNrOfWorkerThreads( void ) const;

    void SetDesiredMaxNrOfThreads( Int32 nrOfThreads );
    
    Int32 GetDesiredMaxNrOfThreads( void ) const;

    void SetDesiredMinNrOfWorkerThreads( UInt32 nrOfThreads );
    
    UInt32 GetDesiredMinNrOfWorkerThreads( void ) const;

    void SetTaskConsumerFactoryTypes( const CString::StringSet& factoryTypes );
    
    const CString::StringSet& GetTaskConsumerFactoryTypes( void ) const;

    CString::StringSet& GetTaskConsumerFactoryTypes( void );

    void SetTaskDataFactoryTypes( const CString::StringSet& factoryTypes );
    
    const CString::StringSet& GetTaskDataFactoryTypes( void ) const;

    CString::StringSet& GetTaskDataFactoryTypes( void );

    void SetAllowMainApplicationThreadToPickUpQueuedTasks( bool allowAppThreadToWork );

    bool GetAllowMainApplicationThreadToPickUpQueuedTasks( void ) const;

    void SetQueuedTaskCount( UInt32 nrOfTasks );
    
    UInt32 GetQueuedTaskCount( void ) const;

    /** Totals of the pool (also available without a task walk through CTaskManager::GetPoolCounters()) */
    void SetCounters( const CThreadPoolCounters& counters );
    const CThreadPoolCounters& GetCounters( void ) const;

    void SetRunningTaskCount( UInt32 runningTaskCount );
    UInt32 GetRunningTaskCount( void ) const;

    /** Scheduled jobs whose runs execute on this pool */
    void SetScheduledJobCount( UInt32 scheduledJobCount );
    UInt32 GetScheduledJobCount( void ) const;

    void Clear( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CString, CString::StringSet > StringToStringSetMap;

    CString m_threadPoolName;
    Int32 m_desiredMaxNrOfThreads;
    UInt32 m_desiredMinNrOfWorkerThreads;
    UInt32 m_activeNrOfDedicatedThreads;
    UInt32 m_activeNrOfWorkerThreads;
    CString::StringSet m_taskConsumerFactoryTypes;
    CString::StringSet m_taskDataFactoryTypes;
    bool m_allowAppThreadToWork;
    UInt32 m_queuedTaskCount;
    CString::StringSet m_taskIds;
    CThreadPoolCounters m_counters;
    UInt32 m_runningTaskCount;
    UInt32 m_scheduledJobCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

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

#ifndef GUCEF_CORE_CTASKMANAGERINFO_H
#define GUCEF_CORE_CTASKMANAGERINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#include "gucefCORE_CThreadPoollnfo.h"
#define GUCEF_CORE_CTHREADPOOLINFO_H
#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

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
 *  Class for representing a snapshot of information obtained from the task
 *  manager class 
 */
class GUCEF_CORE_PUBLIC_CPP CTaskManagerInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CTaskManagerInfo( void );

    CTaskManagerInfo( const CTaskManagerInfo& src );

    virtual ~CTaskManagerInfo();

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

    void SetThreadPoolNames( const CString::StringSet& pools );
    
    const CString::StringSet& GetThreadPoolNames( void ) const;

    CString::StringSet& GetThreadPoolNames( void );
    
    void SetActiveGlobalNrOfThreads( UInt32 nrOfThreads );
    
    UInt32 GetActiveGlobalNrOfThreads( void ) const;

    void SetDesiredGlobalNrOfThreads( UInt32 nrOfThreads );
    
    UInt32 GetDesiredGlobalNrOfThreads( void ) const;
    
    void SetGlobalNrOfQueuedTasks( UInt32 nrOfTasks );
    
    UInt32 GetGlobalNrOfQueuedTasks( void ) const;    

    void SetTaskConsumerFactoryTypes( const CString::StringSet& factoryTypes );
    
    const CString::StringSet& GetTaskConsumerFactoryTypes( void ) const;

    CString::StringSet& GetTaskConsumerFactoryTypes( void );

    void SetTaskDataFactoryTypes( const CString::StringSet& factoryTypes );
    
    const CString::StringSet& GetTaskDataFactoryTypes( void ) const;

    CString::StringSet& GetTaskDataFactoryTypes( void );

    /** Job schedulers registered with the task manager */
    void SetJobSchedulerCount( UInt32 jobSchedulerCount );
    UInt32 GetJobSchedulerCount( void ) const;

    /** Jobs of all job schedulers */
    void SetScheduledJobCount( UInt32 scheduledJobCount );
    UInt32 GetScheduledJobCount( void ) const;

    /** Jobs whose runs execute on the main thread */
    void SetMainThreadScheduledJobCount( UInt32 mainThreadScheduledJobCount );
    UInt32 GetMainThreadScheduledJobCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CString, CString::StringSet > StringToStringSetMap;

    CString::StringSet m_threadPoolNames;
    UInt32 m_desiredGlobalNrOfThreads;
    Int32 m_activeGlobalNrOfThreads;
    CString::StringSet m_taskConsumerFactoryTypes;
    CString::StringSet m_taskDataFactoryTypes;
    bool m_allowAppThreadToWork;
    UInt32 m_globalNrOfQueuedTasks;
    UInt32 m_jobSchedulerCount;
    UInt32 m_scheduledJobCount;
    UInt32 m_mainThreadScheduledJobCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKMANAGERINFO_H ? */

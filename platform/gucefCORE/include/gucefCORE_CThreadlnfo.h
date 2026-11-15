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

#ifndef GUCEF_CORE_CTHREADINFO_H
#define GUCEF_CORE_CTHREADINFO_H

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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_THREADSTATUS_H
#include "gucefCORE_ThreadStatus.h"
#define GUCEF_CORE_THREADSTATUS_H
#endif /* GUCEF_CORE_THREADSTATUS_H ? */

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
class GUCEF_CORE_PUBLIC_CPP CThreadInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CThreadInfo( void );

    CThreadInfo( const CThreadInfo& src );

    virtual ~CThreadInfo() GUCEF_VIRTUAL_OVERRIDE;

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

    void SetThreadId( TSystemThreadId threadId );
    
    TSystemThreadId GetThreadId( void ) const;

    void SetThreadStatus( TThreadStatus threadStatus );

    TThreadStatus GetThreadStatus( void ) const;

    void SetThreadAffinityMask( UInt64 affinityMask );

    UInt64 GetThreadAffinityMask( void ) const;
    
    /**
     *  Optional name of the thread, set for threads GUCEF did not create (see CTaskManager::RegisterExternalThread)
     */
    void SetThreadName( const CString& threadName );

    const CString& GetThreadName( void ) const;

    /**
     *  True for a thread that was created outside of the task manager and only announced itself to it.
     *  Such threads are tracked but never scheduled.
     */
    void SetIsExternal( bool isExternal );

    bool GetIsExternal( void ) const;

    void SetThreadAffinityByCpuId( UInt32 cpuId );

    UInt32 GetThreadAffinityByCpuId( void ) const;

    /** The pool of the thread (the external pseudo pool for external threads) */
    void SetThreadPoolName( const CString& threadPoolName );
    const CString& GetThreadPoolName( void ) const;

    /** The task the thread works on; 0 = none */
    void SetCurrentTaskId( UInt32 currentTaskId );
    UInt32 GetCurrentTaskId( void ) const;

    void SetCurrentTaskTypeName( const CString& currentTaskTypeName );
    const CString& GetCurrentTaskTypeName( void ) const;

    /** Monotonic time at which the thread started its current task; 0 = idle */
    void SetBusySinceMs( UInt64 busySinceMs );
    UInt64 GetBusySinceMs( void ) const;

    void Clear( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    TSystemThreadId m_threadId;
    TThreadStatus m_threadStatus;
    UInt64 m_affinityMask;
    UInt32 m_threadCpuId;
    CString m_threadName;
    bool m_isExternal;
    CString m_threadPoolName;
    UInt32 m_currentTaskId;
    CString m_currentTaskTypeName;
    UInt64 m_busySinceMs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTHREADINFO_H ? */

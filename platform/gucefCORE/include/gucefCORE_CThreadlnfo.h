/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

    void SetThreadId( UInt32 threadId );
    
    UInt32 GetThreadId( void ) const;

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

    void Clear( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    UInt32 m_threadId;
    TThreadStatus m_threadStatus;
    UInt64 m_affinityMask;
    UInt32 m_threadCpuId;
    CString m_threadName;
    bool m_isExternal;
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

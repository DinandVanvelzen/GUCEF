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

#ifndef GUCEF_CORE_CTASKINFO_H
#define GUCEF_CORE_CTASKINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#include "gucefCORE_CThreadPoollnfo.h"
#define GUCEF_CORE_CTHREADPOOLINFO_H
#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

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
class GUCEF_CORE_PUBLIC_CPP CTaskInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CTaskInfo( void );

    CTaskInfo( const CTaskInfo& src );

    virtual ~CTaskInfo();

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

    void SetTaskId( UInt32 taskId );
    
    UInt32 GetTaskId( void ) const;

    void SetThreadId( TSystemThreadId threadId );
    
    TSystemThreadId GetThreadId( void ) const;

    void SetTaskStatus( TTaskStatus taskStatus );

    TTaskStatus GetTaskStatus( void ) const;

    void SetHasTaskData( bool hasTaskData );
    
    bool GetHasTaskData( void ) const;

    void SetCustomTaskDataIsSerializable( bool isSerializable );
    
    bool GetCustomTaskDataIsSerializable( void ) const;

    void SetTaskTypeName( const CString& taskTypeName );

    const CString& GetTaskTypeName( void ) const;

    const CDataNode& GetCustomTaskData( void ) const;

    CDataNode& GetCustomTaskData( void );

    void Clear( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    UInt32 m_taskId;
    TSystemThreadId m_threadId;
    TTaskStatus m_taskStatus;
    bool m_hasTaskData;
    bool m_customTaskDataIsSerializable;
    CDataNode m_taskData;
    CString m_taskTypeName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKINFO_H ? */

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

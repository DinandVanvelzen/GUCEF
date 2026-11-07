/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CDATABASECLIENTOPERATION_H
#define GUCEF_COMCORE_CDATABASECLIENTOPERATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#include "gucefCOMCORE_CDatabaseClientBase.h"
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The task data of an async call of a CDatabaseClientBase */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseClientOperationData : public CORE::CICloneable
{
    public:

    CDatabaseClientBase* client;
    CDatabaseClientBase::EOperation operation;
    CDatabaseClientSettings settings;
    CORE::CString tableName;
    CORE::CDataTableDefinition definition;
    bool deleteData;
    CORE::CFutureResultSource source;

    CDatabaseClientOperationData( void );

    CDatabaseClientOperationData( const CDatabaseClientOperationData& src );

    virtual ~CDatabaseClientOperationData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The task that carries out the async calls of CDatabaseClientBase on a pool thread */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseClientOperation : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CDatabaseClientOperation( void );

    virtual ~CDatabaseClientOperation() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CDatabaseClientOperation, MT::CMutex > TDatabaseClientOperationTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDATABASECLIENTOPERATION_H ? */

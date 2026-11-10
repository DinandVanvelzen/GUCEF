/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCCALLTASK_H
#define GUCEF_COMCORE_CRPCCALLTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

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

/** The task data of an RPC_EXECUTION_THREAD_POOL handler: the server call */
class GUCEF_COMCORE_EXPORT_CPP CRpcCallTaskData : public CORE::CICloneable
{
    public:

    CRpcServerCallPtr call;

    explicit CRpcCallTaskData( const CRpcServerCallPtr& serverCall );
    CRpcCallTaskData( const CRpcCallTaskData& src );
    virtual ~CRpcCallTaskData() GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Runs a handler in a thread pool task (gucefCOMCORE RPC design 10.6, RPC_EXECUTION_THREAD_POOL): the same OnCall()
 *  as the other modes. Registered with the task manager by gucefCOMCORE.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcCallTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CRpcCallTaskConsumer( void );
    virtual ~CRpcCallTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCALLTASK_H ? */

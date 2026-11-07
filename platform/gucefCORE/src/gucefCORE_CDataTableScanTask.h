/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATATABLESCANTASK_H
#define GUCEF_CORE_CDATATABLESCANTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

class CDataTableScanState;

/** The task data of a scan task: a reference on the shared state of the scan, never on the scan object */
class GUCEF_HIDDEN CDataTableScanTaskData : public CICloneable
{
    public:

    explicit CDataTableScanTaskData( CDataTableScanState* state );

    CDataTableScanTaskData( const CDataTableScanTaskData& src );

    virtual ~CDataTableScanTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CDataTableScanState* GetState( void ) const;

    private:

    CDataTableScanTaskData& operator=( const CDataTableScanTaskData& src );     /**< not implemented */

    CDataTableScanState* m_state;
};

/*-------------------------------------------------------------------------*/

/**
 *  One worker of an async CDataTableScan on a thread pool: plans files and decodes row groups until it has no credit
 *  or no work left, then ends; the scan queues it again when credits are freed. It never waits on the consumer.
 */
class GUCEF_HIDDEN CDataTableScanTask : public CTaskConsumer
{
    public:

    static const CString TaskType;

    CDataTableScanTask( void );

    virtual ~CDataTableScanTask() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDataTableScanTask( const CDataTableScanTask& src );                /**< not implemented */
    CDataTableScanTask& operator=( const CDataTableScanTask& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTFactory< CTaskConsumer, CDataTableScanTask, MT::CMutex > TDataTableScanTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLESCANTASK_H ? */

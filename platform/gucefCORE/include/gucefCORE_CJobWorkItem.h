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

#ifndef GUCEF_CORE_CJOBWORKITEM_H
#define GUCEF_CORE_CJOBWORKITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/**
 *  One unit of value-driven work that a CIJobWorkSource hands the scheduler. Like a job it is a task of the task
 *  manager: the task type (empty = the task type of the work source's job) and the payload its task data factory
 *  deserializes. Only where a payload in data form is not possible the source gives ready task data instead (taskData,
 *  owned by the item, used as it is).
 */
struct GUCEF_CORE_PUBLIC_CPP SJobWorkItem
{
    CString key;                        /**< unique among the source's pending items; may be private (never published) */
    CString publicLabel;                /**< what views may show; empty = the key is withheld */
    CString laneName;                   /**< empty = the lane of the work source's job */
    UInt32 priority;
    UInt32 estimatedCost;
    CString taskTypeName;
    CDataNode payload;
    CICloneable* taskData;              /**< optional, owned; copies clone it */

    SJobWorkItem( void );

    SJobWorkItem( const SJobWorkItem& src );

    ~SJobWorkItem();

    SJobWorkItem& operator=( const SJobWorkItem& src );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< SJobWorkItem > TJobWorkItemVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBWORKITEM_H ? */

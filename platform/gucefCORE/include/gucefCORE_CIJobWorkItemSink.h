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

#ifndef GUCEF_CORE_CIJOBWORKITEMSINK_H
#define GUCEF_CORE_CIJOBWORKITEMSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBWORKITEM_H
#include "gucefCORE_CJobWorkItem.h"
#define GUCEF_CORE_CJOBWORKITEM_H
#endif /* GUCEF_CORE_CJOBWORKITEM_H ? */

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
 *  Where a CIJobWorkSource puts the work items that are due during CIJobWorkSource::Evaluate()
 */
class GUCEF_CORE_PUBLIC_CPP CIJobWorkItemSink
{
    public:

    CIJobWorkItemSink( void );

    virtual ~CIJobWorkItemSink();

    /** Queues a run for the item (the item is copied); false when the scheduler refuses it (unknown lane, item key already pending) */
    virtual bool Dispatch( const SJobWorkItem& item ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIJOBWORKITEMSINK_H ? */

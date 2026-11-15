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

#ifndef GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H
#define GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  Where a job scheduler keeps its state across restarts: per job the last completed occurrence (UTC), enabled and
 *  priority overrides, consecutive failures, the next attempt instant; per lane the mode override; the jobs submitted as
 *  data; the state of throttle policies and work sources; optionally the last runs. The service picks the storage
 *  (CJobSchedulerFileStateStore is the file implementation of gucefCORE).
 */
class GUCEF_CORE_PUBLIC_CPP CIJobSchedulerStateStore
{
    public:

    CIJobSchedulerStateStore( void );

    virtual ~CIJobSchedulerStateStore();

    /** Dispatcher thread, first cycle */
    virtual bool LoadState( CDataNode& state, CString& error ) = 0;

    /** Dispatcher thread, outside the scheduler lock, debounced */
    virtual bool SaveState( const CDataNode& state, CString& error ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIJOBSCHEDULERSTATESTORE_H ? */

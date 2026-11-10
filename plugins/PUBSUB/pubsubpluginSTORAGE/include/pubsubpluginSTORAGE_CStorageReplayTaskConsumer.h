/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality
 *                       providing a storage interface
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_MACROS_H
#include "pubsubpluginSTORAGE_macros.h"
#define PUBSUBPLUGIN_STORAGE_MACROS_H
#endif /* PUBSUBPLUGIN_STORAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CStorageReplayTaskData;

/**
 *  Stateless task consumer that drives a single out-of-band replay request.
 *
 *  All per-replay state lives in CStorageReplayTaskData (the task data object).
 *  This consumer is registered with CTaskManager under TaskTypeName() and
 *  instantiated by the framework when a replay task is queued.
 *
 *  Lifecycle per replay:
 *    OnTaskStart  - resolves start position from index; builds ordered file list
 *    OnTaskCycle  - reads and delivers one container file per call (one chunk per yield)
 *    OnTaskEnded  - calls requestingSide->OnReplayComplete() on clean finish
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStorageReplayTaskConsumer : public CORE::CTaskConsumer
{
    public:

    CStorageReplayTaskConsumer( void );

    virtual ~CStorageReplayTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const CORE::CString& TaskTypeName( void );

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CStorageReplayTaskConsumer( const CStorageReplayTaskConsumer& src ); // not implemented
    CStorageReplayTaskConsumer& operator=( const CStorageReplayTaskConsumer& src ); // not implemented
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKCONSUMER_H ? */

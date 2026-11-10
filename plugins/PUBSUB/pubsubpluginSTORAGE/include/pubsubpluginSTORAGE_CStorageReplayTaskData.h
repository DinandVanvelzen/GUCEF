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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexDef.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H ? */

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
namespace PUBSUB {
    class CPubSubClientSide;
    class CPubSubClientTopic;
}
}

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Data object (unit-of-work) for a single replay request.
 *  Holds all replay parameters supplied at queue time plus the iteration
 *  state that persists across successive OnTaskCycle calls.
 *
 *  The CStorageReplayTaskConsumer operates entirely on this data; it holds
 *  no per-replay state of its own.
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStorageReplayTaskData : public CORE::CICloneable
{
    public:

    typedef GUCEF::vector< CORE::CString, gucef_allocator< CORE::CString > >           TStringVector;
    typedef GUCEF::vector< CStoragePubSubIndexDef, gucef_allocator<CStoragePubSubIndexDef> > TIndexDefVector;

    CStorageReplayTaskData( void );

    CStorageReplayTaskData( const CStorageReplayTaskData& src );

    virtual ~CStorageReplayTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    // --- replay request parameters (set before queuing) ---

    PUBSUB::CPubSubBookmark         startBookmark;           /**< inclusive lower bound for replay */
    PUBSUB::CPubSubBookmark         endBookmark;             /**< inclusive upper bound; NOT_INITIALIZED means "to end of data" */
    CORE::UInt64                    replayRequestId;         /**< router-assigned unique ID */
    PUBSUB::CPubSubClientSide*      requestingSide;          /**< receives OnReplayMsgsReceived / OnReplayComplete */
    PUBSUB::CPubSubClientTopic*     requestingTopic;         /**< topic context passed through to the side callbacks */

    // --- config snapshot (copied from topic config at queue time) ---

    CORE::CString                   vfsRootPath;             /**< resolved VFS root directory for this topic */
    CORE::CString                   containerFileFilter;     /**< VFS glob filter for container files, e.g. "*.gfs"; used when no index is available */
    bool                            bestEffortDeserializeIsAllowed; /**< tolerate minor container corruption */
    TIndexDefVector                 indexDefinitions;        /**< copy of the topic's index definitions */

    // --- iteration state (populated by OnTaskStart, advanced by OnTaskCycle) ---

    TStringVector                   containerFilePaths;      /**< ordered list of container VFS paths to replay */
    CORE::UInt32                    currentFileIndex;        /**< index into containerFilePaths for the next cycle */
    CORE::UInt32                    startMsgIndex;           /**< first-file only: skip messages before this index */
    CORE::UInt32                    endFileIndex;            /**< last file to replay (inclusive); ~0U = no upper limit */
    CORE::UInt32                    endMsgIndex;             /**< last message index within endFile (inclusive); ~0U = no upper limit */
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

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEREPLAYTASKDATA_H ? */

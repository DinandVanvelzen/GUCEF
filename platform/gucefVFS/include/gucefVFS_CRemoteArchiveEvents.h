/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#define GUCEF_VFS_CREMOTEARCHIVEEVENTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_VFS_CVFSARRIVALINFO_H
#include "gucefVFS_CVfsArrivalInfo.h"
#define GUCEF_VFS_CVFSARRIVALINFO_H
#endif /* GUCEF_VFS_CVFSARRIVALINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The events of a remote archive: what it raises when content arrives that a miss asked for, when a background fetch
 *  fails, when the revision of a file moves and when the link to the remote place comes or goes. The archive raises them
 *  (an archive is a notifier) and the VFS passes them on with the mount path in front on the thread that drives its pulse
 *  generator, as it does for the directory watcher events.
 *
 *  The events are edge notifications, not a queue: an observer that subscribes late does not get what it missed and asks
 *  for the state of a path instead, and a caller that missed an event repeats its call.
 *
 *  Every background job that is accepted ends with exactly one of FileArrivedEvent, ListingArrivedEvent,
 *  MetadataArrivedEvent or FetchFailedEvent; the one exception is a check of stale metadata that finds nothing changed,
 *  which ends with none. Nothing is raised once the archive is unloaded.
 */
class GUCEF_VFS_PUBLIC_CPP CRemoteArchiveEvents
{
    public:

    static const CORE::CEvent FileArrivedEvent;                 /**< data: CVfsArrivalInfo (KIND_FILE) */
    static const CORE::CEvent ListingArrivedEvent;              /**< data: CVfsArrivalInfo (KIND_LISTING) */
    static const CORE::CEvent MetadataArrivedEvent;             /**< data: CVfsArrivalInfo (KIND_METADATA) */
    static const CORE::CEvent FetchFailedEvent;                 /**< data: CVfsArrivalInfo (KIND_FAILURE) */
    static const CORE::CEvent FileChangedEvent;                 /**< data: CVfsArrivalInfo (KIND_CHANGE) */
    static const CORE::CEvent ConnectionStateChangedEvent;      /**< data of the archive: CORE::TCloneableBool, true when the link is usable; of the VFS: TConnectionStateChangedVfsEventData */

    typedef CORE::TCloneableBool TConnectionStateChangedEventData;

    /** What the VFS passes on for ConnectionStateChangedEvent: which mount, and the state */
    struct SConnectionStateInfo
    {
        CORE::CString mountPath;
        bool connected;

        SConnectionStateInfo( void ) : mountPath(), connected( false ) {}
    };
    typedef CORE::CTCloneableObj< struct SConnectionStateInfo > TConnectionStateChangedVfsEventData;

    static void RegisterEvents( void );

    /** Whether the event is one of the events of this class */
    static bool IsRemoteArchiveEvent( const CORE::CEvent& eventId );

    /** Whether the event has a CVfsArrivalInfo as its data */
    static bool IsArrivalEvent( const CORE::CEvent& eventId );

    private:

    CRemoteArchiveEvents( void );                                           /**< not implemented: only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CREMOTEARCHIVEEVENTS_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefVFS_CRemoteArchiveEvents.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteArchiveEvents::FileArrivedEvent = "GUCEF::VFS::CRemoteArchiveEvents::FileArrivedEvent";
const CORE::CEvent CRemoteArchiveEvents::ListingArrivedEvent = "GUCEF::VFS::CRemoteArchiveEvents::ListingArrivedEvent";
const CORE::CEvent CRemoteArchiveEvents::MetadataArrivedEvent = "GUCEF::VFS::CRemoteArchiveEvents::MetadataArrivedEvent";
const CORE::CEvent CRemoteArchiveEvents::FetchFailedEvent = "GUCEF::VFS::CRemoteArchiveEvents::FetchFailedEvent";
const CORE::CEvent CRemoteArchiveEvents::FileChangedEvent = "GUCEF::VFS::CRemoteArchiveEvents::FileChangedEvent";
const CORE::CEvent CRemoteArchiveEvents::ConnectionStateChangedEvent = "GUCEF::VFS::CRemoteArchiveEvents::ConnectionStateChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CRemoteArchiveEvents::RegisterEvents( void )
{GUCEF_TRACE;

    FileArrivedEvent.Initialize();
    ListingArrivedEvent.Initialize();
    MetadataArrivedEvent.Initialize();
    FetchFailedEvent.Initialize();
    FileChangedEvent.Initialize();
    ConnectionStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CRemoteArchiveEvents::IsArrivalEvent( const CORE::CEvent& eventId )
{GUCEF_TRACE;

    return eventId == FileArrivedEvent     ||
           eventId == ListingArrivedEvent  ||
           eventId == MetadataArrivedEvent ||
           eventId == FetchFailedEvent     ||
           eventId == FileChangedEvent     ;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteArchiveEvents::IsRemoteArchiveEvent( const CORE::CEvent& eventId )
{GUCEF_TRACE;

    return IsArrivalEvent( eventId ) || eventId == ConnectionStateChangedEvent;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pubsubpluginSTORAGE_CStorageReplayTaskData.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStorageReplayTaskData::CStorageReplayTaskData( void )
    : CORE::CICloneable()
    , startBookmark()
    , endBookmark()
    , replayRequestId( 0 )
    , requestingSide( GUCEF_NULL )
    , requestingTopic( GUCEF_NULL )
    , vfsRootPath()
    , containerFileFilter()
    , bestEffortDeserializeIsAllowed( false )
    , indexDefinitions()
    , containerFilePaths()
    , currentFileIndex( 0 )
    , startMsgIndex( 0 )
    , endFileIndex( static_cast< CORE::UInt32 >( -1 ) )
    , endMsgIndex( static_cast< CORE::UInt32 >( -1 ) )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStorageReplayTaskData::CStorageReplayTaskData( const CStorageReplayTaskData& src )
    : CORE::CICloneable( src )
    , startBookmark( src.startBookmark )
    , endBookmark( src.endBookmark )
    , replayRequestId( src.replayRequestId )
    , requestingSide( src.requestingSide )
    , requestingTopic( src.requestingTopic )
    , vfsRootPath( src.vfsRootPath )
    , containerFileFilter( src.containerFileFilter )
    , bestEffortDeserializeIsAllowed( src.bestEffortDeserializeIsAllowed )
    , indexDefinitions( src.indexDefinitions )
    , containerFilePaths( src.containerFilePaths )
    , currentFileIndex( src.currentFileIndex )
    , startMsgIndex( src.startMsgIndex )
    , endFileIndex( src.endFileIndex )
    , endMsgIndex( src.endMsgIndex )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStorageReplayTaskData::~CStorageReplayTaskData()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CStorageReplayTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CStorageReplayTaskData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

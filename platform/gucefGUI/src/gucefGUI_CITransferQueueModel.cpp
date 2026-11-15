/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGUI_CITransferQueueModel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CITransferQueueModel::EntriesChangedEvent = "GUCEF::GUI::CITransferQueueModel::EntriesChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CITransferQueueModel::RegisterEvents( void )
{GUCEF_TRACE;

    EntriesChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CITransferQueueModel::SEntry::SEntry( void )
    : id()
    , name()
    , detail()
    , kind()
    , totalBytes( 0 )
    , doneBytes( 0 )
    , fraction( -1.0f )
    , bytesPerSecond( 0 )
    , secondsLeft( 0 )
    , state( TRANSFERSTATE_QUEUED )
    , stateText()
    , message()
    , folder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Float32
CITransferQueueModel::SEntry::GetProgress( void ) const
{GUCEF_TRACE;

    if ( TRANSFERSTATE_DONE == state )
        return 1.0f;
    if ( fraction >= 0.0f )
        return fraction > 1.0f ? 1.0f : fraction;
    if ( 0 == totalBytes )
        return 0.0f;
    if ( doneBytes >= totalBytes )
        return 1.0f;
    return (Float32) ( (Float64) doneBytes / (Float64) totalBytes );
}

/*-------------------------------------------------------------------------*/

bool
CITransferQueueModel::SEntry::IsEnded( void ) const
{GUCEF_TRACE;

    return TRANSFERSTATE_DONE == state || TRANSFERSTATE_FAILED == state || TRANSFERSTATE_CANCELLED == state;
}

/*-------------------------------------------------------------------------*/

CITransferQueueModel::CITransferQueueModel( void )
    : CORE::CObservingNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CITransferQueueModel::~CITransferQueueModel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CITransferQueueModel::PerformAction( const CString& entryId, ETransferAction action )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CITransferQueueModel::IsActionAvailable( const TEntry& entry, ETransferAction action ) const
{GUCEF_TRACE;

    switch ( action )
    {
        case TRANSFERACTION_PAUSE:
            return TRANSFERSTATE_QUEUED == entry.state || TRANSFERSTATE_RUNNING == entry.state;
        case TRANSFERACTION_RESUME:
            return TRANSFERSTATE_PAUSED == entry.state;
        case TRANSFERACTION_CANCEL:
            return !entry.IsEnded();
        case TRANSFERACTION_RETRY:
            return TRANSFERSTATE_FAILED == entry.state || TRANSFERSTATE_CANCELLED == entry.state;
        case TRANSFERACTION_REMOVE:
            return entry.IsEnded();
        case TRANSFERACTION_OPEN_FOLDER:
            return !entry.folder.IsNULLOrEmpty();
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CString
CITransferQueueModel::StateToString( ETransferState state )
{GUCEF_TRACE;

    static const char* const names[] = { "Queued", "Running", "Paused", "Finishing", "Done", "Failed", "Cancelled" };
    return (UInt32) state < (UInt32) TRANSFERSTATE_COUNT ? CString( names[ state ] ) : CString( "Unknown" );
}

/*-------------------------------------------------------------------------*/

CString
CITransferQueueModel::ActionToString( ETransferAction action )
{GUCEF_TRACE;

    static const char* const names[] = { "Pause", "Resume", "Cancel", "Retry", "Remove", "Open folder" };
    return (UInt32) action < (UInt32) TRANSFERACTION_COUNT ? CString( names[ action ] ) : CString( "Unknown" );
}

/*-------------------------------------------------------------------------*/

const CString&
CITransferQueueModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CITransferQueueModel";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

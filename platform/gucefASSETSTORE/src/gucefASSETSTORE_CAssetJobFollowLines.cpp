/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetJobFollowLines.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The first whole number in the bytes of the text from the byte index on; false when there is none */
bool
FirstNumber( const CORE::CString& text, UInt32 from, UInt32 to, UInt32& number )
{
    const char* chars = text.C_String();
    UInt32 i = from;
    while ( i < to && ( chars[ i ] < '0' || chars[ i ] > '9' ) )
        ++i;
    if ( i >= to )
        return false;
    number = 0;
    while ( i < to && chars[ i ] >= '0' && chars[ i ] <= '9' )
    {
        number = number * 10 + (UInt32) ( chars[ i ] - '0' );
        ++i;
    }
    return true;
}
}

/*-------------------------------------------------------------------------*/

CAssetJobFollowLines::CAssetJobFollowLines( UInt32 step, UInt32 quietMs )
    : m_step( 0 == step ? 1 : step )
    , m_quietMs( quietMs )
    , m_started( false )
    , m_phase()
    , m_count( 0 )
    , m_lastLineMs( 0 )
    , m_lastLine()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetJobFollowLines::GetPhase( const CAssetJobEntry& job, CORE::CString& phase, UInt32& count )
{GUCEF_TRACE;

    count = 0;
    // bytes, not characters: the text may hold a title of the store in any script
    const char* chars = job.stateText.C_String();
    const UInt32 bytes = job.stateText.ByteSize() > 0 ? job.stateText.ByteSize() - 1 : 0;
    UInt32 colon = 0;
    while ( colon < bytes && ':' != chars[ colon ] )
        ++colon;
    UInt32 digitBefore = 0;
    const bool named = colon > 0 && colon < bytes && !FirstNumber( job.stateText, 0, colon, digitBefore );
    const CORE::CString state = AssetJobStateToString( job.state );
    const CORE::CString status = AssetStoreStatusToString( job.status );
    if ( named )
    {
        phase = state + "|" + status + "|" + CORE::CString( chars, colon );
        if ( !FirstNumber( job.stateText, colon + 1, bytes, count ) )
            count = 0;
        return;
    }
    // a phase without a name (the downloads of a run: "3 of 12 queued, ..."): counted by the downloads done
    phase = state + "|" + status + "|" + ( job.itemsTotal > 0 ? CORE::CString( "downloads" ) : job.stateText );
    count = job.itemsDone;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetJobFollowLines::DescribeProgress( const CAssetJobEntry& job )
{GUCEF_TRACE;

    CORE::CString text = job.stateText.IsNULLOrEmpty() ? CORE::CString( AssetJobStateToString( job.state ) ) : job.stateText;
    if ( job.itemsTotal > 0 )
        text += " (" + CORE::ToString( job.itemsDone ) + " of " + CORE::ToString( job.itemsTotal ) + " downloads)";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CAssetJobFollowLines::Next( const CAssetJobEntry& job, Int64 nowMs, CORE::CString& line )
{GUCEF_TRACE;

    line.Clear();
    CORE::CString phase;
    UInt32 count = 0;
    GetPhase( job, phase, count );
    const CORE::CString text = DescribeProgress( job );
    bool show = false;
    if ( !m_started || phase != m_phase )
        show = true;                                            // a new phase
    else
    if ( count / m_step != m_count / m_step )
        show = true;                                            // another step of counts within the phase
    else
    if ( text != m_lastLine && nowMs - m_lastLineMs >= (Int64) m_quietMs )
        show = true;                                            // a quiet phase that moved
    if ( !show )
        return false;
    m_started = true;
    m_phase = phase;
    m_count = count;
    m_lastLineMs = nowMs;
    m_lastLine = text;
    line = text;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetJobFollowLines::DescribeDetached( const CORE::CString& jobId, const CORE::CString& statusCommand, const CORE::CString& cancelCommand )
{GUCEF_TRACE;

    return "Stopped following; the job continues on the service: " + jobId + ". Its progress: " + statusCommand + "; to stop it: " + cancelCommand;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

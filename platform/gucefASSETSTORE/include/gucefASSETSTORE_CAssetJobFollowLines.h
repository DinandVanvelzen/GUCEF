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

#ifndef GUCEF_ASSETSTORE_CASSETJOBFOLLOWLINES_H
#define GUCEF_ASSETSTORE_CASSETJOBFOLLOWLINES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#include "gucefASSETSTORE_CAssetJobEntry.h"
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The progress lines of a console that follows a job of the service (a library sync of the probe or of a tool): one line when
 *  the phase of the job changes, then one every 'step' counts within the phase, and one after 'quietMs' without a line, so a
 *  long phase is not a line per item. The phase is the text of the state up to its first ':' (when that has no digit; "listing",
 *  "details of the items"), with the state and status of the job; the count is the first number after that ':', or the downloads
 *  done of a phase without one. Not thread safe: one follower uses it.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetJobFollowLines
{
    public:

    explicit CAssetJobFollowLines( UInt32 step = 10, UInt32 quietMs = 30000 );

    /** The line to show for the job as it is now; false when nothing new needs a line */
    bool Next( const CAssetJobEntry& job, Int64 nowMs, CORE::CString& line );

    /** The phase and the count of the job as Next() sees them */
    static void GetPhase( const CAssetJobEntry& job, CORE::CString& phase, UInt32& count );

    /** The progress of the job in one line: its state text, and the downloads of a phase that counts them */
    static CORE::CString DescribeProgress( const CAssetJobEntry& job );

    /**
     *  What a console says when its follower stops (Ctrl-C) while the job goes on at the service: that it goes on, and the commands
     *  that show and cancel it
     */
    static CORE::CString DescribeDetached( const CORE::CString& jobId, const CORE::CString& statusCommand, const CORE::CString& cancelCommand );

    private:

    UInt32 m_step;
    UInt32 m_quietMs;
    bool m_started;
    CORE::CString m_phase;
    UInt32 m_count;
    Int64 m_lastLineMs;
    CORE::CString m_lastLine;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETJOBFOLLOWLINES_H ? */

/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CREQUESTBUDGET_H
#define ANCESTRYBACKUP_CREQUESTBUDGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <string>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How much the tool asks of the site, and how fast. Requests are spaced out with a random variation so they do
 *  not come at a fixed beat, a run has a maximum number of requests and so does a day. The count of the day is
 *  kept in a file so that it survives the tool being started again.
 */
class CRequestBudget
{
    public:

    CRequestBudget( void );

    /**
     *  @param delayInMs        the pause between two requests
     *  @param jitterPercent    how much the pause varies: 40 makes it between 60% and 140% of the delay
     *  @param maxPerRun        the most requests in one run of the tool, 0 for no limit
     *  @param maxPerDay        the most requests in one day (UTC), 0 for no limit
     */
    void Configure( UInt32 delayInMs, UInt32 jitterPercent, UInt32 maxPerRun, UInt32 maxPerDay );

    /** Keeps the count of the day in this file, and reads it now */
    void SetStateFile( const CORE::CString& path, UInt64 nowInMs );

    /** May there be another request? If not, 'reason' says why */
    bool CanRequest( UInt64 nowInMs, std::string& reason );

    /** How long to wait from now before the next request may be sent */
    UInt32 GetDelayBeforeNextInMs( UInt64 nowInMs ) const;

    /** To be called for every request that is sent */
    void RecordRequest( UInt64 nowInMs );

    UInt32 GetRunCount( void ) const;

    UInt32 GetTodayCount( void ) const;

    /** 20261017 for the day (UTC) the time is in */
    static std::string DayOf( UInt64 unixEpochInMs );

    private:

    void Save( void ) const;

    private:

    UInt32 m_delayInMs;
    UInt32 m_jitterPercent;
    UInt32 m_maxPerRun;
    UInt32 m_maxPerDay;
    CORE::CString m_stateFile;
    std::string m_day;
    UInt32 m_todayCount;
    UInt32 m_runCount;
    UInt64 m_lastRequestInMs;
    mutable UInt32 m_nextDelayInMs;
    mutable bool m_nextDelayChosen;
};

/**
 *  The tool stops when the site shows signs of being unhappy with it: an error that says to slow down or go
 *  away, a challenge page. It writes down why, and does not go on, in this run or in later ones, until the user has
 *  looked at it and says it is fine. There are no retries and no waiting it out.
 */
class CHaltState
{
    public:

    static CORE::CString GetPath( const CORE::CString& stateDir );

    /** Is the tool halted? 'reason' is what was written down */
    static bool IsHalted( const CORE::CString& stateDir, std::string& reason );

    static bool Halt( const CORE::CString& stateDir, const std::string& reason );

    static void Clear( const CORE::CString& stateDir );
};

/** Looks at a response and decides whether the site is telling us to stop */
class CSiteResponseCheck
{
    public:

    /**
     *  @param status           http status
     *  @param contentType      of the response
     *  @param mitigationHeader the value of the cf-mitigated header, which is how Cloudflare says a challenge was served
     *  @param bodyStart        the first part of the body
     *  @param expectJson       the request was for JSON, so an html page in return is not what was asked
     *  @param reason           receives what is wrong
     *  @return true if the site says to stop
     */
    static bool IsTrouble( Int32 status, const std::string& contentType, const std::string& mitigationHeader, const std::string& bodyStart, bool expectJson, std::string& reason );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CREQUESTBUDGET_H ? */

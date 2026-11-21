/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CSITEDISCOVERYJOURNAL_H
#define GUCEF_WEB_CSITEDISCOVERYJOURNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#include "gucefWEB_CIWebObservationSink.h"
#define GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#endif /* GUCEF_WEB_CIWEBOBSERVATIONSINK_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The journal of site discovery: an append-only JSON Lines file, one line per observed request and per user action,
 *  from which the endpoint catalog and its report can be rebuilt at any time without a browser (Load()).
 *
 *  Every line is redacted by the redactor (see CSiteDiscoveryRedactor, SetRedactor()) before it is written. A line that
 *  would still contain a secret value the redactor registered is not written as it is: a stub takes its place, with the
 *  sequence number, the time, the method and the host only (or the action number of an action) and the class of the
 *  secret that was found, never its value, and the line is counted (GetRefusedCount()). The journal is thereby complete
 *  in its numbering and a catalog rebuilt from it counts the stubs. The file is made owner-only (0600) and
 *  every line is flushed when it is written. Request lines start with {"seq": so GetNextSequenceOf() finds the next
 *  sequence number from the end of the file; the format of the lines is described in the design of site discovery.
 *
 *  Not thread-safe.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteDiscoveryJournal : public CIWebObservationSink
{
    public:

    /** What Scrub() did */
    struct SScrubResult
    {
        UInt32 lines;               /**< lines read */
        UInt32 stubbedRequests;     /**< request lines that matched a never-record rule and were reduced to the line of a never-record request */
        UInt32 droppedRequests;     /**< the same, left out altogether because noteNeverRecorded is off */
        UInt32 stubbedActions;      /**< action lines whose page matched a never-record rule: label and page removed */
        UInt32 unreadableLines;     /**< lines that could not be read, kept as they were */
        CORE::CString backupPath;   /**< the copy of the journal as it was, owner-only */

        SScrubResult( void ) : lines( 0 ), stubbedRequests( 0 ), droppedRequests( 0 ), stubbedActions( 0 ), unreadableLines( 0 ), backupPath() {}
    };

    CSiteDiscoveryJournal( void );

    virtual ~CSiteDiscoveryJournal();

    /** Shares the redactor of a session; GUCEF_NULL returns to the journal's own, with the default settings */
    void SetRedactor( CSiteDiscoveryRedactor* redactor );

    CSiteDiscoveryRedactor& GetRedactor( void ) const;

    /** Opens the journal for appending, creating it and a missing folder owner-only, and continues its sequence numbers */
    bool Open( const CORE::CString& path );

    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetPath( void ) const;

    /** Appends a request line; false when the journal is not open, a stub had to be written in its place or the line could not be written */
    bool Append( const CObservedWebRequest& observation );

    bool AppendAction( const CObservedUserAction& action );

    /** The sequence number to give the next observation: one past the highest in the journal */
    UInt32 GetNextSequence( void ) const;

    /** The lines that were replaced by a stub because they would have contained a secret value */
    UInt32 GetRefusedCount( void ) const;

    /** The same lines per class of the secret that was found: "cookie: 2, query-value: 1"; empty when there were none */
    CORE::CString GetRefusedSummary( void ) const;

    const CSiteDiscoveryRedactor::TClassCountMap& GetRefusedClassCounts( void ) const;

    virtual void OnWebRequestObserved( const CObservedWebRequest& observation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUserActionObserved( const CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE;

    /** The journal line of a request, redacted, without the line end */
    CORE::CString ToJournalLine( const CObservedWebRequest& observation ) const;

    CORE::CString ToJournalLine( const CObservedUserAction& action ) const;

    /** A request line read back; false when it is not a request line */
    static bool FromJournalDocument( const CORE::CDataNode& document, CObservedWebRequest& observation );

    /** An action line read back; false when it is not an action line */
    static bool ActionFromJournalDocument( const CORE::CDataNode& document, CObservedUserAction& action );

    /**
     *  Reads a journal and hands every line to the sink, in order. A request line without a response shape whose saved
     *  response is still there (bodyFile, relative to the folder of the journal) gets its shape from that file when the
     *  sink wants one. Lines that cannot be read are skipped. Needs the json codec plugin.
     *  @return the number of requests read
     */
    UInt32 Load( const CORE::CString& path, CIWebObservationSink& sink, UInt32* nextSequence = GUCEF_NULL ) const;

    /**
     *  Applies the never-record rules of the redactor's settings to an existing journal file: copies it first to a backup
     *  next to it (the path with .bak, or .bak.1, .bak.2 ... when that exists; owner-only), then rewrites it so that
     *  a request line that matches a rule but was written as an ordinary request is reduced to the line of a never-record
     *  request (method, host, path template, status; no query, headers, shapes, bodies, cookies), or left out when
     *  noteNeverRecorded is off, and an action line whose page matches a rule loses its label and page. Other lines are
     *  kept byte for byte, so sequence numbers stay. The journal must not be open for appending. Needs the json codec plugin.
     */
    bool Scrub( const CORE::CString& path, SScrubResult& result, CORE::CString& problem ) const;

    /** The sequence number after the highest in the journal file, from its last part only. 1 if there is no journal */
    static UInt32 GetNextSequenceOf( const CORE::CString& path );

    /** Reads a whole file, in large pieces */
    static bool ReadWholeFile( const CORE::CString& path, CORE::CString& content );

    private:

    CSiteDiscoveryJournal( const CSiteDiscoveryJournal& src );              /**< not implemented */
    CSiteDiscoveryJournal& operator=( const CSiteDiscoveryJournal& src );   /**< not implemented */

    bool WriteLine( const CORE::CString& line );
    void NoteRefused( CSiteDiscoveryRedactor::ESecretClass secretClass, const CORE::CString& what );
    CORE::CString ToStubLine( const CObservedWebRequest& observation, const CORE::CString& className ) const;
    CORE::CString ToStubLine( const CObservedUserAction& action, const CORE::CString& className ) const;

    private:

    CSiteDiscoveryRedactor m_ownRedactor;
    CSiteDiscoveryRedactor* m_redactor;
    CORE::CFileAccess m_file;
    CORE::CString m_path;
    UInt32 m_nextSequence;
    UInt32 m_refusedCount;
    CSiteDiscoveryRedactor::TClassCountMap m_refusedByClass;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSITEDISCOVERYJOURNAL_H ? */

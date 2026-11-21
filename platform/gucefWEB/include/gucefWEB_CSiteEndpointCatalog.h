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

#ifndef GUCEF_WEB_CSITEENDPOINTCATALOG_H
#define GUCEF_WEB_CSITEENDPOINTCATALOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#include "gucefWEB_CIWebObservationSink.h"
#define GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#endif /* GUCEF_WEB_CIWEBOBSERVATIONSINK_H ? */

#ifndef GUCEF_WEB_CDISCOVEREDENDPOINT_H
#include "gucefWEB_CDiscoveredEndpoint.h"
#define GUCEF_WEB_CDISCOVEREDENDPOINT_H
#endif /* GUCEF_WEB_CDISCOVEREDENDPOINT_H ? */

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
 *  Collects the observations of site discovery into endpoints (same method, host and path template) and user actions,
 *  and renders the Markdown report that tells how the web application works: which endpoints there are, how they take
 *  their parameters and bodies, how they page, which responses are files, which requests follow which user action.
 *
 *  What is kept follows the privacy rules of its redactor (see CSiteDiscoveryRedactor, SetRedactor()): without example
 *  values only names, types and counts; a never-record request only as having happened. A part of the report (an
 *  endpoint, a host row, an action, a summary line) that would contain a secret value the redactor registered is not
 *  shown: a placeholder names its number and the class of the secret, never the value, and the rest of the report is
 *  written whole; GetRedactedEntryCount() and GetRedactionSummary() tell what the last render left out. The report is
 *  deterministic: the same observations in the same order give the same text, so a catalog rebuilt from a journal
 *  renders what the live one rendered, except that a journal line that was reduced to a stub (see CSiteDiscoveryJournal)
 *  is only counted.
 *
 *  Not thread-safe.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteEndpointCatalog : public CIWebObservationSink
{
    public:

    enum EKind
    {
        KIND_DATA   ,     /**< XHR, fetch and anything else that returns JSON or XML */
        KIND_MEDIA  ,     /**< images, audio, video, documents and downloaded files */
        KIND_PAGE   ,     /**< html documents */
        KIND_OTHER        /**< scripts, styles, fonts, tracking: only counted */
    };

    /** A user action and the endpoints of the requests that followed it */
    struct SActionRecord
    {
        CObservedUserAction action;
        CORE::CString::StringVector steps;      /**< "METHOD host/path-template", consecutive repeats once */
        UInt32 requestCount;
        UInt32 droppedSteps;

        SActionRecord( void ) : action(), steps(), requestCount( 0 ), droppedSteps( 0 ) {}
    };

    typedef GUCEF::vector< CDiscoveredEndpoint* >   TEndpointVector;
    typedef GUCEF::vector< SActionRecord >          TActionVector;

    static const UInt32 MaxExampleUrls = 3;
    static const UInt32 MaxExampleValues = 4;
    static const UInt32 MaxPostDataExamples = 2;
    static const UInt32 MaxPostDataLength = 600;
    static const UInt32 MaxBodyFiles = 3;
    static const UInt32 MaxStepsPerAction = 30;

    CSiteEndpointCatalog( void );

    virtual ~CSiteEndpointCatalog();

    /** Shares the redactor of a session; GUCEF_NULL returns to the catalog's own, with the default settings */
    void SetRedactor( CSiteDiscoveryRedactor* redactor );

    CSiteDiscoveryRedactor& GetRedactor( void ) const;

    /** Decides what a request is, from its resource type and the content type of its response */
    static EKind Classify( const CORE::CString& resourceType, const CORE::CString& mimeType );

    static EKind Classify( const CObservedWebRequest& observation );

    /**
     *  Adds an observation
     *  @return true if this is the first observation of its endpoint
     */
    bool Add( const CObservedWebRequest& observation );

    void AddAction( const CObservedUserAction& action );

    /** Is this a data response whose shape is worth describing: has its endpoint no shape yet, or fewer than maxShapesPerEndpoint merged ones? */
    bool NeedsShape( const CObservedWebRequest& observation ) const;

    virtual void OnWebRequestObserved( const CObservedWebRequest& observation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUserActionObserved( const CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool WantsResponseShape( const CObservedWebRequest& observation ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsNewResponseShape( const CObservedWebRequest& observation, const CWebBodyShape& shape ) const GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetRequestCount( void ) const;

    UInt32 GetEndpointCount( EKind kind ) const;

    /** Endpoints in the order they were first seen */
    const TEndpointVector& GetEntries( void ) const;

    const CDiscoveredEndpoint* FindEntry( const CORE::CString& method, const CORE::CString& url ) const;

    const TActionVector& GetActions( void ) const;

    /**
     *  The Markdown report
     *  @param extraSummaryLines    lines of the caller, shown after the summary
     *  @return true when the report is rendered, also when parts of it were left out for holding a secret value; false,
     *          with an empty report, only when the finished report still holds one (a value that spans parts)
     */
    bool RenderReport( CORE::CString& report, const CORE::CString::StringVector& extraSummaryLines = CORE::CString::StringVector() ) const;

    /** The parts the last RenderReport() left out because they would have held a secret value */
    UInt32 GetRedactedEntryCount( void ) const;

    /** "cookie: 2, query-value: 1" for the parts the last RenderReport() left out; empty when none */
    CORE::CString GetRedactionSummary( void ) const;

    /** The journal lines that were stubs (see CSiteDiscoveryJournal) among the observations added: they are counted, not catalogued */
    UInt32 GetStubCount( void ) const;

    CORE::CString GetStubSummary( void ) const;

    void Clear( void );

    private:

    CSiteEndpointCatalog( const CSiteEndpointCatalog& src );                /**< not implemented */
    CSiteEndpointCatalog& operator=( const CSiteEndpointCatalog& src );     /**< not implemented */

    typedef GUCEF::map< CORE::CString, CDiscoveredEndpoint* >   TEndpointMap;
    typedef GUCEF::map< CORE::CString, UInt32 >                 TCountMap;
    typedef GUCEF::map< UInt32, size_t >                        TActionIndexMap;

    /** What the requests to one host showed of headers and cookies, by name only */
    struct SHostInfo
    {
        CORE::CString::StringSet requestHeaders;
        CORE::CString::StringSet cookiesSent;
        CORE::CString::StringSet cookiesSet;
    };

    typedef GUCEF::map< CORE::CString, SHostInfo >              THostInfoMap;

    void RenderEntry( const CDiscoveredEndpoint& entry, CORE::CString& out ) const;
    bool Guard( const CORE::CString& block, CORE::CString& reason ) const;
    void AddStep( UInt32 actionId, const CORE::CString& step );
    UInt32 GetMergedShapeLimit( void ) const;

    private:

    CSiteDiscoveryRedactor m_ownRedactor;
    CSiteDiscoveryRedactor* m_redactor;
    TEndpointMap m_entryMap;
    TEndpointVector m_entries;
    TCountMap m_hostCounts;
    THostInfoMap m_hostInfo;
    TCountMap m_ignoredCounts;
    TActionVector m_actions;
    TActionIndexMap m_actionIndex;
    UInt32 m_droppedActions;
    UInt32 m_requestCount;
    Float64 m_firstTimestamp;
    Float64 m_lastTimestamp;
    UInt32 m_stubCount;
    CSiteDiscoveryRedactor::TClassCountMap m_stubByClass;
    mutable UInt32 m_lastRedactedCount;
    mutable CSiteDiscoveryRedactor::TClassCountMap m_lastRedactedByClass;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSITEENDPOINTCATALOG_H ? */

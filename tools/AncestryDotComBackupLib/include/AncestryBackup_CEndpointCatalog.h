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

#ifndef ANCESTRYBACKUP_CENDPOINTCATALOG_H
#define ANCESTRYBACKUP_CENDPOINTCATALOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <map>
#include <set>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ANCESTRYBACKUP_CJSONSHAPE_H
#include "AncestryBackup_CJsonShape.h"
#define ANCESTRYBACKUP_CJSONSHAPE_H
#endif /* ANCESTRYBACKUP_CJSONSHAPE_H ? */

#ifndef GUCEF_WEB_CSITEENDPOINTCATALOG_H
#include "gucefWEB_CSiteEndpointCatalog.h"
#define GUCEF_WEB_CSITEENDPOINTCATALOG_H
#endif /* GUCEF_WEB_CSITEENDPOINTCATALOG_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */

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

/** One request the browser made, as seen over the DevTools protocol */
struct SObservedRequest
{
    UInt32 sequence;
    Float64 timestamp;                                  /**< seconds, as reported by the browser */
    std::string method;
    std::string url;
    std::string resourceType;                           /**< Document, XHR, Fetch, Image, Script, ... */
    std::string mimeType;                               /**< of the response */
    Int32 status;                                       /**< 0 when there was no response */
    UInt64 encodedDataLength;
    bool failed;
    std::string errorText;
    std::map< std::string, std::string > requestHeaders;    /**< names in lower case, secrets are not recorded */
    std::string postData;
    std::string bodyFile;                               /**< relative to the discovery directory, empty if the body was not saved */

    SObservedRequest( void ) : sequence( 0 ), timestamp( 0.0 ), method(), url(), resourceType(), mimeType(), status( 0 ),
                               encodedDataLength( 0 ), failed( false ), errorText(), requestHeaders(), postData(), bodyFile() {}
};

/** What is known about one kind of request: same method, host and path template */
struct SEndpointEntry
{
    std::string method;
    std::string host;
    std::string pathTemplate;
    Int32 kind;                 /**< CEndpointCatalog::EKind of the first observation */
    UInt32 firstSequence;
    UInt32 requestCount;
    std::map< Int32, UInt32 > statusCounts;
    std::set< std::string > resourceTypes;
    std::set< std::string > mimeTypes;
    std::vector< std::string > exampleUrls;
    std::map< std::string, std::vector< std::string > > queryParams;    /**< name to a few distinct example values */
    std::set< std::string > requestHeaderNames;
    std::vector< std::string > postDataExamples;
    std::vector< std::string > bodyFiles;
    UInt64 totalBytes;
    bool hasShape;
    bool shapeTried;            /**< the response was looked at once, whether that gave a shape or not */
    SJsonShape shape;

    SEndpointEntry( void ) : method(), host(), pathTemplate(), kind( 0 ), firstSequence( 0 ), requestCount( 0 ), statusCounts(), resourceTypes(), mimeTypes(),
                             exampleUrls(), queryParams(), requestHeaderNames(), postDataExamples(), bodyFiles(), totalBytes( 0 ), hasShape( false ), shapeTried( false ), shape() {}
};

/**
 *  Collects the observations of discovery into endpoints and turns them into the report that tells how the web
 *  application gets its data. The work is done by WEB::CSiteEndpointCatalog; this adds the policy of the tool:
 *
 *    - example values are kept (query values, request bodies, saved responses): the report stays on the user's disk
 *    - sign-in, account and credential requests are noted, never their content (the generic never-record rules)
 *    - so are the DNA pages, which hold the data of other people, unless DNA recording is on
 *    - the ids that follow a "tree" or "trees" path segment are collected as candidates for the tree id
 *
 *  It is a sink of observations, so the generic recorder and journal reader can feed it directly.
 */
class CEndpointCatalog : public WEB::CIWebObservationSink
{
    public:

    enum EKind
    {
        KIND_DATA   = WEB::CSiteEndpointCatalog::KIND_DATA  ,     /**< XHR, fetch and anything else that returns JSON or XML */
        KIND_MEDIA  = WEB::CSiteEndpointCatalog::KIND_MEDIA ,     /**< images, audio, video and documents */
        KIND_PAGE   = WEB::CSiteEndpointCatalog::KIND_PAGE  ,     /**< html documents */
        KIND_OTHER  = WEB::CSiteEndpointCatalog::KIND_OTHER       /**< scripts, styles, fonts, tracking: only counted */
    };

    CEndpointCatalog( void );

    virtual ~CEndpointCatalog();

    /** The settings of site discovery with the policy of the tool, see the class description */
    static WEB::CSiteDiscoverySettings MakeSettings( bool recordDna );

    /** Decides what an observation is, from its resource type and mime type */
    static EKind Classify( const SObservedRequest& observation );

    /** Is this a saved JSON response the shape of which is worth analyzing, and does the catalog not have one yet for its endpoint? */
    bool NeedsShape( const SObservedRequest& observation ) const;

    /** Kept for the interface: an observation with a saved response marks its endpoint as looked at when it is added */
    void MarkShapeTried( const SObservedRequest& observation );

    /**
     *  Adds an observation.
     *  @param shape    what the response body looks like, if it was analyzed
     *  @return true if this is the first observation of its endpoint
     */
    bool Add( const SObservedRequest& observation, const SJsonShape* shape = GUCEF_NULL );

    /** Adds an observation of the generic recorder or journal; @return true if this is the first observation of its endpoint */
    bool AddObservation( const WEB::CObservedWebRequest& observation );

    UInt32 GetRequestCount( void ) const;

    UInt32 GetEndpointCount( EKind kind ) const;

    /** Entries in the order they were first seen */
    const std::vector< SEndpointEntry* >& GetEntries( void ) const;

    const SEndpointEntry* FindEntry( const CORE::CString& method, const CORE::CString& url ) const;

    /** The ids that show up after a "tree" or "trees" path segment, candidates for the tree id */
    const std::set< std::string >& GetTreeIds( void ) const;

    /** The Markdown report; empty when it was refused because it would contain a secret value */
    CORE::CString RenderReport( void ) const;

    /** Sign in, account and security requests, and the DNA pages: what IsCredentialUrl() or IsDnaUrl() says */
    static bool IsSensitiveUrl( const std::string& url );

    /** The requests that carry credentials: the never-record rules of WEB::CSiteDiscoverySettings */
    static bool IsCredentialUrl( const std::string& url );

    /** The DNA pages and services */
    static bool IsDnaUrl( const std::string& url );

    /** Record the bodies of DNA requests as well. Off unless the DNA facility is being studied, credentials stay out regardless. */
    void SetRecordDna( bool record );

    bool IsRecordDna( void ) const;

    /** Is this a request of which the content is kept out of the report and the journal? Credentials always, DNA unless it is recorded. */
    bool IsExcluded( const std::string& url ) const;

    /** Is this a header of which the value is a secret: the name stays so we learn what the site expects, the value goes */
    static bool IsSecretHeader( const std::string& lowerCaseName );

    /** Shares the redactor of a recorder, so the secrets it saw are known; GUCEF_NULL returns to the catalog's own */
    void SetSharedRedactor( WEB::CSiteDiscoveryRedactor* redactor );

    WEB::CSiteDiscoveryRedactor& GetRedactor( void ) const;

    WEB::CSiteEndpointCatalog& GetGeneric( void );

    virtual void OnWebRequestObserved( const WEB::CObservedWebRequest& observation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUserActionObserved( const WEB::CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool WantsResponseShape( const WEB::CObservedWebRequest& observation ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsNewResponseShape( const WEB::CObservedWebRequest& observation, const WEB::CWebBodyShape& shape ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The generic form of an observation of the tool */
    static WEB::CObservedWebRequest ToObservation( const SObservedRequest& observation, const SJsonShape* shape );

    private:

    CEndpointCatalog( const CEndpointCatalog& src );
    CEndpointCatalog& operator=( const CEndpointCatalog& src );

    typedef std::map< const WEB::CDiscoveredEndpoint*, SEndpointEntry* > TEntryCache;

    SEndpointEntry* Refresh( const WEB::CDiscoveredEndpoint& endpoint ) const;

    private:

    WEB::CSiteDiscoveryRedactor m_ownRedactor;
    WEB::CSiteEndpointCatalog m_catalog;
    std::set< std::string > m_treeIds;
    bool m_recordDna;
    mutable TEntryCache m_cache;
    mutable std::vector< SEndpointEntry* > m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CENDPOINTCATALOG_H ? */

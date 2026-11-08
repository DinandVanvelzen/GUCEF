/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#ifndef GUCEF_MCPLOCALLLM_LLMCLIENT_H
#define GUCEF_MCPLOCALLLM_LLMCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <vector>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** GET url and parse the body as JSON. @return false with the reason in 'error' unless there is a 200 response with JSON */
bool HttpGetJson( const GUCEF::CORE::CString& url    ,
                  const GUCEF::CORE::CString& apiKey ,
                  GUCEF::UInt32 timeoutInMs          ,
                  GUCEF::CORE::CDataNode& root       ,
                  GUCEF::CORE::CString& error        );

/** POST a JSON body to url and parse the response as JSON */
bool HttpPostJson( const GUCEF::CORE::CString& url    ,
                   const GUCEF::CORE::CString& apiKey ,
                   const GUCEF::CORE::CDataNode& body ,
                   GUCEF::UInt32 timeoutInMs          ,
                   GUCEF::CORE::CDataNode& root       ,
                   GUCEF::CORE::CString& error        ,
                   bool* connectionFailed = GUCEF_NULL );

/*-------------------------------------------------------------------------*/

/** A chat completion request to an OpenAI compatible server */
struct SChatRequest
{
    GUCEF::CORE::CString model;         /**< the model name the server reports; empty: not sent */
    GUCEF::CORE::CString system;
    GUCEF::CORE::CString user;
    GUCEF::UInt32 maxTokens;
    bool thinking;                      /**< let a reasoning model think first: slower, better on hard questions */
    GUCEF::UInt32 timeoutInMs;

    SChatRequest( void );
};

/**
 *  @param connectionFailed  when not null: set to whether the request failed without a response of the server,
 *                           e.g. because the server could not be reached or went away
 *  @return false with the reason in 'error' when there is no answer
 */
bool ChatCompletion( const GUCEF::CORE::CString& baseUrl ,
                     const GUCEF::CORE::CString& apiKey  ,
                     const SChatRequest& request         ,
                     GUCEF::CORE::CString& answer        ,
                     GUCEF::CORE::CString& error         ,
                     bool* connectionFailed = GUCEF_NULL );

/**
 *  Whether host (a name or IPv4 address) is this machine: a loopback address or an address of one of its interfaces
 *  @param resolved  when not null: set to whether the host name could be resolved
 */
bool IsLocalHost( const GUCEF::CORE::CString& host, bool* resolved = GUCEF_NULL );

/** The host of an http(s) URL: http://host:8080/path -> host */
GUCEF::CORE::CString HostOfUrl( const GUCEF::CORE::CString& url );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a probe found out about one LLM server (llama.cpp llama-server) */
struct SEndpointState
{
    GUCEF::CORE::CString url;
    bool reachable;                     /**< /health answered with status 200 */
    GUCEF::CORE::CString error;         /**< why it is not reachable */
    GUCEF::CORE::CString model;         /**< the model alias, empty when unknown */
    GUCEF::UInt32 contextSize;          /**< context size per request slot, 0 when unknown */
    GUCEF::UInt32 slots;                /**< request slots, 0 when unknown */
    GUCEF::UInt32 idleSlots;            /**< slots not processing a request */
    GUCEF::UInt32 inFlight;             /**< work of this server running on the endpoint */
    bool isLocal;                       /**< served by this machine: available for as long as this server runs */
    bool isEphemeral;                   /**< on a machine that is not always on, and not this one: may go away */
    GUCEF::UInt32 downForMs;            /**< not reachable: since when it was last checked, without checking again */

    SEndpointState( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The LLM servers work can run on. The order of preference:
 *
 *    1. endpoints served by this machine, which are available for as long as this server runs. That includes the
 *       endpoint of an ephemeral node when this server runs on that node.
 *    2. other endpoints, in the order given
 *    3. ephemeral endpoints of other machines, which are not always on and may go away during work
 *
 *  Work gets the first endpoint that is reachable, has a large enough context and has a slot that no other work of
 *  this server occupies. When every usable endpoint is busy the work goes to the first usable one, where the server
 *  queues it. An endpoint that is not reachable is not checked again for RetryDelayInMs, so a machine that is off
 *  costs no time; once it is on again, it is used from the next check.
 *
 *  Thread-safe.
 */
class CLlmEndpointPool
{
    public:

    static const GUCEF::UInt32 RetryDelayInMs = 30000;

    /**
     *  @param urls           all endpoints, in order of preference
     *  @param ephemeralUrls  those of urls on machines that are not always on
     */
    CLlmEndpointPool( const GUCEF::CORE::CString::StringVector& urls          ,
                      const GUCEF::CORE::CString::StringVector& ephemeralUrls ,
                      const GUCEF::CORE::CString& apiKey                      ,
                      GUCEF::UInt32 minContextSize                            );

    /**
     *  Probes the endpoints. The state of each is in the order of preference
     *  @param recheck  also probe endpoints that were not reachable less than RetryDelayInMs ago
     */
    void ProbeAll( std::vector< SEndpointState >& states, bool recheck = false );

    /**
     *  After work on an endpoint failed without a response: checks it at once and, when it does not answer, it counts
     *  as not reachable. @return whether it is reachable
     */
    bool Recheck( const GUCEF::CORE::CString& url );

    /**
     *  Picks an endpoint and counts the work as running on it until Release()
     *  @param chosen   receives the state of the chosen endpoint
     *  @param report   receives a description of all endpoints
     *  @param model    only endpoints that run this model (the name they report, case ignored); empty: any
     *  @return false when no endpoint is usable
     */
    bool Acquire( SEndpointState& chosen, GUCEF::CORE::CString& report, const GUCEF::CORE::CString& model = GUCEF::CORE::CString::Empty );

    /**
     *  As Acquire(), with probe results the caller already has; for work that starts several tasks at once
     */
    bool AcquireFrom( const std::vector< SEndpointState >& states, SEndpointState& chosen, const GUCEF::CORE::CString& model = GUCEF::CORE::CString::Empty );

    void Release( const GUCEF::CORE::CString& url );

    /** Request slots that are idle and not occupied by work of this server, over all usable endpoints of the model (empty: any) */
    GUCEF::UInt32 CountFreeSlots( const std::vector< SEndpointState >& states, const GUCEF::CORE::CString& model = GUCEF::CORE::CString::Empty ) const;

    bool IsUsable( const SEndpointState& state ) const;

    /** Whether the endpoint runs the model: the name it reports, case ignored. Every endpoint runs model "" */
    static bool RunsModel( const SEndpointState& state, const GUCEF::CORE::CString& model );

    /** A text for the model about all endpoints */
    GUCEF::CORE::CString Describe( const std::vector< SEndpointState >& states ) const;

    /** The models of the usable endpoints, each once: "a (2 servers, 1 idle slot), b (1 server, 0 idle slots)"; empty when none */
    GUCEF::CORE::CString DescribeModels( const std::vector< SEndpointState >& states ) const;

    /**
     *  A file that lists more endpoints, read again whenever it changed, so endpoints come and go while this server
     *  runs. One URL per line, "ephemeral URL" for an endpoint on a machine that is not always on, # starts a comment.
     *  Its endpoints follow those given to the constructor. A file that does not exist lists none
     */
    void SetEndpointsFile( const GUCEF::CORE::CString& path );

    /** Whether no endpoint is configured: none given and no endpoints file */
    bool IsEmpty( void ) const;

    const GUCEF::CORE::CString& GetApiKey( void ) const;

    GUCEF::UInt32 GetMinContextSize( void ) const;

    private:

    struct SEndpoint
    {
        GUCEF::CORE::CString url;
        bool ephemeral;
        bool localityKnown;             /**< the host name was resolved */
        bool isLocal;
        GUCEF::UInt64 downSinceInMs;    /**< when it was found not reachable, 0 when reachable */
        GUCEF::CORE::CString downError;

        SEndpoint( void ) : url(), ephemeral( false ), localityKnown( false ), isLocal( false ), downSinceInMs( 0 ), downError() {}
    };
    typedef std::vector< SEndpoint > TEndpointVector;
    typedef std::map< GUCEF::CORE::CString, GUCEF::UInt32 > TInFlightMap;

    void Probe( const GUCEF::CORE::CString& url, SEndpointState& state ) const;

    /** Takes in the endpoints file when it changed */
    void RefreshFromFile( void );

    GUCEF::UInt32 InFlight( const GUCEF::CORE::CString& url ) const;

    /** 0: this machine, 1: other, 2: ephemeral on another machine. Resolves the host name until that succeeds */
    static GUCEF::UInt32 PreferenceClass( SEndpoint& endpoint );

    TEndpointVector m_endpoints;
    TEndpointVector m_givenEndpoints;               /**< those given to the constructor */
    GUCEF::CORE::CString m_endpointsFile;
    GUCEF::CORE::CString m_endpointsFileStamp;      /**< modification time and size the file had when it was read */
    GUCEF::CORE::CString m_apiKey;
    GUCEF::UInt32 m_minContextSize;
    mutable GUCEF::MT::CMutex m_lock;
    TInFlightMap m_inFlight;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPLOCALLLM_LLMCLIENT_H ? */

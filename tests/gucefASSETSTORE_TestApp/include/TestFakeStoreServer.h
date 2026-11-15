/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTFAKESTORESERVER_H
#define TESTFAKESTORESERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the fake store answers the downloads of one file */
class CFakeFileBehaviour
{
    public:

    bool supportsRange;             /**< a Range header gets 206 with the rest, else 200 with everything */
    GUCEF::UInt32 fail503;          /**< answers 503 this many times first */
    GUCEF::UInt32 fail429;          /**< answers 429 with Retry-After this many times first */
    GUCEF::UInt32 retryAfterSeconds;
    GUCEF::UInt32 expireFirst;      /**< answers 403 (an expired address) this many times first */
    bool corrupt;                   /**< serves other bytes than the digest says */

    CFakeFileBehaviour( void );
};

/** What the fake store was asked */
class CFakeStoreStats
{
    public:

    GUCEF::UInt32 libraryRequests;
    GUCEF::UInt32 itemRequests;
    GUCEF::UInt32 fileRequests;
    GUCEF::UInt32 rangeRequests;
    GUCEF::UInt32 refusedSessions;
    GUCEF::UInt32 tokensIssued;
    GUCEF::UInt32 expiredAnswers;
    GUCEF::UInt32 rateLimitedAnswers;
    GUCEF::UInt32 unavailableAnswers;

    CFakeStoreStats( void );
};

/**
 *  A store that lives in the test: an HTTP server on 127.0.0.1 and an ephemeral port, on a thread and a pulse generator
 *  of its own. It serves the library (/library?cursor=&pageSize=&text=&type=), item details (/items/<id>), the files
 *  of the items (/files/<file id>, with byte ranges and the misbehaviour of CFakeFileBehaviour) and a token endpoint
 *  (POST /token) for the renewal of sessions; with requireAuth set, files need "Authorization: Bearer <token>" with the
 *  token the endpoint issued last.
 */
class CFakeStoreServer : public GUCEF::MT::CActiveObject
{
    public:

    CFakeStoreServer( void );
    virtual ~CFakeStoreServer();

    bool Start( void );
    void Stop( void );

    GUCEF::CORE::CString GetBaseUrl( void ) const;

    /** Adds an item; the content of its files by file id */
    void AddItem( const GUCEF::ASSETSTORE::CAssetStoreItem& item );
    void SetFileContent( const GUCEF::CORE::CString& fileId, const GUCEF::CORE::CDynamicBuffer& content );
    void SetFileBehaviour( const GUCEF::CORE::CString& fileId, const CFakeFileBehaviour& behaviour );
    void SetRequireAuth( bool requireAuth );
    void SetTokenEndpointWorks( bool works );
    void ResetStats( void );
    CFakeStoreStats GetStats( void ) const;

    void HandleRequest( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void SendJson( GUCEF::WEB::CHttpResponseData& response, int status, const GUCEF::CORE::CDataNode& node );
    void HandleFile( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response, const GUCEF::CORE::CString& fileId );

    CFakeStoreServer( const CFakeStoreServer& src );              /**< not implemented */
    CFakeStoreServer& operator=( const CFakeStoreServer& src );   /**< not implemented */

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CDynamicBuffer > TContentMap;
    typedef GUCEF::map< GUCEF::CORE::CString, CFakeFileBehaviour > TBehaviourMap;

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    GUCEF::UInt16 m_port;
    bool m_running;
    GUCEF::ASSETSTORE::TAssetStoreItemVector m_items;
    TContentMap m_contents;
    TBehaviourMap m_behaviours;
    bool m_requireAuth;
    bool m_tokenEndpointWorks;
    GUCEF::CORE::CString m_currentToken;
    CFakeStoreStats m_stats;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKESTORESERVER_H ? */

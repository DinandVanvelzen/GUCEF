/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MCPCI_TESTAPP_TESTFAKEFORGEJO_H
#define GUCEF_MCPCI_TESTAPP_TESTFAKEFORGEJO_H

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

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A stand-in for Forgejo on 127.0.0.1 with just what gucefMcpCi uses, answering with the shapes of the Forgejo 15
 *  API (swagger.v1.json): every request needs "Authorization: token <token>", others get 401.
 *
 *    GET   /api/v1/repos/{o}/{r}                                       the repository, default branch "main"
 *    POST  /api/v1/repos/{o}/{r}/actions/workflows/{file}/dispatches   starts a run on a branch of the bare repository,
 *                                                                       with the stages given to ScriptNextRun()
 *    GET   /api/v1/repos/{o}/{r}/actions/runs[?head_sha=..]            the runs, newest first
 *    GET   /api/v1/repos/{o}/{r}/actions/runs/{id}                     a run; every answer counts as a poll of its stage
 *    POST  /api/v1/repos/{o}/{r}/actions/runs/{id}/cancel              when SetCancelSupported( true ), 404 otherwise
 *    GET   /api/v1/repos/{o}/{r}/actions/tasks                         the jobs of all runs, newest first
 *    GET   /api/v1/repos/{o}/{r}/commits/{sha}/statuses                none
 *    GET   /api/v1/repos/{o}/{r}/branches, DELETE .../branches/{name}  the branches of the bare repository
 *    GET/PUT /api/packages/{owner}/generic/{name}/{version}/{file}, DELETE /api/packages/{owner}/generic/{name}/{version}
 *    GET   /api/v1/packages/{owner}?type=generic&q=..                  the package versions
 */
class CFakeForgejo : public GUCEF::MT::CActiveObject
{
    public:

    /** A state of a run: its status and the states of its jobs, answered for 'polls' polls of the run */
    class CStage
    {
        public:

        GUCEF::CORE::CString runStatus;
        GUCEF::CORE::CString::StringVector jobNames;
        GUCEF::CORE::CString::StringVector jobStatuses;
        GUCEF::UInt32 polls;

        CStage( void );
        CStage( const GUCEF::CORE::CString& status, GUCEF::UInt32 pollCount );

        CStage& AddJob( const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& status );
    };

    typedef GUCEF::vector< CStage > TStageVector;

    CFakeForgejo( const GUCEF::CORE::CString& token, const GUCEF::CORE::CString& owner, const GUCEF::CORE::CString& repo );
    virtual ~CFakeForgejo();

    bool Start( void );
    void Stop( void );

    /** http://127.0.0.1:<port> */
    GUCEF::CORE::CString GetUrl( void ) const;

    /** The bare repository that stands for the scratch repository: branches come from it */
    void SetBareRepo( const GUCEF::CORE::CString& path );

    /** Whether a dispatch with return_run_info answers with the run (201) or with nothing (204), default true */
    void SetReturnRunInfo( bool returnRunInfo );

    void SetCancelSupported( bool cancelSupported );

    /** The stages of the run of the next dispatch; the last one stays */
    void ScriptNextRun( const TStageVector& stages );

    /** A file of a generic package */
    void AddPackageFile( const GUCEF::CORE::CString& owner, const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& version,
                         const GUCEF::CORE::CString& fileName, const GUCEF::CORE::CDynamicBuffer& content, const GUCEF::CORE::CString& createdAt );

    void AddPackageFile( const GUCEF::CORE::CString& owner, const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& version,
                         const GUCEF::CORE::CString& fileName, const GUCEF::CORE::CString& content, const GUCEF::CORE::CString& createdAt );

    bool HasPackageVersion( const GUCEF::CORE::CString& owner, const GUCEF::CORE::CString& name, const GUCEF::CORE::CString& version ) const;

    /** The id of the last run that was started, 0 when none */
    GUCEF::Int64 GetLastRunId( void ) const;
    GUCEF::CORE::CString GetLastDispatchBody( void ) const;
    GUCEF::UInt32 GetDispatchCount( void ) const;
    GUCEF::UInt32 GetUnauthorizedCount( void ) const;
    GUCEF::UInt32 GetRunListCount( void ) const;

    /** "METHOD path?query" of every request, one per line */
    GUCEF::CORE::CString GetRequestLog( void ) const;

    void Handle( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CRun
    {
        public:

        GUCEF::Int64 id;
        GUCEF::Int64 number;
        GUCEF::CORE::CString ref;
        GUCEF::CORE::CString sha;
        TStageVector stages;
        GUCEF::UInt32 stage;
        GUCEF::UInt32 pollsInStage;
        bool cancelled;

        CRun( void );
        const CStage& Current( void ) const;
    };

    class CPackageFile
    {
        public:

        GUCEF::CORE::CString owner;
        GUCEF::CORE::CString name;
        GUCEF::CORE::CString version;
        GUCEF::CORE::CString fileName;
        GUCEF::CORE::CDynamicBuffer content;
        GUCEF::CORE::CString createdAt;
    };

    typedef GUCEF::vector< CRun > TRunVector;
    typedef GUCEF::vector< CPackageFile > TPackageFileVector;

    CFakeForgejo( const CFakeForgejo& src );                /**< not implemented */
    CFakeForgejo& operator=( const CFakeForgejo& src );     /**< not implemented */

    void HandleRepo( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& rest, const GUCEF::CORE::CString& query,
                     const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    void HandlePackages( const GUCEF::CORE::CString& method, const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& query,
                         const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );
    GUCEF::CORE::CString RunJson( const CRun& run ) const;
    CRun* FindRun( GUCEF::Int64 id );

    GUCEF::CORE::CString m_token;
    GUCEF::CORE::CString m_owner;
    GUCEF::CORE::CString m_repo;
    GUCEF::CORE::CString m_bareRepo;
    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    bool m_running;
    bool m_returnRunInfo;
    bool m_cancelSupported;
    TStageVector m_nextStages;
    TRunVector m_runs;
    TPackageFileVector m_packageFiles;
    GUCEF::Int64 m_nextRunId;
    GUCEF::CORE::CString m_lastDispatchBody;
    GUCEF::UInt32 m_dispatchCount;
    GUCEF::UInt32 m_unauthorizedCount;
    GUCEF::UInt32 m_runListCount;
    GUCEF::CORE::CString m_requestLog;
};

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTFAKEFORGEJO_H ? */

/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CMAILOPERATIONTASK_H
#define GUCEF_COM_CMAILOPERATIONTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COM_CSMTPCLIENT_H
#include "gucefCOM_CSmtpClient.h"
#define GUCEF_COM_CSMTPCLIENT_H
#endif /* GUCEF_COM_CSMTPCLIENT_H ? */

#ifndef GUCEF_COM_CPOP3CLIENT_H
#include "gucefCOM_CPop3Client.h"
#define GUCEF_COM_CPOP3CLIENT_H
#endif /* GUCEF_COM_CPOP3CLIENT_H ? */

#ifndef GUCEF_COM_CIMAPCLIENT_H
#include "gucefCOM_CImapClient.h"
#define GUCEF_COM_CIMAPCLIENT_H
#endif /* GUCEF_COM_CIMAPCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EMailOperation
{
    MAILOPERATION_NONE = 0,
    MAILOPERATION_SMTP_SEND,
    MAILOPERATION_POP3_FETCH,
    MAILOPERATION_IMAP_FETCH
};

/** What an asynchronous mail operation is asked to do, and what came of it */
class GUCEF_COM_PUBLIC_CPP CMailOperationTaskData : public CORE::CICloneable
{
    public:

    EMailOperation operation;

    CSmtpClientSettings smtpSettings;
    CMailMessage message;
    CSmtpSendResult smtpResult;

    CPop3ClientSettings pop3Settings;
    CPop3FetchOptions pop3Options;
    CPop3FetchResult pop3Result;

    CImapClientSettings imapSettings;
    CImapFetchOptions imapOptions;
    CImapFetchResult imapResult;

    CMailOperationTaskData( void );
    CMailOperationTaskData( const CMailOperationTaskData& src );
    virtual ~CMailOperationTaskData() GUCEF_VIRTUAL_OVERRIDE;
    CMailOperationTaskData& operator=( const CMailOperationTaskData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The task that carries out a mail operation on a thread of a pool: it calls the same blocking function that the
 *  caller can call itself (CSmtpClient::SendMessageSync() and the like), so the work is the same and only the thread
 *  that does it differs.
 */
class GUCEF_COM_PUBLIC_CPP CMailOperationTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CMailOperationTaskConsumer( void );

    virtual ~CMailOperationTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the result */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMailOperationTaskConsumer( const CMailOperationTaskConsumer& src );
    CMailOperationTaskConsumer& operator=( const CMailOperationTaskConsumer& src );
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CMailOperationTaskConsumer, MT::CMutex > TMailOperationTaskConsumerFactory;

/*-------------------------------------------------------------------------*/

/**
 *  The asynchronous forms of the one shot mail operations: a task on a thread pool and a CFutureResult for it. The
 *  settings are copied, so the credentials come from the settings (the user name and password or token): a
 *  credential provider belongs to the thread it lives on and is not used from a pool thread.
 *
 *      CORE::CFutureResult future = CMailOperations::SendMessageAsync( settings, message );
 *      future.Await( 60000 );
 *      CSmtpSendResult result;
 *      CMailOperations::TryGetSendResult( future, result );
 *
 *  A future that has no future (HasNoFuture()) was refused: no pool, no task type.
 */
class GUCEF_COM_PUBLIC_CPP CMailOperations
{
    public:

    static CORE::CFutureResult SendMessageAsync( const CSmtpClientSettings& settings, const CMailMessage& message, const CORE::CString& threadPoolName = CORE::CString::Empty );

    static CORE::CFutureResult FetchNewPop3Async( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, const CORE::CString& threadPoolName = CORE::CString::Empty );

    static CORE::CFutureResult FetchNewImapAsync( const CImapClientSettings& settings, const CImapFetchOptions& options, const CORE::CString& threadPoolName = CORE::CString::Empty );

    /** The result of a finished task. False while it is not finished, or when the future is not one of this operation */
    static bool TryGetSendResult( const CORE::CFutureResult& future, CSmtpSendResult& result );
    static bool TryGetPop3FetchResult( const CORE::CFutureResult& future, CPop3FetchResult& result );
    static bool TryGetImapFetchResult( const CORE::CFutureResult& future, CImapFetchResult& result );

    private:

    CMailOperations( void );

    static CORE::CFutureResult Queue( CMailOperationTaskData& data, const CORE::CString& threadPoolName );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILOPERATIONTASK_H ? */

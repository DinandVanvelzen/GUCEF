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

#ifndef GUCEF_COM_CEMAILOPERATIONS_H
#define GUCEF_COM_CEMAILOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#include "gucefCOM_CEMailOperationTaskData.h"
#define GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#endif /* GUCEF_COM_CEMAILOPERATIONTASKDATA_H ? */

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

/**
 *  The asynchronous forms of the one shot mail operations: a task on a thread pool and a CFutureResult for it. The
 *  settings are copied, so the credentials come from the settings (the user name and password or token): a
 *  credential provider belongs to the thread it lives on and is not used from a pool thread.
 *
 *      CORE::CFutureResult future = CEMailOperations::SendMessageAsync( settings, message );
 *      future.Await( 60000 );
 *      CSmtpSendResult result;
 *      CEMailOperations::TryGetSendResult( future, result );
 *
 *  A future that has no future (HasNoFuture()) was refused: no pool, no task type.
 */
class GUCEF_COM_PUBLIC_CPP CEMailOperations
{
    public:

    static CORE::CFutureResult SendMessageAsync( const CSmtpClientSettings& settings, const CEMailMessage& message, const CORE::CString& threadPoolName = CORE::CString::Empty );

    static CORE::CFutureResult FetchNewPop3Async( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, const CORE::CString& threadPoolName = CORE::CString::Empty );

    static CORE::CFutureResult FetchNewImapAsync( const CImapClientSettings& settings, const CImapFetchOptions& options, const CORE::CString& threadPoolName = CORE::CString::Empty );

    /** The result of a finished task. False while it is not finished, or when the future is not one of this operation */
    static bool TryGetSendResult( const CORE::CFutureResult& future, CSmtpSendResult& result );
    static bool TryGetPop3FetchResult( const CORE::CFutureResult& future, CPop3FetchResult& result );
    static bool TryGetImapFetchResult( const CORE::CFutureResult& future, CImapFetchResult& result );

    private:

    CEMailOperations( void );

    static CORE::CFutureResult Queue( CEMailOperationTaskData& data, const CORE::CString& threadPoolName );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILOPERATIONS_H ? */

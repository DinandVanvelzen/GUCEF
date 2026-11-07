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

#ifndef GUCEF_COM_CEMAILOPERATIONTASKDATA_H
#define GUCEF_COM_CEMAILOPERATIONTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

/** What an asynchronous mail operation is asked to do, and what came of it */
class GUCEF_COM_PUBLIC_CPP CEMailOperationTaskData : public CORE::CICloneable
{
    public:

    EMailOperation operation;

    CSmtpClientSettings smtpSettings;
    CEMailMessage message;
    CSmtpSendResult smtpResult;

    CPop3ClientSettings pop3Settings;
    CPop3FetchOptions pop3Options;
    CPop3FetchResult pop3Result;

    CImapClientSettings imapSettings;
    CImapFetchOptions imapOptions;
    CImapFetchResult imapResult;

    CEMailOperationTaskData( void );
    CEMailOperationTaskData( const CEMailOperationTaskData& src );
    virtual ~CEMailOperationTaskData() GUCEF_VIRTUAL_OVERRIDE;
    CEMailOperationTaskData& operator=( const CEMailOperationTaskData& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CEMAILOPERATIONTASKDATA_H ? */

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

#ifndef GUCEF_COM_CSMTPSENDRESULT_H
#define GUCEF_COM_CSMTPSENDRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILCLIENTBASE_H
#include "gucefCOM_CEMailClientBase.h"
#define GUCEF_COM_CEMAILCLIENTBASE_H
#endif /* GUCEF_COM_CEMAILCLIENTBASE_H ? */

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

/** What the server said about one recipient */
class GUCEF_COM_PUBLIC_CPP CSmtpRecipientResult
{
    public:

    CORE::CString address;
    bool accepted;
    UInt32 replyCode;
    CORE::CString enhancedCode;
    CORE::CString replyText;
    EMailErrorClass errorClass;

    CSmtpRecipientResult( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The outcome of sending one message. 'success' means the server accepted the message for the recipients that it
 *  accepted (see 'recipients'); the other fields say what failed otherwise, 'errorClass' tells a failure
 *  that is worth another try later (MAILERR_TEMPORARY, MAILERR_CONNECTION, MAILERR_TIMEOUT) from one that is not.
 */
class GUCEF_COM_PUBLIC_CPP CSmtpSendResult : public CORE::CICloneable
{
    public:

    UInt32 requestId;
    bool success;
    EMailErrorClass errorClass;
    UInt32 replyCode;                           /**< the reply that decided: to the end of the data when it was accepted */
    CORE::CString enhancedCode;
    CORE::CString replyText;                    /**< what the server said, for example 'OK queued as 4F2A1'. Not for logs without redaction */
    CORE::CString errorDescription;             /**< a phrase of ours, never has addresses or content */
    GUCEF::vector< CSmtpRecipientResult > recipients;

    CSmtpSendResult( void );
    CSmtpSendResult( const CSmtpSendResult& src );
    virtual ~CSmtpSendResult() GUCEF_VIRTUAL_OVERRIDE;
    CSmtpSendResult& operator=( const CSmtpSendResult& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );

    UInt32 GetAcceptedCount( void ) const;
    UInt32 GetRejectedCount( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSMTPSENDRESULT_H ? */

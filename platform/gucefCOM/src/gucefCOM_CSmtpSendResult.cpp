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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#include "gucefCOM_CSmtpSendResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSmtpRecipientResult::CSmtpRecipientResult( void )
    : address()
    , accepted( false )
    , replyCode( 0 )
    , enhancedCode()
    , replyText()
    , errorClass( MAILERR_NONE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpSendResult::CSmtpSendResult( void )
    : CORE::CICloneable()
    , requestId( 0 )
    , success( false )
    , errorClass( MAILERR_NONE )
    , replyCode( 0 )
    , enhancedCode()
    , replyText()
    , errorDescription()
    , recipients()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpSendResult::CSmtpSendResult( const CSmtpSendResult& src )
    : CORE::CICloneable( src )
    , requestId( src.requestId )
    , success( src.success )
    , errorClass( src.errorClass )
    , replyCode( src.replyCode )
    , enhancedCode( src.enhancedCode )
    , replyText( src.replyText )
    , errorDescription( src.errorDescription )
    , recipients( src.recipients )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpSendResult::~CSmtpSendResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpSendResult&
CSmtpSendResult::operator=( const CSmtpSendResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        requestId = src.requestId;
        success = src.success;
        errorClass = src.errorClass;
        replyCode = src.replyCode;
        enhancedCode = src.enhancedCode;
        replyText = src.replyText;
        errorDescription = src.errorDescription;
        recipients = src.recipients;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CSmtpSendResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CSmtpSendResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CSmtpSendResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CSmtpSendResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CSmtpSendResult";
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpSendResult::GetAcceptedCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<recipients.size(); ++i )
        if ( recipients[ i ].accepted )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpSendResult::GetRejectedCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<recipients.size(); ++i )
        if ( !recipients[ i ].accepted )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

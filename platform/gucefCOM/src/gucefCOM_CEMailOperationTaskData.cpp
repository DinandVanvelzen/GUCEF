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

#include "gucefCOM_CEMailOperationTaskData.h"

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

CEMailOperationTaskData::CEMailOperationTaskData( void )
    : CORE::CICloneable()
    , operation( MAILOPERATION_NONE )
    , smtpSettings()
    , message()
    , smtpResult()
    , pop3Settings()
    , pop3Options()
    , pop3Result()
    , imapSettings()
    , imapOptions()
    , imapResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailOperationTaskData::CEMailOperationTaskData( const CEMailOperationTaskData& src )
    : CORE::CICloneable( src )
    , operation( src.operation )
    , smtpSettings( src.smtpSettings )
    , message( src.message )
    , smtpResult( src.smtpResult )
    , pop3Settings( src.pop3Settings )
    , pop3Options( src.pop3Options )
    , pop3Result( src.pop3Result )
    , imapSettings( src.imapSettings )
    , imapOptions( src.imapOptions )
    , imapResult( src.imapResult )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailOperationTaskData::~CEMailOperationTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailOperationTaskData&
CEMailOperationTaskData::operator=( const CEMailOperationTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        operation = src.operation;
        smtpSettings = src.smtpSettings;
        message = src.message;
        smtpResult = src.smtpResult;
        pop3Settings = src.pop3Settings;
        pop3Options = src.pop3Options;
        pop3Result = src.pop3Result;
        imapSettings = src.imapSettings;
        imapOptions = src.imapOptions;
        imapResult = src.imapResult;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* CEMailOperationTaskData::Clone( void ) const { return GUCEF_NEW CEMailOperationTaskData( *this ); }
const char* CEMailOperationTaskData::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CEMailOperationTaskData::CloneableTypeName( void ) { return "GUCEF::COM::CEMailOperationTaskData"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

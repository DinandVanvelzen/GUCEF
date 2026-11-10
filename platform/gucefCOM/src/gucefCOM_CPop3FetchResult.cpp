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


#include "gucefCOM_CPop3FetchResult.h"

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

CPop3FetchedMessage::CPop3FetchedMessage( void )
    : uid()
    , number( 0 )
    , message()
    , parsed( false )
    , deleted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::CPop3FetchResult( void )
    : CORE::CICloneable()
    , success( false )
    , errorClass( MAILERR_NONE )
    , errorDescription()
    , messageCountOnServer( 0 )
    , allUids()
    , messages()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::CPop3FetchResult( const CPop3FetchResult& src )
    : CORE::CICloneable( src )
    , success( src.success )
    , errorClass( src.errorClass )
    , errorDescription( src.errorDescription )
    , messageCountOnServer( src.messageCountOnServer )
    , allUids( src.allUids )
    , messages( src.messages )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::~CPop3FetchResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult&
CPop3FetchResult::operator=( const CPop3FetchResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        success = src.success;
        errorClass = src.errorClass;
        errorDescription = src.errorDescription;
        messageCountOnServer = src.messageCountOnServer;
        allUids = src.allUids;
        messages = src.messages;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPop3FetchResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPop3FetchResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CPop3FetchResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CPop3FetchResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CPop3FetchResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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


#include "gucefCOM_CImapFetchResult.h"

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

CImapFetchedMessage::CImapFetchedMessage( void )
    : uid( 0 )
    , size( 0 )
    , flags()
    , message()
    , parsed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapFetchResult::CImapFetchResult( void )
    : CORE::CICloneable()
    , success( false )
    , errorClass( MAILERR_NONE )
    , errorDescription()
    , uidValidity( 0 )
    , uidValidityChanged( false )
    , uidNext( 0 )
    , highestUid( 0 )
    , messages()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapFetchResult::CImapFetchResult( const CImapFetchResult& src )
    : CORE::CICloneable( src )
    , success( src.success )
    , errorClass( src.errorClass )
    , errorDescription( src.errorDescription )
    , uidValidity( src.uidValidity )
    , uidValidityChanged( src.uidValidityChanged )
    , uidNext( src.uidNext )
    , highestUid( src.highestUid )
    , messages( src.messages )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapFetchResult::~CImapFetchResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapFetchResult&
CImapFetchResult::operator=( const CImapFetchResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        success = src.success;
        errorClass = src.errorClass;
        errorDescription = src.errorDescription;
        uidValidity = src.uidValidity;
        uidValidityChanged = src.uidValidityChanged;
        uidNext = src.uidNext;
        highestUid = src.highestUid;
        messages = src.messages;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* CImapFetchResult::Clone( void ) const { return GUCEF_NEW CImapFetchResult( *this ); }
const char* CImapFetchResult::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CImapFetchResult::CloneableTypeName( void ) { return "GUCEF::COM::CImapFetchResult"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

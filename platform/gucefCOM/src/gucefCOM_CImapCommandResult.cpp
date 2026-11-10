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


#include "gucefCOM_CImapCommandResult.h"

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

CImapMailboxEntry::CImapMailboxEntry( void )
    : name()
    , hierarchyDelimiter()
    , attributes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapMailboxStatus::CImapMailboxStatus( void )
    : name()
    , exists( 0 )
    , recent( 0 )
    , unseen( 0 )
    , uidNext( 0 )
    , uidValidity( 0 )
    , readOnly( false )
    , flags()
    , permanentFlags()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapCommandResult::CImapCommandResult( void )
    : CORE::CICloneable()
    , requestId( 0 )
    , command( IMAPCMD_NOOP )
    , ok( false )
    , status()
    , errorClass( MAILERR_NONE )
    , text()
    , code()
    , mailboxes()
    , mailbox()
    , uidValidityChanged( false )
    , numbers()
    , fetched()
    , appendUid( 0 )
    , copyUid( 0 )
    , capabilities()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapCommandResult::CImapCommandResult( const CImapCommandResult& src )
    : CORE::CICloneable( src )
    , requestId( src.requestId )
    , command( src.command )
    , ok( src.ok )
    , status( src.status )
    , errorClass( src.errorClass )
    , text( src.text )
    , code( src.code )
    , mailboxes( src.mailboxes )
    , mailbox( src.mailbox )
    , uidValidityChanged( src.uidValidityChanged )
    , numbers( src.numbers )
    , fetched( src.fetched )
    , appendUid( src.appendUid )
    , copyUid( src.copyUid )
    , capabilities( src.capabilities )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapCommandResult::~CImapCommandResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapCommandResult&
CImapCommandResult::operator=( const CImapCommandResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        requestId = src.requestId;
        command = src.command;
        ok = src.ok;
        status = src.status;
        errorClass = src.errorClass;
        text = src.text;
        code = src.code;
        mailboxes = src.mailboxes;
        mailbox = src.mailbox;
        uidValidityChanged = src.uidValidityChanged;
        numbers = src.numbers;
        fetched = src.fetched;
        appendUid = src.appendUid;
        copyUid = src.copyUid;
        capabilities = src.capabilities;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* CImapCommandResult::Clone( void ) const { return GUCEF_NEW CImapCommandResult( *this ); }
const char* CImapCommandResult::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CImapCommandResult::CloneableTypeName( void ) { return "GUCEF::COM::CImapCommandResult"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

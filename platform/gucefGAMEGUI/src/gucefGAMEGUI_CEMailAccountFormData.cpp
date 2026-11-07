/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#include "gucefGAMEGUI_CEMailAccountFormData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailAccountFormData::CEMailAccountFormData( void )
    : CORE::CICloneable()
    , address()
    , displayName()
    , userName()
    , password()
    , useImap( true )
    , incomingHost()
    , incomingPort( 0 )
    , incomingSecurity( 0 )
    , outgoingHost()
    , outgoingPort( 0 )
    , outgoingSecurity( 1 )
    , mailbox( "INBOX" )
    , pollSeconds( 60 )
    , useIdle( true )
    , processedAction( 1 )
    , processedFolder()
    , pop3Delete( false )
    , autoAccept( true )
    , providerId()
    , authMode( 0 )
    , oauthProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAccountFormData::CEMailAccountFormData( const CEMailAccountFormData& src )
    : CORE::CICloneable( src )
    , address( src.address )
    , displayName( src.displayName )
    , userName( src.userName )
    , password( src.password )
    , useImap( src.useImap )
    , incomingHost( src.incomingHost )
    , incomingPort( src.incomingPort )
    , incomingSecurity( src.incomingSecurity )
    , outgoingHost( src.outgoingHost )
    , outgoingPort( src.outgoingPort )
    , outgoingSecurity( src.outgoingSecurity )
    , mailbox( src.mailbox )
    , pollSeconds( src.pollSeconds )
    , useIdle( src.useIdle )
    , processedAction( src.processedAction )
    , processedFolder( src.processedFolder )
    , pop3Delete( src.pop3Delete )
    , autoAccept( src.autoAccept )
    , providerId( src.providerId )
    , authMode( src.authMode )
    , oauthProvider( src.oauthProvider )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAccountFormData::~CEMailAccountFormData()
{GUCEF_TRACE;

    CORE::SecureWipeString( password );
}

/*-------------------------------------------------------------------------*/

CEMailAccountFormData&
CEMailAccountFormData::operator=( const CEMailAccountFormData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        address = src.address;
        displayName = src.displayName;
        userName = src.userName;
        CORE::SecureWipeString( password );
        password = src.password;
        useImap = src.useImap;
        incomingHost = src.incomingHost;
        incomingPort = src.incomingPort;
        incomingSecurity = src.incomingSecurity;
        outgoingHost = src.outgoingHost;
        outgoingPort = src.outgoingPort;
        outgoingSecurity = src.outgoingSecurity;
        mailbox = src.mailbox;
        pollSeconds = src.pollSeconds;
        useIdle = src.useIdle;
        processedAction = src.processedAction;
        processedFolder = src.processedFolder;
        pop3Delete = src.pop3Delete;
        autoAccept = src.autoAccept;
        providerId = src.providerId;
        authMode = src.authMode;
        oauthProvider = src.oauthProvider;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailAccountFormData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailAccountFormData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailAccountFormData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailAccountFormData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CEMailAccountFormData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

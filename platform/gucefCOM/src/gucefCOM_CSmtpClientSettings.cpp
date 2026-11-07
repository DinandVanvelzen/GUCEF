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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOM_CSmtpClientSettings.h"

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

CSmtpClientSettings::CSmtpClientSettings( void )
    : CMailClientSettings()
    , heloName()
    , usePipelining( true )
    , failIfAnyRecipientRejected( false )
    , allowSmtpUtf8( true )
    , dataTimeoutInMs( GUCEF_COM_MAIL_DEFAULT_DATA_TIMEOUT_MS )
    , maxRecipients( 100 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpClientSettings::CSmtpClientSettings( const CSmtpClientSettings& src )
    : CMailClientSettings( src )
    , heloName( src.heloName )
    , usePipelining( src.usePipelining )
    , failIfAnyRecipientRejected( src.failIfAnyRecipientRejected )
    , allowSmtpUtf8( src.allowSmtpUtf8 )
    , dataTimeoutInMs( src.dataTimeoutInMs )
    , maxRecipients( src.maxRecipients )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpClientSettings::~CSmtpClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSmtpClientSettings&
CSmtpClientSettings::operator=( const CSmtpClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CMailClientSettings::operator=( src );
        heloName = src.heloName;
        usePipelining = src.usePipelining;
        failIfAnyRecipientRejected = src.failIfAnyRecipientRejected;
        allowSmtpUtf8 = src.allowSmtpUtf8;
        dataTimeoutInMs = src.dataTimeoutInMs;
        maxRecipients = src.maxRecipients;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt16
CSmtpClientSettings::GetDefaultPort( EMailTlsMode mode ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case MAILTLS_IMPLICIT: return 465;
        case MAILTLS_NONE: return 25;
        default: return 587;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClientSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !CMailClientSettings::SaveConfig( config ) )
        return false;
    config.SetAttribute( "heloName", heloName );
    config.SetAttribute( "usePipelining", usePipelining );
    config.SetAttribute( "failIfAnyRecipientRejected", failIfAnyRecipientRejected );
    config.SetAttribute( "allowSmtpUtf8", allowSmtpUtf8 );
    config.SetAttribute( "dataTimeoutInMs", dataTimeoutInMs );
    config.SetAttribute( "maxRecipients", maxRecipients );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClientSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !CMailClientSettings::LoadConfig( config ) )
        return false;
    heloName = config.GetAttributeValueOrChildValueByName( "heloName" ).AsString( heloName, true );
    usePipelining = config.GetAttributeValueOrChildValueByName( "usePipelining" ).AsBool( usePipelining, true );
    failIfAnyRecipientRejected = config.GetAttributeValueOrChildValueByName( "failIfAnyRecipientRejected" ).AsBool( failIfAnyRecipientRejected, true );
    allowSmtpUtf8 = config.GetAttributeValueOrChildValueByName( "allowSmtpUtf8" ).AsBool( allowSmtpUtf8, true );
    dataTimeoutInMs = config.GetAttributeValueOrChildValueByName( "dataTimeoutInMs" ).AsUInt32( dataTimeoutInMs, true );
    maxRecipients = config.GetAttributeValueOrChildValueByName( "maxRecipients" ).AsUInt32( maxRecipients, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSmtpClientSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CSmtpClientSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

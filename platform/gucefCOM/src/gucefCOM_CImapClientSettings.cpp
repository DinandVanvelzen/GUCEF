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

#include "gucefCOM_CImapClientSettings.h"

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

CImapClientSettings::CImapClientSettings( void )
    : CMailClientSettings()
    , idleRenewInMs( 25 * 60 * 1000 )
    , maxFetchResults( 10000 )
    , useLiteralPlus( false )
{GUCEF_TRACE;

    maxLineLength = 65536;

}

/*-------------------------------------------------------------------------*/

CImapClientSettings::CImapClientSettings( const CImapClientSettings& src )
    : CMailClientSettings( src )
    , idleRenewInMs( src.idleRenewInMs )
    , maxFetchResults( src.maxFetchResults )
    , useLiteralPlus( src.useLiteralPlus )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapClientSettings::~CImapClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapClientSettings&
CImapClientSettings::operator=( const CImapClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CMailClientSettings::operator=( src );
        idleRenewInMs = src.idleRenewInMs;
        maxFetchResults = src.maxFetchResults;
        useLiteralPlus = src.useLiteralPlus;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt16
CImapClientSettings::GetDefaultPort( EMailTlsMode mode ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case MAILTLS_IMPLICIT: return 993;
        case MAILTLS_NONE: return 143;
        default: return 143;
    }
}

/*-------------------------------------------------------------------------*/

bool
CImapClientSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !CMailClientSettings::SaveConfig( config ) )
        return false;
    config.SetAttribute( "idleRenewInMs", idleRenewInMs );
    config.SetAttribute( "maxFetchResults", maxFetchResults );
    config.SetAttribute( "useLiteralPlus", useLiteralPlus );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapClientSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !CMailClientSettings::LoadConfig( config ) )
        return false;
    idleRenewInMs = config.GetAttributeValueOrChildValueByName( "idleRenewInMs" ).AsUInt32( idleRenewInMs, true );
    maxFetchResults = config.GetAttributeValueOrChildValueByName( "maxFetchResults" ).AsUInt32( maxFetchResults, true );
    useLiteralPlus = config.GetAttributeValueOrChildValueByName( "useLiteralPlus" ).AsBool( useLiteralPlus, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImapClientSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CImapClientSettings";
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

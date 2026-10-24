/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CPlayerReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPlayerReport::CPlayerReport( void )
    : CORE::CIDataNodeSerializable()
    , reporterId()
    , reporterName()
    , reportedId()
    , reportedName()
    , reportedPublicKey()
    , reason()
    , lobbyId()
    , sessionName()
    , time()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerReport::CPlayerReport( const CPlayerReport& src )
    : CORE::CIDataNodeSerializable( src )
    , reporterId( src.reporterId )
    , reporterName( src.reporterName )
    , reportedId( src.reportedId )
    , reportedName( src.reportedName )
    , reportedPublicKey( src.reportedPublicKey )
    , reason( src.reason )
    , lobbyId( src.lobbyId )
    , sessionName( src.sessionName )
    , time( src.time )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerReport::~CPlayerReport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerReport&
CPlayerReport::operator=( const CPlayerReport& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        reporterId = src.reporterId;
        reporterName = src.reporterName;
        reportedId = src.reportedId;
        reportedName = src.reportedName;
        reportedPublicKey = src.reportedPublicKey;
        reason = src.reason;
        lobbyId = src.lobbyId;
        sessionName = src.sessionName;
        time = src.time;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerReport::Serialize( CORE::CDataNode& domRootNode                        ,
                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "reporterId", reporterId );
    domRootNode.SetAttribute( "reporterName", reporterName );
    domRootNode.SetAttribute( "reportedId", reportedId );
    domRootNode.SetAttribute( "reportedName", reportedName );
    domRootNode.SetAttribute( "reportedPublicKey", reportedPublicKey );
    domRootNode.SetAttribute( "reason", reason );
    domRootNode.SetAttribute( "lobbyId", lobbyId );
    domRootNode.SetAttribute( "sessionName", sessionName );
    domRootNode.SetAttribute( "time", time.ToIso8601DateTimeString( true, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerReport::Deserialize( const CORE::CDataNode& domRootNode                  ,
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    reporterId = domRootNode.GetAttributeValueOrChildValueByName( "reporterId" ).AsString( CORE::CString::Empty, true );
    reporterName = domRootNode.GetAttributeValueOrChildValueByName( "reporterName" ).AsString( CORE::CString::Empty, true );
    reportedId = domRootNode.GetAttributeValueOrChildValueByName( "reportedId" ).AsString( CORE::CString::Empty, true );
    reportedName = domRootNode.GetAttributeValueOrChildValueByName( "reportedName" ).AsString( CORE::CString::Empty, true );
    reportedPublicKey = domRootNode.GetAttributeValueOrChildValueByName( "reportedPublicKey" ).AsString( CORE::CString::Empty, true );
    reason = domRootNode.GetAttributeValueOrChildValueByName( "reason" ).AsString( CORE::CString::Empty, true );
    lobbyId = domRootNode.GetAttributeValueOrChildValueByName( "lobbyId" ).AsString( CORE::CString::Empty, true );
    sessionName = domRootNode.GetAttributeValueOrChildValueByName( "sessionName" ).AsString( CORE::CString::Empty, true );
    time.FromIso8601DateTimeString( domRootNode.GetAttributeValueOrChildValueByName( "time" ).AsString( CORE::CString::Empty, true ) );
    return !reportedId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPlayerReport::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPlayerReport( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPlayerReport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CPlayerReport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

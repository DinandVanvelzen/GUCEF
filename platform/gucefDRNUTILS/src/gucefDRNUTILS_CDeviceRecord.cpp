/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDeviceRecord.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether the text is 64 hexadecimal characters */
bool
IsHexKey( const CORE::CString& text )
{GUCEF_TRACE;

    if ( 64 != text.Length() )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<64; ++i )
    {
        char ch = c[ i ];
        bool hex = ( ch >= '0' && ch <= '9' ) || ( ch >= 'a' && ch <= 'f' ) || ( ch >= 'A' && ch <= 'F' );
        if ( !hex )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
SetText( CORE::CDataNode& node, const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    node.SetAttribute( name, value );
}

}

/*-------------------------------------------------------------------------*/

CDeviceRecord::CDeviceRecord( void )
    : deviceId()
    , userId()
    , name()
    , kind()
    , platform()
    , appVersion()
    , roles()
    , status( DEVICESTATUS_PENDING_CONFIRMATION )
    , publicKey()
    , fingerprint()
    , pairedUtcMs( 0 )
    , pairedBy()
    , tokenExpiresUtcMs( 0 )
    , lastSeenUtcMs( 0 )
    , lastAddress()
    , revokedUtcMs( 0 )
    , revokedBy()
    , revokedReason()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRecord::IsActive( void ) const
{GUCEF_TRACE;

    return DEVICESTATUS_ACTIVE == status;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRecord::HasRole( const CORE::CString& role ) const
{GUCEF_TRACE;

    return roles.find( role ) != roles.end();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRecord::HasAnyRole( const CORE::CString::StringSet& wanted ) const
{GUCEF_TRACE;

    CORE::CString::StringSet::const_iterator i = wanted.begin();
    for ( ; i != wanted.end(); ++i )
    {
        if ( roles.find( *i ) != roles.end() )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDeviceRecord::GetRolesText( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    CORE::CString::StringSet::const_iterator i = roles.begin();
    for ( ; i != roles.end(); ++i )
    {
        if ( !text.IsNULLOrEmpty() )
            text += ',';
        text += *i;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CDeviceRecord::ToDataNode( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.Clear();
    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( node.GetName().IsNULLOrEmpty() )
        node.SetName( deviceId );
    SetText( node, "deviceId", deviceId );
    SetText( node, "userId", userId );
    SetText( node, "name", name );
    SetText( node, "kind", kind );
    SetText( node, "platform", platform );
    SetText( node, "appVersion", appVersion );
    CORE::CDataNode* roleList = node.AddChild( "roles", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator i = roles.begin();
    for ( ; i != roles.end(); ++i )
        roleList->AddValueAsChild( *i );
    SetText( node, "status", DeviceStatusToString( status ) );
    SetText( node, "publicKey", publicKey );
    SetText( node, "fingerprint", fingerprint );
    node.SetAttribute( "pairedUtcMs", pairedUtcMs );
    SetText( node, "pairedBy", pairedBy );
    node.SetAttribute( "tokenExpiresUtcMs", tokenExpiresUtcMs );
    node.SetAttribute( "lastSeenUtcMs", lastSeenUtcMs );
    SetText( node, "lastAddress", lastAddress );
    node.SetAttribute( "revokedUtcMs", revokedUtcMs );
    SetText( node, "revokedBy", revokedBy );
    SetText( node, "revokedReason", revokedReason );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRecord::FromDataNode( const CORE::CDataNode& node, CORE::CString& error )
{GUCEF_TRACE;

    CDeviceRecord record;
    record.deviceId = node.GetAttributeValueOrChildValueByName( "deviceId" ).AsString();
    if ( record.deviceId.IsNULLOrEmpty() )
    {
        error = "a device has no deviceId";
        return false;
    }
    CORE::CString statusText = node.GetAttributeValueOrChildValueByName( "status" ).AsString();
    if ( !DeviceStatusFromString( statusText, record.status ) )
    {
        error = "the device " + record.deviceId + " has no valid status (\"" + statusText + "\")";
        return false;
    }
    record.publicKey = node.GetAttributeValueOrChildValueByName( "publicKey" ).AsString().Lowercase();
    if ( !IsHexKey( record.publicKey ) )
    {
        error = "the device " + record.deviceId + " has no valid public key";
        return false;
    }
    record.userId = node.GetAttributeValueOrChildValueByName( "userId" ).AsString();
    record.name = node.GetAttributeValueOrChildValueByName( "name" ).AsString();
    record.kind = node.GetAttributeValueOrChildValueByName( "kind" ).AsString();
    record.platform = node.GetAttributeValueOrChildValueByName( "platform" ).AsString();
    record.appVersion = node.GetAttributeValueOrChildValueByName( "appVersion" ).AsString();
    const CORE::CDataNode* roleList = node.FindChild( "roles" );
    if ( GUCEF_NULL != roleList )
    {
        CORE::CDataNode::const_iterator r = roleList->ConstBegin();
        for ( ; r != roleList->ConstEnd(); ++r )
        {
            CORE::CString role = (*r)->GetValue().AsString().Trim( true ).Trim( false );
            if ( !role.IsNULLOrEmpty() )
                record.roles.insert( role );
        }
    }
    record.fingerprint = node.GetAttributeValueOrChildValueByName( "fingerprint" ).AsString();
    record.pairedUtcMs = node.GetAttributeValueOrChildValueByName( "pairedUtcMs" ).AsUInt64( 0 );
    record.pairedBy = node.GetAttributeValueOrChildValueByName( "pairedBy" ).AsString();
    record.tokenExpiresUtcMs = node.GetAttributeValueOrChildValueByName( "tokenExpiresUtcMs" ).AsUInt64( 0 );
    record.lastSeenUtcMs = node.GetAttributeValueOrChildValueByName( "lastSeenUtcMs" ).AsUInt64( 0 );
    record.lastAddress = node.GetAttributeValueOrChildValueByName( "lastAddress" ).AsString();
    record.revokedUtcMs = node.GetAttributeValueOrChildValueByName( "revokedUtcMs" ).AsUInt64( 0 );
    record.revokedBy = node.GetAttributeValueOrChildValueByName( "revokedBy" ).AsString();
    record.revokedReason = node.GetAttributeValueOrChildValueByName( "revokedReason" ).AsString();
    *this = record;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

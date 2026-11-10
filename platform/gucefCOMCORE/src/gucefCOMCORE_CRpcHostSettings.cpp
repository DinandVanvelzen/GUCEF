/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcHostSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcHostSettings::SListener::SListener( void )
    : uri()
    , config()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcHostSettings::CRpcHostSettings( void )
    : CORE::CIConfigurable()
    , listeners()
    , endpointSettings()
    , maxSessions( 0 )
    , maxSessionsPerSubject( 0 )
    , helloTimeoutMs( 5000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRpcHostSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "maxSessions", maxSessions );
    config.SetAttribute( "maxSessionsPerSubject", maxSessionsPerSubject );
    config.SetAttribute( "helloTimeoutMs", helloTimeoutMs );
    CORE::CDataNode* list = config.AddChild( "listeners", GUCEF_DATATYPE_ARRAY );
    for ( UInt32 i=0; i<listeners.size(); ++i )
    {
        CORE::CDataNode* listener = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        listener->SetAttribute( "uri", listeners[ i ].uri.ToUriInStringForm() );
        CORE::CDataNode* listenerConfig = listener->AddChild( "config", GUCEF_DATATYPE_OBJECT );
        listenerConfig->Copy( listeners[ i ].config );
        listenerConfig->SetName( "config" );
    }
    CORE::CDataNode* endpoint = config.AddChild( "endpoint", GUCEF_DATATYPE_OBJECT );
    return endpointSettings.SaveConfig( *endpoint );
}

/*-------------------------------------------------------------------------*/

bool
CRpcHostSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    maxSessions = config.GetAttributeValueOrChildValueByName( "maxSessions" ).AsUInt32( maxSessions );
    maxSessionsPerSubject = config.GetAttributeValueOrChildValueByName( "maxSessionsPerSubject" ).AsUInt32( maxSessionsPerSubject );
    helloTimeoutMs = config.GetAttributeValueOrChildValueByName( "helloTimeoutMs" ).AsUInt32( helloTimeoutMs );
    const CORE::CDataNode* list = config.FindChild( "listeners" );
    if ( GUCEF_NULL != list )
    {
        listeners.clear();
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        for ( ; i != list->ConstEnd(); ++i )
        {
            SListener listener;
            listener.uri = CORE::CUri( (*i)->GetAttributeValue( "uri" ).AsString() );
            const CORE::CDataNode* listenerConfig = (*i)->FindChild( "config" );
            if ( GUCEF_NULL != listenerConfig )
                listener.config.Copy( *listenerConfig );
            listeners.push_back( listener );
        }
    }
    const CORE::CDataNode* endpoint = config.FindChild( "endpoint" );
    if ( GUCEF_NULL != endpoint )
        return endpointSettings.LoadConfig( *endpoint );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcHostSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcHostSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

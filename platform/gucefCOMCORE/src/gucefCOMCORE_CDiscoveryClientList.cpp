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

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOMCORE_CDiscoveryClientList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CDiscoveryClientList::ClientTypesKey = "discoveryClients";
const char* CDiscoveryClientList::ClientNodeName = "DiscoveryClient";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDiscoveryClientList::GetDefaultServiceClientTypes( CORE::CString::StringVector& types )
{GUCEF_TRACE;

    types.push_back( "broadcast" );
    types.push_back( "multicast" );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientList::NamesClients( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !config.GetAttributeValueOrChildValueByName( ClientTypesKey ).AsString( CORE::CString::Empty, true ).IsNULLOrEmpty() )
        return true;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        if ( ClientNodeName == (*i)->GetName() )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientList::ParseTypes( const CORE::CString& typeList, CORE::CString::StringVector& types )
{GUCEF_TRACE;

    CORE::CString::StringVector parts = typeList.ParseElements( ',', false );
    for ( size_t n=0; n<parts.size(); ++n )
    {
        CORE::CString type = parts[ n ].Trim( true ).Trim( false );
        if ( !type.IsNULLOrEmpty() )
            types.push_back( type );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDiscoveryClientList::JoinTypes( const CORE::CString::StringVector& types )
{GUCEF_TRACE;

    CORE::CString list;
    for ( size_t i=0; i<types.size(); ++i )
    {
        if ( i > 0 )
            list += ',';
        list += types[ i ];
    }
    return list;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientList::CreateFromTypes( const CORE::CString::StringVector& types ,
                                       TDiscoveryClientVector& clients          )
{GUCEF_TRACE;

    CDiscoveryClientFactory& factory = CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
    bool allMade = true;
    for ( size_t i=0; i<types.size(); ++i )
    {
        CIDiscoveryClientPtr client = factory.Create( types[ i ] );
        if ( client.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: There is no discovery client of type \"" << types[ i ] << "\"" );
            allMade = false;
            continue;
        }
        clients.push_back( client );
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientList::CreateFromConfig( const CORE::CDataNode& config                 ,
                                        const CORE::CString::StringVector& defaultTypes ,
                                        TDiscoveryClientVector& clients               )
{GUCEF_TRACE;

    CDiscoveryClientFactory& factory = CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
    bool allMade = true;
    bool anyChild = false;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        const CORE::CDataNode& child = *(*i);
        ++i;
        if ( ClientNodeName != child.GetName() )
            continue;
        anyChild = true;

        CORE::CString type = child.GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
        CIDiscoveryClientPtr client = factory.Create( type );
        if ( client.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: There is no discovery client of type \"" << type << "\"" );
            allMade = false;
            continue;
        }
        if ( !client->LoadConfig( child ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: The config of the discovery client of type \"" << type << "\" has problems" );
            allMade = false;
        }
        clients.push_back( client );
    }
    if ( anyChild )
        return allMade;

    CORE::CString::StringVector types;
    CORE::CString typeList = config.GetAttributeValueOrChildValueByName( ClientTypesKey ).AsString( CORE::CString::Empty, true );
    if ( typeList.IsNULLOrEmpty() )
        types = defaultTypes;
    else
        ParseTypes( typeList, types );
    return CreateFromTypes( types, clients );
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientList::CopyClientConfig( const CORE::CDataNode& from, CORE::CDataNode& to )
{GUCEF_TRACE;

    CORE::CString typeList = from.GetAttributeValueOrChildValueByName( ClientTypesKey ).AsString( CORE::CString::Empty, true );
    if ( !typeList.IsNULLOrEmpty() )
        to.SetAttribute( ClientTypesKey, typeList );
    CORE::CDataNode::const_iterator i = from.ConstBegin();
    while ( i != from.ConstEnd() )
    {
        if ( ClientNodeName == (*i)->GetName() )
            to.AddChild( *(*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefMULTIPLAY_CLobbyCreateParams.h"

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

CLobbyCreateParams::CLobbyCreateParams( void )
    : CORE::CIConfigurable()
    , name()
    , password()
    , maxPlayers( 8 )
    , allowSpectators( true )
    , isPublic( false )
    , gameId()
    , gameVersion()
    , schema()
    , initialSettings()
    , slots()
    , topology( GAMETOPOLOGY_CLIENT_SERVER )
    , securityPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyCreateParams::CLobbyCreateParams( const CLobbyCreateParams& src )
    : CORE::CIConfigurable( src )
    , name( src.name )
    , password( src.password )
    , maxPlayers( src.maxPlayers )
    , allowSpectators( src.allowSpectators )
    , isPublic( src.isPublic )
    , gameId( src.gameId )
    , gameVersion( src.gameVersion )
    , schema( src.schema )
    , initialSettings( src.initialSettings )
    , slots( src.slots )
    , topology( src.topology )
    , securityPolicy( src.securityPolicy )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyCreateParams::~CLobbyCreateParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLobbyCreateParams&
CLobbyCreateParams::operator=( const CLobbyCreateParams& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        name = src.name;
        password = src.password;
        maxPlayers = src.maxPlayers;
        allowSpectators = src.allowSpectators;
        isPublic = src.isPublic;
        gameId = src.gameId;
        gameVersion = src.gameVersion;
        schema = src.schema;
        initialSettings = src.initialSettings;
        slots = src.slots;
        topology = src.topology;
        securityPolicy = src.securityPolicy;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CLobbyCreateParams::GetStartSettings( CORE::CValueList& settings ) const
{GUCEF_TRACE;

    settings.DeleteAll();
    settings.SetAllowMultipleValues( false );
    for ( size_t i=0; i<schema.settings.size(); ++i )
        settings.Set( schema.settings[ i ].key, schema.settings[ i ].defaultValue );
    UInt32 keyCount = initialSettings.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        settings.Set( initialSettings.GetKey( i ).AsString(), initialSettings.GetValueAlways( initialSettings.GetKey( i ) ).AsString() );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyCreateParams::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "name", name );
    config.SetAttribute( "password", password );
    config.SetAttribute( "maxPlayers", maxPlayers );
    config.SetAttribute( "allowSpectators", allowSpectators );
    config.SetAttribute( "isPublic", isPublic );
    config.SetAttribute( "gameId", gameId );
    config.SetAttribute( "gameVersion", gameVersion );
    config.SetAttribute( "topology", CORE::CString( GameTopologyToString( topology ) ) );
    schema.SaveConfig( *config.AddChild( "schema", GUCEF_DATATYPE_OBJECT ) );
    securityPolicy.SaveConfig( *config.AddChild( "securityPolicy", GUCEF_DATATYPE_OBJECT ) );
    config.SetAttributesOfChildByName( "initialSettings", initialSettings );
    CORE::CDataNode* slotsNode = config.AddChild( "slots", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<slots.size(); ++i )
        slots[ i ].Serialize( *slotsNode->AddChild( "slot", GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyCreateParams::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    name = config.GetAttributeValueOrChildValueByName( "name" ).AsString( name, true );
    password = config.GetAttributeValueOrChildValueByName( "password" ).AsString( password, true );
    maxPlayers = config.GetAttributeValueOrChildValueByName( "maxPlayers" ).AsUInt32( maxPlayers, true );
    allowSpectators = config.GetAttributeValueOrChildValueByName( "allowSpectators" ).AsBool( allowSpectators, true );
    isPublic = config.GetAttributeValueOrChildValueByName( "isPublic" ).AsBool( isPublic, true );
    gameId = config.GetAttributeValueOrChildValueByName( "gameId" ).AsString( gameId, true );
    gameVersion = config.GetAttributeValueOrChildValueByName( "gameVersion" ).AsString( gameVersion, true );
    topology = StringToGameTopology( config.GetAttributeValueOrChildValueByName( "topology" ).AsString( CORE::CString( GameTopologyToString( topology ) ), true ) );
    const CORE::CDataNode* schemaNode = config.FindChild( "schema" );
    if ( GUCEF_NULL != schemaNode )
        schema.LoadConfig( *schemaNode );
    const CORE::CDataNode* policyNode = config.FindChild( "securityPolicy" );
    if ( GUCEF_NULL != policyNode && !securityPolicy.LoadConfig( *policyNode ) )
        return false;
    config.GetAttributesOfChildByName( "initialSettings", initialSettings );
    const CORE::CDataNode* slotsNode = config.FindChild( "slots" );
    if ( GUCEF_NULL != slotsNode )
    {
        slots.clear();
        CORE::CDataNode::const_iterator i = slotsNode->ConstBegin();
        while ( i != slotsNode->ConstEnd() )
        {
            slots.push_back( CLobbySlot() );
            slots.back().Deserialize( *(*i) );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyCreateParams::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLobbyCreateParams";
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

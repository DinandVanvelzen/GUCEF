/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include "pubsubpluginFIX_CFIXServerPubSubClientTopicConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig::CFIXServerPubSubClientTopicConfig( void )
    : PUBSUB::CPubSubClientTopicConfig()
    , CORE::CTSharedObjCreator< CFIXServerPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
    , remoteCompId()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig::CFIXServerPubSubClientTopicConfig( const CFIXServerPubSubClientTopicConfig& src )
    : PUBSUB::CPubSubClientTopicConfig( src )
    , CORE::CTSharedObjCreator< CFIXServerPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( src.includeSessionLevelMsgs )
    , msgTypeFilter( src.msgTypeFilter )
    , remoteCompId( src.remoteCompId )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig::CFIXServerPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig )
    : PUBSUB::CPubSubClientTopicConfig( genericConfig )
    , CORE::CTSharedObjCreator< CFIXServerPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
    , remoteCompId()
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig::~CFIXServerPubSubClientTopicConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig&
CFIXServerPubSubClientTopicConfig::operator=( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopicConfig&
CFIXServerPubSubClientTopicConfig::operator=( const CFIXServerPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        includeSessionLevelMsgs = src.includeSessionLevelMsgs;
        msgTypeFilter           = src.msgTypeFilter;
        remoteCompId            = src.remoteCompId;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopicConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    includeSessionLevelMsgs = config.GetAttributeValueOrChildValueByName( "includeSessionLevelMsgs" ).AsBool( includeSessionLevelMsgs, true );
    msgTypeFilter           = config.GetAttributeValueOrChildValueByName( "msgTypeFilter" ).AsAsciiString( msgTypeFilter, true );
    remoteCompId            = config.GetAttributeValueOrChildValueByName( "remoteCompId" ).AsAsciiString( remoteCompId, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopicConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "includeSessionLevelMsgs", includeSessionLevelMsgs );
    config.SetAttribute( "msgTypeFilter",           CORE::CString( msgTypeFilter ) );
    config.SetAttribute( "remoteCompId",            CORE::CString( remoteCompId ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopicConfig::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
    }
    return LoadCustomConfig( src.customConfig );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CFIXServerPubSubClientTopicConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CFIXServerPubSubClientTopicConfig( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

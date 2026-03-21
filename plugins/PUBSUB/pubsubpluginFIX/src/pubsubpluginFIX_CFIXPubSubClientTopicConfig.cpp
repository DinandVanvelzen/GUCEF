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

#include "pubsubpluginFIX_CFIXPubSubClientTopicConfig.h"

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

CFIXPubSubClientTopicConfig::CFIXPubSubClientTopicConfig( void )
    : PUBSUB::CPubSubClientTopicConfig()
    , CORE::CTSharedObjCreator< CFIXPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopicConfig::CFIXPubSubClientTopicConfig( const CFIXPubSubClientTopicConfig& src )
    : PUBSUB::CPubSubClientTopicConfig( src )
    , CORE::CTSharedObjCreator< CFIXPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( src.includeSessionLevelMsgs )
    , msgTypeFilter( src.msgTypeFilter )
{GUCEF_TRACE;

    customConfig = src.customConfig;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopicConfig::CFIXPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig )
    : PUBSUB::CPubSubClientTopicConfig( genericConfig )
    , CORE::CTSharedObjCreator< CFIXPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopicConfig::~CFIXPubSubClientTopicConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopicConfig&
CFIXPubSubClientTopicConfig::operator=( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopicConfig&
CFIXPubSubClientTopicConfig::operator=( const CFIXPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        includeSessionLevelMsgs = src.includeSessionLevelMsgs;
        msgTypeFilter           = src.msgTypeFilter;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopicConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    includeSessionLevelMsgs = config.GetAttributeValueOrChildValueByName( "includeSessionLevelMsgs" ).AsBool( includeSessionLevelMsgs, true );
    msgTypeFilter           = config.GetAttributeValueOrChildValueByName( "msgTypeFilter" ).AsString( msgTypeFilter, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopicConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "includeSessionLevelMsgs", includeSessionLevelMsgs );
    config.SetAttribute( "msgTypeFilter",           msgTypeFilter );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopicConfig::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
    }
    return LoadCustomConfig( src.customConfig );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CFIXPubSubClientTopicConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CFIXPubSubClientTopicConfig( *this );
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

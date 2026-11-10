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

#include "pubsubpluginFIX_CFIXClientPubSubClientTopicConfig.h"

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

CFIXClientPubSubClientTopicConfig::CFIXClientPubSubClientTopicConfig( void )
    : PUBSUB::CPubSubClientTopicConfig()
    , CORE::CTSharedObjCreator< CFIXClientPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientTopicConfig::CFIXClientPubSubClientTopicConfig( const CFIXClientPubSubClientTopicConfig& src )
    : PUBSUB::CPubSubClientTopicConfig( src )
    , CORE::CTSharedObjCreator< CFIXClientPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( src.includeSessionLevelMsgs )
    , msgTypeFilter( src.msgTypeFilter )
{GUCEF_TRACE;

    customConfig = src.customConfig;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientTopicConfig::CFIXClientPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig )
    : PUBSUB::CPubSubClientTopicConfig( genericConfig )
    , CORE::CTSharedObjCreator< CFIXClientPubSubClientTopicConfig, MT::CMutex >( this )
    , includeSessionLevelMsgs( false )
    , msgTypeFilter()
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientTopicConfig::~CFIXClientPubSubClientTopicConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientTopicConfig&
CFIXClientPubSubClientTopicConfig::operator=( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        LoadCustomConfig( src.customConfig );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientTopicConfig&
CFIXClientPubSubClientTopicConfig::operator=( const CFIXClientPubSubClientTopicConfig& src )
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
CFIXClientPubSubClientTopicConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    includeSessionLevelMsgs = config.GetAttributeValueOrChildValueByName( "includeSessionLevelMsgs" ).AsBool( includeSessionLevelMsgs, true );
    msgTypeFilter           = config.GetAttributeValueOrChildValueByName( "msgTypeFilter" ).AsAsciiString( msgTypeFilter, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientTopicConfig::SaveCustomConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "includeSessionLevelMsgs", includeSessionLevelMsgs );
    config.SetAttribute( "msgTypeFilter",           CORE::CString( msgTypeFilter ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClientTopicConfig::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
    }
    return LoadCustomConfig( src.customConfig );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CFIXClientPubSubClientTopicConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CFIXClientPubSubClientTopicConfig( *this );
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

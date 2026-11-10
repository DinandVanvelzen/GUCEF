/*
 *  pubsubpluginAWSSNS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SNS
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

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#include "pubsubpluginAWSSNS_CAwsSnsPubSubClientTopicConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSNS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig::CAwsSnsPubSubClientTopicConfig( void )
    : PUBSUB::CPubSubClientTopicConfig()
    , CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopicConfig, MT::CMutex >( this )
    , createTopicIfMissing( false )
    , tryToUsePublishBatch( false )
    , addPrefixWhenSendingKvPairs( false )
    , kvPairPrefixToAddOnSend()
    , addPrefixWhenSendingMetaDataKvPairs( false )
    , metaDatakvPairPrefixToAddOnSend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig::CAwsSnsPubSubClientTopicConfig( const CAwsSnsPubSubClientTopicConfig& src )
    : PUBSUB::CPubSubClientTopicConfig( src )
    , CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopicConfig, MT::CMutex >( this )
    , createTopicIfMissing( src.createTopicIfMissing )
    , tryToUsePublishBatch( src.tryToUsePublishBatch )
    , addPrefixWhenSendingKvPairs( src.addPrefixWhenSendingKvPairs )
    , kvPairPrefixToAddOnSend( src.kvPairPrefixToAddOnSend )
    , addPrefixWhenSendingMetaDataKvPairs( src.addPrefixWhenSendingMetaDataKvPairs )
    , metaDatakvPairPrefixToAddOnSend( src.metaDatakvPairPrefixToAddOnSend )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig::CAwsSnsPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig )
    : PUBSUB::CPubSubClientTopicConfig( genericConfig )
    , CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopicConfig, MT::CMutex >( this )
    , createTopicIfMissing( false )
    , tryToUsePublishBatch( false )
    , addPrefixWhenSendingKvPairs( false )
    , kvPairPrefixToAddOnSend()
    , addPrefixWhenSendingMetaDataKvPairs( false )
    , metaDatakvPairPrefixToAddOnSend()
{GUCEF_TRACE;

    LoadCustomConfig( genericConfig.customConfig );  
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig::~CAwsSnsPubSubClientTopicConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClientTopicConfig::LoadCustomConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;
        
    createTopicIfMissing = config.GetAttributeValueOrChildValueByName( "createTopicIfMissing" ).AsBool( createTopicIfMissing, true );
    tryToUsePublishBatch = config.GetAttributeValueOrChildValueByName( "tryToUsePublishBatch" ).AsBool( tryToUsePublishBatch, true );
    addPrefixWhenSendingKvPairs = config.GetAttributeValueOrChildValueByName( "addPrefixWhenSendingKvPairs" ).AsBool( addPrefixWhenSendingKvPairs, true );
    kvPairPrefixToAddOnSend = config.GetAttributeValueOrChildValueByName( "kvPairPrefixToAddOnSend" ).AsAsciiString( kvPairPrefixToAddOnSend, true );
    addPrefixWhenSendingMetaDataKvPairs = config.GetAttributeValueOrChildValueByName( "addPrefixWhenSendingMetaDataKvPairs" ).AsBool( addPrefixWhenSendingMetaDataKvPairs, true );
    metaDatakvPairPrefixToAddOnSend = config.GetAttributeValueOrChildValueByName( "metaDatakvPairPrefixToAddOnSend" ).AsAsciiString( metaDatakvPairPrefixToAddOnSend, true );    
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSnsPubSubClientTopicConfig::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        return LoadCustomConfig( src.customConfig );    
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig& 
CAwsSnsPubSubClientTopicConfig::operator=( const PUBSUB::CPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        LoadCustomConfig( src.customConfig );    
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfig& 
CAwsSnsPubSubClientTopicConfig::operator=( const CAwsSnsPubSubClientTopicConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        PUBSUB::CPubSubClientTopicConfig::operator=( src );
        createTopicIfMissing = src.createTopicIfMissing;
        tryToUsePublishBatch = src.tryToUsePublishBatch;
        addPrefixWhenSendingKvPairs = src.addPrefixWhenSendingKvPairs;
        kvPairPrefixToAddOnSend = src.kvPairPrefixToAddOnSend;
        addPrefixWhenSendingMetaDataKvPairs = src.addPrefixWhenSendingMetaDataKvPairs;
        metaDatakvPairPrefixToAddOnSend = src.metaDatakvPairPrefixToAddOnSend;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CAwsSnsPubSubClientTopicConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAwsSnsPubSubClientTopicConfig( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

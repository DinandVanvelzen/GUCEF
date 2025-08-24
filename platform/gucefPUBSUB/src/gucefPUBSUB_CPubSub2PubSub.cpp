/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefPUBSUB_CPubSub2PubSub.h"

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_TICKET_REFILLS_ON_BUSY_CYCLE                  10000
#define GUCEF_DEFAULT_PUBSUB_RECONNECT_DELAY_IN_MS                  100
#define GUCEF_DEFAULT_PUBSUB_MAX_PUBLISHED_MSG_INFLIGHT_TIME_IN_MS  ( 30 * 1000 )
#define GUCEF_DEFAULT_PUBSUB_SIDE_MAX_IN_FLIGHT                     1000

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

RestApiPubSub2PubSubInfoResource::RestApiPubSub2PubSubInfoResource( PubSub2PubSub* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSubClientChannelConfigResource::RestApiPubSubClientChannelConfigResource( PubSub2PubSub* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSubClientChannelConfigResource::~RestApiPubSubClientChannelConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSubClientChannelConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                                     CORE::CDataNode& output             ,
                                                     const CORE::CString& representation ,
                                                     const CORE::CString& params         )
{GUCEF_TRACE;

    CORE::Int32 channelsOffset = resourcePath.HasSubstr( "channels/" );
    if ( channelsOffset < 0 )
        return false;
    channelsOffset += 9;

    CORE::Int32 channelId = CORE::StringToInt32( resourcePath.SubstrToIndex( channelsOffset, false ) );

    CPubSubClientChannelPtr channel = m_app->GetChannelByChannelId( channelId );
    if ( !channel.IsNULL() )
        return channel->GetChannelSettings().SaveConfig( output );
    else
        return false;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubInfoResource::~RestApiPubSub2PubSubInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSub2PubSubInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                             CORE::CDataNode& output             ,
                                             const CORE::CString& representation ,
                                             const CORE::CString& params         )
{GUCEF_TRACE;

    output.Clear();
    output.SetName( "info" );
    output.SetNodeType( GUCEF_DATATYPE_OBJECT );
    output.SetAttribute( "application", "pubsub2pubsub" );
    output.SetAttribute( "appBuildDateTime", PubSub2PubSub::GetAppCompileDateTime().ToIso8601DateTimeString( true, true ) );
    output.SetAttribute( "platformBuildDateTime", CORE::CDateTime::CompileDateTime().ToIso8601DateTimeString( true, true ) );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( false ) );
    #else
    output.SetAttribute( "isReleaseBuild", CORE::CVariant( true ) );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubHealthResource::RestApiPubSub2PubSubHealthResource( PubSub2PubSub* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = false;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubHealthResource::~RestApiPubSub2PubSubHealthResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSub2PubSubHealthResource::Serialize( const CORE::CString& resourcePath   ,
                                               CORE::CDataNode& output             ,
                                               const CORE::CString& representation ,
                                               const CORE::CString& params         )
{GUCEF_TRACE;

    output.Clear();
    output.SetName( "health" );
    output.SetNodeType( GUCEF_DATATYPE_OBJECT );
    output.SetAttribute( "isHealthy", CORE::CVariant( m_app->GetLatestIsHealthyState() ) );
    output.SetAttribute( "isHealthyLastChange", m_app->GetLatestIsHealthyStateChangeDt().ToIso8601DateTimeString( true, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubConfigResource::RestApiPubSub2PubSubConfigResource( PubSub2PubSub* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubConfigResource::~RestApiPubSub2PubSubConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiPubSub2PubSubConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                               CORE::CDataNode& output             ,
                                               const CORE::CString& representation ,
                                               const CORE::CString& params         )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const CORE::CDataNode* appConfigData = m_app->GetAppConfig();
        if ( GUCEF_NULL != appConfigData )
        {
            output.Copy( *appConfigData );
            return true;
        }
        return false;
    }
    else
    {
        const CORE::CDataNode& globalConfigData = m_app->GetGlobalConfig();
        output.Copy( globalConfigData );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubConfigResource::TDeserializeState
RestApiPubSub2PubSubConfigResource::UpdateGlobalConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    // First put the app in standby mode before we mess with the settings
    if ( !m_app->SetStandbyMode( true ) )
        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;

    // Grab a copy of the current global config for rollback if needed
    CORE::CDataNode oldGlobalConfig = m_app->GetGlobalConfig();
    const CORE::CDataNode* oldAppConfigData = m_app->GetAppConfig( oldGlobalConfig );

    if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( cfg ) )
    {
        const CORE::CDataNode* appConfigData = m_app->GetAppConfig();
        if ( GUCEF_NULL == appConfigData )
        {
            // We should not lose access to the app config section, this indicates a serious issue
            // Attempt to roll back so we are not entirely broken
            if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( oldGlobalConfig ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiPubSub2PubSubConfigResource: Failed to obtain access to app config after applying new consolidated global config. Successfully rolled back to previous global config" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RestApiPubSub2PubSubConfigResource: Failed to obtain access to app config after applying new consolidated global config and subsequently also failed to roll back to previous global config" );
            }

            return TDeserializeState::DESERIALIZESTATE_CORRUPTEDINPUT;
        }

        if ( GUCEF_NULL != oldAppConfigData )
        {
            bool allowRestApiAppGlobalConfigToPersist = oldAppConfigData->GetAttributeValueOrChildValueByName( "allowPeristanceOfNewGlobalConfigViaRestApi" ).AsBool( false, true );
            if ( allowRestApiAppGlobalConfigToPersist )
            {
                if ( CORE::CCoreGlobal::Instance()->GetConfigStore().SaveConsolidatedConfig( cfg ) )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiPubSub2PubSubConfigResource: Successfully saved new consolidated global config" );
                }
                else
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiPubSub2PubSubConfigResource: Failed to save new consolidated global config" );    
                }
            }
        }

        if ( !m_app->IsGlobalStandbyEnabled() )
        {
            if ( m_app->SetStandbyMode( false ) )
                return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
            else
                return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiPubSub2PubSubConfigResource: IsGlobalStandbyEnabled is true. We will leave the app in standby mode" );
            return TDeserializeState::DESERIALIZESTATE_SUCCEEDED;
        }
    }
    else
    {
        // We ran into an issue applying the config
        // Attempt to roll back so we are not entirely broken
        if ( CORE::CCoreGlobal::Instance()->GetConfigStore().ApplyConsolidatedConfig( oldGlobalConfig ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RestApiPubSub2PubSubConfigResource: Failed to apply new global config. Successfully rolled back to previous global config" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "RestApiUdp2RedisConfigResource: Failed to apply new global config and subsequently also failed to roll back to previous global config" );
        }                
        return TDeserializeState::DESERIALIZESTATE_UNABLETOUPDATE;
    }        
}

/*-------------------------------------------------------------------------*/

RestApiPubSub2PubSubConfigResource::TDeserializeState
RestApiPubSub2PubSubConfigResource::Deserialize( const CORE::CString& resourcePath   ,
                                                 const CORE::CDataNode& input        ,
                                                 const CORE::CString& representation ,
                                                 bool isDeltaUpdateOnly              )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        // Make a copy of the entire global cfg
        CORE::CDataNode globalCfg = m_app->GetGlobalConfig();
        CORE::CDataNode* appConfigData = m_app->GetAppConfig( globalCfg );

        if ( isDeltaUpdateOnly )
        {
            // Not supported yet. @TODO
            // requires proper tree merging support

            return TDeserializeState::DESERIALIZESTATE_NOTSUPPORTED;
        }
        else
        {
            // Provided input content replaces the entire app config segment
            // We also protect against dropping the node name if only the values are provided in this subset
            CORE::CString nodeName = appConfigData->GetName();
            *appConfigData = input;
            if ( appConfigData->GetName().IsNULLOrEmpty() )
                appConfigData->SetName( nodeName ); 
        }
        return UpdateGlobalConfig( globalCfg );
    }
    else
    {
        if ( isDeltaUpdateOnly )
        {
            // Not supported yet. @TODO
            // requires proper tree merging support

            return TDeserializeState::DESERIALIZESTATE_NOTSUPPORTED;
        }
        else
        {
            return UpdateGlobalConfig( input );
        }
    }
}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::ExplicitChannelSideTopicOverlayConfig( void )
    : CORE::CIConfigurable()
    , topicName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::ExplicitChannelSideTopicOverlayConfig( const ExplicitChannelSideTopicOverlayConfig& src )
    : CORE::CIConfigurable( src )
    , topicName( src.topicName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::~ExplicitChannelSideTopicOverlayConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "topicName", topicName ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    topicName = cfg.GetAttributeValue( "topicName" ).AsString( topicName, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSubConfig::ExplicitChannelSideTopicOverlayConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::ExplicitChannelSideOverlayConfig( void )
    : CORE::CIConfigurable()
    , sideId()
    , remoteAddresses()
    , topics()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::ExplicitChannelSideOverlayConfig( const ExplicitChannelSideOverlayConfig& src )
    : CORE::CIConfigurable( src )
    , sideId( src.sideId )
    , remoteAddresses( src.remoteAddresses )
    , topics( src.topics )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::~ExplicitChannelSideOverlayConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = cfg.SetAttribute( "sideId", sideId ) && totalSuccess;

    CORE::CDataNode* remoteAddressesNode = cfg.FindOrAddChild( "remoteAddresses" );
    if ( GUCEF_NULL != remoteAddressesNode )
    {
        remoteAddressesNode->SetNodeType( GUCEF_DATATYPE_ARRAY );

        THostAddressVector::const_iterator a = remoteAddresses.begin();
        while ( a != remoteAddresses.end() )
        {
            remoteAddressesNode->AddValueAsChild( (*a).HostnameAndPortAsString() );
            ++a;
        }
    }
    
    CORE::CDataNode* topicsNode = cfg.FindOrAddChild( "topics" );
    if ( GUCEF_NULL == topicsNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelSideOverlayConfig:SaveConfig: config is malformed, failed to save a mandatory topics section" );
        return false;
    }
    topicsNode->SetNodeType( GUCEF_DATATYPE_ARRAY );

    ExplicitChannelSideTopicOverlayConfigVector::const_iterator i = topics.begin();
    while ( i != topics.end() )
    {
        CORE::CDataNode* topicConfigNode = topicsNode->AddChild( "ExplicitChannelSideTopicOverlayConfig", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != topicConfigNode )
        {
            const ExplicitChannelSideTopicOverlayConfig& topicCfg = (*i);
            if ( !topicCfg.SaveConfig( *topicConfigNode ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelSideOverlayConfig:SaveConfig: Unable to save a child node in the topics section as ExplicitChannelSideTopicOverlayConfig with topic name \"" + topicCfg.topicName + "\"" );
            }
        }

        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    sideId = cfg.GetAttributeValue( "sideId" ).AsString( sideId, true );
    
    // Find the optional list of remote addressess
    // If it exists we apply it otherwise assume the default template setting is sufficient
    const CORE::CDataNode* remoteAddressesNode = cfg.FindChild( "remoteAddresses" );
    if ( GUCEF_NULL != remoteAddressesNode )
    {
        CORE::CDataNode::TVariantVector values = remoteAddressesNode->GetChildrenValues();
        CORE::CDataNode::TVariantVector::iterator v = values.begin();
        while ( v != values.end() )
        {
            COMCORE::CHostAddress hostAddress;
            if ( !hostAddress.SetHostnameAndPort( (*v).AsString() ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelSideOverlayConfig:LoadConfig: Failed to resolve remote address: " + (*v).AsString() );
            }
            remoteAddresses.push_back( hostAddress );
            ++v;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelSideOverlayConfig:LoadConfig: Loaded " + CORE::ToString( remoteAddresses.size() ) + " remote addresses for side overlay" );
    }

    const CORE::CDataNode* topicsNode = cfg.FindChild( "topics" );
    if ( GUCEF_NULL == topicsNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelSideOverlayConfig:LoadConfig: config is malformed, failed to find a mandatory topics section in absence of auto 'from side' topic matching" );
        return false;
    }

    // try to parse each child in the topics section
    CORE::CDataNode::const_iterator i = topicsNode->ConstBegin();
    while ( i != topicsNode->ConstEnd() )
    {        
        ExplicitChannelSideTopicOverlayConfig topicConfig;
        if ( topicConfig.LoadConfig( *(*i) ) )
        {            
            topics.push_back( topicConfig );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to load a child node in the topics section as ExplicitChannelSideTopicOverlayConfig" );
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSubConfig::ExplicitChannelSideOverlayConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelOverlayConfig::ExplicitChannelOverlayConfig( void )
    : CORE::CIConfigurable()
    , usingTemplate()
    , channelId( -1 )
    , channelName( "ch{channelId}" ) 
    , sides()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelOverlayConfig::ExplicitChannelOverlayConfig( const ExplicitChannelOverlayConfig& src )
    : CORE::CIConfigurable( src )
    , usingTemplate( src.usingTemplate )
    , channelId( src.channelId )
    , channelName( src.channelName )
    , sides( src.sides )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::ExplicitChannelOverlayConfig::~ExplicitChannelOverlayConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelOverlayConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = cfg.SetAttribute( "usingTemplate", usingTemplate ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "channelId", channelId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "channelName", channelName ) && totalSuccess;

    CORE::CDataNode* sidesNode = cfg.FindOrAddChild( "sides" );
    if ( GUCEF_NULL == sidesNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelOverlayConfig:SaveConfig: config is malformed, failed to save a mandatory sides section" );
        return false;
    }
    sidesNode->SetNodeType( GUCEF_DATATYPE_ARRAY );

    ExplicitChannelSideOverlayConfigVector::const_iterator i = sides.begin();
    while ( i != sides.end() )
    {
        CORE::CDataNode* sideConfigNode = sidesNode->AddChild( "ExplicitChannelSideOverlayConfig", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != sideConfigNode )
        {
            const ExplicitChannelSideOverlayConfig& sideCfg = (*i);
            if ( !sideCfg.SaveConfig( *sideConfigNode ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelOverlayConfig:SaveConfig: Unable to save a child node in the sides section as ExplicitChannelSideOverlayConfig with side id \"" + sideCfg.sideId + "\"" );
            }
        }

        ++i;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::ExplicitChannelOverlayConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    usingTemplate = cfg.GetAttributeValue( "usingTemplate" ).AsString( usingTemplate, true );
    channelId = cfg.GetAttributeValue( "channelId" ).AsInt32( channelId, true );
    channelName = cfg.GetAttributeValue( "channelName" ).AsString( channelName, true );

    const CORE::CDataNode* sidesNode = cfg.FindChild( "sides" );
    if ( GUCEF_NULL == sidesNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelOverlayConfig:LoadConfig: config is malformed, failed to find a mandatory sides section" );
        return false;
    }

    // try to parse each child in the topics section
    CORE::CDataNode::const_iterator i = sidesNode->ConstBegin();
    while ( i != sidesNode->ConstEnd() )
    {        
        ExplicitChannelSideOverlayConfig sideConfig;
        if ( sideConfig.LoadConfig( *(*i) ) )
        {            
            sides.push_back( sideConfig );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:ExplicitChannelOverlayConfig:LoadConfig: Unable to load a child node in the sides section as ExplicitChannelSideOverlayConfig" );
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSubConfig::ExplicitChannelOverlayConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSubConfig::ExplicitChannelOverlayConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::NumericalAutoChannelConfig::NumericalAutoChannelConfig( void )
    : CORE::CIConfigurable()
    , usingTemplate()
    , channelCount( 0 )
    , firstChannelId( 1 )
    , channelIds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::NumericalAutoChannelConfig::NumericalAutoChannelConfig( const NumericalAutoChannelConfig& src )
    : CORE::CIConfigurable( src )
    , usingTemplate( src.usingTemplate )
    , channelCount( src.channelCount )
    , firstChannelId( src.firstChannelId )
    , channelIds( src.channelIds )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::NumericalAutoChannelConfig::~NumericalAutoChannelConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::NumericalAutoChannelConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = cfg.SetAttribute( "usingTemplate", usingTemplate ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "channelCount", channelCount ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "firstChannelId", firstChannelId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "channelIds", CORE::ToString( channelIds ) ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::NumericalAutoChannelConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    usingTemplate = cfg.GetAttributeValue( "usingTemplate" ).AsString( usingTemplate, true );
    channelCount = cfg.GetAttributeValue( "channelCount" ).AsUInt32( channelCount, true );
    firstChannelId = cfg.GetAttributeValue( "firstChannelId" ).AsUInt32( firstChannelId, true );
    channelIds = cfg.GetAttributeValueOrChildValueByName( "channelIds" ).AsString( CORE::CString::Empty, true ).ParseUniqueElements( ',', false );

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSubConfig::NumericalAutoChannelConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSubConfig::NumericalAutoChannelConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::PubSub2PubSubConfig( void )
    : CORE::CIConfigurable()
    , channelConfigTemplates()
    , explicitOverlayChannels()
    , numericalAutoChannelConfig()
    , channelConfigs()
    , globalStandbyEnabled( false )
    , applyCpuThreadAffinityByDefault( false )
    , enableRestApi( true )
    , restApiPort( 10000 )
    , restBasicHealthUri( "/health/basic" )
    , transmitMetrics( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::PubSub2PubSubConfig( const PubSub2PubSubConfig& src )
    : CORE::CIConfigurable( src )
    , channelConfigTemplates( src.channelConfigTemplates )
    , explicitOverlayChannels( src.explicitOverlayChannels )
    , numericalAutoChannelConfig( src.numericalAutoChannelConfig )
    , channelConfigs( src.channelConfigs )
    , globalStandbyEnabled( src.globalStandbyEnabled )
    , applyCpuThreadAffinityByDefault( src.applyCpuThreadAffinityByDefault )
    , enableRestApi( src.enableRestApi )
    , restApiPort( src.restApiPort )
    , restBasicHealthUri( src.restBasicHealthUri )
    , transmitMetrics( src.transmitMetrics )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

PubSub2PubSubConfig::~PubSub2PubSubConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = cfg.SetAttribute( "globalStandbyEnabled", globalStandbyEnabled ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "applyCpuThreadAffinityByDefault", applyCpuThreadAffinityByDefault ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "enableRestApi", enableRestApi ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "restApiPort", restApiPort ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "restBasicHealthUri", restBasicHealthUri ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "transmitMetrics", transmitMetrics ) && totalSuccess;    
    

    CORE::CDataNode* channelConfigTemplatesNode = cfg.FindOrAddChild( "channelConfigTemplates" );
    if ( GUCEF_NULL == channelConfigTemplatesNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:SaveConfig: config is malformed, failed to save a mandatory channelConfigTemplates section" );
        return false;
    }
    channelConfigTemplatesNode->SetNodeType( GUCEF_DATATYPE_ARRAY );

    StringToPubSubChannelConfigMap::const_iterator i = channelConfigTemplates.begin();
    while ( i != channelConfigTemplates.end() )
    {
        CORE::CDataNode* pubSubChannelConfigNode = channelConfigTemplatesNode->AddChild( "PubSubChannelConfig", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != pubSubChannelConfigNode )
        {
            const CPubSubChannelSettings& channelCfg = (*i).second;
            if ( !channelCfg.SaveConfig( *pubSubChannelConfigNode ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:SaveConfig: Unable to save a child node in the channelConfigTemplates section as PubSubChannelConfig with template name \"" + channelCfg.channelName + "\"" );
            }
        }

        ++i;
    }

    CORE::CDataNode* explicitChannelOverlaysNode = cfg.FindOrAddChild( "explicitChannelOverlays" );
    if ( GUCEF_NULL == explicitChannelOverlaysNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: config is malformed, failed to save a mandatory explicitChannelOverlays section" );
        return false;
    }
    explicitChannelOverlaysNode->SetNodeType( GUCEF_DATATYPE_ARRAY );
    
    // try to parse each child in the explicit channels section
    ExplicitChannelOverlayConfigVector::const_iterator n = explicitOverlayChannels.begin();
    while ( n != explicitOverlayChannels.end() )
    {        
        CORE::CDataNode* pubSubExplicitChannelConfigNode = explicitChannelOverlaysNode->AddChild( "ExplicitChannelOverlayConfig", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != pubSubExplicitChannelConfigNode )
        {
            const ExplicitChannelOverlayConfig& explicitChannelConfig = (*n);
            if ( !explicitChannelConfig.SaveConfig( *pubSubExplicitChannelConfigNode ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:SaveConfig: Unable to save a child node in the explicitChannelOverlays section as ExplicitChannelOverlayConfig" );
            }
        }
        ++n;
    }

    CORE::CDataNode* numericalAutoChannelsConfigNode = cfg.FindOrAddChild( "numericalAutoChannels" );
    if ( GUCEF_NULL == numericalAutoChannelsConfigNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: config is malformed, failed to save a mandatory numericalAutoChannels section" );
        return false;
    }
    totalSuccess = numericalAutoChannelConfig.SaveConfig( *numericalAutoChannelsConfigNode ) && totalSuccess; 

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSubConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    bool totalSuccess = true;
    
    globalStandbyEnabled = cfg.GetAttributeValueOrChildValueByName( "globalStandbyEnabled" ).AsBool( globalStandbyEnabled, true );
    applyCpuThreadAffinityByDefault = cfg.GetAttributeValueOrChildValueByName( "applyCpuThreadAffinityByDefault" ).AsBool( false, true );
    enableRestApi = cfg.GetAttributeValueOrChildValueByName( "enableRestApi" ).AsBool( enableRestApi, true );
    restApiPort = cfg.GetAttributeValueOrChildValueByName( "restApiPort" ).AsUInt16( restApiPort, true );
    restBasicHealthUri = cfg.GetAttributeValueOrChildValueByName( "restBasicHealthUri" ).AsString( restBasicHealthUri, true );
    transmitMetrics = cfg.GetAttributeValueOrChildValueByName( "transmitMetrics" ).AsBool( transmitMetrics, true );

    const CORE::CDataNode* channelConfigTemplateRefsNode = cfg.FindChild( "channelConfigTemplateRefs" );
    if ( GUCEF_NULL == channelConfigTemplateRefsNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: config is malformed, failed to find a mandatory channelConfigTemplateRefs section" );
        return false;
    }

    // try to parse each child in the template refs section
    CORE::CDataNode::const_iterator i = channelConfigTemplateRefsNode->ConstBegin();
    while ( i != channelConfigTemplateRefsNode->ConstEnd() )
    {        
        CORE::CString templateRef = (*i)->GetValue().AsString();

        // @TODO: Later this would be resolved more generically but for now we just code the 'uri' handling here
        if ( 0 == templateRef.HasSubstr( "vfs://", true ) )
        {
            templateRef = templateRef.CutChars( 6, true );
            
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            
            CORE::CDataNode channelTemplate;            
            if ( vfs.LoadFile( channelTemplate, templateRef ) )
            {
                CPubSubChannelSettings channelConfig;
                if ( channelConfig.LoadConfig( channelTemplate ) )
                {            
                    channelConfigTemplates[ channelConfig.channelName ] = channelConfig;
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Loaded a channel config template with name \"" + channelConfig.channelName + "\"" );
                }
                else
                {
                    totalSuccess = false;
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to parse PubSubChannelConfig from content from vfs file: " + templateRef );
                }                    
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to load vfs file " + templateRef + " as document for further processing as a template PubSubChannelConfig" );
            }
        }
        ++i;
    }

    const CORE::CDataNode* channelConfigTemplatesNode = cfg.FindChild( "channelConfigTemplates" );
    if ( GUCEF_NULL == channelConfigTemplatesNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: config is malformed, failed to find a mandatory channelConfigTemplates section" );
        return false;
    }

    // try to parse each child in the template section
    i = channelConfigTemplatesNode->ConstBegin();
    while ( i != channelConfigTemplatesNode->ConstEnd() )
    {        
        CPubSubChannelSettings channelConfig;
        if ( channelConfig.LoadConfig( *(*i) ) )
        {            
            channelConfigTemplates[ channelConfig.channelName ] = channelConfig;
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Loaded a channel config template with name \"" + channelConfig.channelName + "\"" );
        }
        else
        {
            totalSuccess = false;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to load a child node in the channelConfigTemplates section as PubSubChannelConfig" );
        }
        ++i;
    }

    const CORE::CDataNode* explicitChannelOverlaysNode = cfg.FindChild( "explicitChannelOverlays" );
    if ( GUCEF_NULL == explicitChannelOverlaysNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: config is malformed, failed to find a mandatory explicitChannelOverlays section" );
        return false;
    }
    
    // try to parse each child in the explicit channels section
    bool hasChannelOverlays = false;
    i = explicitChannelOverlaysNode->ConstBegin();
    while ( i != explicitChannelOverlaysNode->ConstEnd() )
    {        
        ExplicitChannelOverlayConfig explicitChannelOverlayConfig;
        if ( explicitChannelOverlayConfig.LoadConfig( *(*i) ) )
        {            
            explicitOverlayChannels.push_back( explicitChannelOverlayConfig );
            hasChannelOverlays = true;
            
            CORE::CString channelName = explicitChannelOverlayConfig.channelName.ReplaceSubstr( "{channelId}", CORE::ToString( explicitChannelOverlayConfig.channelId ) );            
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Loaded an explicit channel overlay config with name \"" + channelName + "\"" );
        }
        else
        {
            totalSuccess = false;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to load a child node in the explicitChannelOverlays section as ExplicitChannelOverlayConfig" );
        }
        ++i;
    }

    const CORE::CDataNode* numericalAutoChannelsConfigNode = cfg.FindChild( "numericalAutoChannels" );
    if ( GUCEF_NULL == numericalAutoChannelsConfigNode && !hasChannelOverlays )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:LoadConfig: Unable to find mandatory numericalAutoChannels section" );
        return false;
    }
    if ( GUCEF_NULL != numericalAutoChannelsConfigNode )
        totalSuccess = numericalAutoChannelConfig.LoadConfig( *numericalAutoChannelsConfigNode ) && totalSuccess; 
    
    return NormalizeConfig() && totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
PubSub2PubSubConfig::NormalizeConfig( void )
{GUCEF_TRACE;

    // @TODO: We dont have generic datanode merge/overlay functionality yet and as such we make do with the below use-case specific code
    
    bool totalSuccess = true;
    channelConfigs.clear();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSubConfig:NormalizeConfig: Normalizing " + CORE::ToString( explicitOverlayChannels.size() ) + " explicit channel overlay configs" );

    ExplicitChannelOverlayConfigVector::iterator i = explicitOverlayChannels.begin();
    while ( i != explicitOverlayChannels.end() )
    {
        ExplicitChannelOverlayConfig& overlayConfig = (*i);
        
        StringToPubSubChannelConfigMap::iterator n = channelConfigTemplates.find( overlayConfig.usingTemplate );
        if ( n != channelConfigTemplates.end() )
        {
            // grab a copy from the template and apply the overlay values

            CPubSubChannelSettings channelConfig( (*n).second );

            // apply the overlay values
            channelConfig.channelId = overlayConfig.channelId;
            channelConfig.channelName = overlayConfig.channelName.ReplaceSubstr( "{channelId}", CORE::ToString( channelConfig.channelId ) );
            
            ExplicitChannelOverlayConfig::ExplicitChannelSideOverlayConfigVector::iterator s = overlayConfig.sides.begin();
            while ( s != overlayConfig.sides.end() )
            {
                ExplicitChannelSideOverlayConfig& sideOverlayConfig = (*s);
                CPubSubSideChannelSettingsPtr sideConfig = channelConfig.GetPubSubSideSettings( sideOverlayConfig.sideId );
                if ( !sideConfig.IsNULL() )
                {
                    if ( !sideOverlayConfig.remoteAddresses.empty() )
                        sideConfig->pubsubClientConfig.remoteAddresses = sideOverlayConfig.remoteAddresses;
                    
                    ExplicitChannelSideOverlayConfig::ExplicitChannelSideTopicOverlayConfigVector::iterator t = sideOverlayConfig.topics.begin();
                    while ( t != sideOverlayConfig.topics.end() )
                    {
                        ExplicitChannelSideTopicOverlayConfig& topicOverlayConfig = (*t);
                        if ( !sideConfig->pubsubClientConfig.defaultTopicConfig.IsNULL() )
                        {
                            CPubSubClientTopicConfigPtr topicConfig( GUCEF_NEW CPubSubClientTopicConfig( *sideConfig->pubsubClientConfig.defaultTopicConfig.GetPointerAlways() ) );
                            
                            // apply the overlay values
                            topicConfig->topicName = topicOverlayConfig.topicName;

                            sideConfig->pubsubClientConfig.topics.push_back( topicConfig );
                        }
                        ++t;
                    }                    
                }
                
                ++s;
            }

            // If auto topic association is enabled we have to apply it as such for the above loaded channels and associated topics
            channelConfig.AutoAssociateTopicsAnyToAnyAcrossSides();

            channelConfigs[ channelConfig.channelId ] = channelConfig;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSubConfig:NormalizeConfig: Overlay references template \"" + overlayConfig.usingTemplate + "\" which cannot be found" );
            totalSuccess = false;
        }
        ++i;
    }

    // Now normalize the auto numeric channels

    // Validate the channel IDs.
    // Depending on the use case these could be vital identifiers not just an index so some validation is in order
    Int32Set channelIds;
    CORE::CString::StringSet::iterator n = numericalAutoChannelConfig.channelIds.begin();
    while ( n != numericalAutoChannelConfig.channelIds.end() )
    {
        const CORE::CString& str = (*n);
        if ( str.HasChar( '-' ) >= 0 )
        {
            CORE::Int32 startId = GUCEF_MT_INT32MIN;
            CORE::Int32 endId = GUCEF_MT_INT32MIN;

            CORE::CString::StringVector rangeStrs = str.ParseElements( '-', false );
            if ( rangeStrs.size() > 0 )
                startId = CORE::StringToInt32( rangeStrs[ 0 ], GUCEF_MT_INT32MIN );
            if ( rangeStrs.size() > 1 )
                endId = CORE::StringToInt32( rangeStrs[ 1 ], GUCEF_MT_INT32MIN );

            if ( startId != GUCEF_MT_INT32MIN && endId != GUCEF_MT_INT32MIN )
            {
                if ( endId < startId )
                {
                    CORE::Int32 tmp = startId;
                    startId = endId;
                    endId = tmp;
                }

                for ( CORE::Int32 i=startId; i<=endId; i++ )
                    channelIds.insert( i );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSubConfig:NormalizeConfig: Invalid channel ID range provided. Fix the config. Str: " + str );
                return false;
            }
        }
        else
        {
            CORE::Int32 id = CORE::StringToInt32( str, GUCEF_MT_INT32MIN );
            if ( id != GUCEF_MT_INT32MIN )
                channelIds.insert( id );
        }
        ++n;
    }
    if ( channelIds.size() < numericalAutoChannelConfig.channelIds.size() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSubConfig:NormalizeConfig: Only " + CORE::ToString( channelIds.size() ) + " numerical channel IDs were obtained from the channel list which contained more strings. Fix the config" );
        return false;
    }

    // Make sure we have enough channel IDs specified to cover the channel count
    if ( channelIds.size() < numericalAutoChannelConfig.channelCount )
    {
        // auto-generate additional channel IDs
        // This allows a configuration style where you don't have to specify all channel IDs
        CORE::Int32 lastAutoGenChannelId = numericalAutoChannelConfig.firstChannelId;
        CORE::Int32 missingChannels = numericalAutoChannelConfig.channelCount - (CORE::Int32) channelIds.size();
        for ( CORE::Int32 i=0; i<missingChannels; ++i )
        {
            while ( channelIds.find( lastAutoGenChannelId ) != channelIds.end() )
                ++lastAutoGenChannelId;
            channelIds.insert( lastAutoGenChannelId );
            ++lastAutoGenChannelId;
        }
    }
    else
    if ( numericalAutoChannelConfig.channelCount < channelIds.size() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSubConfig::NormalizeConfig: " + CORE::ToString( channelIds.size() ) + " numerical channel IDs were provided but a total channel count of " +
            CORE::ToString( numericalAutoChannelConfig.channelCount ) + " was configured. Channel count will be increased to match the nr of IDs" );
        numericalAutoChannelConfig.channelCount = (CORE::UInt32) channelIds.size();
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSubConfig:NormalizeConfig: There are now " + CORE::ToString( channelConfigs.size() ) + " channel configs defined" );

    // Auto assign CPU affinity, if desired, now that we have defined all the channel configs
    
    CORE::UInt32 logicalCpuCount = CORE::GetLogicalCPUCount();

    CORE::UInt32 currentCpu = 0;
    Int32ToPubSubChannelConfigMap::iterator c = channelConfigs.begin();
    while ( c != channelConfigs.end() )
    {
        Int32 channelId = (*c).first;
        CPubSubChannelSettings& channelConfig = (*c).second;

        // Assign CPU affinity but note that channels have X sides and each side can potentially have their own thread
        CPubSubChannelSettings::TStringToPubSubSideChannelSettingsMap::iterator n = channelConfig.pubSubSideChannelSettingsMap.begin();
        while ( n != channelConfig.pubSubSideChannelSettingsMap.end() )
        {
            CPubSubSideChannelSettingsPtr sideSettings = (*n).second;
            if ( !sideSettings.IsNULL() )
            {
                if ( sideSettings->applyThreadCpuAffinity || applyCpuThreadAffinityByDefault )
                {
                    if ( sideSettings->performPubSubInDedicatedThread )
                    {
                        sideSettings->cpuAffinityForPubSubThread = currentCpu;

                        ++currentCpu;
                        if ( currentCpu >= logicalCpuCount ) // Wrap around if we run out of CPUs
                            currentCpu = 0;
                    }
                }
            }
            ++n;
        }
        ++c;
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSubConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSubConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

PubSub2PubSub::PubSub2PubSub( void )
    : CORE::CObserver()
    , CORE::CGloballyConfigurable( false )
    , m_isInStandby( false )
    , m_desiredInStandby( false )
    , m_channels()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_isHealthy( true )
    , m_lastIsHealthyChange( CORE::CDateTime::NowUTCDateTime() )
    , m_config()
    , m_globalConfig()
    , m_lock()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

PubSub2PubSub::~PubSub2PubSub()
{GUCEF_TRACE;

    m_httpServer.Close();
}

/*-------------------------------------------------------------------------*/

void
PubSub2PubSub::RegisterEventHandlers( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    TEventCallback callback( this, &PubSub2PubSub::OnVfsInit );
    SubscribeTo( &vfs                                       ,
                 VFS::CVFS::VfsInitializationCompletedEvent ,
                 callback                                   );

    TEventCallback callback2( this, &PubSub2PubSub::OnAppShutdown );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetApplication() ,
                 CORE::CGUCEFApplication::AppShutdownEvent        ,
                 callback2                                        );
    
}

/*-------------------------------------------------------------------------*/

void 
PubSub2PubSub::RegisterChannelEventHandlers( CPubSubClientChannelPtr channel )
{GUCEF_TRACE;

    TEventCallback callback( this, &PubSub2PubSub::OnChannelHealthStatusChanged );
    SubscribeTo( channel.GetPointerAlways()                    ,
                 CPubSubClientChannel::HealthStatusChangeEvent ,
                 callback                                      );
    
    bool wasHealthy = m_isHealthy;
    m_isHealthy = m_isHealthy && channel->IsHealthy();
    if ( m_isHealthy != wasHealthy )
        m_lastIsHealthyChange = CORE::CDateTime::NowUTCDateTime();    
}

/*-------------------------------------------------------------------------*/

CPubSubClientChannelPtr
PubSub2PubSub::GetChannelByChannelId( CORE::Int32 cid ) const
{GUCEF_TRACE;

    PubSubClientChannelMap::const_iterator i = m_channels.find( cid );
    if ( i != m_channels.end() )
    {
        return (*i).second;
        ++i;
    }
    return CPubSubClientChannelPtr();
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::IsGlobalStandbyEnabled( void ) const
{GUCEF_TRACE;

    return m_config.globalStandbyEnabled;
}

/*-------------------------------------------------------------------------*/

bool 
PubSub2PubSub::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    
    // Aggregate the health status across all channels
    bool fullyHealthy = true;
    try
    {
        PubSubClientChannelMap::const_iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CPubSubClientChannelPtr channel = (*i).second;
            if ( !channel.IsNULL() && !channel->IsHealthy() )
            {
                fullyHealthy = false;
                break;
            }
            ++i;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:IsHealthy: std exception while checking for channel health. what=" + CORE::ToString( e.what() ) );
        fullyHealthy = false;
    }

    // Log if there was a change in status
    if ( fullyHealthy != m_isHealthy )
    {
        m_isHealthy = fullyHealthy;        
        m_lastIsHealthyChange = CORE::CDateTime::NowUTCDateTime();

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:IsHealthy: overall health is now Ok" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:IsHealthy: overall health status is now unhealthy" );         
        }    
    }
    
    return fullyHealthy;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::GetLatestIsHealthyState( void ) const
{GUCEF_TRACE;

    return m_isHealthy;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
PubSub2PubSub::GetLatestIsHealthyStateChangeDt( void ) const
{GUCEF_TRACE;

    return m_lastIsHealthyChange;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::Start( void )
{GUCEF_TRACE;

    m_isInStandby = true;
    bool errorOccured = !SetStandbyMode( m_config.globalStandbyEnabled );
    return !errorOccured;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::SetStandbyMode( bool putInStandbyMode )
{GUCEF_TRACE;
        
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().IsInitialized() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:SetStandbyMode( " + CORE::BoolToString( putInStandbyMode ) + " ): Deferring state change attempt since the VFS is not initialized yet" );
        m_desiredInStandby = putInStandbyMode;
        return true;
    }
    
    // Check if we need to do anything
    if ( m_isInStandby == putInStandbyMode )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( " + CORE::BoolToString( putInStandbyMode ) + " ): Already in the desired mode (" + CORE::BoolToString( m_isInStandby ) + "), nothing to do" );
        return true;
    }

    // Before we do anything drastic make sure our logs are flushed to a known point to help diagnostics
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
    
    if ( putInStandbyMode )
    {
        bool totalSuccess = true;
        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        // Signal all channel threads to stop gracefully
        // Since this standby operation is global not per channel we signal all to stop before
        // we starting any waiting operation
        PubSubClientChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CPubSubClientChannelPtr channel = (*i).second;
            if ( !threadPool->RequestTaskToStop( channel.StaticCast< CORE::CTaskConsumer >(), false ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( true ): Requested channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        // Now actually wait for the threads to be finished
        i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CPubSubClientChannelPtr channel = (*i).second;
            if ( !channel->WaitForTaskToFinish( 30000 ) )
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( true ): Failed to signal task to stop for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) )
            }
            else
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( true ): Successfully waited for channel " + CORE::Int32ToString( channel->GetChannelSettings().channelId ) + "'s task to stop" );
            }
            ++i;
        }

        m_isInStandby = totalSuccess;
        return totalSuccess;
    }
    else
    {
        bool totalSuccess = true;

        // Channel config could have changed such that we need to remove channels that should no longer exist
        PubSubClientChannelMap::iterator i = m_channels.begin();
        while ( i != m_channels.end() )
        {
            CORE::Int32 channelId = (*i).first;
            PubSub2PubSubConfig::Int32ToPubSubChannelConfigMap::iterator n = m_config.channelConfigs.find( channelId );
            if ( n == m_config.channelConfigs.end() )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( false ): Found channel which no longer has corresponding channel settings, deleting channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels.erase( i );
                i = m_channels.begin();
                break;
            }
            ++i;
        }

        // Alternatively channel config could have changed such that we have new channels
        PubSub2PubSubConfig::Int32ToPubSubChannelConfigMap::iterator n = m_config.channelConfigs.begin();
        while ( n != m_config.channelConfigs.end() )
        {
            CORE::Int32 channelId = (*n).first;
            PubSubClientChannelMap::iterator i = m_channels.find( channelId );
            if ( i == m_channels.end() )
            {
                // This is a brand new channel. Lets add the channel object for it
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:SetStandbyMode( false ): Found channel settings whith no corresponding channel object, creating new channel with ID " + CORE::Int32ToString( channelId ) );
                m_channels[ channelId ] = CPubSubClientChannelPtr( GUCEF_NEW CPubSubClientChannel() );
            }
            ++n;
        }

        CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

        n = m_config.channelConfigs.begin();
        while ( n != m_config.channelConfigs.end() )
        {
            CORE::Int32 channelId = (*n).first;
            PubSubClientChannelMap::iterator i = m_channels.find( channelId );
            if ( i != m_channels.end() )
            {
                const CPubSubChannelSettings& channelSettings = (*n).second;
                CPubSubClientChannelPtr channel = (*i).second;

                if ( !channel->LoadConfig( channelSettings ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub::SetStandbyMode( false ): Failed to set channel settings on channel " + CORE::Int32ToString( channelId ) );
                    totalSuccess = false;
                    break;
                }

                CORE::CFutureResult result = threadPool->StartTaskWithConsumer( channel );
                if ( result.HasNoFuture() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub::SetStandbyMode( false ): Failed to start task (dedicated thread) for channel " + CORE::Int32ToString( channelId ) );
                    totalSuccess = false;
                    break;
                }
            }
            ++n;
        }

        m_isInStandby = !totalSuccess;
        return totalSuccess;
    }

    // After anything drastic make sure our logs are flushed to a known point to help diagnostics        
    CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::SaveConfig( CORE::CDataNode& outCfg ) const
{GUCEF_TRACE;

    return m_config.SaveConfig( outCfg );
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    // input sanity check for the main application config ...
    const CORE::CDataNode* appConfig = GetAppConfig( globalConfig );
    if ( GUCEF_NULL == appConfig )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:LoadConfig: Unable to locate app config in global config" );
        return false;
    }

    m_globalConfig.Copy( globalConfig );

    if ( VFS::CVfsGlobal::Instance()->GetVfs().IsInitialized() )
        return LoadConfigAfterVfsInit( globalConfig );    
    
    // defer load until after VFS init
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PubSub2PubSub::LoadConfigAfterVfsInit( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    // First we parse the config
    const CORE::CDataNode* pubsub2pubsubConfigNode = globalConfig.FindChild( "PubSub2PubSubConfig" );
    if ( GUCEF_NULL == pubsub2pubsubConfigNode )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:LoadConfig: config is malformed, failed to find a mandatory PubSub2PubSubConfig section" );
        return false;
    }

    if ( !m_config.LoadConfig( *pubsub2pubsubConfigNode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PubSub2PubSub:LoadConfig: config load failed" );
        return false;
    }

    m_httpServer.SetPort( m_config.restApiPort );

    m_httpRouter.SetResourceMapping( "/info", ( GUCEF_NEW RestApiPubSub2PubSubInfoResource( this ) )->CreateSharedPtr() );
    m_httpRouter.SetResourceMapping( "/config/appargs", ( GUCEF_NEW RestApiPubSub2PubSubConfigResource( this, true ) )->CreateSharedPtr() );
    m_httpRouter.SetResourceMapping( "/config", ( GUCEF_NEW RestApiPubSub2PubSubConfigResource( this, false ) )->CreateSharedPtr()  );
    m_httpRouter.SetResourceMapping( "/config/channels/", ( GUCEF_NEW TWebChannelCfgMapIndexMap( "channels", "channel", GUCEF_NULL, &m_config.channelConfigs, &m_lock, false ) )->CreateSharedPtr() );
    //m_httpRouter.SetResourceMapping( "/config/channels/*", ( GUCEF_NEW RestApiPubSubClientChannelConfigResource( this ) )->CreateSharedPtr() );
    m_httpRouter.SetResourceMapping( "/health", ( GUCEF_NEW RestApiPubSub2PubSubHealthResource( this ) )->CreateSharedPtr() );    
    m_httpRouter.SetResourceMapping( m_config.restBasicHealthUri, ( GUCEF_NEW WEB::CDummyHTTPServerResource() )->CreateSharedPtr() );

    if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub:LoadConfig: Failed to set up task management API" );
    }
    
    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );

    bool success = SetStandbyMode( m_desiredInStandby );

    if ( m_config.enableRestApi )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub: Opening REST API on port " + CORE::ToString( m_httpServer.GetPort() ) );
        if ( !m_httpServer.Listen() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "PubSub2PubSub: Failed to open REST API on port " + CORE::ToString( m_httpServer.GetPort() ) );
            return false;
        }
    }

    return success;
}

/*-------------------------------------------------------------------------*/

const CORE::CDateTime&
PubSub2PubSub::GetAppCompileDateTime( void )
{GUCEF_TRACE;

    static CORE::CDateTime compileDt = CORE::CDateTime::CompileDateTime( __DATE__, __TIME__ );
    return compileDt;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
PubSub2PubSub::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUB::PubSub2PubSub";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
PubSub2PubSub::OnVfsInit( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    LoadConfigAfterVfsInit( m_globalConfig );
}

/*-------------------------------------------------------------------------*/

void
PubSub2PubSub::OnAppShutdown( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // First stop all the work gracefully
    SetStandbyMode( true );

    // Since we are shutting down the app gracefully close the C&C API now
    m_httpServer.Close();
    m_httpRouter.RemoveAllResourceMappings();

    // Now get rid of all the channels we created based on the settings
    m_channels.clear();
}

/*-------------------------------------------------------------------------*/

void
PubSub2PubSub::OnChannelHealthStatusChanged( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // things could have changed since the event message was sent, we reevaluate etc
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode* 
PubSub2PubSub::GetAppConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    const CORE::CDataNode* appConfig = globalConfig.Search( "Main/AppArgs", '/', true, true );
    return appConfig;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode* 
PubSub2PubSub::GetAppConfig( CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    CORE::CDataNode* appConfig = globalConfig.Structure( "Main/AppArgs", '/' );
    return appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode* 
PubSub2PubSub::GetAppConfig( void ) const
{GUCEF_TRACE;

    return GetAppConfig( m_globalConfig );
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
PubSub2PubSub::GetGlobalConfig( void ) const
{GUCEF_TRACE;

    return m_globalConfig;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/



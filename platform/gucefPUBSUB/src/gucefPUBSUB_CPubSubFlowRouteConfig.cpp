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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefPUBSUB_CPubSubFlowRouteConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfig::CPubSubFlowRouteTopicConfig( void )
    : CORE::CIConfigurable()
    , CORE::CTSharedObjCreator< CPubSubFlowRouteTopicConfig, MT::CMutex >( this )
    , fromSideTopicName()
    , toSideTopicName()
    , failoverSideTopicName()
    , spilloverSideTopicName()
    , deadLetterSideTopicName()
    , persistenceSideTopicName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfig::CPubSubFlowRouteTopicConfig( const CPubSubFlowRouteTopicConfig& src )
    : CORE::CIConfigurable( src )
    , CORE::CTSharedObjCreator< CPubSubFlowRouteTopicConfig, MT::CMutex >( this )
    , fromSideTopicName( src.fromSideTopicName )
    , toSideTopicName( src.toSideTopicName )
    , failoverSideTopicName( src.failoverSideTopicName )
    , spilloverSideTopicName( src.spilloverSideTopicName )
    , deadLetterSideTopicName( src.deadLetterSideTopicName )
    , persistenceSideTopicName( src.persistenceSideTopicName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfig::~CPubSubFlowRouteTopicConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfig& 
CPubSubFlowRouteTopicConfig::operator=( const CPubSubFlowRouteTopicConfig& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIConfigurable::operator=( src ) ;
        fromSideTopicName = src.fromSideTopicName;
        toSideTopicName = src.toSideTopicName;
        failoverSideTopicName = src.failoverSideTopicName;
        spilloverSideTopicName = src.spilloverSideTopicName;
        deadLetterSideTopicName = src.deadLetterSideTopicName;
        persistenceSideTopicName = src.persistenceSideTopicName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouteTopicConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;

    totalSuccess = cfg.SetAttribute( "fromSideTopicName", fromSideTopicName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "toSideTopicName", toSideTopicName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "failoverSideTopicName", failoverSideTopicName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "spilloverSideTopicName", spilloverSideTopicName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "deadLetterSideTopicName", deadLetterSideTopicName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "persistenceSideTopicName", persistenceSideTopicName ) && totalSuccess;

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubFlowRouteTopicConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    bool totalSuccess = true;

    fromSideTopicName = cfg.GetAttributeValueOrChildValueByName( "fromSideTopicName" ).AsString( fromSideTopicName, true );
    toSideTopicName = cfg.GetAttributeValueOrChildValueByName( "toSideTopicName" ).AsString( toSideTopicName, true );
    failoverSideTopicName = cfg.GetAttributeValueOrChildValueByName( "failoverSideTopicName" ).AsString( failoverSideTopicName, true );
    spilloverSideTopicName = cfg.GetAttributeValueOrChildValueByName( "spilloverSideTopicName" ).AsString( spilloverSideTopicName, true );
    deadLetterSideTopicName = cfg.GetAttributeValueOrChildValueByName( "deadLetterSideTopicName" ).AsString( deadLetterSideTopicName, true );
    persistenceSideTopicName = cfg.GetAttributeValueOrChildValueByName( "persistenceSideTopicName" ).AsString( persistenceSideTopicName, true );

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPubSubFlowRouteTopicConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::CPubSubFlowRouteTopicConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteConfig::CPubSubFlowRouteConfig( void )
    : CORE::CIConfigurable() 
    , CORE::CTSharedObjCreator< CPubSubFlowRouteConfig, MT::CMutex >( this )
    , fromSideId()
    , toSideId()
    , failoverSideId()
    , spilloverBufferSideId()
    , deadLetterSideId()
    , persistenceSideId()
    , topicAssociations()
    , toSideTopicsAutoMatchFromSide( false )
    , failoverSideTopicsAutoMatchFromSide( false )
    , spilloverSideTopicsAutoMatchFromSide( false )
    , deadLetterSideTopicsAutoMatchFromSide( false )
    , persistenceSideTopicsAutoMatchFromSide( false )
    , preferFromTopicThreadForDestination( false )
    , egressAllDiscoveredSpilloverTopicsOnStart( true )
    , autoAssociateTopicsAnyToAnyAcrossSides( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteConfig::CPubSubFlowRouteConfig( const CPubSubFlowRouteConfig& src )
    : CORE::CIConfigurable( src ) 
    , CORE::CTSharedObjCreator< CPubSubFlowRouteConfig, MT::CMutex >( this )
    , fromSideId( src.fromSideId )
    , toSideId( src.toSideId )
    , failoverSideId( src.failoverSideId )
    , spilloverBufferSideId( src.spilloverBufferSideId )
    , deadLetterSideId( src.deadLetterSideId )
    , persistenceSideId( src.persistenceSideId )
    , topicAssociations( src.topicAssociations )
    , toSideTopicsAutoMatchFromSide( src.toSideTopicsAutoMatchFromSide )
    , failoverSideTopicsAutoMatchFromSide( src.failoverSideTopicsAutoMatchFromSide )
    , spilloverSideTopicsAutoMatchFromSide( src.spilloverSideTopicsAutoMatchFromSide )
    , deadLetterSideTopicsAutoMatchFromSide( src.deadLetterSideTopicsAutoMatchFromSide )
    , persistenceSideTopicsAutoMatchFromSide( src.persistenceSideTopicsAutoMatchFromSide )
    , preferFromTopicThreadForDestination( src.preferFromTopicThreadForDestination )
    , egressAllDiscoveredSpilloverTopicsOnStart( src.egressAllDiscoveredSpilloverTopicsOnStart )
    , autoAssociateTopicsAnyToAnyAcrossSides( src.autoAssociateTopicsAnyToAnyAcrossSides )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteConfig::~CPubSubFlowRouteConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteConfig& 
CPubSubFlowRouteConfig::operator=( const CPubSubFlowRouteConfig& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIConfigurable::operator=( src ) ;
        fromSideId = src.fromSideId;
        toSideId = src.toSideId;
        failoverSideId = src.failoverSideId;
        spilloverBufferSideId = src.spilloverBufferSideId;
        deadLetterSideId = src.deadLetterSideId;
        persistenceSideId = src.persistenceSideId;
        topicAssociations = src.topicAssociations;
        toSideTopicsAutoMatchFromSide = src.toSideTopicsAutoMatchFromSide;
        failoverSideTopicsAutoMatchFromSide = src.failoverSideTopicsAutoMatchFromSide;
        spilloverSideTopicsAutoMatchFromSide = src.spilloverSideTopicsAutoMatchFromSide;
        deadLetterSideTopicsAutoMatchFromSide = src.deadLetterSideTopicsAutoMatchFromSide;
        persistenceSideTopicsAutoMatchFromSide = src.persistenceSideTopicsAutoMatchFromSide;
        preferFromTopicThreadForDestination = src.preferFromTopicThreadForDestination;
        egressAllDiscoveredSpilloverTopicsOnStart = src.egressAllDiscoveredSpilloverTopicsOnStart;
        autoAssociateTopicsAnyToAnyAcrossSides = src.autoAssociateTopicsAnyToAnyAcrossSides;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPubSubFlowRouteConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "fromSideId", fromSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "toSideId", toSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "failoverSideId", failoverSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "spilloverBufferSideId", spilloverBufferSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "deadLetterSideId", deadLetterSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "persistenceSideId", persistenceSideId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "toSideTopicsAutoMatchFromSide", toSideTopicsAutoMatchFromSide ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "failoverSideTopicsAutoMatchFromSide", failoverSideTopicsAutoMatchFromSide ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "spilloverSideTopicsAutoMatchFromSide", spilloverSideTopicsAutoMatchFromSide ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "deadLetterSideTopicsAutoMatchFromSide", deadLetterSideTopicsAutoMatchFromSide ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "persistenceSideTopicsAutoMatchFromSide", persistenceSideTopicsAutoMatchFromSide ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "preferFromTopicThreadForDestination", preferFromTopicThreadForDestination ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "egressAllDiscoveredSpilloverTopicsOnStart", egressAllDiscoveredSpilloverTopicsOnStart ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "autoAssociateTopicsAnyToAnyAcrossSides", autoAssociateTopicsAnyToAnyAcrossSides ) && totalSuccess;
    
    CORE::CDataNode* topicAssociationsNode = cfg.FindOrAddChild( "topicAssociations" );
    if ( GUCEF_NULL != topicAssociationsNode )
    {
        topicAssociationsNode->DelSubTree();
        topicAssociationsNode->SetNodeType( GUCEF_DATATYPE_ARRAY );
        PubSubFlowRouteTopicConfigPtrVector::const_iterator i = topicAssociations.begin();
        while ( i != topicAssociations.end() )
        {
            const CPubSubFlowRouteTopicConfigPtr topicConfig = (*i);
            CORE::CDataNode* topicConfigNode = topicAssociationsNode->AddChild();
            totalSuccess = topicConfig->SaveConfig( *topicConfigNode ) && totalSuccess;
            ++i;
        }
    }    

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubFlowRouteConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    bool totalSuccess = true;

    fromSideId = cfg.GetAttributeValueOrChildValueByName( "fromSideId" ).AsString( fromSideId, true );
    toSideId = cfg.GetAttributeValueOrChildValueByName( "toSideId" ).AsString( toSideId, true );
    failoverSideId = cfg.GetAttributeValueOrChildValueByName( "failoverSideId" ).AsString( failoverSideId, true );
    spilloverBufferSideId = cfg.GetAttributeValueOrChildValueByName( "spilloverBufferSideId" ).AsString( spilloverBufferSideId, true );
    deadLetterSideId = cfg.GetAttributeValueOrChildValueByName( "deadLetterSideId" ).AsString( deadLetterSideId, true );
    persistenceSideId = cfg.GetAttributeValueOrChildValueByName( "persistenceSideId" ).AsString( persistenceSideId, true );
    toSideTopicsAutoMatchFromSide = cfg.GetAttributeValueOrChildValueByName( "toSideTopicsAutoMatchFromSide" ).AsBool( toSideTopicsAutoMatchFromSide, true );
    failoverSideTopicsAutoMatchFromSide = cfg.GetAttributeValueOrChildValueByName( "failoverSideTopicsAutoMatchFromSide" ).AsBool( failoverSideTopicsAutoMatchFromSide, true );
    spilloverSideTopicsAutoMatchFromSide = cfg.GetAttributeValueOrChildValueByName( "spilloverSideTopicsAutoMatchFromSide" ).AsBool( spilloverSideTopicsAutoMatchFromSide, true );
    deadLetterSideTopicsAutoMatchFromSide = cfg.GetAttributeValueOrChildValueByName( "deadLetterSideTopicsAutoMatchFromSide" ).AsBool( deadLetterSideTopicsAutoMatchFromSide, true );
    persistenceSideTopicsAutoMatchFromSide = cfg.GetAttributeValueOrChildValueByName( "persistenceSideTopicsAutoMatchFromSide" ).AsBool( persistenceSideTopicsAutoMatchFromSide, true );
    preferFromTopicThreadForDestination = cfg.GetAttributeValueOrChildValueByName( "preferFromTopicThreadForDestination" ).AsBool( preferFromTopicThreadForDestination, true );
    egressAllDiscoveredSpilloverTopicsOnStart = cfg.GetAttributeValueOrChildValueByName( "egressAllDiscoveredSpilloverTopicsOnStart" ).AsBool( egressAllDiscoveredSpilloverTopicsOnStart, true );
    autoAssociateTopicsAnyToAnyAcrossSides = cfg.GetAttributeValueOrChildValueByName( "autoAssociateTopicsAnyToAnyAcrossSides" ).AsBool( autoAssociateTopicsAnyToAnyAcrossSides, true );

    const CORE::CDataNode* topicAssociationsNode = cfg.FindChild( "topicAssociations" );
    if ( GUCEF_NULL != topicAssociationsNode )
    {
        CORE::CDataNode::const_iterator i = topicAssociationsNode->ConstBegin();
        while ( i != topicAssociationsNode->ConstEnd() )
        {
            CPubSubFlowRouteTopicConfigPtr topicConfig = CPubSubFlowRouteTopicConfig::CreateSharedObj();
            if ( topicConfig->LoadConfig( *(*i) ) )
            {
                topicAssociations.push_back( topicConfig );
            }
            else
            {
                totalSuccess = false;
            }            
            ++i;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CString& 
CPubSubFlowRouteConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PUBSUB::CPubSubFlowRouteConfig";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CPubSubFlowRouteConfig::IsAnyAutoTopicMatchingNeeded( void ) const
{GUCEF_TRACE;

    return toSideTopicsAutoMatchFromSide          ||
           failoverSideTopicsAutoMatchFromSide    ||
           spilloverSideTopicsAutoMatchFromSide   ||
           deadLetterSideTopicsAutoMatchFromSide  ||
           persistenceSideTopicsAutoMatchFromSide;
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfigPtr
CPubSubFlowRouteConfig::FindTopicAssociation( const CORE::CString& fromTopicName ,
                                              bool performPatternMatch           )
{GUCEF_TRACE;

    // first look for an exact match, those should take priority regardless
    PubSubFlowRouteTopicConfigPtrVector::iterator i = topicAssociations.begin();
    while ( i != topicAssociations.end() )
    {
        CPubSubFlowRouteTopicConfigPtr topicConfig = (*i);
        if ( topicConfig->fromSideTopicName == fromTopicName )
            return topicConfig;
        ++i;
    }

    if ( performPatternMatch )
    {
        // if there is no exact match the request is to try and pattern match next

        PubSubFlowRouteTopicConfigPtrVector::iterator i = topicAssociations.begin();
        while ( i != topicAssociations.end() )
        {
            CPubSubFlowRouteTopicConfigPtr topicConfig = (*i);
            if ( topicConfig->fromSideTopicName.WildcardEquals( fromTopicName, '*', true, true ) )
                return topicConfig;
            ++i;
        }
    }

    // Didnt find anything
    return CPubSubFlowRouteTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

CPubSubFlowRouteTopicConfigPtr
CPubSubFlowRouteConfig::FindOrCreateTopicAssociation( const CORE::CString& fromTopicName ,
                                                      bool performPatternMatch           )
{GUCEF_TRACE;

    CPubSubFlowRouteTopicConfigPtr topicRouteConfig = FindTopicAssociation( fromTopicName, performPatternMatch );
    if ( !performPatternMatch && !topicRouteConfig.IsNULL() )
        return topicRouteConfig;

    if ( IsAnyAutoTopicMatchingNeeded() )
    {
        if ( !topicRouteConfig.IsNULL() )
        {
            // is the one we have a template? if so treat it as such 
            if ( performPatternMatch                                     &&
                 ( topicRouteConfig->fromSideTopicName.HasChar( '*' ) > -1 ||
                   fromTopicName.HasChar( '*' ) > -1                        ) )
            {
                
                // Create the new association using the template format

                CPubSubFlowRouteTopicConfigPtr templateTopicRouteConfig = topicRouteConfig;
                topicRouteConfig = CPubSubFlowRouteTopicConfig::CreateSharedObj();
                if ( topicRouteConfig.IsNULL() )
                    return topicRouteConfig;

                topicRouteConfig->fromSideTopicName = fromTopicName;
                
                // we now copy the settings from the template since its possible that some target topics are explicitly
                // defined and not auto matching while some others are
                if ( toSideTopicsAutoMatchFromSide && !toSideId.IsNULLOrEmpty() )
                {
                    if( !templateTopicRouteConfig->toSideTopicName.IsNULLOrEmpty() && templateTopicRouteConfig->toSideTopicName != templateTopicRouteConfig->fromSideTopicName )
                        topicRouteConfig->toSideTopicName = templateTopicRouteConfig->toSideTopicName;    
                    else
                        topicRouteConfig->toSideTopicName = fromTopicName;
                }
                if ( failoverSideTopicsAutoMatchFromSide && !failoverSideId.IsNULLOrEmpty() )
                {
                    if( !templateTopicRouteConfig->failoverSideTopicName.IsNULLOrEmpty() && templateTopicRouteConfig->failoverSideTopicName != templateTopicRouteConfig->fromSideTopicName )
                        topicRouteConfig->failoverSideTopicName = templateTopicRouteConfig->failoverSideTopicName;    
                    else
                        topicRouteConfig->failoverSideTopicName = fromTopicName;
                }
                if ( spilloverSideTopicsAutoMatchFromSide && !spilloverBufferSideId.IsNULLOrEmpty() )
                {
                    if( !templateTopicRouteConfig->spilloverSideTopicName.IsNULLOrEmpty() && templateTopicRouteConfig->spilloverSideTopicName != templateTopicRouteConfig->fromSideTopicName )
                        topicRouteConfig->spilloverSideTopicName = templateTopicRouteConfig->spilloverSideTopicName;    
                    else
                        topicRouteConfig->spilloverSideTopicName = fromTopicName;
                }
                if ( deadLetterSideTopicsAutoMatchFromSide && !deadLetterSideId.IsNULLOrEmpty() )
                {
                    if( !templateTopicRouteConfig->deadLetterSideTopicName.IsNULLOrEmpty() && templateTopicRouteConfig->deadLetterSideTopicName != templateTopicRouteConfig->fromSideTopicName )
                        topicRouteConfig->deadLetterSideTopicName = templateTopicRouteConfig->deadLetterSideTopicName;
                    else
                        topicRouteConfig->deadLetterSideTopicName = fromTopicName;
                }
                if ( persistenceSideTopicsAutoMatchFromSide && !persistenceSideId.IsNULLOrEmpty() )
                {
                    if( !templateTopicRouteConfig->persistenceSideTopicName.IsNULLOrEmpty() && templateTopicRouteConfig->persistenceSideTopicName != templateTopicRouteConfig->fromSideTopicName )
                        topicRouteConfig->persistenceSideTopicName = templateTopicRouteConfig->persistenceSideTopicName;
                    else
                        topicRouteConfig->persistenceSideTopicName = fromTopicName;
                }

                topicAssociations.push_back( topicRouteConfig );

                return topicRouteConfig;
            }
        }

        // Create the new association using default format
        topicRouteConfig = CPubSubFlowRouteTopicConfig::CreateSharedObj();
        if ( !topicRouteConfig.IsNULL() )
        {
            topicRouteConfig->fromSideTopicName = fromTopicName;

            // Init with matching topic names as desired
            // Once initialized we respect the state as-is but at init the other config plays a role in the manner of initialization
        
            if ( toSideTopicsAutoMatchFromSide && !toSideId.IsNULLOrEmpty() && topicRouteConfig->toSideTopicName.IsNULLOrEmpty() )
                topicRouteConfig->toSideTopicName = fromTopicName;
            if ( failoverSideTopicsAutoMatchFromSide && !failoverSideId.IsNULLOrEmpty() && topicRouteConfig->failoverSideTopicName.IsNULLOrEmpty() )
                topicRouteConfig->failoverSideTopicName = fromTopicName;
            if ( spilloverSideTopicsAutoMatchFromSide && !spilloverBufferSideId.IsNULLOrEmpty() && topicRouteConfig->spilloverSideTopicName.IsNULLOrEmpty() )
                topicRouteConfig->spilloverSideTopicName = fromTopicName;
            if ( deadLetterSideTopicsAutoMatchFromSide && !deadLetterSideId.IsNULLOrEmpty() && topicRouteConfig->deadLetterSideTopicName.IsNULLOrEmpty() )
                topicRouteConfig->deadLetterSideTopicName = fromTopicName;
            if ( persistenceSideTopicsAutoMatchFromSide && !persistenceSideId.IsNULLOrEmpty() && topicRouteConfig->persistenceSideTopicName.IsNULLOrEmpty() )
                topicRouteConfig->persistenceSideTopicName = fromTopicName;

            topicAssociations.push_back( topicRouteConfig );
        }
    }

    return topicRouteConfig;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

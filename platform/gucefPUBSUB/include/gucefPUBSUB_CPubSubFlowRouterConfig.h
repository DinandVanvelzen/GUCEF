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

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H
#include "gucefPUBSUB_CPubSubFlowRouteConfig.h"
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTECONFIG_H ? */

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PUBSUB_EXPORT_CPP CPubSubFlowRouterConfig : public CORE::CIConfigurable
{
    public:

    typedef CPubSubFlowRouteConfig::PubSubFlowRouteConfigPtrVector     PubSubFlowRouteConfigPtrVector;

    enum AckStyle : Int32
    {
        AllOrNothing    = 0 ,   /**< all sides have to ack for successful ack (default) */
        FireAndForget   = 1 ,   /**< dont pass along any acks */
        AnyOneForAll    = 2	    /**< any ack counts as ack for all sides */
    };

    AckStyle ackStyle;
    PubSubFlowRouteConfigPtrVector routes;
    CORE::UInt32 minBadHealthDurationBeforeFailoverInMs;
    CORE::UInt32 minBadHealthDurationBeforeSpilloverInMs;
    CORE::UInt32 minPrimarySideGoodHealthDurationBeforeActivationInMs;
    CORE::UInt32 minFailoverSideGoodHealthDurationBeforeActivationInMs;
    CORE::UInt32 minSpilloverSideGoodHealthDurationBeforeActivationInMs;
    CORE::UInt32 routeSwitchingTimerIntervalInMs;

    static CORE::CString AckStyleToString( AckStyle ackStyle );
    static AckStyle StringToAckStyle( const CORE::CString& ackStyleStr );

    CPubSubFlowRouteConfigPtr FindRouteWithFromSideAndToSide( const CORE::CString& fromSideId , 
                                                              const CORE::CString& toSideId   ) const;

    void Clear( void );

    CPubSubFlowRouterConfig( void );

    CPubSubFlowRouterConfig( const CPubSubFlowRouterConfig& src );

    virtual ~CPubSubFlowRouterConfig();

    CPubSubFlowRouterConfig& operator=( const CPubSubFlowRouterConfig& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H ? */


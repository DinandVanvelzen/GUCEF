/*
 *  AssetWorkbenchLib: what a thread of a pool is asked to do for the connection to a remote asset store service, and what came of it
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

#include "AssetWorkbenchLib_CStoreConnectionTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoreConnectionTaskData::CStoreConnectionTaskData( void )
    : CORE::CICloneable()
    , kind( KIND_PAIR )
    , pairingEndpoint()
    , serviceType()
    , serviceName()
    , sessionName()
    , deviceName()
    , clientKind()
    , credentialsSlot()
    , code()
    , store( GUCEF_NULL )
    , timeoutInMs( 20000 )
    , lanDiscovery( true )
    , service()
    , done( false )
    , succeeded( false )
    , problem()
    , failure( 0 )
    , fingerprint()
    , transient( false )
    , serverName()
    , serverVersion()
    , info()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoreConnectionTaskData::CStoreConnectionTaskData( const CStoreConnectionTaskData& src )
    : CORE::CICloneable( src )
    , kind( src.kind )
    , pairingEndpoint( src.pairingEndpoint )
    , serviceType( src.serviceType )
    , serviceName( src.serviceName )
    , sessionName( src.sessionName )
    , deviceName( src.deviceName )
    , clientKind( src.clientKind )
    , credentialsSlot( src.credentialsSlot )
    , code( src.code )
    , store( src.store )
    , timeoutInMs( src.timeoutInMs )
    , lanDiscovery( src.lanDiscovery )
    , service( src.service )
    , done( src.done )
    , succeeded( src.succeeded )
    , problem( src.problem )
    , failure( src.failure )
    , fingerprint( src.fingerprint )
    , transient( src.transient )
    , serverName( src.serverName )
    , serverVersion( src.serverVersion )
    , info( src.info )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStoreConnectionTaskData::~CStoreConnectionTaskData()
{GUCEF_TRACE;

    code.Clear();
}

/*-------------------------------------------------------------------------*/

CStoreConnectionTaskData&
CStoreConnectionTaskData::operator=( const CStoreConnectionTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        kind = src.kind;
        pairingEndpoint = src.pairingEndpoint;
        serviceType = src.serviceType;
        serviceName = src.serviceName;
        sessionName = src.sessionName;
        deviceName = src.deviceName;
        clientKind = src.clientKind;
        credentialsSlot = src.credentialsSlot;
        code = src.code;
        store = src.store;
        timeoutInMs = src.timeoutInMs;
        lanDiscovery = src.lanDiscovery;
        service = src.service;
        done = src.done;
        succeeded = src.succeeded;
        problem = src.problem;
        failure = src.failure;
        fingerprint = src.fingerprint;
        transient = src.transient;
        serverName = src.serverName;
        serverVersion = src.serverVersion;
        info = src.info;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CStoreConnectionTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CStoreConnectionTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CStoreConnectionTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CStoreConnectionTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETWORKBENCH::CStoreConnectionTaskData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#define GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#include "gucefCOMCORE_CIDiscoveryClient.h"
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Makes the discovery clients a configuration asks for, through the discovery client factory of CComCoreGlobal (the
 *  built in types "broadcast", "multicast", "mdns" and "ssdp" are registered by gucefCOM):
 *
 *  - one client per "DiscoveryClient" child: its attribute "type" names the client, the whole child is the config of
 *    the client (port, extraTargets, the answer limits, ...);
 *  - else one per type in the comma separated attribute "discoveryClients";
 *  - else the default types the caller gives.
 *
 *  Types the factory does not know are skipped with a warning.
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryClientList
{
    public:

    typedef GUCEF::vector< CIDiscoveryClientPtr > TDiscoveryClientVector;

    static const char* ClientTypesKey;          /**< "discoveryClients" */
    static const char* ClientNodeName;          /**< "DiscoveryClient" */

    /**
     *  The client types a service uses when its config names none: "broadcast" and "multicast", the GUCEF beacon
     *  protocol whose answers are bounded against reflection. mDNS and SSDP are standard protocols with larger answers
     *  than questions; they are added by config where interoperability is wanted.
     */
    static void GetDefaultServiceClientTypes( CORE::CString::StringVector& types );

    /** Whether the config names clients itself, with children or with the type list */
    static bool NamesClients( const CORE::CDataNode& config );

    static bool CreateFromConfig( const CORE::CDataNode& config                 ,
                                  const CORE::CString::StringVector& defaultTypes ,
                                  TDiscoveryClientVector& clients               );

    static bool CreateFromTypes( const CORE::CString::StringVector& types ,
                                 TDiscoveryClientVector& clients          );

    /** Copies what names the clients (the type list and the "DiscoveryClient" children) from one node to another */
    static void CopyClientConfig( const CORE::CDataNode& from, CORE::CDataNode& to );

    /** Splits a comma separated type list, trimmed, without empty entries */
    static void ParseTypes( const CORE::CString& typeList, CORE::CString::StringVector& types );

    /** The types as a comma separated list */
    static CORE::CString JoinTypes( const CORE::CString::StringVector& types );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H ? */

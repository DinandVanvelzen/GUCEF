/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_PROTOCOL_H
#define GUCEF_LABLINK_PROTOCOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CIPV4INFO_H
#include "gucefCOMCORE_CIPv4Info.h"
#define GUCEF_COMCORE_CIPV4INFO_H
#endif /* GUCEF_COMCORE_CIPV4INFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The DNS-SD service type the cluster advertises */
extern const char* ServiceType;

/** The DRN sessions of the two endpoints */
extern const char* LabSessionName;
extern const char* AccessSessionName;

/** The ports of the two endpoints */
static const GUCEF::UInt16 DefaultLabPort = 47810;
static const GUCEF::UInt16 DefaultAccessPort = 47811;

/** The contracts and the paths of their methods */
extern const char* LabContractName;
extern const char* AccessContractName;
extern const char* HelloPath;
extern const char* JoinPath;
extern const char* ClientConfigPath;
extern const char* RequestAccessPath;

/** Attributes of the discovery entry */
extern const char* AttributeCluster;         /**< the name of the cluster */
extern const char* AttributeKey;             /**< the public key of the lab endpoint, the form DRN pins */
extern const char* AttributeLabPort;
extern const char* AttributeAccessPort;

/** Roles in the tokens of the lab endpoint */
extern const char* RoleLab;                  /**< a token made with the lab secret */
extern const char* RoleApproved;             /**< a token the cluster gave an approved machine */

/** What RequestAccess answers */
extern const char* AccessPending;
extern const char* AccessApproved;
extern const char* AccessRefused;

/** Seconds a token made with the lab secret is valid, a token of an approved machine */
static const GUCEF::UInt64 LabTokenLifetimeInSecs = 600;
static const GUCEF::UInt64 ApprovedTokenLifetimeInSecs = 365 * 24 * 3600;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The contract of the lab endpoint, gucef.lab 1.0, service Lab:
 *    Hello( HelloRequest{} ) -> HelloResponse{ 1 cluster, 2 k3sVersion, 3 proof }
 *    Join( JoinRequest{ 1 name, 2 os, 3 arch, 4 ephemeral } ) -> JoinResponse{ 1 server, 2 k3sVersion, 3 token, 4 ephemeral }
 *    ClientConfig( ClientConfigRequest{} ) -> ClientConfigResponse{ 1 caCertificate, 2 mcpToken, 3 nodes (repeated "name url") }
 */
GUCEF::COMCORE::CRpcContractPtr BuildLabContract( GUCEF::CORE::CString& error );

/**
 *  The contract of the access endpoint, gucef.lab.access 1.0, service Access:
 *    RequestAccess( AccessRequest{ 1 machineId, 2 name, 3 os } ) -> AccessResponse{ 1 state, 2 token, 3 clusterKey, 4 cluster }
 */
GUCEF::COMCORE::CRpcContractPtr BuildAccessContract( GUCEF::CORE::CString& error );

/** A new lab secret: 32 random bytes as hex. Empty without randomness */
GUCEF::CORE::CString MakeLabSecret( void );

/** The key of the tokens of the lab endpoint, derived from the lab secret */
GUCEF::CORE::CString DeriveTokenKey( const GUCEF::CORE::CString& labSecret );

/** What the cluster answers Hello with: proves it has the lab secret, bound to the public key of its lab endpoint */
GUCEF::CORE::CString MakeClusterProof( const GUCEF::CORE::CString& labSecret, const GUCEF::CORE::CString& clusterKey );

/** The text without white space at either end: spaces, tabs and line ends */
GUCEF::CORE::CString StripText( const GUCEF::CORE::CString& text );

/** Compares in time independent of where they differ */
bool EqualInConstantTime( const GUCEF::CORE::CString& a, const GUCEF::CORE::CString& b );

/** A machine id: 16 random bytes as hex */
GUCEF::CORE::CString MakeMachineId( void );

/** Address ranges in CIDR notation, "172.16.1.0/24". An empty list allows every address */
class CAddressRanges
{
    public:

    /** @return false when an entry is not CIDR notation; problem names it */
    bool Set( const GUCEF::CORE::CString::StringVector& ranges, GUCEF::CORE::CString& problem );

    /** Whether the dotted IPv4 address is in one of the ranges */
    bool Allows( const GUCEF::CORE::CString& address ) const;

    bool IsEmpty( void ) const;

    GUCEF::CORE::CString Describe( void ) const;

    private:

    GUCEF::vector< GUCEF::COMCORE::CIPv4Info > m_ranges;
    GUCEF::CORE::CString::StringVector m_texts;
};

/**
 *  Runs a program to its end without a shell and gives its output, standard output and error together
 *  @return false when it could not start; exitCode is that of the program, -1 when it was stopped at the timeout
 */
bool RunProgram( const GUCEF::CORE::CString& exe                         ,
                 const GUCEF::CORE::CProcess::TStringVector& args        ,
                 const GUCEF::CORE::CProcess::TStringMap& environment    ,
                 GUCEF::UInt32 timeoutInMs                               ,
                 std::string& output                                     ,
                 GUCEF::Int32& exitCode                                  );

/** The value of "key=" in the lines of the text, empty when it is not there */
GUCEF::CORE::CString ValueOfLine( const std::string& text, const char* key );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_PROTOCOL_H ? */

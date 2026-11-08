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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefLabLink_Protocol.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* ServiceType = "_gucef-lab._tcp";
const char* LabSessionName = "gucef-lab";
const char* AccessSessionName = "gucef-lab-access";
const char* LabContractName = "gucef.lab";
const char* AccessContractName = "gucef.lab.access";
const char* HelloPath = "gucef.lab.Lab/Hello";
const char* JoinPath = "gucef.lab.Lab/Join";
const char* ClientConfigPath = "gucef.lab.Lab/ClientConfig";
const char* RequestAccessPath = "gucef.lab.access.Access/RequestAccess";
const char* AttributeCluster = "cluster";
const char* AttributeKey = "key";
const char* AttributeLabPort = "labPort";
const char* AttributeAccessPort = "accessPort";
const char* RoleLab = "lab";
const char* RoleApproved = "approved";
const char* AccessPending = "pending";
const char* AccessApproved = "approved";
const char* AccessRefused = "refused";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
BuildLabContract( CORE::CString& error )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder b( LabContractName, 1, 0 );
    b.AddType( "HelloRequest" );
    b.AddType( "HelloResponse" );
    b.AddField( "HelloResponse", 1, "cluster", "string" );
    b.AddField( "HelloResponse", 2, "k3sVersion", "string" );
    b.AddField( "HelloResponse", 3, "proof", "string" );
    b.AddType( "JoinRequest" );
    b.AddField( "JoinRequest", 1, "name", "string" );
    b.AddField( "JoinRequest", 2, "os", "string" );
    b.AddField( "JoinRequest", 3, "arch", "string" );
    b.AddField( "JoinRequest", 4, "ephemeral", "bool" );
    b.AddType( "JoinResponse" );
    b.AddField( "JoinResponse", 1, "server", "string" );
    b.AddField( "JoinResponse", 2, "k3sVersion", "string" );
    b.AddField( "JoinResponse", 3, "token", "string" );
    b.AddField( "JoinResponse", 4, "ephemeral", "bool" );
    b.AddType( "ClientConfigRequest" );
    b.AddType( "ClientConfigResponse" );
    b.AddField( "ClientConfigResponse", 1, "caCertificate", "string" );
    b.AddField( "ClientConfigResponse", 2, "mcpToken", "string" );
    b.AddField( "ClientConfigResponse", 3, "nodes", "string", COMCORE::RPC_CARDINALITY_REPEATED );
    b.AddService( "Lab", 1 );
    b.AddMethod( "Lab", 1, "Hello", COMCORE::RPC_METHODKIND_UNARY, "HelloRequest", "HelloResponse" );
    b.AddMethod( "Lab", 2, "Join", COMCORE::RPC_METHODKIND_UNARY, "JoinRequest", "JoinResponse" );
    b.AddMethod( "Lab", 3, "ClientConfig", COMCORE::RPC_METHODKIND_UNARY, "ClientConfigRequest", "ClientConfigResponse" );
    return b.Build( error );
}

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
BuildAccessContract( CORE::CString& error )
{GUCEF_TRACE;

    COMCORE::CRpcContractBuilder b( AccessContractName, 1, 0 );
    b.AddType( "AccessRequest" );
    b.AddField( "AccessRequest", 1, "machineId", "string" );
    b.AddField( "AccessRequest", 2, "name", "string" );
    b.AddField( "AccessRequest", 3, "os", "string" );
    b.AddType( "AccessResponse" );
    b.AddField( "AccessResponse", 1, "state", "string" );
    b.AddField( "AccessResponse", 2, "token", "string" );
    b.AddField( "AccessResponse", 3, "clusterKey", "string" );
    b.AddField( "AccessResponse", 4, "cluster", "string" );
    b.AddService( "Access", 1 );
    b.AddMethod( "Access", 1, "RequestAccess", COMCORE::RPC_METHODKIND_UNARY, "AccessRequest", "AccessResponse" );
    return b.Build( error );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
RandomHex( UInt32 bytes )
{GUCEF_TRACE;

    UInt8 buffer[ 64 ];
    if ( bytes > sizeof( buffer ) || !DRN::CDRNCrypto::RandomBytes( buffer, bytes ) )
        return CORE::CString::Empty;
    return DRN::CDRNCrypto::ToHex( buffer, bytes );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeLabSecret( void )
{GUCEF_TRACE;

    return RandomHex( 32 );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeMachineId( void )
{GUCEF_TRACE;

    return RandomHex( 16 );
}

/*-------------------------------------------------------------------------*/

/** HKDF-SHA256 of the secret with the label and the data, as hex */
static CORE::CString
Derive( const CORE::CString& labSecret, const char* label, const CORE::CString& data )
{GUCEF_TRACE;

    const CORE::CString info = CORE::CString( label ) + "|" + data;
    UInt8 okm[ 32 ];
    const char* salt = "gucef-lab";
    if ( !DRN::CDRNCrypto::Hkdf( salt, (UInt32) strlen( salt ), labSecret.C_String(), labSecret.Length(), info.C_String(), info.Length(), okm, sizeof( okm ) ) )
        return CORE::CString::Empty;
    return DRN::CDRNCrypto::ToHex( okm, sizeof( okm ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
DeriveTokenKey( const CORE::CString& labSecret )
{GUCEF_TRACE;

    return Derive( labSecret, "token-key", CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeClusterProof( const CORE::CString& labSecret, const CORE::CString& clusterKey )
{GUCEF_TRACE;

    return Derive( labSecret, "cluster-proof", clusterKey );
}

/*-------------------------------------------------------------------------*/

CORE::CString
StripText( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 start = 0;
    UInt32 end = text.Length();
    while ( start < end && ::strchr( " \t\r\n", text[ start ] ) != GUCEF_NULL && '\0' != text[ start ] )
        ++start;
    while ( end > start && ::strchr( " \t\r\n", text[ end - 1 ] ) != GUCEF_NULL && '\0' != text[ end - 1 ] )
        --end;
    return start == 0 && end == text.Length() ? text : CORE::CString( text.C_String() + start, end - start );
}

/*-------------------------------------------------------------------------*/

bool
EqualInConstantTime( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    if ( a.Length() != b.Length() )
        return false;
    UInt8 difference = 0;
    for ( UInt32 i=0; i<a.Length(); ++i )
        difference |= (UInt8) ( a.C_String()[ i ] ^ b.C_String()[ i ] );
    return 0 == difference;
}

/*-------------------------------------------------------------------------*/

bool
CAddressRanges::Set( const CORE::CString::StringVector& ranges, CORE::CString& problem )
{GUCEF_TRACE;

    m_ranges.clear();
    m_texts.clear();
    for ( size_t i=0; i<ranges.size(); ++i )
    {
        const CORE::CString text = StripText( ranges[ i ] );
        if ( text.IsNULLOrEmpty() )
            continue;
        COMCORE::CIPv4Info range;
        if ( !range.TrySetFromCIDRNotationString( text ) )
        {
            problem = "\"" + text + "\" is not an address range in CIDR notation, such as 172.16.1.0/24";
            return false;
        }
        m_ranges.push_back( range );
        m_texts.push_back( text );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAddressRanges::Allows( const CORE::CString& address ) const
{GUCEF_TRACE;

    if ( m_ranges.empty() )
        return true;
    COMCORE::CIPv4Address ip;
    if ( !ip.SetAddress( address ) )
        return false;
    for ( size_t i=0; i<m_ranges.size(); ++i )
        if ( m_ranges[ i ].Contains( ip ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAddressRanges::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_ranges.empty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAddressRanges::Describe( void ) const
{GUCEF_TRACE;

    if ( m_texts.empty() )
        return "every address";
    CORE::CString text;
    for ( size_t i=0; i<m_texts.size(); ++i )
        text += ( i > 0 ? ", " : "" ) + m_texts[ i ];
    return text;
}

/*-------------------------------------------------------------------------*/

bool
RunProgram( const CORE::CString& exe                        ,
            const CORE::CProcess::TStringVector& args       ,
            const CORE::CProcess::TStringMap& environment   ,
            UInt32 timeoutInMs                              ,
            std::string& output                             ,
            Int32& exitCode                                 )
{GUCEF_TRACE;

    output.clear();
    exitCode = -1;
    CORE::CProcess process;
    if ( !process.LaunchWithPipes( exe, args, true, true, true, CORE::CString::Empty, environment ) )
        return false;
    process.CloseStdinPipe();

    CORE::CProcessPipe* pipes[ 2 ] = { process.GetStdoutPipe(), process.GetStderrPipe() };
    bool open[ 2 ] = { GUCEF_NULL != pipes[ 0 ], GUCEF_NULL != pipes[ 1 ] };
    const UInt64 start = MT::PrecisionTickCount();
    const UInt64 resolution = MT::PrecisionTimerResolution();
    char buffer[ 4096 ];
    while ( open[ 0 ] || open[ 1 ] )
    {
        bool waited = false;
        for ( int i=0; i<2; ++i )
        {
            if ( !open[ i ] )
                continue;
            if ( pipes[ i ]->WaitForData( waited ? 0 : 100 ) )
            {
                const UInt32 got = pipes[ i ]->ReadSome( buffer, sizeof( buffer ) );
                if ( 0 == got )
                    open[ i ] = false;
                else
                    output.append( buffer, got );
            }
            waited = true;
        }
        if ( 0 != resolution && ( MT::PrecisionTickCount() - start ) * 1000 / resolution > timeoutInMs )
        {
            process.Terminate( true, 5000 );
            output += "\n(stopped after " + std::string( CORE::ToString( timeoutInMs / 1000 ).C_String() ) + " seconds)";
            return true;
        }
    }
    if ( !process.WaitForExit( 10000, exitCode ) )
    {
        process.Terminate( false, 0 );
        exitCode = -1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ValueOfLine( const std::string& text, const char* key )
{GUCEF_TRACE;

    const std::string prefix = std::string( key ) + "=";
    size_t start = 0;
    while ( start < text.size() )
    {
        size_t end = text.find( '\n', start );
        if ( std::string::npos == end )
            end = text.size();
        std::string line = text.substr( start, end - start );
        if ( !line.empty() && '\r' == line[ line.size() - 1 ] )
            line.erase( line.size() - 1 );
        if ( 0 == line.compare( 0, prefix.size(), prefix ) )
            return CORE::CString( line.substr( prefix.size() ).c_str() );
        start = end + 1;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

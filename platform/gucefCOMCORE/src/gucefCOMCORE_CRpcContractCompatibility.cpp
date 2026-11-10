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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOMCORE_CRpcContractCompatibility.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

Int32
SignedRank( TRpcFieldType t )
{
    switch ( t )
    {
        case RPC_FIELDTYPE_INT8: return 1;
        case RPC_FIELDTYPE_INT16: return 2;
        case RPC_FIELDTYPE_INT32: return 3;
        case RPC_FIELDTYPE_INT64: return 4;
        default: return 0;
    }
}

Int32
UnsignedRank( TRpcFieldType t )
{
    switch ( t )
    {
        case RPC_FIELDTYPE_UINT8: return 1;
        case RPC_FIELDTYPE_UINT16: return 2;
        case RPC_FIELDTYPE_UINT32: return 3;
        case RPC_FIELDTYPE_UINT64: return 4;
        default: return 0;
    }
}

class CChecker
{
    public:

    CChecker( const CRpcContract& older, const CRpcContract& newer, CRpcCompatibilityReport& report )
        : m_older( older ), m_newer( newer ), m_report( report ), m_sameMajor( older.GetMajor() == newer.GetMajor() ),
          m_newMinor( newer.GetMinor() > older.GetMinor() )
    {}

    /** A change that is allowed within the major */
    void Note( const CORE::CString& path, const char* rule, const CORE::CString& message )
    {
        m_report.Add( path, rule, message, false );
    }

    /** A change that breaks within the major; allowed (as a warning) across majors */
    void Break( const CORE::CString& path, const char* rule, const CORE::CString& message )
    {
        m_report.Add( path, rule, message, m_sameMajor );
    }

    void CheckSince( const CORE::CString& path, UInt32 since )
    {
        // an element new to this step was added in one of the minors after the older one, up to the newer one
        if ( m_sameMajor && ( since <= m_older.GetMinor() || since > m_newer.GetMinor() ) )
            Break( path, "SINCE_MISMATCH", "new since minor " + CORE::ToString( m_older.GetMinor() ) + " but marked since " + CORE::ToString( since ) );
    }

    void Run( void )
    {
        if ( m_older.GetName() != m_newer.GetName() )
        {
            m_report.Add( "contract", "NAME_CHANGED", "\"" + m_older.GetName() + "\" -> \"" + m_newer.GetName() + "\"", true );
            return;
        }
        if ( !m_sameMajor )
            Note( "version", "MAJOR_CHANGED", CORE::ToString( m_older.GetMajor() ) + " -> " + CORE::ToString( m_newer.GetMajor() ) );
        else if ( m_newer.GetMinor() < m_older.GetMinor() )
            Break( "version", "MINOR_LOWERED", CORE::ToString( m_older.GetMinor() ) + " -> " + CORE::ToString( m_newer.GetMinor() ) );
        if ( m_older.GetDoc() != m_newer.GetDoc() )
            Note( "doc", "DOC_CHANGED", "documentation changed" );

        CheckEnums();
        CheckTypes();
        CheckServices();
    }

    void CheckEnums( void )
    {
        for ( UInt32 i=0; i<m_older.GetEnumCount(); ++i )
        {
            const CRpcEnumDescriptor& o = m_older.GetEnumAt( i );
            CORE::CString path = "enums." + o.GetName();
            const CRpcEnumDescriptor* n = m_newer.FindEnum( o.GetName() );
            if ( GUCEF_NULL == n )
            {
                Break( path, "REMOVED", "enum removed" );
                continue;
            }
            for ( UInt32 v=0; v<o.GetValueCount(); ++v )
            {
                const CRpcEnumDescriptor::SValue& ov = o.GetValueAt( v );
                CORE::CString valuePath = path + ".values." + ov.name;
                const CRpcEnumDescriptor::SValue* byNumber = n->FindValueByNumber( ov.number );
                const CRpcEnumDescriptor::SValue* byName = n->FindValueByName( ov.name );
                if ( GUCEF_NULL == byNumber && GUCEF_NULL == byName )
                    Break( valuePath, "REMOVED", "enum value removed" );
                else if ( GUCEF_NULL == byNumber )
                    Break( valuePath, "RENUMBERED", "enum value renumbered" );
                else if ( byNumber->name != ov.name )
                    Break( valuePath, "RENAMED", "enum value " + CORE::ToString( ov.number ) + " renamed to " + byNumber->name );
            }
            for ( UInt32 v=0; v<n->GetValueCount(); ++v )
            {
                const CRpcEnumDescriptor::SValue& nv = n->GetValueAt( v );
                if ( GUCEF_NULL != o.FindValueByNumber( nv.number ) || GUCEF_NULL != o.FindValueByName( nv.name ) )
                    continue;
                CORE::CString valuePath = path + ".values." + nv.name;
                if ( o.GetReservedNumbers().find( nv.number ) != o.GetReservedNumbers().end() ||
                     o.GetReservedNames().find( nv.name ) != o.GetReservedNames().end() )
                    Break( valuePath, "RESERVED_REUSED", "uses a reserved number or name" );
                CheckSince( valuePath, nv.since );
                Note( valuePath, "ADDED", "enum value added" );
            }
        }
        for ( UInt32 i=0; i<m_newer.GetEnumCount(); ++i )
            if ( GUCEF_NULL == m_older.FindEnum( m_newer.GetEnumAt( i ).GetName() ) )
                Note( "enums." + m_newer.GetEnumAt( i ).GetName(), "ADDED", "enum added" );
    }

    void CheckField( const CORE::CString& path, const CRpcFieldDescriptor& o, const CRpcFieldDescriptor& n )
    {
        if ( o.GetName() != n.GetName() )
            Break( path, "RENAMED", "renamed to " + n.GetName() );

        bool sameType = o.GetType() == n.GetType() && o.GetTypeRef() == n.GetTypeRef() && o.GetDecimalScale() == n.GetDecimalScale();
        if ( !sameType )
        {
            if ( CRpcContractCompatibility::IsWireCompatibleWidening( o.GetType(), n.GetType() ) )
                Note( path, "WIDENED", CORE::CString( RpcFieldTypeToString( o.GetType() ) ) + " -> " + RpcFieldTypeToString( n.GetType() ) );
            else
                Break( path, "TYPE_CHANGED", CORE::CString( RpcFieldTypeToString( o.GetType() ) ) + " -> " + RpcFieldTypeToString( n.GetType() ) );
        }
        if ( o.IsMap() && n.IsMap() && o.GetMapKeyType() != n.GetMapKeyType() &&
             !CRpcContractCompatibility::IsWireCompatibleWidening( o.GetMapKeyType(), n.GetMapKeyType() ) )
            Break( path, "TYPE_CHANGED", "map key type changed" );
        if ( o.GetCardinality() != n.GetCardinality() )
        {
            if ( o.IsSingular() && n.IsSingular() )
                Note( path, "OPTIONALITY_CHANGED", "single <-> optional" );
            else
                Break( path, "CARDINALITY_CHANGED", "cardinality changed" );
        }
        if ( !o.IsRequired() && n.IsRequired() )
            Break( path, "REQUIRED_ADDED", "made required" );
        if ( o.GetFlags() != n.GetFlags() )
            Note( path, "FLAGS_CHANGED", "deprecated, sensitive or required flag changed" );
        if ( o.GetDoc() != n.GetDoc() )
            Note( path, "DOC_CHANGED", "documentation changed" );
    }

    void CheckTypes( void )
    {
        for ( UInt32 i=0; i<m_older.GetTypeCount(); ++i )
        {
            const CRpcTypeDescriptor& o = m_older.GetTypeAt( i );
            CORE::CString path = "types." + o.GetName();
            const CRpcTypeDescriptor* n = m_newer.FindType( o.GetName() );
            if ( GUCEF_NULL == n )
            {
                Break( path, "REMOVED", "type removed" );
                continue;
            }
            for ( UInt32 f=0; f<o.GetFieldCount(); ++f )
            {
                const CRpcFieldDescriptor& of = o.GetFieldAt( f );
                CORE::CString fieldPath = path + ".fields." + of.GetName();
                const CRpcFieldDescriptor* nf = n->FindFieldByNumber( of.GetNumber() );
                if ( GUCEF_NULL == nf )
                {
                    if ( GUCEF_NULL != n->FindFieldByName( of.GetName() ) )
                        Break( fieldPath, "RENUMBERED", "field renumbered" );
                    else
                        Break( fieldPath, "REMOVED", "field removed" );
                    continue;
                }
                CheckField( fieldPath, of, *nf );
            }
            for ( UInt32 f=0; f<n->GetFieldCount(); ++f )
            {
                const CRpcFieldDescriptor& nf = n->GetFieldAt( f );
                if ( GUCEF_NULL != o.FindFieldByNumber( nf.GetNumber() ) )
                    continue;
                CORE::CString fieldPath = path + ".fields." + nf.GetName();
                if ( GUCEF_NULL != o.FindFieldByName( nf.GetName() ) )
                    continue;   // reported as renumbered above
                if ( o.GetReservedNumbers().find( nf.GetNumber() ) != o.GetReservedNumbers().end() ||
                     o.GetReservedNames().find( nf.GetName() ) != o.GetReservedNames().end() )
                    Break( fieldPath, "RESERVED_REUSED", "uses a reserved number or name" );
                if ( nf.IsRequired() )
                    Break( fieldPath, "REQUIRED_ADDED", "a required field was added" );
                CheckSince( fieldPath, nf.GetSince() );
                Note( fieldPath, "ADDED", "field added" );
            }
        }
        for ( UInt32 i=0; i<m_newer.GetTypeCount(); ++i )
            if ( GUCEF_NULL == m_older.FindType( m_newer.GetTypeAt( i ).GetName() ) )
                Note( "types." + m_newer.GetTypeAt( i ).GetName(), "ADDED", "type added" );
    }

    void CheckMethod( const CORE::CString& path, const CRpcMethodDescriptor& o, const CRpcMethodDescriptor& n )
    {
        if ( o.GetName() != n.GetName() )
            Break( path, "RENAMED", "renamed to " + n.GetName() );
        if ( o.GetKind() != n.GetKind() )
            Break( path, "KIND_CHANGED", CORE::CString( RpcMethodKindToString( o.GetKind() ) ) + " -> " + RpcMethodKindToString( n.GetKind() ) );
        if ( o.GetRequestTypeName() != n.GetRequestTypeName() || o.GetResponseTypeName() != n.GetResponseTypeName() )
            Break( path, "TYPE_CHANGED", "request or response type changed" );
        const UInt32 semanticFlags = GUCEF_RPC_METHODFLAG_IDEMPOTENT | GUCEF_RPC_METHODFLAG_MAYANSWERASYNC | GUCEF_RPC_METHODFLAG_RESUMABLESTREAM;
        if ( ( o.GetFlags() & semanticFlags ) != ( n.GetFlags() & semanticFlags ) )
            Break( path, "FLAG_CHANGED", "idempotent, mayAnswerAsync or resumableStream changed" );
        else if ( o.GetFlags() != n.GetFlags() )
            Note( path, "FLAGS_CHANGED", "sensitive flag changed" );
        if ( RPC_PROTECTION_CONFIDENTIAL == o.GetProtection() && RPC_PROTECTION_PUBLIC == n.GetProtection() )
            Break( path, "PROTECTION_WEAKENED", "protection confidential -> public" );
        if ( RPC_PROTECTION_CONFIDENTIAL == o.GetRequestProtection() && RPC_PROTECTION_PUBLIC == n.GetRequestProtection() )
            Break( path, "PROTECTION_WEAKENED", "requestProtection confidential -> public" );
        if ( o.GetProtection() != n.GetProtection() || o.GetRequestProtection() != n.GetRequestProtection() )
            Note( path, "PROTECTION_CHANGED", "protection class changed" );
        if ( o.GetDefaultDeadlineMs() != n.GetDefaultDeadlineMs() )
            Note( path, "DEADLINE_CHANGED", "default deadline changed" );
        if ( o.GetInitialCredit() != n.GetInitialCredit() )
            Note( path, "CREDIT_CHANGED", "initial credit changed" );
    }

    void CheckServices( void )
    {
        for ( UInt32 i=0; i<m_older.GetServiceCount(); ++i )
        {
            const CRpcServiceDescriptor& o = m_older.GetServiceAt( i );
            CORE::CString path = "services." + o.GetName();
            const CRpcServiceDescriptor* n = m_newer.FindServiceByNumber( o.GetNumber() );
            if ( GUCEF_NULL == n )
            {
                if ( GUCEF_NULL != m_newer.FindService( o.GetName() ) )
                    Break( path, "RENUMBERED", "service renumbered" );
                else
                    Break( path, "REMOVED", "service removed" );
                continue;
            }
            if ( n->GetName() != o.GetName() )
                Break( path, "RENAMED", "renamed to " + n->GetName() );
            for ( UInt32 m=0; m<o.GetMethodCount(); ++m )
            {
                const CRpcMethodDescriptor& om = o.GetMethodAt( m );
                CORE::CString methodPath = path + ".methods." + om.GetName();
                const CRpcMethodDescriptor* nm = n->FindMethodByNumber( om.GetNumber() );
                if ( GUCEF_NULL == nm )
                {
                    if ( GUCEF_NULL != n->FindMethodByName( om.GetName() ) )
                        Break( methodPath, "RENUMBERED", "method renumbered" );
                    else
                        Break( methodPath, "REMOVED", "method removed" );
                    continue;
                }
                CheckMethod( methodPath, om, *nm );
            }
            for ( UInt32 m=0; m<n->GetMethodCount(); ++m )
            {
                const CRpcMethodDescriptor& nm = n->GetMethodAt( m );
                if ( GUCEF_NULL != o.FindMethodByNumber( nm.GetNumber() ) || GUCEF_NULL != o.FindMethodByName( nm.GetName() ) )
                    continue;
                CORE::CString methodPath = path + ".methods." + nm.GetName();
                if ( o.GetReservedNumbers().find( nm.GetNumber() ) != o.GetReservedNumbers().end() )
                    Break( methodPath, "RESERVED_REUSED", "uses a reserved number" );
                CheckSince( methodPath, nm.GetSince() );
                Note( methodPath, "ADDED", "method added" );
            }
        }
        for ( UInt32 i=0; i<m_newer.GetServiceCount(); ++i )
        {
            const CRpcServiceDescriptor& n = m_newer.GetServiceAt( i );
            if ( GUCEF_NULL != m_older.FindServiceByNumber( n.GetNumber() ) || GUCEF_NULL != m_older.FindService( n.GetName() ) )
                continue;
            CheckSince( "services." + n.GetName(), n.GetSince() );
            Note( "services." + n.GetName(), "ADDED", "service added" );
        }
    }

    private:

    const CRpcContract& m_older;
    const CRpcContract& m_newer;
    CRpcCompatibilityReport& m_report;
    bool m_sameMajor;
    bool m_newMinor;
};

}

/*-------------------------------------------------------------------------*/

bool
CRpcContractCompatibility::IsWireCompatibleWidening( TRpcFieldType from, TRpcFieldType to )
{GUCEF_TRACE;

    if ( from == to )
        return true;
    Int32 fs = SignedRank( from );
    Int32 ts = SignedRank( to );
    if ( fs > 0 && ts > 0 )
        return ts >= fs;
    Int32 fu = UnsignedRank( from );
    Int32 tu = UnsignedRank( to );
    if ( fu > 0 && tu > 0 )
        return tu >= fu;
    return false;
}

/*-------------------------------------------------------------------------*/

CRpcContractCompatibility::TResult
CRpcContractCompatibility::Check( const CRpcContract& older, const CRpcContract& newer, CRpcCompatibilityReport& report )
{GUCEF_TRACE;

    report.Clear();
    CChecker checker( older, newer, report );
    checker.Run();
    if ( report.GetBreakingCount() > 0 )
        return RESULT_BREAKING;
    for ( UInt32 i=0; i<report.GetEntries().size(); ++i )
    {
        const CORE::CString& rule = report.GetEntries()[ i ].ruleId;
        if ( "ADDED" != rule && "DOC_CHANGED" != rule )
            return RESULT_COMPATIBLE_WITH_WARNINGS;
    }
    return RESULT_COMPATIBLE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

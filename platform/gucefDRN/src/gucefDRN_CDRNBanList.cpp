/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNBanList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNBanList::ClassTypeName = "GUCEF::DRN::CDRNBanList";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Writes a field of a ban list line: tabs, line breaks and backslashes escaped */
static CORE::CString
EscapeField( const CORE::CString& field )
{GUCEF_TRACE;

    CORE::CString result;
    const char* text = field.C_String();
    UInt32 size = field.ByteSize();
    char single[ 2 ] = { 0, 0 };
    for ( UInt32 i=0; i<size && '\0' != text[ i ]; ++i )
    {
        switch ( text[ i ] )
        {
            case '\\':  result += "\\\\"; break;
            case '\t':  result += "\\t"; break;
            case '\n':  result += "\\n"; break;
            case '\r':  result += "\\r"; break;
            default:
            {
                single[ 0 ] = text[ i ];
                result += single;
                break;
            }
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
UnescapeField( const CORE::CString& field )
{GUCEF_TRACE;

    CORE::CString result;
    const char* text = field.C_String();
    UInt32 size = field.ByteSize();
    char single[ 2 ] = { 0, 0 };
    for ( UInt32 i=0; i<size && '\0' != text[ i ]; ++i )
    {
        if ( '\\' == text[ i ] && i+1 < size && '\0' != text[ i+1 ] )
        {
            ++i;
            switch ( text[ i ] )
            {
                case 't':   single[ 0 ] = '\t'; break;
                case 'n':   single[ 0 ] = '\n'; break;
                case 'r':   single[ 0 ] = '\r'; break;
                default:    single[ 0 ] = text[ i ]; break;
            }
        }
        else
        {
            single[ 0 ] = text[ i ];
        }
        result += single;
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNBanList::CDRNBanList( void )
    : CIDRNBanList()
    , CORE::CIConfigurable()
    , m_entries()
    , m_filePath()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNBanList::~CDRNBanList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CDRNBanList::GetUnixTimeNow( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() / 1000;
}

/*-------------------------------------------------------------------------*/

Int32
CDRNBanList::Find( TDRNBanKind kind, const CORE::CString& value ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].kind == kind && m_entries[ i ].value == value )
            return (Int32) i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::IsBanned( TDRNBanKind kind, const CORE::CString& value, CDRNBanEntry* entry ) const
{GUCEF_TRACE;

    if ( value.IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_lock );
    Int32 index = Find( kind, value );
    if ( index < 0 || m_entries[ index ].IsExpired( GetUnixTimeNow() ) )
        return false;
    if ( GUCEF_NULL != entry )
        *entry = m_entries[ index ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::Ban( TDRNBanKind kind, const CORE::CString& value, UInt64 expiryUnixSeconds, const CORE::CString& reason )
{GUCEF_TRACE;

    CDRNBanEntry entry;
    entry.kind = kind;
    entry.value = value;
    entry.expiryUnixSeconds = expiryUnixSeconds;
    entry.reason = reason;
    return Ban( entry );
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::Ban( const CDRNBanEntry& entry )
{GUCEF_TRACE;

    if ( DRN_BAN_NONE == entry.kind || entry.value.IsNULLOrEmpty() )
        return false;

    MT::CScopeMutex lock( m_lock );
    Int32 index = Find( entry.kind, entry.value );
    if ( index >= 0 )
        m_entries[ index ] = entry;
    else
        m_entries.push_back( entry );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNBanList: Banned " << CDRNBanEntry::BanKindToString( entry.kind ) << " \"" << entry.value << "\": " << entry.reason );
    return SaveIfFileSet();
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::Unban( TDRNBanKind kind, const CORE::CString& value )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Int32 index = Find( kind, value );
    if ( index < 0 )
        return false;
    m_entries.erase( m_entries.begin() + index );
    SaveIfFileSet();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNBanList::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    SaveIfFileSet();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNBanList::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetUnixTimeNow();
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( !m_entries[ i ].IsExpired( now ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CDRNBanList::GetEntries( TDRNBanEntryVector& entries ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetUnixTimeNow();
    entries.clear();
    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( !m_entries[ i ].IsExpired( now ) )
            entries.push_back( m_entries[ i ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNBanList::PurgeExpired( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetUnixTimeNow();
    UInt32 purged = 0;
    for ( UInt32 i=0; i<m_entries.size(); )
    {
        if ( m_entries[ i ].IsExpired( now ) )
        {
            m_entries.erase( m_entries.begin() + i );
            ++purged;
        }
        else
        {
            ++i;
        }
    }
    if ( purged > 0 )
        SaveIfFileSet();
    return purged;
}

/*-------------------------------------------------------------------------*/

void
CDRNBanList::SetFilePath( const CORE::CString& filePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_filePath = filePath;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNBanList::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::SaveIfFileSet( void ) const
{GUCEF_TRACE;

    if ( m_filePath.IsNULLOrEmpty() )
        return true;
    return SaveToFile( m_filePath );
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::LoadFromFile( const CORE::CString& filePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    if ( !CORE::FileExists( filePath ) )
        return true;

    CORE::CString content;
    if ( !CORE::LoadTextFileAsString( filePath, content ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNBanList: Failed to read " << filePath );
        return false;
    }

    bool allValid = true;
    CORE::CString::StringVector lines = content.ParseElements( '\n', false );
    for ( UInt32 i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ].RemoveChar( '\r' );
        if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
            continue;

        CORE::CString::StringVector fields = line.ParseElements( '\t', true );
        if ( fields.size() < 3 )
        {
            allValid = false;
            continue;
        }
        CDRNBanEntry entry;
        entry.kind = CDRNBanEntry::StringToBanKind( fields[ 0 ] );
        entry.expiryUnixSeconds = CORE::StringToUInt64( fields[ 1 ], 0 );
        entry.value = UnescapeField( fields[ 2 ] );
        if ( fields.size() > 3 )
            entry.reason = UnescapeField( fields[ 3 ] );
        if ( DRN_BAN_NONE == entry.kind || entry.value.IsNULLOrEmpty() )
        {
            allValid = false;
            continue;
        }
        Int32 index = Find( entry.kind, entry.value );
        if ( index >= 0 )
            m_entries[ index ] = entry;
        else
            m_entries.push_back( entry );
    }
    if ( !allValid )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNBanList: Skipped lines of " << filePath << " that are not bans" );
    return allValid;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::SaveToFile( const CORE::CString& filePath ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetUnixTimeNow();
    CORE::CString content = "# DRN ban list: kind, expiry (Unix seconds, 0 = never), value, reason; tab separated\n";
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        const CDRNBanEntry& entry = m_entries[ i ];
        if ( entry.IsExpired( now ) )
            continue;
        content += CDRNBanEntry::BanKindToString( entry.kind ) + "\t" + CORE::UInt64ToString( entry.expiryUnixSeconds ) + "\t" +
                   EscapeField( entry.value ) + "\t" + EscapeField( entry.reason ) + "\n";
    }
    if ( !CORE::WriteStringAsTextFile( filePath, content ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNBanList: Failed to write " << filePath );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetUnixTimeNow();
    bool totalSuccess = true;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        const CDRNBanEntry& entry = m_entries[ i ];
        if ( entry.IsExpired( now ) )
            continue;
        CORE::CDataNode* child = cfg.AddChild( "Ban" );
        if ( GUCEF_NULL == child )
        {
            totalSuccess = false;
            continue;
        }
        totalSuccess = child->SetAttribute( "kind", CDRNBanEntry::BanKindToString( entry.kind ) ) && totalSuccess;
        totalSuccess = child->SetAttribute( "expiry", entry.expiryUnixSeconds ) && totalSuccess;
        totalSuccess = child->SetAttribute( "value", entry.value ) && totalSuccess;
        totalSuccess = child->SetAttribute( "reason", entry.reason ) && totalSuccess;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBanList::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    bool allValid = true;
    CORE::CDataNode::const_iterator i = cfg.ConstBegin();
    while ( i != cfg.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        if ( GUCEF_NULL != child && child->GetName() == "Ban" )
        {
            CDRNBanEntry entry;
            entry.kind = CDRNBanEntry::StringToBanKind( child->GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ) );
            entry.expiryUnixSeconds = child->GetAttributeValueOrChildValueByName( "expiry" ).AsUInt64( 0, true );
            entry.value = child->GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true );
            entry.reason = child->GetAttributeValueOrChildValueByName( "reason" ).AsString( CORE::CString::Empty, true );
            if ( DRN_BAN_NONE != entry.kind && !entry.value.IsNULLOrEmpty() )
                m_entries.push_back( entry );
            else
                allValid = false;
        }
        ++i;
    }
    return allValid;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNBanList::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

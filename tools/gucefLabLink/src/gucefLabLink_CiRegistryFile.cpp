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

#include <string.h>

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <sys/stat.h>
#endif

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefLabLink_WebJson.h"
#include "gucefLabLink_CiRegistryFile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A member of the top-level object: where its key starts and its value ends */
struct SJsonMember
{
    CORE::CString key;
    UInt32 start;
    UInt32 valueStart;
    UInt32 end;
};

/*-------------------------------------------------------------------------*/

/** The size of the text in bytes, without its terminator */
static UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

static bool
IsJsonSpace( char c )
{
    return ' ' == c || '\t' == c || '\r' == c || '\n' == c;
}

/*-------------------------------------------------------------------------*/

static UInt32
SkipSpace( const char* text, UInt32 size, UInt32 pos )
{
    while ( pos < size && IsJsonSpace( text[ pos ] ) )
        ++pos;
    return pos;
}

/*-------------------------------------------------------------------------*/

/** From the opening quote to after the closing one; false when the string does not end */
static bool
SkipString( const char* text, UInt32 size, UInt32& pos )
{
    if ( pos >= size || '"' != text[ pos ] )
        return false;
    ++pos;
    while ( pos < size )
    {
        if ( '\\' == text[ pos ] )
            pos += 2;
        else
        if ( '"' == text[ pos ] )
        {
            ++pos;
            return true;
        }
        else
            ++pos;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** One value, from its first character to after its last; the text has been checked by the JSON codec */
static bool
SkipValue( const char* text, UInt32 size, UInt32& pos )
{
    if ( pos >= size )
        return false;
    if ( '"' == text[ pos ] )
        return SkipString( text, size, pos );
    if ( '{' == text[ pos ] || '[' == text[ pos ] )
    {
        UInt32 depth = 0;
        while ( pos < size )
        {
            const char c = text[ pos ];
            if ( '"' == c )
            {
                if ( !SkipString( text, size, pos ) )
                    return false;
                continue;
            }
            if ( '{' == c || '[' == c )
                ++depth;
            else
            if ( '}' == c || ']' == c )
            {
                if ( 0 == depth )
                    return false;
                --depth;
                if ( 0 == depth )
                {
                    ++pos;
                    return true;
                }
            }
            ++pos;
        }
        return false;
    }
    // a number, true, false or null
    const UInt32 start = pos;
    while ( pos < size && !IsJsonSpace( text[ pos ] ) && ',' != text[ pos ] && '}' != text[ pos ] && ']' != text[ pos ] )
        ++pos;
    return pos > start;
}

/*-------------------------------------------------------------------------*/

/** The members of the top-level object; false when the text is not one object */
static bool
TopLevelMembers( const CORE::CString& json, GUCEF::vector< SJsonMember >& members )
{
    const char* text = json.C_String();
    const UInt32 size = ByteLength( json );
    UInt32 pos = SkipSpace( text, size, 0 );
    if ( pos >= size || '{' != text[ pos ] )
        return false;
    pos = SkipSpace( text, size, pos + 1 );
    if ( pos < size && '}' == text[ pos ] )
        return SkipSpace( text, size, pos + 1 ) == size;
    while ( pos < size )
    {
        SJsonMember member;
        member.start = pos;
        if ( !SkipString( text, size, pos ) )
            return false;
        member.key = CORE::CString( text + member.start + 1, pos - member.start - 2 );
        pos = SkipSpace( text, size, pos );
        if ( pos >= size || ':' != text[ pos ] )
            return false;
        pos = SkipSpace( text, size, pos + 1 );
        member.valueStart = pos;
        if ( !SkipValue( text, size, pos ) )
            return false;
        member.end = pos;
        members.push_back( member );
        pos = SkipSpace( text, size, pos );
        if ( pos >= size )
            return false;
        if ( '}' == text[ pos ] )
            return SkipSpace( text, size, pos + 1 ) == size;
        if ( ',' != text[ pos ] )
            return false;
        pos = SkipSpace( text, size, pos + 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Whether the JSON codec takes the text */
static bool
IsJson( const CORE::CString& json )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( ByteLength( json ), json.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    CORE::CDataNode tree;
    return codec->BuildDataTree( &tree, &access );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiRegistryFile::CCiRegistryFile( const CORE::CString& path, const CORE::CString& gatewayUrl )
    : m_path( path )
    , m_gatewayUrl( gatewayUrl )
    , m_read( false )
    , m_existed( false )
    , m_size( 0 )
    , m_modifiedInMs( 0 )
    , m_state( CIREGISTRY_MISSING )
    , m_merged()
    , m_problem()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCiRegistryFile::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCiRegistryFile::GetGatewayUrl( void ) const
{GUCEF_TRACE;

    return m_gatewayUrl;
}

/*-------------------------------------------------------------------------*/

bool
CCiRegistryFile::IsJsonCodecAvailable( void )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL() )
        return true;

    // loaded from the directory of the program as the hub database loads it, once
    static MT::CMutex loadLock;
    static bool loadTried = false;
    {
        MT::CScopeMutex lock( loadLock );
        if ( !loadTried )
        {
            loadTried = true;
            CORE::CPluginControl& plugins = CORE::CCoreGlobal::Instance()->GetPluginControl();
            CORE::CPluginMetaData metaData;
            metaData.SetFullModulePath( CORE::RelativePath( "$MODULEDIR$" ) );
            metaData.SetPluginType( "GucefDataStoreCodecPlugin" );
            metaData.SetModuleFilename( "dstorepluginJSONPARSER" );
            if ( !plugins.AddPluginMetaData( metaData, "gucefLabLink-ci", true ) )
            {
                metaData.SetModuleFilename( "dstorepluginJSONPARSER_d" );
                plugins.AddPluginMetaData( metaData, "gucefLabLink-ci", true );
            }
        }
    }
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

TCiRegistryState
CCiRegistryFile::Merge( const CORE::CString& json       ,
                        const CORE::CString& gatewayUrl ,
                        CORE::CString& merged           ,
                        CORE::CString& problem          )
{GUCEF_TRACE;

    merged.Clear();
    problem.Clear();
    if ( ByteLength( json ) > MaxSizeInBytes )
    {
        problem = "it is larger than " + CORE::ToString( MaxSizeInBytes / 1024 ) + " KiB";
        return CIREGISTRY_INVALID;
    }
    if ( GUCEF_NULL != ::memchr( json.C_String(), '\0', ByteLength( json ) ) )
    {
        problem = "it is not text";
        return CIREGISTRY_INVALID;
    }
    if ( !IsJsonCodecAvailable() )
    {
        problem = "the plugin dstorepluginJSONPARSER is not loaded, so the registry cannot be checked";
        return CIREGISTRY_NO_PARSER;
    }
    if ( !IsJson( json ) )
    {
        problem = "it is not valid JSON";
        return CIREGISTRY_INVALID;
    }
    GUCEF::vector< SJsonMember > members;
    if ( !TopLevelMembers( json, members ) )
    {
        problem = "it is not a JSON object";
        return CIREGISTRY_INVALID;
    }
    bool haveList = false;
    for ( size_t i=0; i<members.size(); ++i )
        if ( members[ i ].key == "ci" )
            haveList = '[' == json.C_String()[ members[ i ].valueStart ];
    if ( !haveList )
    {
        problem = "it has no list \"ci\" of CI definitions at the top";
        return CIREGISTRY_INVALID;
    }

    // the object "gateway" of the hub first, then the members of the file as they are, without one of their own
    merged = "{\n  \"gateway\": { \"url\": " + CWebJson::Quote( gatewayUrl ) + ", \"caFile\": \"ca.crt\" }";
    for ( size_t i=0; i<members.size(); ++i )
    {
        if ( members[ i ].key == "gateway" )
            continue;
        merged += ",\n  " + CORE::CString( json.C_String() + members[ i ].start, members[ i ].end - members[ i ].start );
    }
    merged += "\n}\n";
    return CIREGISTRY_OK;
}

/*-------------------------------------------------------------------------*/

TCiRegistryState
CCiRegistryFile::Get( CORE::CString& merged, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    // what the file is now: read again only when that changed, or when the codec was not there
    bool exists = false;
    bool regular = false;
    UInt64 size = 0;
    UInt64 modifiedInMs = 0;
    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    struct stat info;
    if ( 0 == ::stat( m_path.C_String(), &info ) )
    {
        exists = true;
        regular = S_ISREG( info.st_mode );
        size = (UInt64) info.st_size;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        modifiedInMs = (UInt64) info.st_mtim.tv_sec * 1000 + (UInt64) info.st_mtim.tv_nsec / 1000000;
        #else
        modifiedInMs = (UInt64) info.st_mtime * 1000;
        #endif
    }
    #else
    if ( CORE::FileExists( m_path ) )
    {
        exists = true;
        regular = true;
        size = CORE::FileSize( m_path );
        modifiedInMs = CORE::GetFileModificationTime( m_path ).ToUnixEpochBasedTicksInMillisecs();
    }
    #endif

    const bool unchanged = m_read && exists == m_existed && size == m_size && modifiedInMs == m_modifiedInMs && CIREGISTRY_NO_PARSER != m_state;
    if ( !unchanged )
    {
        const TCiRegistryState before = m_state;
        const CORE::CString problemBefore = m_problem;
        const bool firstRead = !m_read;
        m_read = true;
        m_existed = exists;
        m_size = size;
        m_modifiedInMs = modifiedInMs;
        m_merged.Clear();
        m_problem.Clear();
        if ( !exists )
        {
            m_state = CIREGISTRY_MISSING;
        }
        else
        if ( !regular )
        {
            m_state = CIREGISTRY_INVALID;
            m_problem = "it is not a regular file";
        }
        else
        if ( size > MaxSizeInBytes )
        {
            m_state = CIREGISTRY_INVALID;
            m_problem = "it is larger than " + CORE::ToString( MaxSizeInBytes / 1024 ) + " KiB";
        }
        else
        {
            CORE::CDynamicBuffer content;
            if ( !content.LoadContentFromFile( m_path ) )
            {
                m_state = CIREGISTRY_INVALID;
                m_problem = "it cannot be read";
            }
            else
            {
                const CORE::CString text = 0 == content.GetDataSize() ? CORE::CString::Empty :
                                           CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
                m_state = Merge( text, m_gatewayUrl, m_merged, m_problem );
            }
        }

        // said when it changes, not for every machine that asks
        if ( firstRead || before != m_state || problemBefore != m_problem )
        {
            switch ( m_state )
            {
                case CIREGISTRY_OK:
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The CI registry " << m_path << " is read (" << (UInt32) m_size <<
                                      " bytes); approved machines get it with the gateway " << m_gatewayUrl );
                    break;
                case CIREGISTRY_MISSING:
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: There is no CI registry (" << m_path << "): machines get no CI configuration" );
                    break;
                case CIREGISTRY_NO_PARSER:
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: The CI registry " << m_path << " is not handed out: " << m_problem );
                    break;
                default:
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefLabLink: The CI registry " << m_path << " is not handed out: " << m_problem );
                    break;
            }
        }
    }
    merged = m_merged;
    problem = m_problem;
    return m_state;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

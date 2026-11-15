/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_GUI_CHELPMARKUPPARSER_H
#include "gucefGUI_CHelpMarkupParser.h"
#define GUCEF_GUI_CHELPMARKUPPARSER_H
#endif /* GUCEF_GUI_CHELPMARKUPPARSER_H ? */

#include "gucefGUI_CHelpCatalogue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CHelpCatalogue::DocumentExtension = "help";
const char* CHelpCatalogue::BriefTableExtension = "briefs";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether the file name ends in "." and the extension, letters in any case */
bool
HasExtension( const CString& fileName, const char* extension )
{
    const CString suffix = CString( "." ) + extension;
    const UInt32 length = fileName.Length();
    if ( length <= suffix.Length() )
        return false;
    return fileName.SubstrFromRange( length - suffix.Length(), length ).Lowercase() == suffix;
}

bool
TopicEntryLess( const CIHelpSource::TTopicEntry& a, const CIHelpSource::TTopicEntry& b )
{
    if ( a.group != b.group )
        return a.group < b.group;
    if ( a.title != b.title )
        return a.title < b.title;
    return a.id < b.id;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHelpCatalogue::CHelpCatalogue( void )
    : CIHelpSource()
    , m_documents()
    , m_briefs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHelpCatalogue::~CHelpCatalogue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::AddDocument( const CHelpDocument& document )
{GUCEF_TRACE;

    if ( !document.IsValid() || !CHelpMarkupParser::IsValidTopicId( document.id ) )
        return false;
    m_documents[ document.id ] = document;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::AddDocumentText( const CString& text, const CString& origin, CString& error )
{GUCEF_TRACE;

    CHelpDocument document;
    CString reason;
    if ( !CHelpMarkupParser::ParseDocument( text, document, reason ) )
    {
        error = origin + ": " + reason;
        return false;
    }
    if ( m_documents.find( document.id ) != m_documents.end() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "HelpCatalogue: " << origin << " replaces the document " << document.id );
    return AddDocument( document );
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::AddBrief( const CString& id, const CString& text, const CString& topicId )
{GUCEF_TRACE;

    if ( !CHelpMarkupParser::IsValidTopicId( id ) || text.IsNULLOrEmpty() )
        return false;
    SBrief& brief = m_briefs[ id ];
    brief.text = text;
    brief.topic = topicId;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpCatalogue::AddBriefTableText( const CString& text, const CString& origin, TStringVector& problems )
{GUCEF_TRACE;

    CHelpMarkupParser::TBriefEntryVector entries;
    TStringVector parseProblems;
    CHelpMarkupParser::ParseBriefTable( text, entries, parseProblems );
    for ( UInt32 i=0; i<(UInt32)parseProblems.size(); ++i )
        problems.push_back( origin + ": " + parseProblems[ i ] );
    UInt32 added = 0;
    for ( UInt32 i=0; i<(UInt32)entries.size(); ++i )
    {
        if ( m_briefs.find( entries[ i ].id ) != m_briefs.end() )
            problems.push_back( origin + ": the brief " + entries[ i ].id + " is given more than once; the last one is kept" );
        if ( AddBrief( entries[ i ].id, entries[ i ].text, entries[ i ].topic ) )
            ++added;
    }
    return added;
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::LoadFromVfs( const CString& vfsDirectory, TStringVector& problems )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( vfsDirectory ) )
    {
        problems.push_back( vfsDirectory + ": the directory of the help catalogue does not exist" );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HelpCatalogue: the directory " << vfsDirectory << " does not exist" );
        return false;
    }

    VFS::CVFS::TStringVector names;
    vfs.GetFileList( names, vfsDirectory, false, false, CString::Empty );
    std::sort( names.begin(), names.end() );

    UInt32 documents = 0, briefs = 0, refused = 0;
    for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
    {
        const CString& name = names[ i ];
        const bool isDocument = HasExtension( name, DocumentExtension );
        const bool isBriefTable = HasExtension( name, BriefTableExtension );
        if ( !isDocument && !isBriefTable )
            continue;

        const UInt32 dirLength = vfsDirectory.Length();
        const bool endsWithSlash = dirLength > 0 && '/' == vfsDirectory[ dirLength - 1 ];
        const CString path = endsWithSlash ? vfsDirectory + name : vfsDirectory + "/" + name;
        CORE::CDynamicBuffer buffer;
        if ( !vfs.LoadFile( buffer, path, "rb" ) )
        {
            problems.push_back( name + ": the file cannot be read" );
            ++refused;
            continue;
        }
        if ( buffer.GetDataSize() > (UInt32) MaxFileBytes )
        {
            problems.push_back( name + ": the file is larger than " + CORE::ToString( (UInt32) MaxFileBytes ) + " bytes and is not read" );
            ++refused;
            continue;
        }
        CORE::CUtf8String utf8;
        if ( buffer.GetDataSize() > 0 )
            utf8.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
        const CString text = CORE::ToString( utf8 );

        if ( isDocument )
        {
            CString error;
            if ( AddDocumentText( text, name, error ) )
                ++documents;
            else
            {
                problems.push_back( error );
                ++refused;
            }
        }
        else
        {
            briefs += AddBriefTableText( text, name, problems );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HelpCatalogue: " << documents << " documents and " << briefs << " briefs read from " << vfsDirectory << ", " << refused << " files refused" );
    if ( !problems.empty() )
    {
        for ( UInt32 i=0; i<(UInt32)problems.size(); ++i )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "HelpCatalogue: " << problems[ i ] );
    }
    return 0 == refused;
}

/*-------------------------------------------------------------------------*/

void
CHelpCatalogue::Clear( void )
{GUCEF_TRACE;

    m_documents.clear();
    m_briefs.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpCatalogue::GetTopicCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_documents.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CHelpCatalogue::GetBriefCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_briefs.size();
}

/*-------------------------------------------------------------------------*/

const CHelpDocument*
CHelpCatalogue::FindTopic( const CString& topicId ) const
{GUCEF_TRACE;

    TDocumentMap::const_iterator i = m_documents.find( topicId );
    return i != m_documents.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CHelpCatalogue::GetTopicIds( TStringVector& ids ) const
{GUCEF_TRACE;

    ids.clear();
    for ( TDocumentMap::const_iterator i = m_documents.begin(); i != m_documents.end(); ++i )
        ids.push_back( (*i).first );
}

/*-------------------------------------------------------------------------*/

void
CHelpCatalogue::GetBriefIds( TStringVector& ids ) const
{GUCEF_TRACE;

    ids.clear();
    for ( TBriefMap::const_iterator i = m_briefs.begin(); i != m_briefs.end(); ++i )
        ids.push_back( (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::Validate( TStringVector& problems ) const
{GUCEF_TRACE;

    const size_t before = problems.size();
    for ( TDocumentMap::const_iterator i = m_documents.begin(); i != m_documents.end(); ++i )
    {
        const CHelpDocument& document = (*i).second;
        if ( document.title.IsNULLOrEmpty() )
            problems.push_back( "the topic " + document.id + " has no title" );
        if ( document.brief.IsNULLOrEmpty() )
            problems.push_back( "the topic " + document.id + " has no brief" );
        if ( document.blocks.empty() )
            problems.push_back( "the topic " + document.id + " has no content" );
        for ( UInt32 s=0; s<(UInt32)document.seeAlso.size(); ++s )
            if ( m_documents.find( document.seeAlso[ s ] ) == m_documents.end() )
                problems.push_back( "the topic " + document.id + " sees also " + document.seeAlso[ s ] + ", which is not a topic" );
        TStringVector links;
        for ( UInt32 b=0; b<(UInt32)document.blocks.size(); ++b )
        {
            const CHelpDocument::TBlock& block = document.blocks[ b ];
            CHelpMarkupParser::CollectLinks( block.term, links );
            CHelpMarkupParser::CollectLinks( block.text, links );
            for ( UInt32 c=0; c<(UInt32)block.cells.size(); ++c )
                CHelpMarkupParser::CollectLinks( block.cells[ c ], links );
        }
        CHelpMarkupParser::CollectLinks( document.brief, links );
        for ( UInt32 l=0; l<(UInt32)links.size(); ++l )
            if ( m_documents.find( links[ l ] ) == m_documents.end() )
                problems.push_back( "the topic " + document.id + " links to " + links[ l ] + ", which is not a topic" );
        if ( m_briefs.find( document.id ) != m_briefs.end() )
            problems.push_back( "the id " + document.id + " is both a topic and a brief" );
    }
    for ( TBriefMap::const_iterator i = m_briefs.begin(); i != m_briefs.end(); ++i )
    {
        if ( (*i).second.text.IsNULLOrEmpty() )
            problems.push_back( "the brief " + (*i).first + " has no text" );
        if ( !(*i).second.topic.IsNULLOrEmpty() && m_documents.find( (*i).second.topic ) == m_documents.end() )
            problems.push_back( "the brief " + (*i).first + " names the topic " + (*i).second.topic + ", which is not a topic" );
        TStringVector links;
        CHelpMarkupParser::CollectLinks( (*i).second.text, links );
        for ( UInt32 l=0; l<(UInt32)links.size(); ++l )
            if ( m_documents.find( links[ l ] ) == m_documents.end() )
                problems.push_back( "the brief " + (*i).first + " links to " + links[ l ] + ", which is not a topic" );
    }
    return problems.size() == before;
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::GetTopic( const CString& topicId, CHelpDocument& document ) const
{GUCEF_TRACE;

    const CHelpDocument* found = FindTopic( topicId );
    if ( GUCEF_NULL == found )
        return false;
    document = *found;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHelpCatalogue::ListTopics( TTopicEntryVector& topics ) const
{GUCEF_TRACE;

    topics.clear();
    topics.reserve( m_documents.size() );
    for ( TDocumentMap::const_iterator i = m_documents.begin(); i != m_documents.end(); ++i )
        topics.push_back( TTopicEntry( (*i).second.id, (*i).second.title, (*i).second.group ) );
    std::sort( topics.begin(), topics.end(), TopicEntryLess );
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::GetBrief( const CString& id, CString& text, CString& topicId ) const
{GUCEF_TRACE;

    TBriefMap::const_iterator brief = m_briefs.find( id );
    if ( brief != m_briefs.end() )
    {
        text = (*brief).second.text;
        topicId = (*brief).second.topic;
        return true;
    }
    const CHelpDocument* document = FindTopic( id );
    if ( GUCEF_NULL == document || document->brief.IsNULLOrEmpty() )
        return false;
    text = document->brief;
    topicId = document->id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHelpCatalogue::HasTopic( const CString& topicId ) const
{GUCEF_TRACE;

    return GUCEF_NULL != FindTopic( topicId );
}

/*-------------------------------------------------------------------------*/

CString
CHelpCatalogue::GetTopicTitle( const CString& topicId ) const
{GUCEF_TRACE;

    const CHelpDocument* document = FindTopic( topicId );
    return GUCEF_NULL != document && !document->title.IsNULLOrEmpty() ? document->title : topicId;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

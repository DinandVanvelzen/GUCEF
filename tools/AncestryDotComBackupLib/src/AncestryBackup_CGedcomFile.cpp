/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "AncestryBackup_CGedcomReader.h"

#include "AncestryBackup_CGedcomFile.h"

#include <cstring>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const std::string EmptyString;

/*-------------------------------------------------------------------------*/

const SGedcomNode*
SGedcomNode::Child( const char* childTag ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<children.size(); ++i )
        if ( children[ i ].tag == childTag )
            return &children[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const std::string&
SGedcomNode::ChildValue( const char* childTag ) const
{GUCEF_TRACE;

    const SGedcomNode* child = Child( childTag );
    return child != GUCEF_NULL ? child->value : EmptyString;
}

/*-------------------------------------------------------------------------*/

void
SGedcomNode::Children( const char* childTag, std::vector< const SGedcomNode* >& list ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<children.size(); ++i )
        if ( children[ i ].tag == childTag )
            list.push_back( &children[ i ] );
}

/*-------------------------------------------------------------------------*/

/** A value as the file has it: an '@@' stands for a single '@' */
static void
AssignValue( std::string& target, const char* value, UInt32 length )
{
    target.clear();
    target.reserve( length );
    for ( UInt32 i=0; i<length; ++i )
    {
        target += value[ i ];
        if ( value[ i ] == '@' && i + 1 < length && value[ i + 1 ] == '@' )
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

/** Finds where the records start */
class CRecordIndexer : public CIGedcomHandler
{
    public:

    CRecordIndexer( const char* text, std::vector< SGedcomRecordRef >& records ) : m_text( text ), m_records( records ) {}

    virtual void OnLine( const SGedcomLine& line ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( line.level != 0 )
            return;

        // the start of the line: the first thing of it is the level digits
        const char* p = line.xrefLength > 0 ? line.xref - 1 : line.tag;
        while ( p > m_text && p[ -1 ] != '\n' && p[ -1 ] != '\r' )
            --p;

        if ( !m_records.empty() )
            m_records.back().end = (size_t) ( p - m_text );

        SGedcomRecordRef record;
        record.type.assign( line.tag, line.tagLength );
        if ( line.xrefLength > 0 )
            record.xref.assign( line.xref, line.xrefLength );
        record.begin = (size_t) ( p - m_text );
        m_records.push_back( record );
    }

    virtual void OnMalformedLine( UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    const char* m_text;
    std::vector< SGedcomRecordRef >& m_records;
};

/*-------------------------------------------------------------------------*/

/** Builds the tree of the lines of one record */
class CTreeBuilder : public CIGedcomHandler
{
    public:

    CTreeBuilder( SGedcomNode& root ) : m_root( root ), m_stack(), m_gotRoot( false ) {}

    virtual void OnLine( const SGedcomLine& line ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_gotRoot )
        {
            if ( line.level != 0 )
                return;
            m_gotRoot = true;
            m_root = SGedcomNode();
            m_root.tag.assign( line.tag, line.tagLength );
            if ( line.xrefLength > 0 )
                m_root.xref.assign( line.xref, line.xrefLength );
            AssignValue( m_root.value, line.value, line.valueLength );
            m_stack.clear();
            m_stack.push_back( &m_root );
            return;
        }
        if ( line.level == 0 )
            return;

        // the node on the stack at the level above is the parent. Levels which skip are attached to the deepest node.
        size_t level = line.level;
        if ( level > m_stack.size() )
            level = m_stack.size();
        m_stack.resize( level );
        SGedcomNode* parent = m_stack.back();

        if ( line.tagLength == 4 && ( ::memcmp( line.tag, "CONT", 4 ) == 0 || ::memcmp( line.tag, "CONC", 4 ) == 0 ) )
        {
            if ( line.tag[ 3 ] == 'T' )
                parent->value += '\n';
            std::string part;
            AssignValue( part, line.value, line.valueLength );
            parent->value += part;
            return;
        }

        parent->children.push_back( SGedcomNode() );
        SGedcomNode& node = parent->children.back();
        node.tag.assign( line.tag, line.tagLength );
        if ( line.xrefLength > 0 )
            node.xref.assign( line.xref, line.xrefLength );
        AssignValue( node.value, line.value, line.valueLength );
        m_stack.push_back( &node );
    }

    virtual void OnMalformedLine( UInt32 ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    SGedcomNode& m_root;
    std::vector< SGedcomNode* > m_stack;
    bool m_gotRoot;
};

/*-------------------------------------------------------------------------*/

CGedcomFile::CGedcomFile( void )
    : m_text()
    , m_records()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
CGedcomFile::Load( const CORE::CString& path )
{GUCEF_TRACE;

    m_text.clear();
    m_records.clear();

    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    m_text.resize( (size_t) file.GetSize() );
    size_t done = 0;
    while ( done < m_text.size() )
    {
        const UInt32 chunk = (UInt32) ( m_text.size() - done > 16 * 1024 * 1024 ? 16 * 1024 * 1024 : m_text.size() - done );
        if ( file.Read( &m_text[ done ], 1, chunk ) != chunk )
            return false;
        done += chunk;
    }
    IndexRecords();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGedcomFile::LoadFromText( const std::string& text )
{GUCEF_TRACE;

    m_text.assign( text.begin(), text.end() );
    m_records.clear();
    IndexRecords();
}

/*-------------------------------------------------------------------------*/

void
CGedcomFile::IndexRecords( void )
{GUCEF_TRACE;

    m_records.clear();
    if ( m_text.empty() )
        return;
    CRecordIndexer indexer( &m_text[ 0 ], m_records );
    CGedcomReader::Parse( &m_text[ 0 ], m_text.size(), indexer );
    if ( !m_records.empty() )
        m_records.back().end = m_text.size();
}

/*-------------------------------------------------------------------------*/

const std::vector< SGedcomRecordRef >&
CGedcomFile::GetRecords( void ) const
{GUCEF_TRACE;

    return m_records;
}

/*-------------------------------------------------------------------------*/

bool
CGedcomFile::ParseRecord( const SGedcomRecordRef& record, SGedcomNode& root ) const
{GUCEF_TRACE;

    if ( record.end <= record.begin || record.end > m_text.size() )
        return false;
    CTreeBuilder builder( root );
    CGedcomReader::Parse( &m_text[ record.begin ], record.end - record.begin, builder );
    return true;
}

/*-------------------------------------------------------------------------*/

const SGedcomRecordRef*
CGedcomFile::FindRecord( const std::string& type, const std::string& xref ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_records.size(); ++i )
        if ( m_records[ i ].type == type && m_records[ i ].xref == xref )
            return &m_records[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

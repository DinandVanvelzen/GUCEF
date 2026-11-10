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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <sstream>

#include "AncestryBackup_CGedcomSummary.h"

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

SGedcomSummary::SGedcomSummary( void )
    : sourceSystem(), sourceName(), sourceVersion(), gedcomVersion(), gedcomForm(), characterSet(), headerDate(), headerTime(), fileName(), treeName(), treeId()
    , individuals( 0 ), families( 0 ), sources( 0 ), repositories( 0 ), notes( 0 ), mediaObjects( 0 ), submitters( 0 )
    , individualsWithNotes( 0 ), citations( 0 ), mediaLinks( 0 ), mediaFiles( 0 ), mediaFilesThatAreUrls( 0 ), ancestryIds( 0 )
    , individualTags(), familyTags(), individualIds(), mediaFileNames()
    , lines( 0 ), malformedLines( 0 ), hasHeader( false ), hasTrailer( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGedcomSummarizer::CGedcomSummarizer( void )
    : CIGedcomHandler()
    , m_summary()
    , m_tagByLevel()
    , m_currentIndividualHasNote( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const SGedcomSummary&
CGedcomSummarizer::GetSummary( void ) const
{GUCEF_TRACE;

    return m_summary;
}

/*-------------------------------------------------------------------------*/

void
CGedcomSummarizer::OnMalformedLine( UInt32 lineNumber )
{GUCEF_TRACE;

    ++m_summary.malformedLines;
}

/*-------------------------------------------------------------------------*/

static bool
Is( const SGedcomLine& line, const char* tag )
{
    return strlen( tag ) == line.tagLength && memcmp( line.tag, tag, line.tagLength ) == 0;
}

/*-------------------------------------------------------------------------*/

static bool
Is( const std::string& tag, const char* expected )
{
    return tag == expected;
}

/*-------------------------------------------------------------------------*/

void
CGedcomSummarizer::OnLine( const SGedcomLine& line )
{GUCEF_TRACE;

    ++m_summary.lines;

    // the tag of each level that is open
    if ( m_tagByLevel.size() <= line.level )
        m_tagByLevel.resize( line.level + 1 );
    m_tagByLevel[ line.level ].assign( line.tag, line.tagLength );
    const std::string empty;
    const std::string& parent = line.level > 0 ? m_tagByLevel[ line.level - 1 ] : empty;
    const std::string& grandParent = line.level > 1 ? m_tagByLevel[ line.level - 2 ] : empty;
    const std::string& record = m_tagByLevel[ 0 ];

    if ( line.level == 0 )
    {
        if ( m_currentIndividualHasNote )
            ++m_summary.individualsWithNotes;
        m_currentIndividualHasNote = false;

        if ( Is( line, "HEAD" ) ) m_summary.hasHeader = true;
        else if ( Is( line, "TRLR" ) ) m_summary.hasTrailer = true;
        else if ( Is( line, "INDI" ) )
        {
            ++m_summary.individuals;
            m_summary.individualIds.push_back( std::string( line.xref, line.xrefLength ) );
        }
        else if ( Is( line, "FAM" ) ) ++m_summary.families;
        else if ( Is( line, "SOUR" ) ) ++m_summary.sources;
        else if ( Is( line, "REPO" ) ) ++m_summary.repositories;
        else if ( Is( line, "NOTE" ) ) ++m_summary.notes;
        else if ( Is( line, "OBJE" ) ) ++m_summary.mediaObjects;
        else if ( Is( line, "SUBM" ) ) ++m_summary.submitters;
        return;
    }

    const std::string value( line.value != GUCEF_NULL ? line.value : "", line.valueLength );

    if ( Is( record, "HEAD" ) )
    {
        if ( line.level == 1 && Is( line, "SOUR" ) ) m_summary.sourceSystem = value;
        else if ( line.level == 2 && Is( parent, "SOUR" ) && Is( line, "NAME" ) ) m_summary.sourceName = value;
        else if ( line.level == 2 && Is( parent, "SOUR" ) && Is( line, "VERS" ) ) m_summary.sourceVersion = value;
        else if ( line.level == 2 && Is( parent, "GEDC" ) && Is( line, "VERS" ) ) m_summary.gedcomVersion = value;
        else if ( line.level == 2 && Is( parent, "GEDC" ) && Is( line, "FORM" ) ) m_summary.gedcomForm = value;
        else if ( line.level == 2 && Is( parent, "SOUR" ) && Is( line, "_TREE" ) ) m_summary.treeName = value;
        else if ( line.level == 3 && Is( parent, "_TREE" ) && Is( line, "RIN" ) ) m_summary.treeId = value;
        else if ( line.level == 1 && Is( line, "CHAR" ) ) m_summary.characterSet = value;
        else if ( line.level == 1 && Is( line, "FILE" ) ) m_summary.fileName = value;
        else if ( line.level == 1 && Is( line, "DATE" ) ) m_summary.headerDate = value;
        else if ( line.level == 2 && Is( parent, "DATE" ) && Is( line, "TIME" ) ) m_summary.headerTime = value;
        return;
    }

    if ( line.level == 1 && ( Is( record, "INDI" ) || Is( record, "FAM" ) ) )
    {
        std::map< std::string, UInt32 >& tags = Is( record, "INDI" ) ? m_summary.individualTags : m_summary.familyTags;
        ++tags[ std::string( line.tag, line.tagLength ) ];
        if ( Is( record, "INDI" ) && Is( line, "NOTE" ) )
            m_currentIndividualHasNote = true;
    }

    if ( Is( line, "SOUR" ) && line.valueLength > 0 && line.value[ 0 ] == '@' && ( Is( record, "INDI" ) || Is( record, "FAM" ) ) )
        ++m_summary.citations;

    if ( Is( line, "OBJE" ) )
        ++m_summary.mediaLinks;

    if ( Is( line, "FILE" ) && Is( parent, "OBJE" ) )
    {
        ++m_summary.mediaFiles;
        if ( value.compare( 0, 7, "http://" ) == 0 || value.compare( 0, 8, "https://" ) == 0 )
            ++m_summary.mediaFilesThatAreUrls;
        m_summary.mediaFileNames.push_back( value );
    }

    if ( Is( line, "_APID" ) )
        ++m_summary.ancestryIds;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGedcomSummarizer::Render( const SGedcomSummary& s )
{GUCEF_TRACE;

    std::ostringstream out;
    out << "GEDCOM file\n";
    out << "  produced by: " << s.sourceSystem << ( s.sourceName.empty() ? "" : " / " + s.sourceName ) << ( s.sourceVersion.empty() ? "" : " " + s.sourceVersion ) << "\n";
    out << "  gedcom version: " << s.gedcomVersion << ( s.gedcomForm.empty() ? "" : " (" + s.gedcomForm + ")" ) << ", character set: " << s.characterSet << "\n";
    if ( !s.headerDate.empty() )
        out << "  date: " << s.headerDate << "\n";
    if ( !s.treeId.empty() )
        out << "  tree: " << s.treeName << " (id " << s.treeId << ")\n";
    out << "  lines: " << s.lines << ", lines that are not gedcom: " << s.malformedLines << "\n";
    out << "Records\n";
    out << "  individuals: " << s.individuals << "\n  families: " << s.families << "\n  sources: " << s.sources << "\n  repositories: " << s.repositories << "\n";
    out << "  notes: " << s.notes << " (of their own), individuals with a note: " << s.individualsWithNotes << "\n";
    out << "  media objects: " << s.mediaObjects << "\n";
    out << "Links\n";
    out << "  source citations: " << s.citations << ", ancestry record ids (_APID): " << s.ancestryIds << "\n";
    out << "  media links: " << s.mediaLinks << ", media files: " << s.mediaFiles << " (" << s.mediaFilesThatAreUrls << " are urls)\n";

    out << "Tags directly below an individual\n";
    for ( std::map< std::string, UInt32 >::const_iterator i = s.individualTags.begin(); i != s.individualTags.end(); ++i )
        out << "  " << i->first << ": " << i->second << "\n";
    out << "Tags directly below a family\n";
    for ( std::map< std::string, UInt32 >::const_iterator i = s.familyTags.begin(); i != s.familyTags.end(); ++i )
        out << "  " << i->first << ": " << i->second << "\n";

    out << "Checks\n";
    if ( !s.hasHeader ) out << "  - there is no header record, this does not look like a GEDCOM file\n";
    if ( !s.hasTrailer ) out << "  - there is no TRLR record, the file may be cut short\n";
    if ( s.malformedLines > 0 ) out << "  - " << s.malformedLines << " lines are not GEDCOM\n";
    if ( !s.characterSet.empty() && s.characterSet != "UTF-8" && s.characterSet != "UNICODE" && s.characterSet != "ASCII" )
        out << "  - the character set is " << s.characterSet << ", only UTF-8 and ASCII are read correctly\n";
    if ( s.individuals == 0 ) out << "  - there are no individuals\n";
    if ( s.mediaFiles == 0 ) out << "  - the file does not name any media files: pictures and documents have to come from the site\n";
    else if ( s.mediaFilesThatAreUrls == s.mediaFiles ) out << "  - all media files are urls and not files that came with the export\n";
    out << "  - a GEDCOM file does not hold comments, stories or record transcriptions\n";
    return CORE::CString( out.str().c_str() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

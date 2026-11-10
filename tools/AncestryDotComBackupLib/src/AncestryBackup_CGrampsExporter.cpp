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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AncestryBackup_CGedcomFile.h"
#include "AncestryBackup_CGrampsText.h"
#include "AncestryBackup_CGrampsDate.h"

#include "AncestryBackup_CGrampsExporter.h"

#include <map>
#include <set>
#include <cstdio>
#include <cstdlib>
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

/** A section of the output which is built while the records are walked and put in its place at the end */
class CSectionFile
{
    public:

    CSectionFile( void ) : m_path(), m_file( GUCEF_NULL ), m_buffer(), m_size( 0 ), m_ok( true ) {}

    ~CSectionFile() { Close(); }

    bool Open( const CORE::CString& path )
    {
        Close();
        m_path = path;
        m_size = 0;
        m_ok = true;
        m_buffer.clear();
        m_file = new CORE::CFileAccess( path, "wb" );
        if ( !m_file->Opened() )
        {
            delete m_file;
            m_file = GUCEF_NULL;
            m_ok = false;
        }
        return m_ok;
    }

    void Append( const std::string& text )
    {
        m_buffer += text;
        m_size += text.size();
        if ( m_buffer.size() >= 1024 * 1024 )
            Flush();
    }

    void Flush( void )
    {
        if ( m_file != GUCEF_NULL && !m_buffer.empty() )
            if ( m_file->Write( m_buffer.data(), 1, (UInt32) m_buffer.size() ) != m_buffer.size() )
                m_ok = false;
        m_buffer.clear();
    }

    void Close( void )
    {
        if ( m_file != GUCEF_NULL )
        {
            Flush();
            m_file->Close();
            delete m_file;
            m_file = GUCEF_NULL;
        }
    }

    UInt64 GetSize( void ) const { return m_size; }
    bool IsOk( void ) const { return m_ok; }
    const CORE::CString& GetPath( void ) const { return m_path; }

    private:

    CSectionFile( const CSectionFile& );
    CSectionFile& operator=( const CSectionFile& );

    CORE::CString m_path;
    CORE::CFileAccess* m_file;
    std::string m_buffer;
    UInt64 m_size;
    bool m_ok;
};

/*-------------------------------------------------------------------------*/

struct SPlaceInfo
{
    std::string name;
    std::string parentKey;
    UInt32 number;
    SPlaceInfo( void ) : name(), parentKey(), number( 0 ) {}
};

struct SMediaFile
{
    std::string fileName;
    SMediaFile( void ) : fileName() {}
};

/** What the person records need to know of a media object */
struct SObjectInfo
{
    std::string oid;
    UInt32 width;
    UInt32 height;
    SObjectInfo( void ) : oid(), width( 0 ), height( 0 ) {}
};

/** How a GEDCOM tag becomes an event of Gramps */
struct SEventSpec
{
    const char* tag;
    const char* type;
};

static const SEventSpec PersonEvents[] =
{
    { "BIRT", "Birth" }, { "DEAT", "Death" }, { "BURI", "Burial" }, { "CREM", "Cremation" }, { "CHR", "Christening" },
    { "BAPM", "Baptism" }, { "BARM", "Bar Mitzvah" }, { "BASM", "Bas Mitzvah" }, { "BLES", "Blessing" }, { "CHRA", "Adult Christening" },
    { "CONF", "Confirmation" }, { "FCOM", "First Communion" }, { "ORDN", "Ordination" }, { "NATU", "Naturalization" },
    { "EMIG", "Emigration" }, { "IMMI", "Immigration" }, { "CENS", "Census" }, { "PROB", "Probate" }, { "WILL", "Will" },
    { "GRAD", "Graduation" }, { "RETI", "Retirement" }, { "ADOP", "Adopted" }, { "EDUC", "Education" }, { "OCCU", "Occupation" },
    { "RESI", "Residence" }, { "RELI", "Religion" }, { "TITL", "Nobility Title" }, { "PROP", "Property" }, { "_MILT", "Military Service" },
    { "_DCAUSE", "Cause Of Death" }, { "_EMPLOY", "Employment" }, { "_ORIG", "Origin" }, { "_DEST", "Destination" }, { "_MISN", "Mission" },
    { "BAPL", "LDS Baptism" }, { "MARR", "Marriage" }, { "DIV", "Divorce" }
};

static const SEventSpec FamilyEvents[] =
{
    { "MARR", "Marriage" }, { "DIV", "Divorce" }, { "DIVF", "Divorce Filing" }, { "ENGA", "Engagement" }, { "MARB", "Marriage Banns" },
    { "MARC", "Marriage Contract" }, { "MARL", "Marriage License" }, { "MARS", "Marriage Settlement" }, { "ANUL", "Annulment" },
    { "_SEPR", "Separation" }, { "CENS", "Census" }, { "RESI", "Residence" }
};

static const char* const CountryNames[] =
{
    "nederland", "netherlands", "the netherlands", "belgie", "belgium", "belgique", "germany", "deutschland", "duitsland", "france", "frankrijk",
    "usa", "united states", "united states of america", "england", "scotland", "wales", "ireland", "canada", "australia", "poland", "polen",
    "indonesia", "indonesie", "suriname", "south africa", "italy", "italia", "spain", "austria", "switzerland", "denmark", "norway", "sweden",
    "luxembourg", "india", "mexico", "brazil", "russia", "united kingdom", "uk", "great britain", "curacao", "new zealand", "israel", "turkey", "greece"
};

/*-------------------------------------------------------------------------*/

static std::string
Handle( const char* kind, const std::string& key )
{
    return "_" + CGrampsText::HashHex( std::string( kind ) + ":" + key );
}

static std::string
NumberedId( char prefix, UInt32 number )
{
    char buffer[ 24 ];
    ::snprintf( buffer, sizeof( buffer ), "%c%04u", prefix, (unsigned) number );
    return std::string( buffer );
}

static std::string
Num( UInt64 value )
{
    char buffer[ 32 ];
    ::snprintf( buffer, sizeof( buffer ), "%llu", (unsigned long long) value );
    return std::string( buffer );
}

/** An XML safe version of what the export has, with the HTML escapes of Ancestry taken out */
static std::string
X( const std::string& text )
{
    return CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( text ) );
}

/** Is the value a pointer to another record, @I1@. Gives the id without the @ signs. */
static bool
PointerOf( const std::string& value, std::string& xref )
{
    if ( value.size() > 2 && value[ 0 ] == '@' && value[ value.size() - 1 ] == '@' )
    {
        xref = value.substr( 1, value.size() - 2 );
        return true;
    }
    return false;
}

static bool
EqualsNoCase( const std::string& a, const char* b )
{
    return CGrampsText::ToLowerAscii( a ) == b;
}

/*-------------------------------------------------------------------------*/

static std::string
MimeOfExtension( const std::string& extIn )
{
    const std::string ext = CGrampsText::ToLowerAscii( extIn );
    if ( ext == "jpg" || ext == "jpeg" || ext == "pjpeg" || ext == "jpe" ) return "image/jpeg";
    if ( ext == "png" ) return "image/png";
    if ( ext == "gif" ) return "image/gif";
    if ( ext == "bmp" ) return "image/bmp";
    if ( ext == "tif" || ext == "tiff" ) return "image/tiff";
    if ( ext == "webp" ) return "image/webp";
    if ( ext == "pdf" ) return "application/pdf";
    if ( ext == "mp3" ) return "audio/mpeg";
    if ( ext == "mp4" ) return "video/mp4";
    if ( ext == "txt" ) return "text/plain";
    return "application/octet-stream";
}

/*-------------------------------------------------------------------------*/

/** The conversion, with the state it needs while it walks the records */
class CExporter
{
    public:

    CExporter( const SGrampsExportOptions& options, SGrampsExportResult& result );

    bool Run( void );

    private:

    // preparation
    bool LoadInput( void );
    void ScanMediaDir( void );
    void ReadHeader( void );
    void PrepareObjects( void );
    bool OpenSections( void );
    bool Assemble( void );

    // building blocks
    std::string PlaceHandle( const std::string& place );
    std::string DateXml( const std::string& date );
    std::string AddNote( const char* noteType, const std::string& key, const std::string& text, bool html );
    std::string NoteRefs( const SGedcomNode& node, const char* noteType, const std::string& ownerKey );
    std::string CitationHandle( const SGedcomNode& sour );
    std::string CitationRefs( const SGedcomNode& node );
    std::string ObjectRefs( const SGedcomNode& node, bool primaryFirst );
    std::string Attribute( const std::string& type, const std::string& value );
    std::string EmitEvent( const std::string& type, const SGedcomNode& node, const std::string& ownerKey, UInt32 ordinal, const std::string& description );
    const char* FindEventType( const SEventSpec* specs, size_t count, const std::string& tag );

    // records
    void EmitPerson( const SGedcomNode& indi );
    void EmitFamily( const SGedcomNode& fam );
    void EmitSource( const SGedcomNode& sour );
    void EmitRepository( const SGedcomNode& repo );
    void EmitObject( const SGedcomNode& obje );
    void EmitTextSource( const std::string& title, const std::string& handle );
    std::string BuildName( const SGedcomNode& name, bool alternate, const std::string& ownerKey );
    std::string Change( void ) const { return m_change; }
    void Warn( const std::string& text );

    const SGrampsExportOptions& m_options;
    SGrampsExportResult& m_result;
    CGedcomFile m_gedcom;

    std::set< std::string > m_personIds;
    std::set< std::string > m_familyIds;
    std::set< std::string > m_sourceIds;
    std::set< std::string > m_repoIds;
    std::set< std::string > m_objectIds;
    std::map< std::string, SMediaFile > m_mediaFiles;       // by media id
    std::map< std::string, SObjectInfo > m_objects;         // by the id of the OBJE record
    std::map< std::string, SPlaceInfo > m_places;
    std::vector< std::string > m_placeOrder;
    std::set< std::string > m_citationHandles;
    std::set< std::string > m_textSources;

    CSectionFile m_tagsOut;
    CSectionFile m_eventsOut;
    CSectionFile m_peopleOut;
    CSectionFile m_familiesOut;
    CSectionFile m_citationsOut;
    CSectionFile m_sourcesOut;
    CSectionFile m_objectsOut;
    CSectionFile m_reposOut;
    CSectionFile m_notesOut;

    std::string m_change;
    std::string m_treeId;
    std::string m_createdDate;
    std::string m_missingTag;
    std::set< std::string > m_warned;
    UInt32 m_eventCount;
    UInt32 m_noteCount;
    UInt32 m_citationCount;
    UInt32 m_sourceTextCount;
};

/*-------------------------------------------------------------------------*/

CExporter::CExporter( const SGrampsExportOptions& options, SGrampsExportResult& result )
    : m_options( options )
    , m_result( result )
    , m_gedcom()
    , m_personIds()
    , m_familyIds()
    , m_sourceIds()
    , m_repoIds()
    , m_objectIds()
    , m_mediaFiles()
    , m_objects()
    , m_places()
    , m_placeOrder()
    , m_citationHandles()
    , m_textSources()
    , m_tagsOut()
    , m_eventsOut()
    , m_peopleOut()
    , m_familiesOut()
    , m_citationsOut()
    , m_sourcesOut()
    , m_objectsOut()
    , m_reposOut()
    , m_notesOut()
    , m_change( "0" )
    , m_treeId()
    , m_createdDate()
    , m_missingTag()
    , m_warned()
    , m_eventCount( 0 )
    , m_noteCount( 0 )
    , m_citationCount( 0 )
    , m_sourceTextCount( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CExporter::Warn( const std::string& text )
{GUCEF_TRACE;

    // a few of each kind is enough to find the cause, the counts say the rest
    if ( m_warned.insert( text ).second && m_result.warnings.size() < 200 )
        m_result.warnings.push_back( text );
}

/*-------------------------------------------------------------------------*/

bool
CExporter::LoadInput( void )
{GUCEF_TRACE;

    if ( !m_gedcom.Load( m_options.gedcomPath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Could not read the GEDCOM file " + m_options.gedcomPath );
        return false;
    }
    const std::vector< SGedcomRecordRef >& records = m_gedcom.GetRecords();
    if ( records.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: The GEDCOM file " + m_options.gedcomPath + " has no records" );
        return false;
    }
    for ( size_t i=0; i<records.size(); ++i )
    {
        const SGedcomRecordRef& r = records[ i ];
        if ( r.type == "INDI" ) m_personIds.insert( r.xref );
        else if ( r.type == "FAM" ) m_familyIds.insert( r.xref );
        else if ( r.type == "SOUR" ) m_sourceIds.insert( r.xref );
        else if ( r.type == "REPO" ) m_repoIds.insert( r.xref );
        else if ( r.type == "OBJE" ) m_objectIds.insert( r.xref );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CExporter::ScanMediaDir( void )
{GUCEF_TRACE;

    m_mediaFiles.clear();
    if ( m_options.mediaDir.IsNULLOrEmpty() )
        return;

    struct CORE::SDI_Data* dir = CORE::DI_First_Dir_Entry( m_options.mediaDir.C_String() );
    if ( dir == GUCEF_NULL )
        return;
    do
    {
        if ( CORE::DI_Is_It_A_File( dir ) == 0 )
            continue;
        const std::string name = CORE::DI_Name( dir );
        // <media id>.<extension>, next to a <name>.json with what the site said about it
        if ( name.size() > 5 && name.compare( name.size() - 5, 5, ".json" ) == 0 )
            continue;
        const size_t dot = name.find( '.' );
        if ( dot == std::string::npos || dot == 0 || dot + 1 >= name.size() )
            continue;
        SMediaFile file;
        file.fileName = name;
        m_mediaFiles[ name.substr( 0, dot ) ] = file;
    }
    while ( CORE::DI_Next_Dir_Entry( dir ) != 0 );
    CORE::DI_Cleanup( dir );
}

/*-------------------------------------------------------------------------*/

void
CExporter::ReadHeader( void )
{GUCEF_TRACE;

    m_createdDate = "1970-01-01";
    const std::vector< SGedcomRecordRef >& records = m_gedcom.GetRecords();
    for ( size_t i=0; i<records.size(); ++i )
    {
        if ( records[ i ].type != "HEAD" )
            continue;
        SGedcomNode head;
        if ( !m_gedcom.ParseRecord( records[ i ], head ) )
            break;
        const SGedcomNode* sour = head.Child( "SOUR" );
        if ( sour != GUCEF_NULL )
        {
            const SGedcomNode* tree = sour->Child( "_TREE" );
            if ( tree != GUCEF_NULL )
                m_treeId = tree->ChildValue( "RIN" );
        }
        const SGedcomNode* date = head.Child( "DATE" );
        if ( date != GUCEF_NULL )
        {
            Int32 y; UInt32 m, d;
            if ( CGrampsDate::ParseSimple( date->value, y, m, d ) && m != 0 && d != 0 )
            {
                UInt32 seconds = 0;
                const std::string& time = date->ChildValue( "TIME" );
                unsigned h = 0, mi = 0, s = 0;
                if ( ::sscanf( time.c_str(), "%u:%u:%u", &h, &mi, &s ) >= 2 )
                    seconds = h * 3600 + mi * 60 + s;
                const Int64 epoch = CGrampsText::DaysSinceEpoch( y, m, d ) * 86400 + seconds;
                m_change = Num( (UInt64) ( epoch > 0 ? epoch : 0 ) );
                char buffer[ 32 ];
                ::snprintf( buffer, sizeof( buffer ), "%04d-%02u-%02u", (int) y, (unsigned) m, (unsigned) d );
                m_createdDate = buffer;
                m_result.gedcomTime = std::string( buffer ) + ( time.empty() ? "" : " " + time );
            }
        }
        break;
    }
    if ( !m_options.treeId.empty() )
        m_treeId = m_options.treeId;
}

/*-------------------------------------------------------------------------*/

void
CExporter::PrepareObjects( void )
{GUCEF_TRACE;

    const std::vector< SGedcomRecordRef >& records = m_gedcom.GetRecords();
    for ( size_t i=0; i<records.size(); ++i )
    {
        if ( records[ i ].type != "OBJE" )
            continue;
        SGedcomNode obje;
        if ( !m_gedcom.ParseRecord( records[ i ], obje ) )
            continue;
        SObjectInfo info;
        info.oid = obje.ChildValue( "_OID" );
        if ( info.oid.empty() )
            info.oid = records[ i ].xref;
        const SGedcomNode* file = obje.Child( "FILE" );
        if ( file != GUCEF_NULL )
        {
            const SGedcomNode* form = file->Child( "FORM" );
            if ( form != GUCEF_NULL )
            {
                info.width = (UInt32) ::atoi( form->ChildValue( "_WDTH" ).c_str() );
                info.height = (UInt32) ::atoi( form->ChildValue( "_HGHT" ).c_str() );
            }
        }
        m_objects[ records[ i ].xref ] = info;
    }
}

/*-------------------------------------------------------------------------*/

bool
CExporter::OpenSections( void )
{GUCEF_TRACE;

    const CORE::CString base = m_options.outputPath;
    bool ok = true;
    ok = m_tagsOut.Open( base + ".tags.part" ) && ok;
    ok = m_eventsOut.Open( base + ".events.part" ) && ok;
    ok = m_peopleOut.Open( base + ".people.part" ) && ok;
    ok = m_familiesOut.Open( base + ".families.part" ) && ok;
    ok = m_citationsOut.Open( base + ".citations.part" ) && ok;
    ok = m_sourcesOut.Open( base + ".sources.part" ) && ok;
    ok = m_objectsOut.Open( base + ".objects.part" ) && ok;
    ok = m_reposOut.Open( base + ".repositories.part" ) && ok;
    ok = m_notesOut.Open( base + ".notes.part" ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::Attribute( const std::string& type, const std::string& value )
{GUCEF_TRACE;

    const std::string v = CGrampsText::Trim( value );
    if ( v.empty() )
        return std::string();
    return "<attribute type=\"" + CGrampsText::XmlEscape( type ) + "\" value=\"" + X( v ) + "\"/>";
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::DateXml( const std::string& date )
{GUCEF_TRACE;

    std::string xml;
    const EGrampsDateKind kind = CGrampsDate::ToXml( date, xml );
    if ( kind == GRAMPSDATE_TEXT )
    {
        ++m_result.datesAsText;
        // dates which are no dates, like (ja), are common in this data, only warn about the first few
        if ( m_result.datesAsText <= 5 )
            Warn( "A date was kept as text because it is not one Gramps can compute with: " + date );
    }
    else
    if ( kind != GRAMPSDATE_NONE )
        ++m_result.datesExact;
    return xml;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::PlaceHandle( const std::string& placeText )
{GUCEF_TRACE;

    // Meerssen, Limburg, Nederland: a place in a place in a place, from the right
    std::vector< std::string > parts;
    {
        std::string current;
        const std::string text = CGrampsText::DecodeHtmlEntities( placeText );
        for ( size_t i=0; i<=text.size(); ++i )
        {
            if ( i == text.size() || text[ i ] == ',' )
            {
                const std::string part = CGrampsText::Trim( current );
                if ( !part.empty() )
                    parts.push_back( part );
                current.clear();
            }
            else
                current += text[ i ];
        }
    }
    if ( parts.empty() )
        return std::string();

    std::string parentKey;
    std::string key;
    for ( size_t n = parts.size(); n > 0; --n )
    {
        // the place made of parts[ n-1 ] and everything after it
        key.clear();
        for ( size_t i = n-1; i < parts.size(); ++i )
        {
            if ( !key.empty() )
                key += ", ";
            key += CGrampsText::ToLowerAscii( parts[ i ] );
        }
        if ( m_places.find( key ) == m_places.end() )
        {
            SPlaceInfo info;
            info.name = parts[ n-1 ];
            info.parentKey = parentKey;
            info.number = (UInt32) m_places.size() + 1;
            m_places[ key ] = info;
            m_placeOrder.push_back( key );
        }
        parentKey = key;
    }
    return Handle( "place", key );
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::AddNote( const char* noteType, const std::string& key, const std::string& rawText, bool html )
{GUCEF_TRACE;

    const std::string text = html ? CGrampsText::HtmlToPlainText( rawText ) : CGrampsText::DecodeHtmlEntities( rawText );
    if ( CGrampsText::Trim( text ).empty() )
        return std::string();

    const std::string handle = Handle( "note", key );
    ++m_noteCount;
    ++m_result.notes;
    m_notesOut.Append( "<note handle=\"" + handle + "\" id=\"" + NumberedId( 'N', m_noteCount ) + "\" change=\"" + m_change + "\" format=\"" + ( text.find( '\n' ) != std::string::npos ? "1" : "0" ) + "\" type=\"" + noteType + "\"><text>" + CGrampsText::XmlEscape( text, false ) + "</text></note>\n" );
    return handle;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::NoteRefs( const SGedcomNode& node, const char* noteType, const std::string& ownerKey )
{GUCEF_TRACE;

    std::string refs;
    UInt32 ordinal = 0;
    for ( size_t i=0; i<node.children.size(); ++i )
    {
        if ( node.children[ i ].tag != "NOTE" )
            continue;
        const std::string handle = AddNote( noteType, ownerKey + "/note" + Num( ordinal++ ), node.children[ i ].value, true );
        if ( !handle.empty() )
            refs += "<noteref hlink=\"" + handle + "\"/>";
    }
    return refs;
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitTextSource( const std::string& title, const std::string& handle )
{GUCEF_TRACE;

    ++m_sourceTextCount;
    ++m_result.sources;
    m_sourcesOut.Append( "<source handle=\"" + handle + "\" id=\"Stxt" + Num( m_sourceTextCount ) + "\" change=\"" + m_change + "\"><stitle>" + X( title ) + "</stitle></source>\n" );
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::CitationHandle( const SGedcomNode& sour )
{GUCEF_TRACE;

    std::string sourceHandle;
    std::string sourceKey;
    std::string xref;
    if ( PointerOf( sour.value, xref ) )
    {
        if ( m_sourceIds.find( xref ) == m_sourceIds.end() )
        {
            ++m_result.danglingReferences;
            Warn( "A citation points to the source " + xref + " which is not in the file, it was left out" );
            return std::string();
        }
        sourceHandle = Handle( "source", xref );
        sourceKey = "@" + xref;
    }
    else
    {
        const std::string title = CGrampsText::Trim( sour.value );
        const bool hasContent = sour.Child( "PAGE" ) != GUCEF_NULL || sour.Child( "DATA" ) != GUCEF_NULL || sour.Child( "_APID" ) != GUCEF_NULL || sour.Child( "NOTE" ) != GUCEF_NULL;
        if ( title.empty() && !hasContent )
            return std::string();           // an empty SOUR line, the export has plenty of them
        const std::string name = title.empty() ? std::string( "Unspecified source" ) : title;
        sourceKey = "text:" + name;
        sourceHandle = Handle( "source", sourceKey );
        if ( m_textSources.insert( sourceKey ).second )
            EmitTextSource( name, sourceHandle );
    }

    const SGedcomNode* data = sour.Child( "DATA" );
    const std::string text = data != GUCEF_NULL ? data->ChildValue( "TEXT" ) : std::string();
    const std::string www = data != GUCEF_NULL ? data->ChildValue( "WWW" ) : std::string();
    const std::string dataDate = data != GUCEF_NULL ? data->ChildValue( "DATE" ) : std::string();
    std::string apids;
    std::string extra;
    for ( size_t i=0; i<sour.children.size(); ++i )
    {
        const SGedcomNode& c = sour.children[ i ];
        if ( c.tag == "_APID" )
        {
            apids += c.value + ";";
        }
        else
        if ( c.tag != "PAGE" && c.tag != "DATA" && c.tag != "QUAY" && c.tag != "NOTE" && c.tag != "OBJE" && c.tag != "EVEN" && !c.value.empty() )
            extra += c.tag + "=" + c.value + ";";
    }
    const std::string page = sour.ChildValue( "PAGE" );
    const std::string quay = sour.ChildValue( "QUAY" );

    // the same citation is on a lot of records, one citation object serves all of them
    const std::string key = sourceKey + "|" + page + "|" + text + "|" + www + "|" + dataDate + "|" + apids + "|" + extra + "|" + quay;
    const std::string handle = Handle( "citation", key );
    if ( !m_citationHandles.insert( handle ).second )
        return handle;

    ++m_citationCount;
    ++m_result.citations;
    std::string xml = "<citation handle=\"" + handle + "\" id=\"" + NumberedId( 'C', m_citationCount ) + "\" change=\"" + m_change + "\">";
    xml += DateXml( dataDate );
    if ( !CGrampsText::Trim( page ).empty() )
        xml += "<page>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( page ), false ) + "</page>";
    int confidence = 2;
    if ( !quay.empty() )
    {
        confidence = ::atoi( quay.c_str() );
        if ( confidence < 0 ) confidence = 0;
        if ( confidence > 3 ) confidence = 3;
    }
    xml += "<confidence>" + Num( (UInt64) confidence ) + "</confidence>";
    const std::string noteHandle = AddNote( "Citation", "citation:" + handle + "/text", text, true );
    if ( !noteHandle.empty() )
        xml += "<noteref hlink=\"" + noteHandle + "\"/>";
    xml += NoteRefs( sour, "Citation", "citation:" + handle );
    for ( size_t i=0; i<sour.children.size(); ++i )
    {
        const SGedcomNode& c = sour.children[ i ];
        if ( c.tag == "_APID" && !c.value.empty() )
            xml += "<srcattribute type=\"Ancestry record id\" value=\"" + X( c.value ) + "\"/>";
    }
    if ( !CGrampsText::Trim( www ).empty() )
        xml += "<srcattribute type=\"Web address\" value=\"" + X( www ) + "\"/>";
    for ( size_t i=0; i<sour.children.size(); ++i )
    {
        const SGedcomNode& c = sour.children[ i ];
        if ( c.tag != "_APID" && c.tag != "PAGE" && c.tag != "DATA" && c.tag != "QUAY" && c.tag != "NOTE" && c.tag != "OBJE" && c.tag != "EVEN" && !c.value.empty() )
            xml += "<srcattribute type=\"" + CGrampsText::XmlEscape( c.tag ) + "\" value=\"" + X( c.value ) + "\"/>";
    }
    xml += "<sourceref hlink=\"" + sourceHandle + "\"/></citation>\n";
    m_citationsOut.Append( xml );
    return handle;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::CitationRefs( const SGedcomNode& node )
{GUCEF_TRACE;

    std::string refs;
    std::set< std::string > seen;
    for ( size_t i=0; i<node.children.size(); ++i )
    {
        if ( node.children[ i ].tag != "SOUR" )
            continue;
        const std::string handle = CitationHandle( node.children[ i ] );
        if ( !handle.empty() && seen.insert( handle ).second )
            refs += "<citationref hlink=\"" + handle + "\"/>";
    }
    return refs;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::ObjectRefs( const SGedcomNode& node, bool primaryFirst )
{GUCEF_TRACE;

    std::string primary;
    std::string others;
    std::set< std::string > seen;
    for ( size_t i=0; i<node.children.size(); ++i )
    {
        const SGedcomNode& o = node.children[ i ];
        if ( o.tag != "OBJE" )
            continue;
        std::string xref;
        if ( !PointerOf( o.value, xref ) )
            continue;           // a file described inline: Ancestry does not do that
        if ( m_objectIds.find( xref ) == m_objectIds.end() )
        {
            ++m_result.danglingReferences;
            Warn( "A media link points to " + xref + " which is not in the file, it was left out" );
            continue;
        }
        if ( !seen.insert( xref ).second )
            continue;

        std::string xml = "<objref hlink=\"" + Handle( "media", xref ) + "\">";

        // the cut out of a profile picture: pixels in the file, percentages in Gramps
        const SGedcomNode* crop = o.Child( "_CROP" );
        std::map< std::string, SObjectInfo >::const_iterator info = m_objects.find( xref );
        if ( crop != GUCEF_NULL && info != m_objects.end() && info->second.width > 0 && info->second.height > 0 )
        {
            const double w = info->second.width;
            const double h = info->second.height;
            double left = ::atof( crop->ChildValue( "_LEFT" ).c_str() );
            double top = ::atof( crop->ChildValue( "_TOP" ).c_str() );
            double cw = ::atof( crop->ChildValue( "_WDTH" ).c_str() );
            double ch = ::atof( crop->ChildValue( "_HGHT" ).c_str() );
            int x1 = (int) ( left / w * 100.0 + 0.5 );
            int y1 = (int) ( top / h * 100.0 + 0.5 );
            int x2 = (int) ( ( left + cw ) / w * 100.0 + 0.5 );
            int y2 = (int) ( ( top + ch ) / h * 100.0 + 0.5 );
            if ( x1 < 0 ) x1 = 0; if ( y1 < 0 ) y1 = 0;
            if ( x2 > 100 ) x2 = 100; if ( y2 > 100 ) y2 = 100;
            if ( x2 > x1 && y2 > y1 && !( x1 == 0 && y1 == 0 && x2 == 100 && y2 == 100 ) )
            {
                char buffer[ 128 ];
                ::snprintf( buffer, sizeof( buffer ), "<region corner1_x=\"%d\" corner1_y=\"%d\" corner2_x=\"%d\" corner2_y=\"%d\"/>", x1, y1, x2, y2 );
                xml += buffer;
            }
        }
        xml += "</objref>";
        if ( primaryFirst && o.ChildValue( "_PRIM" ) == "Y" && primary.empty() )
            primary = xml;
        else
            others += xml;
    }
    return primary + others;
}

/*-------------------------------------------------------------------------*/

const char*
CExporter::FindEventType( const SEventSpec* specs, size_t count, const std::string& tag )
{GUCEF_TRACE;

    for ( size_t i=0; i<count; ++i )
        if ( tag == specs[ i ].tag )
            return specs[ i ].type;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::EmitEvent( const std::string& type, const SGedcomNode& node, const std::string& ownerKey, UInt32 ordinal, const std::string& description )
{GUCEF_TRACE;

    const std::string key = ownerKey + "/event" + Num( ordinal );
    const std::string handle = Handle( "event", key );
    ++m_eventCount;
    ++m_result.events;

    std::string xml = "<event handle=\"" + handle + "\" id=\"" + NumberedId( 'E', m_eventCount ) + "\" change=\"" + m_change + "\">";
    xml += "<type>" + X( type ) + "</type>";
    xml += DateXml( node.ChildValue( "DATE" ) );
    const std::string place = node.ChildValue( "PLAC" );
    if ( !place.empty() )
    {
        const std::string placeHandle = PlaceHandle( place );
        if ( !placeHandle.empty() )
            xml += "<place hlink=\"" + placeHandle + "\"/>";
    }
    const std::string cause = CGrampsText::Trim( node.ChildValue( "CAUS" ) );
    if ( !cause.empty() )
        xml += "<cause>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( cause ), false ) + "</cause>";
    const std::string desc = CGrampsText::Trim( description );
    if ( !desc.empty() )
        xml += "<description>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( desc ), false ) + "</description>";
    xml += Attribute( "Age", node.ChildValue( "AGE" ) );
    xml += Attribute( "Agency", node.ChildValue( "AGNC" ) );
    xml += Attribute( "Address", node.ChildValue( "ADDR" ) );
    xml += NoteRefs( node, "Event Note", key );
    xml += CitationRefs( node );
    xml += ObjectRefs( node, false );
    xml += "</event>\n";
    m_eventsOut.Append( xml );
    return handle;
}

/*-------------------------------------------------------------------------*/

std::string
CExporter::BuildName( const SGedcomNode& name, bool alternate, const std::string& ownerKey )
{GUCEF_TRACE;

    std::string given = CGrampsText::Trim( name.ChildValue( "GIVN" ) );
    std::string surname = CGrampsText::Trim( name.ChildValue( "SURN" ) );
    std::string suffix = CGrampsText::Trim( name.ChildValue( "NSFX" ) );
    const std::string prefix = CGrampsText::Trim( name.ChildValue( "NPFX" ) );
    const std::string nick = CGrampsText::Trim( name.ChildValue( "NICK" ) );

    if ( given.empty() && surname.empty() )
    {
        // Given /Surname/ Suffix
        const std::string& full = name.value;
        const size_t a = full.find( '/' );
        const size_t b = a == std::string::npos ? std::string::npos : full.find( '/', a + 1 );
        if ( a != std::string::npos && b != std::string::npos )
        {
            given = CGrampsText::Trim( full.substr( 0, a ) );
            surname = CGrampsText::Trim( full.substr( a + 1, b - a - 1 ) );
            if ( suffix.empty() )
                suffix = CGrampsText::Trim( full.substr( b + 1 ) );
        }
        else
            given = CGrampsText::Trim( full );
    }
    if ( given.empty() && surname.empty() )
        return std::string();

    std::string type = "Birth Name";
    const std::string kind = CGrampsText::ToLowerAscii( CGrampsText::Trim( name.ChildValue( "TYPE" ) ) );
    if ( kind == "aka" || kind == "also known as" ) type = "Also Known As";
    else if ( kind == "married" ) type = "Married Name";
    else if ( kind == "birth" ) type = "Birth Name";
    else if ( !kind.empty() ) type = CGrampsText::Trim( name.ChildValue( "TYPE" ) );
    else if ( alternate ) type = "Also Known As";

    std::string xml = std::string( "<name" ) + ( alternate ? " alt=\"1\"" : "" ) + " type=\"" + X( type ) + "\">";
    if ( !given.empty() )
        xml += "<first>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( given ), false ) + "</first>";
    if ( !surname.empty() )
        xml += "<surname>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( surname ), false ) + "</surname>";
    if ( !suffix.empty() )
        xml += "<suffix>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( suffix ), false ) + "</suffix>";
    if ( !prefix.empty() )
        xml += "<title>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( prefix ), false ) + "</title>";
    if ( !nick.empty() )
        xml += "<nick>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( nick ), false ) + "</nick>";
    xml += NoteRefs( name, "Person Note", ownerKey + "/name" );
    xml += CitationRefs( name );
    xml += "</name>";
    return xml;
}

/*-------------------------------------------------------------------------*/

static std::string
Prettify( const std::string& tag )
{
    std::string out = tag;
    while ( !out.empty() && out[ 0 ] == '_' )
        out.erase( 0, 1 );
    out = CGrampsText::ToLowerAscii( out );
    if ( !out.empty() && out[ 0 ] >= 'a' && out[ 0 ] <= 'z' )
        out[ 0 ] = (char) ( out[ 0 ] - 'a' + 'A' );
    return out;
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitPerson( const SGedcomNode& indi )
{GUCEF_TRACE;

    const std::string handle = Handle( "person", indi.xref );
    const std::string key = "person:" + indi.xref;

    std::string gender = "U";
    std::string names, eventRefs, addresses, attributes, urls, childOf, parentIn;
    UInt32 nameCount = 0;
    UInt32 ordinal = 0;

    for ( size_t i=0; i<indi.children.size(); ++i )
    {
        const SGedcomNode& c = indi.children[ i ];
        const std::string& tag = c.tag;

        if ( tag == "SEX" )
        {
            if ( EqualsNoCase( c.value, "m" ) ) gender = "M";
            else if ( EqualsNoCase( c.value, "f" ) ) gender = "F";
        }
        else
        if ( tag == "NAME" )
        {
            const std::string name = BuildName( c, nameCount > 0, key + "/name" + Num( nameCount ) );
            if ( !name.empty() )
            {
                names += name;
                ++nameCount;
            }
        }
        else
        if ( tag == "FAMC" || tag == "FAMS" )
        {
            std::string xref;
            if ( !PointerOf( c.value, xref ) )
                continue;
            if ( m_familyIds.find( xref ) == m_familyIds.end() )
            {
                ++m_result.danglingReferences;
                Warn( "The person " + indi.xref + " points to the family " + xref + " which is not in the file, it was left out" );
                continue;
            }
            if ( tag == "FAMC" )
                childOf += "<childof hlink=\"" + Handle( "family", xref ) + "\"/>";
            else
                parentIn += "<parentin hlink=\"" + Handle( "family", xref ) + "\"/>";
        }
        else
        if ( tag == "OBJE" || tag == "SOUR" || tag == "NOTE" || tag == "CHAN" || tag == "RIN" )
        {
            // after the loop
        }
        else
        if ( tag == "EMAIL" )
        {
            if ( !CGrampsText::Trim( c.value ).empty() )
                urls += "<url type=\"E-mail\" href=\"mailto:" + X( CGrampsText::Trim( c.value ) ) + "\"/>";
        }
        else
        if ( tag == "WWW" || tag == "_WLNK" )
        {
            const std::string href = CGrampsText::Trim( tag == "WWW" ? c.value : c.ChildValue( "NOTE" ) );
            if ( !href.empty() )
            {
                const std::string title = c.ChildValue( "TITL" );
                urls += "<url type=\"Web Home\" href=\"" + X( href ) + "\"" + ( title.empty() ? "" : " description=\"" + X( title ) + "\"" ) + "/>";
            }
        }
        else
        if ( tag == "PHON" )
        {
            if ( !CGrampsText::Trim( c.value ).empty() )
                addresses += "<address><phone>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( c.value ), false ) + "</phone></address>";
        }
        else
        if ( tag == "ADDR" )
        {
            const std::string street = CGrampsText::HtmlToPlainText( c.value );
            const std::string place = c.ChildValue( "PLAC" );
            std::string a = "<address>" + DateXml( c.ChildValue( "DATE" ) );
            if ( !street.empty() )
                a += "<street>" + CGrampsText::XmlEscape( street, false ) + "</street>";
            if ( !CGrampsText::Trim( place ).empty() )
                a += "<city>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( place ), false ) + "</city>";
            a += "</address>";
            if ( !street.empty() || !CGrampsText::Trim( place ).empty() )
                addresses += a;
        }
        else
        if ( tag == "DSCR" )
            attributes += Attribute( "Description", CGrampsText::HtmlToPlainText( c.value ) );
        else
        if ( tag == "NATI" )
            attributes += Attribute( "Nationality", c.value );
        else
        if ( tag == "CAST" )
            attributes += Attribute( "Caste", c.value );
        else
        if ( tag == "SSN" )
            attributes += Attribute( "Social Security Number", c.value );
        else
        if ( tag == "NCHI" )
            attributes += Attribute( "Number of Children", c.value );
        else
        if ( tag == "EVEN" || tag == "FACT" )
        {
            const std::string type = CGrampsText::Trim( c.ChildValue( "TYPE" ) );
            if ( type == "_NEW" )
                attributes += Attribute( "Ancestry: added to the tree", c.ChildValue( "DATE" ) );
            else
            if ( type == "_COLOR" )
                attributes += Attribute( "Ancestry: color group", c.ChildValue( "NOTE" ) );
            else
            {
                const std::string name = type.empty() ? ( tag == "FACT" ? "Fact" : "Event" ) : type;
                eventRefs += "<eventref hlink=\"" + EmitEvent( name, c, key, ordinal++, c.value ) + "\" role=\"Primary\"/>";
            }
        }
        else
        {
            const char* type = FindEventType( PersonEvents, sizeof( PersonEvents ) / sizeof( PersonEvents[ 0 ] ), tag );
            if ( type != GUCEF_NULL )
            {
                std::string description = c.value == "Y" ? std::string() : c.value;
                if ( tag == "_DCAUSE" )
                {
                    // the cause is the note, it becomes the description of the event instead
                    SGedcomNode copy( c );
                    description = c.ChildValue( "NOTE" );
                    for ( size_t k = copy.children.size(); k > 0; --k )
                        if ( copy.children[ k-1 ].tag == "NOTE" )
                            copy.children.erase( copy.children.begin() + ( k-1 ) );
                    eventRefs += "<eventref hlink=\"" + EmitEvent( type, copy, key, ordinal++, CGrampsText::HtmlToPlainText( description ) ) + "\" role=\"Primary\"/>";
                }
                else
                    eventRefs += "<eventref hlink=\"" + EmitEvent( type, c, key, ordinal++, description ) + "\" role=\"Primary\"/>";
            }
            else
            if ( c.Child( "DATE" ) != GUCEF_NULL || c.Child( "PLAC" ) != GUCEF_NULL )
                eventRefs += "<eventref hlink=\"" + EmitEvent( Prettify( tag ), c, key, ordinal++, c.value == "Y" ? std::string() : c.value ) + "\" role=\"Primary\"/>";
            else
            if ( !c.value.empty() )
                attributes += Attribute( "Ancestry: " + Prettify( tag ), c.value );
        }
    }

    // the page of the person at the site
    std::string digits;
    for ( size_t i=0; i<indi.xref.size(); ++i )
        if ( indi.xref[ i ] >= '0' && indi.xref[ i ] <= '9' )
            digits += indi.xref[ i ];
    if ( !m_treeId.empty() && !digits.empty() )
        urls += "<url type=\"Web Home\" href=\"https://www.ancestry.com/family-tree/person/tree/" + CGrampsText::XmlEscape( m_treeId ) + "/person/" + digits + "/facts\" description=\"Ancestry.com\"/>";

    std::string xml = "<person handle=\"" + handle + "\" id=\"" + CGrampsText::MakeId( indi.xref ) + "\" change=\"" + m_change + "\">";
    xml += "<gender>" + gender + "</gender>";
    xml += names;
    xml += eventRefs;
    xml += ObjectRefs( indi, true );
    xml += addresses;
    xml += attributes;
    xml += urls;
    xml += childOf;
    xml += parentIn;
    xml += NoteRefs( indi, "Person Note", key );
    xml += CitationRefs( indi );
    xml += "</person>\n";
    m_peopleOut.Append( xml );
    ++m_result.people;
    m_result.personIds.push_back( indi.xref );
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitFamily( const SGedcomNode& fam )
{GUCEF_TRACE;

    const std::string handle = Handle( "family", fam.xref );
    const std::string key = "family:" + fam.xref;

    std::string father, mother, eventRefs, childRefs, attributes;
    bool married = false;
    UInt32 ordinal = 0;

    for ( size_t i=0; i<fam.children.size(); ++i )
    {
        const SGedcomNode& c = fam.children[ i ];
        const std::string& tag = c.tag;
        std::string xref;

        if ( tag == "HUSB" || tag == "WIFE" || tag == "CHIL" )
        {
            if ( !PointerOf( c.value, xref ) )
                continue;
            if ( m_personIds.find( xref ) == m_personIds.end() )
            {
                ++m_result.danglingReferences;
                Warn( "The family " + fam.xref + " points to the person " + xref + " which is not in the file, it was left out" );
                continue;
            }
            const std::string personHandle = Handle( "person", xref );
            if ( tag == "HUSB" )
                father = "<father hlink=\"" + personHandle + "\"/>";
            else
            if ( tag == "WIFE" )
                mother = "<mother hlink=\"" + personHandle + "\"/>";
            else
            {
                std::string relations;
                const std::string mrel = CGrampsText::ToLowerAscii( c.ChildValue( "_MREL" ) );
                const std::string frel = CGrampsText::ToLowerAscii( c.ChildValue( "_FREL" ) );
                const char* const kinds[ 2 ] = { mrel.c_str(), frel.c_str() };
                const char* const names[ 2 ] = { "mrel", "frel" };
                for ( int k=0; k<2; ++k )
                {
                    const std::string v( kinds[ k ] );
                    const char* mapped = GUCEF_NULL;
                    if ( v == "adopted" ) mapped = "Adopted";
                    else if ( v == "step" ) mapped = "Stepchild";
                    else if ( v == "foster" ) mapped = "Foster";
                    else if ( v == "sponsored" ) mapped = "Sponsored";
                    if ( mapped != GUCEF_NULL )
                        relations += std::string( " " ) + names[ k ] + "=\"" + mapped + "\"";
                }
                childRefs += "<childref hlink=\"" + personHandle + "\"" + relations + "/>";
            }
        }
        else
        if ( tag == "OBJE" || tag == "SOUR" || tag == "NOTE" || tag == "CHAN" || tag == "RIN" )
        {
        }
        else
        if ( tag == "EVEN" || tag == "FACT" )
        {
            const std::string type = CGrampsText::Trim( c.ChildValue( "TYPE" ) );
            const std::string name = type.empty() ? ( tag == "FACT" ? "Fact" : "Event" ) : type;
            eventRefs += "<eventref hlink=\"" + EmitEvent( name, c, key, ordinal++, c.value ) + "\" role=\"Family\"/>";
        }
        else
        {
            const char* type = FindEventType( FamilyEvents, sizeof( FamilyEvents ) / sizeof( FamilyEvents[ 0 ] ), tag );
            if ( type != GUCEF_NULL )
            {
                if ( tag == "MARR" )
                    married = true;
                eventRefs += "<eventref hlink=\"" + EmitEvent( type, c, key, ordinal++, c.value == "Y" ? std::string() : c.value ) + "\" role=\"Family\"/>";
            }
            else
            if ( c.Child( "DATE" ) != GUCEF_NULL || c.Child( "PLAC" ) != GUCEF_NULL )
                eventRefs += "<eventref hlink=\"" + EmitEvent( Prettify( tag ), c, key, ordinal++, c.value == "Y" ? std::string() : c.value ) + "\" role=\"Family\"/>";
            else
            if ( !c.value.empty() )
                attributes += Attribute( "Ancestry: " + Prettify( tag ), c.value );
        }
    }

    std::string xml = "<family handle=\"" + handle + "\" id=\"" + CGrampsText::MakeId( fam.xref ) + "\" change=\"" + m_change + "\">";
    xml += std::string( "<rel type=\"" ) + ( married ? "Married" : "Unknown" ) + "\"/>";
    xml += father;
    xml += mother;
    xml += eventRefs;
    xml += ObjectRefs( fam, false );
    xml += childRefs;
    xml += attributes;
    xml += NoteRefs( fam, "Family Note", key );
    xml += CitationRefs( fam );
    xml += "</family>\n";
    m_familiesOut.Append( xml );
    ++m_result.families;
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitSource( const SGedcomNode& sour )
{GUCEF_TRACE;

    const std::string key = "source:" + sour.xref;
    std::string xml = "<source handle=\"" + Handle( "source", sour.xref ) + "\" id=\"" + CGrampsText::MakeId( sour.xref ) + "\" change=\"" + m_change + "\">";

    std::string title = CGrampsText::Trim( sour.ChildValue( "TITL" ) );
    if ( title.empty() )
        title = CGrampsText::Trim( sour.ChildValue( "ABBR" ) );
    if ( title.empty() )
        title = "Source " + sour.xref;
    xml += "<stitle>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( title ), false ) + "</stitle>";
    if ( !CGrampsText::Trim( sour.ChildValue( "AUTH" ) ).empty() )
        xml += "<sauthor>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( sour.ChildValue( "AUTH" ) ), false ) + "</sauthor>";

    const SGedcomNode* publ = sour.Child( "PUBL" );
    if ( publ != GUCEF_NULL )
    {
        std::string info = CGrampsText::Trim( publ->value );
        const std::string date = CGrampsText::Trim( publ->ChildValue( "DATE" ) );
        const std::string place = CGrampsText::Trim( publ->ChildValue( "PLAC" ) );
        if ( !date.empty() || !place.empty() )
        {
            std::string detail = date;
            if ( !place.empty() )
                detail += ( detail.empty() ? "" : ", " ) + place;
            info += ( info.empty() ? "" : ", " ) + detail;
        }
        if ( !info.empty() )
            xml += "<spubinfo>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( info ), false ) + "</spubinfo>";
    }
    if ( !CGrampsText::Trim( sour.ChildValue( "ABBR" ) ).empty() && sour.ChildValue( "ABBR" ) != title )
        xml += "<sabbrev>" + CGrampsText::XmlEscape( CGrampsText::DecodeHtmlEntities( sour.ChildValue( "ABBR" ) ), false ) + "</sabbrev>";

    xml += NoteRefs( sour, "Source Note", key );
    const std::string text = AddNote( "Source text", key + "/text", sour.ChildValue( "TEXT" ), true );
    if ( !text.empty() )
        xml += "<noteref hlink=\"" + text + "\"/>";
    xml += ObjectRefs( sour, false );
    for ( size_t i=0; i<sour.children.size(); ++i )
    {
        const SGedcomNode& c = sour.children[ i ];
        if ( c.tag == "_APID" && !c.value.empty() )
            xml += "<srcattribute type=\"Ancestry database id\" value=\"" + X( c.value ) + "\"/>";
    }
    for ( size_t i=0; i<sour.children.size(); ++i )
    {
        const SGedcomNode& c = sour.children[ i ];
        if ( c.tag != "REPO" )
            continue;
        std::string xref;
        if ( !PointerOf( c.value, xref ) )
            continue;
        if ( m_repoIds.find( xref ) == m_repoIds.end() )
        {
            ++m_result.danglingReferences;
            Warn( "The source " + sour.xref + " points to the repository " + xref + " which is not in the file, it was left out" );
            continue;
        }
        xml += "<reporef hlink=\"" + Handle( "repository", xref ) + "\"";
        if ( !CGrampsText::Trim( c.ChildValue( "CALN" ) ).empty() )
            xml += " callno=\"" + X( c.ChildValue( "CALN" ) ) + "\"";
        xml += "/>";
    }
    xml += "</source>\n";
    m_sourcesOut.Append( xml );
    ++m_result.sources;
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitRepository( const SGedcomNode& repo )
{GUCEF_TRACE;

    const std::string name = CGrampsText::Trim( CGrampsText::DecodeHtmlEntities( repo.ChildValue( "NAME" ) ) );
    const std::string lower = CGrampsText::ToLowerAscii( name );
    const bool isWeb = lower.compare( 0, 4, "www." ) == 0 || lower.compare( 0, 4, "http" ) == 0;

    std::string xml = "<repository handle=\"" + Handle( "repository", repo.xref ) + "\" id=\"" + CGrampsText::MakeId( repo.xref ) + "\" change=\"" + m_change + "\">";
    xml += "<rname>" + CGrampsText::XmlEscape( name.empty() ? "Repository " + repo.xref : name, false ) + "</rname>";
    xml += std::string( "<type>" ) + ( isWeb ? "Web site" : "Unknown" ) + "</type>";
    // the export sometimes has an empty <line></line> where an address should be
    const std::string address = CGrampsText::HtmlToPlainText( repo.ChildValue( "ADDR" ) );
    if ( !address.empty() )
        xml += "<address><street>" + CGrampsText::XmlEscape( address, false ) + "</street></address>";
    if ( isWeb )
        xml += "<url type=\"Web Home\" href=\"" + X( lower.compare( 0, 4, "http" ) == 0 ? name : "https://" + name ) + "\"/>";
    xml += NoteRefs( repo, "Repository Note", "repository:" + repo.xref );
    xml += "</repository>\n";
    m_reposOut.Append( xml );
    ++m_result.repositories;
}

/*-------------------------------------------------------------------------*/

void
CExporter::EmitObject( const SGedcomNode& obje )
{GUCEF_TRACE;

    std::map< std::string, SObjectInfo >::const_iterator info = m_objects.find( obje.xref );
    const std::string oid = info != m_objects.end() ? info->second.oid : obje.xref;

    std::string title, ext;
    const SGedcomNode* file = obje.Child( "FILE" );
    if ( file != GUCEF_NULL )
    {
        title = file->ChildValue( "TITL" );
        ext = CGrampsText::ToLowerAscii( CGrampsText::Trim( file->ChildValue( "FORM" ) ) );
    }

    std::string src;
    bool present = false;
    std::map< std::string, SMediaFile >::const_iterator found = m_mediaFiles.find( oid );
    if ( found != m_mediaFiles.end() )
    {
        src = found->second.fileName;
        present = true;
        const size_t dot = src.rfind( '.' );
        if ( dot != std::string::npos )
            ext = CGrampsText::ToLowerAscii( src.substr( dot + 1 ) );
        ++m_result.mediaFilesPresent;
    }
    else
    {
        // not downloaded (yet): where the file would be, so that it is found when it is there later
        for ( size_t i=0; i<ext.size(); ++i )
            if ( !( ( ext[ i ] >= 'a' && ext[ i ] <= 'z' ) || ( ext[ i ] >= '0' && ext[ i ] <= '9' ) ) )
                ext = std::string();
        if ( ext.empty() )
            ext = "bin";
        src = CGrampsText::MakeId( oid ) + "." + ext;
        ++m_result.mediaFilesMissing;
    }

    std::string description = CGrampsText::Trim( CGrampsText::DecodeHtmlEntities( title ) );
    if ( description.empty() )
        description = oid;

    std::string xml = "<object handle=\"" + Handle( "media", obje.xref ) + "\" id=\"" + CGrampsText::MakeId( obje.xref ) + "\" change=\"" + m_change + "\">";
    xml += "<file src=\"" + CGrampsText::XmlEscape( src ) + "\" mime=\"" + MimeOfExtension( ext ) + "\" description=\"" + CGrampsText::XmlEscape( description ) + "\"/>";
    xml += Attribute( "Ancestry media id", oid );
    xml += Attribute( "Ancestry: created", obje.ChildValue( "_CREA" ) );
    xml += Attribute( "Ancestry: kind", file != GUCEF_NULL && file->Child( "FORM" ) != GUCEF_NULL ? file->Child( "FORM" )->ChildValue( "_MTYPE" ) : std::string() );
    xml += Attribute( "Place", obje.ChildValue( "PLAC" ) );
    const std::string noteHandle = AddNote( "Media Note", "media:" + obje.xref + "/description", obje.ChildValue( "_DSCR" ), true );
    if ( !noteHandle.empty() )
        xml += "<noteref hlink=\"" + noteHandle + "\"/>";
    xml += NoteRefs( obje, "Media Note", "media:" + obje.xref );
    xml += DateXml( obje.ChildValue( "DATE" ) );
    xml += CitationRefs( obje );
    if ( !present )
        xml += "<tagref hlink=\"" + m_missingTag + "\"/>";
    xml += "</object>\n";
    m_objectsOut.Append( xml );
    ++m_result.mediaObjects;
}

/*-------------------------------------------------------------------------*/

bool
CExporter::Assemble( void )
{GUCEF_TRACE;

    const CORE::CString temp = m_options.outputPath + ".tmp";
    CORE::CFileAccess out( temp, "wb" );
    if ( !out.Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Could not write " + temp );
        return false;
    }

    bool ok = true;
    UInt64 total = 0;
    std::string chunk;
    chunk.reserve( 1024 * 1024 );
    #define PUT( text ) { const std::string put_ = ( text ); ok = ok && out.Write( put_.data(), 1, (UInt32) put_.size() ) == put_.size(); total += put_.size(); }

    PUT( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE database PUBLIC \"-//GRAMPS//DTD GRAMPS XML V1.7.2//EN\"\n\"http://gramps-project.org/xml/1.7.2/grampsxml.dtd\">\n" );
    PUT( "<database xmlns=\"http://gramps-project.org/xml/1.7.2/\">\n<header>\n<created date=\"" + m_createdDate + "\" version=\"6.0.0\"/>\n" );
    // the researcher element has to be there, the schema does not take a header without it
    PUT( "<researcher>" );
    if ( !m_options.researcherName.empty() )
        PUT( "<resname>" + CGrampsText::XmlEscape( m_options.researcherName, false ) + "</resname>" );
    if ( !m_options.researcherEmail.empty() )
        PUT( "<resemail>" + CGrampsText::XmlEscape( m_options.researcherEmail, false ) + "</resemail>" );
    PUT( "</researcher>\n" );
    PUT( "<mediapath>" + CGrampsText::XmlEscape( m_options.mediaBasePath, false ) + "</mediapath>\n</header>\n" );

    // the sections in the order the format wants them
    struct SSection { const char* element; const char* attributes; CSectionFile* file; };
    const std::string homeAttribute = m_result.homePerson.empty() ? std::string() : " home=\"" + Handle( "person", m_result.homePerson ) + "\"";
    const SSection sections[] =
    {
        { "tags", "", &m_tagsOut },
        { "events", "", &m_eventsOut },
        { "people", homeAttribute.c_str(), &m_peopleOut },
        { "families", "", &m_familiesOut },
        { "citations", "", &m_citationsOut },
        { "sources", "", &m_sourcesOut },
        { "places", "", GUCEF_NULL },
        { "objects", "", &m_objectsOut },
        { "repositories", "", &m_reposOut },
        { "notes", "", &m_notesOut }
    };

    for ( size_t s=0; s<sizeof( sections ) / sizeof( sections[ 0 ] ) && ok; ++s )
    {
        const SSection& section = sections[ s ];
        if ( section.file == GUCEF_NULL )
        {
            // the places are the ones that were used, each with the one above it
            if ( m_placeOrder.empty() )
                continue;
            PUT( "<places>\n" );
            for ( size_t p=0; p<m_placeOrder.size() && ok; ++p )
            {
                const std::map< std::string, SPlaceInfo >::const_iterator place = m_places.find( m_placeOrder[ p ] );
                std::string type = "Unknown";
                if ( place->second.parentKey.empty() )
                    for ( size_t c=0; c<sizeof( CountryNames ) / sizeof( CountryNames[ 0 ] ); ++c )
                        if ( m_placeOrder[ p ] == CountryNames[ c ] )
                            type = "Country";
                std::string xml = "<placeobj handle=\"" + Handle( "place", m_placeOrder[ p ] ) + "\" id=\"" + NumberedId( 'P', place->second.number ) + "\" change=\"" + m_change + "\" type=\"" + type + "\">";
                xml += "<pname value=\"" + CGrampsText::XmlEscape( place->second.name ) + "\"/>";
                if ( !place->second.parentKey.empty() )
                    xml += "<placeref hlink=\"" + Handle( "place", place->second.parentKey ) + "\"/>";
                xml += "</placeobj>\n";
                PUT( xml );
            }
            PUT( "</places>\n" );
            m_result.places = (UInt32) m_placeOrder.size();
            continue;
        }

        if ( section.file->GetSize() == 0 )
            continue;
        PUT( std::string( "<" ) + section.element + section.attributes + ">\n" );
        CORE::CFileAccess in( section.file->GetPath(), "rb" );
        if ( !in.Opened() )
        {
            ok = false;
            break;
        }
        chunk.resize( 1024 * 1024 );
        for ( ;; )
        {
            const UInt32 got = in.Read( &chunk[ 0 ], 1, (UInt32) chunk.size() );
            if ( got == 0 )
                break;
            ok = ok && out.Write( chunk.data(), 1, got ) == got;
            total += got;
        }
        PUT( std::string( "</" ) + section.element + ">\n" );
    }
    PUT( "</database>\n" );
    #undef PUT

    out.Close();
    if ( !ok )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Writing " + temp + " failed" );
        CORE::DeleteFile( temp );
        return false;
    }
    if ( !CORE::MoveFile( temp, m_options.outputPath, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Could not move " + temp + " to " + m_options.outputPath );
        CORE::DeleteFile( temp );
        return false;
    }
    m_result.bytesWritten = total;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CExporter::Run( void )
{GUCEF_TRACE;

    if ( m_options.gedcomPath.IsNULLOrEmpty() || m_options.outputPath.IsNULLOrEmpty() )
        return false;
    if ( !LoadInput() )
        return false;

    ScanMediaDir();
    ReadHeader();
    PrepareObjects();
    CORE::CreateDirs( CORE::StripFilename( m_options.outputPath ) );
    if ( !OpenSections() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Could not create the working files next to " + m_options.outputPath );
        return false;
    }

    // the home person: the one asked for, or the one the file starts with
    const std::vector< SGedcomRecordRef >& records = m_gedcom.GetRecords();
    if ( !m_options.homePerson.empty() && m_personIds.find( m_options.homePerson ) != m_personIds.end() )
        m_result.homePerson = m_options.homePerson;
    else
    {
        if ( !m_options.homePerson.empty() )
            Warn( "The home person " + m_options.homePerson + " is not in the file, the first person is used" );
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].type == "INDI" )
            {
                m_result.homePerson = records[ i ].xref;
                break;
            }
    }

    m_missingTag = Handle( "tag", "Ancestry: media file not downloaded" );
    m_tagsOut.Append( "<tag handle=\"" + m_missingTag + "\" name=\"Ancestry: media file not downloaded\" color=\"#EF2929\" priority=\"0\" change=\"" + m_change + "\"/>\n" );

    SGedcomNode node;
    for ( size_t i=0; i<records.size(); ++i )
    {
        const SGedcomRecordRef& record = records[ i ];
        const std::string& type = record.type;
        if ( type != "INDI" && type != "FAM" && type != "SOUR" && type != "REPO" && type != "OBJE" )
            continue;
        if ( !m_gedcom.ParseRecord( record, node ) )
            continue;

        if ( type == "INDI" ) EmitPerson( node );
        else if ( type == "FAM" ) EmitFamily( node );
        else if ( type == "SOUR" ) EmitSource( node );
        else if ( type == "REPO" ) EmitRepository( node );
        else EmitObject( node );

        if ( i % 25000 == 24999 )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Converted " + CORE::SizeTToString( i + 1 ) + " of " + CORE::SizeTToString( records.size() ) + " records" );
    }

    bool ok = true;
    CSectionFile* all[] = { &m_tagsOut, &m_eventsOut, &m_peopleOut, &m_familiesOut, &m_citationsOut, &m_sourcesOut, &m_objectsOut, &m_reposOut, &m_notesOut };
    for ( size_t i=0; i<sizeof( all ) / sizeof( all[ 0 ] ); ++i )
    {
        all[ i ]->Close();
        ok = ok && all[ i ]->IsOk();
    }
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AncestryBackup: Writing the working files failed, is the disk full?" );
    ok = ok && Assemble();

    for ( size_t i=0; i<sizeof( all ) / sizeof( all[ 0 ] ); ++i )
        CORE::DeleteFile( all[ i ]->GetPath() );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CGrampsExporter::Export( const SGrampsExportOptions& options, SGrampsExportResult& result )
{GUCEF_TRACE;

    result = SGrampsExportResult();
    SGrampsExportOptions effective( options );
    if ( effective.mediaBasePath.empty() && !effective.mediaDir.IsNULLOrEmpty() )
        effective.mediaBasePath = CORE::RelativePath( effective.mediaDir ).STL_String();
    CExporter exporter( effective, result );
    return exporter.Run();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

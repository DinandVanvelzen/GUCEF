/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "tablecodecpluginXLSX_CXlsxWorkbook.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline UInt32
TextLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

static bool
EndsWith( const CORE::CString& text, const char* ending )
{
    const UInt32 length = TextLength( text );
    const UInt32 endingLength = (UInt32) strlen( ending );
    return length >= endingLength && 0 == memcmp( text.C_String() + length - endingLength, ending, endingLength );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
AttributeText( const CORE::CVariant& value )
{
    CORE::CString holder;
    const char* text = GUCEF_NULL;
    UInt32 length = 0;
    CXlsxXmlCallbacks::TextOf( value, holder, text, length );
    return CORE::CString( text, length );
}

/*-------------------------------------------------------------------------*/

static bool
ParseUInt32Text( const CORE::CString& text, UInt32& value )
{
    const UInt32 length = TextLength( text );
    if ( 0 == length || length > 10 )
        return false;
    UInt64 result = 0;
    for ( UInt32 i = 0; i < length; ++i )
    {
        const char c = text.C_String()[ i ];
        if ( c < '0' || c > '9' )
            return false;
        result = result * 10 + (UInt64) ( c - '0' );
    }
    if ( result > GUCEF_UINT32MAX )
        return false;
    value = (UInt32) result;
    return true;
}

/*-------------------------------------------------------------------------*/

static void
AppendUtf8( CORE::CString& text, UInt32 codePoint )
{
    char bytes[ 5 ];
    UInt32 count = 0;
    if ( codePoint < 0x80 )
    {
        bytes[ count++ ] = (char) codePoint;
    }
    else
    if ( codePoint < 0x800 )
    {
        bytes[ count++ ] = (char) ( 0xC0 | ( codePoint >> 6 ) );
        bytes[ count++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else
    if ( codePoint < 0x10000 )
    {
        bytes[ count++ ] = (char) ( 0xE0 | ( codePoint >> 12 ) );
        bytes[ count++ ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        bytes[ count++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else
    {
        bytes[ count++ ] = (char) ( 0xF0 | ( codePoint >> 18 ) );
        bytes[ count++ ] = (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
        bytes[ count++ ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        bytes[ count++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    bytes[ count ] = '\0';
    text += bytes;
}

/*-------------------------------------------------------------------------*/

/** Reads _xHHHH_ at the position: the code point, and the length (7) when it is there */
static bool
ReadEscape( const char* text, UInt32 length, UInt32 pos, UInt32& code )
{
    if ( pos + 7 > length || '_' != text[ pos ] || 'x' != text[ pos + 1 ] || '_' != text[ pos + 6 ] )
        return false;
    code = 0;
    for ( UInt32 i = 2; i < 6; ++i )
    {
        const char c = text[ pos + i ];
        UInt32 digit = 0;
        if ( c >= '0' && c <= '9' ) digit = (UInt32) ( c - '0' );
        else if ( c >= 'a' && c <= 'f' ) digit = (UInt32) ( c - 'a' + 10 );
        else if ( c >= 'A' && c <= 'F' ) digit = (UInt32) ( c - 'A' + 10 );
        else return false;
        code = code * 16 + digit;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CXlsxWorkbook::DecodeEscapes( const char* text, UInt32 length, CORE::CString& decoded )
{GUCEF_TRACE;

    decoded.Clear();
    if ( GUCEF_NULL == memchr( text, '_', length ) )
    {
        decoded = CORE::CString( text, length );
        return;
    }

    UInt32 plainStart = 0;
    UInt32 i = 0;
    while ( i < length )
    {
        UInt32 code = 0;
        if ( '_' == text[ i ] && ReadEscape( text, length, i, code ) )
        {
            if ( i > plainStart )
                decoded += CORE::CString( text + plainStart, i - plainStart );
            UInt32 consumed = 7;
            if ( code >= 0xD800 && code <= 0xDBFF )
            {
                UInt32 low = 0;
                if ( ReadEscape( text, length, i + 7, low ) && low >= 0xDC00 && low <= 0xDFFF )
                {
                    code = 0x10000 + ( ( code - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                    consumed = 14;
                }
                else
                    code = 0xFFFD;
            }
            else
            if ( code >= 0xDC00 && code <= 0xDFFF )
                code = 0xFFFD;
            AppendUtf8( decoded, code );
            i += consumed;
            plainStart = i;
        }
        else
            ++i;
    }
    if ( length > plainStart )
        decoded += CORE::CString( text + plainStart, length - plainStart );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PARSERS                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The relationships of a part: id, type and target */
class CXlsxRelationshipsParser : public CXlsxXmlCallbacks
{
    public:

    struct SRelationship
    {
        CORE::CString id;
        CORE::CString type;
        CORE::CString target;
        bool external;
        SRelationship( void ) : id(), type(), target(), external( false ) {}
    };

    GUCEF::vector< SRelationship > relationships;

    virtual void OnNodeBegin( const CORE::CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsName( name, length, "Relationship" ) )
            relationships.push_back( SRelationship() );
    }

    virtual void OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( !IsName( name, length, "Relationship" ) || relationships.empty() )
            return;
        const char* attribute = LocalName( attributeId, length );
        SRelationship& relationship = relationships.back();
        if ( IsName( attribute, length, "Id" ) ) relationship.id = AttributeText( attributeValue );
        else if ( IsName( attribute, length, "Type" ) ) relationship.type = AttributeText( attributeValue );
        else if ( IsName( attribute, length, "Target" ) ) relationship.target = AttributeText( attributeValue );
        else if ( IsName( attribute, length, "TargetMode" ) ) relationship.external = AttributeText( attributeValue ).Equals( "External", false );
    }

    virtual void OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnNodeEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE {}

    const SRelationship* FindById( const CORE::CString& id ) const
    {
        for ( size_t i = 0; i < relationships.size(); ++i )
            if ( relationships[ i ].id == id )
                return &relationships[ i ];
        return GUCEF_NULL;
    }

    /** The first relationship whose type ends with the text */
    const SRelationship* FindByType( const char* typeEnding ) const
    {
        for ( size_t i = 0; i < relationships.size(); ++i )
            if ( EndsWith( relationships[ i ].type, typeEnding ) )
                return &relationships[ i ];
        return GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

/** xl/workbook.xml: the date system and the sheets */
class CXlsxWorkbookParser : public CXlsxXmlCallbacks
{
    public:

    struct SSheetEntry
    {
        CORE::CString name;
        CORE::CString relationshipId;
        bool hidden;
        SSheetEntry( void ) : name(), relationshipId(), hidden( false ) {}
    };

    bool date1904;
    GUCEF::vector< SSheetEntry > sheets;

    explicit CXlsxWorkbookParser( UInt32 maxSheets ) : date1904( false ), sheets(), m_maxSheets( maxSheets ) {}

    virtual void OnNodeBegin( const CORE::CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsName( name, length, "sheet" ) )
        {
            if ( sheets.size() >= m_maxSheets )
            {
                Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The workbook has more than " + CORE::UInt32ToString( m_maxSheets ) + " sheets (maxSheets)" );
                return;
            }
            sheets.push_back( SSheetEntry() );
        }
    }

    virtual void OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( IsFailed() )
            return;
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsName( name, length, "workbookPr" ) )
        {
            const char* attribute = LocalName( attributeId, length );
            if ( IsName( attribute, length, "date1904" ) )
            {
                const CORE::CString text = AttributeText( attributeValue );
                date1904 = text.Equals( "1", false ) || text.Equals( "true", false );
            }
        }
        else
        if ( IsName( name, length, "sheet" ) && !sheets.empty() )
        {
            const char* attribute = LocalName( attributeId, length );
            SSheetEntry& sheet = sheets.back();
            if ( IsName( attribute, length, "name" ) ) sheet.name = AttributeText( attributeValue );
            else if ( IsName( attribute, length, "id" ) ) sheet.relationshipId = AttributeText( attributeValue );
            else if ( IsName( attribute, length, "state" ) ) sheet.hidden = !AttributeText( attributeValue ).Equals( "visible", false );
        }
    }

    virtual void OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void OnNodeEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE {}

    private:

    UInt32 m_maxSheets;
};

/*-------------------------------------------------------------------------*/

/** xl/sharedStrings.xml: the text of every si, rich text runs joined and the phonetic runs left out */
class CXlsxSharedStringsParser : public CXlsxXmlCallbacks
{
    public:

    CXlsxSharedStringsParser( CXlsxWorkbook& workbook, const SXlsxReadOptions& options )
        : m_workbook( workbook ), m_options( options ), m_inSi( false ), m_phoneticDepth( 0 ), m_current()
    {
        m_workbook.m_stringBytes.clear();
        m_workbook.m_stringOffsets.clear();
        m_workbook.m_stringOffsets.push_back( 0 );
    }

    virtual void OnNodeBegin( const CORE::CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        UInt8 code = 0;
        if ( IsName( name, length, "si" ) ) { code = 1; m_inSi = true; m_current.Clear(); }
        else if ( IsName( name, length, "rPh" ) ) { code = 2; ++m_phoneticDepth; }
        m_stack.push_back( code );
    }

    virtual void OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsFailed() || !m_inSi || 0 != m_phoneticDepth || !IsName( name, length, "t" ) )
            return;
        CORE::CString holder;
        const char* text = GUCEF_NULL;
        UInt32 textLength = 0;
        TextOf( nodeValue, holder, text, textLength );
        if ( (UInt64) TextLength( m_current ) + textLength > m_options.maxCellBytes )
        {
            Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "A shared string is longer than " + CORE::UInt32ToString( m_options.maxCellBytes ) + " bytes (maxCellBytes)" );
            return;
        }
        m_current += CORE::CString( text, textLength );
    }

    virtual void OnNodeEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_stack.empty() )
            return;
        const UInt8 code = m_stack.back();
        m_stack.pop_back();
        if ( 2 == code && m_phoneticDepth > 0 )
            --m_phoneticDepth;
        if ( 1 == code && m_inSi )
        {
            m_inSi = false;
            if ( IsFailed() )
                return;
            CORE::CString decoded;
            CXlsxWorkbook::DecodeEscapes( m_current.C_String(), TextLength( m_current ), decoded );
            const UInt32 length = TextLength( decoded );
            if ( (UInt64) m_workbook.m_stringBytes.size() + length > m_options.maxSharedStringBytes || m_workbook.m_stringBytes.size() + length >= GUCEF_UINT32MAX )
            {
                Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "The shared strings hold more than " + CORE::UInt64ToString( m_options.maxSharedStringBytes ) + " bytes of text (maxSharedStringBytes)" );
                return;
            }
            m_workbook.m_stringBytes.insert( m_workbook.m_stringBytes.end(), decoded.C_String(), decoded.C_String() + length );
            m_workbook.m_stringOffsets.push_back( (UInt32) m_workbook.m_stringBytes.size() );
        }
    }

    private:

    CXlsxWorkbook& m_workbook;
    const SXlsxReadOptions& m_options;
    GUCEF::vector< UInt8 > m_stack;
    bool m_inSi;
    UInt32 m_phoneticDepth;
    CORE::CString m_current;
};

/*-------------------------------------------------------------------------*/

/** xl/styles.xml: the number format of every cell format */
class CXlsxStylesParser : public CXlsxXmlCallbacks
{
    public:

    CXlsxStylesParser( void ) : m_inCellXfs( false ), m_customKinds(), m_formatIds(), m_numFmtId( 0 ), m_numFmtCode(), m_hasNumFmtId( false ) {}

    virtual void OnNodeBegin( const CORE::CVariant& nodeId, int nodeType ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsName( name, length, "cellXfs" ) )
            m_inCellXfs = true;
        else
        if ( IsName( name, length, "xf" ) && m_inCellXfs )
            m_formatIds.push_back( 0 );
        else
        if ( IsName( name, length, "numFmt" ) )
        {
            m_numFmtId = 0;
            m_hasNumFmtId = false;
            m_numFmtCode.Clear();
        }
    }

    virtual void OnNodeAttribute( const CORE::CVariant& nodeId, const CORE::CVariant& attributeId, const CORE::CVariant& attributeValue ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( IsFailed() )
            return;
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        const bool isXf = IsName( name, length, "xf" );
        const bool isNumFmt = IsName( name, length, "numFmt" );
        if ( !( ( isXf && m_inCellXfs && !m_formatIds.empty() ) || isNumFmt ) )
            return;
        const char* attribute = LocalName( attributeId, length );
        if ( IsName( attribute, length, "numFmtId" ) )
        {
            UInt32 id = 0;
            if ( !ParseUInt32Text( AttributeText( attributeValue ), id ) )
            {
                Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "A number format id is not a number: \"" + AttributeText( attributeValue ) + "\"" );
                return;
            }
            if ( isXf )
                m_formatIds.back() = id;
            else
            {
                m_numFmtId = id;
                m_hasNumFmtId = true;
            }
        }
        else
        if ( isNumFmt && IsName( attribute, length, "formatCode" ) )
            m_numFmtCode = AttributeText( attributeValue );
    }

    virtual void OnNodeValue( const CORE::CVariant& nodeId, const CORE::CVariant& nodeValue ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void OnNodeEnd( const CORE::CVariant& nodeId ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 length = 0;
        const char* name = LocalName( nodeId, length );
        if ( IsName( name, length, "cellXfs" ) )
            m_inCellXfs = false;
        else
        if ( IsName( name, length, "numFmt" ) && m_hasNumFmtId )
            m_customKinds[ m_numFmtId ] = (UInt8) CXlsxDates::KindOfFormatCode( m_numFmtCode );
    }

    void GetKinds( GUCEF::vector< UInt8 >& kinds ) const
    {
        kinds.clear();
        for ( size_t i = 0; i < m_formatIds.size(); ++i )
        {
            GUCEF::map< UInt32, UInt8 >::const_iterator found = m_customKinds.find( m_formatIds[ i ] );
            if ( found != m_customKinds.end() )
                kinds.push_back( found->second );
            else
                kinds.push_back( (UInt8) CXlsxDates::KindOfBuiltInFormat( m_formatIds[ i ] ) );
        }
    }

    private:

    bool m_inCellXfs;
    GUCEF::map< UInt32, UInt8 > m_customKinds;
    GUCEF::vector< UInt32 > m_formatIds;
    UInt32 m_numFmtId;
    CORE::CString m_numFmtCode;
    bool m_hasNumFmtId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SXlsxSheetInfo::SXlsxSheetInfo( void )
    : name()
    , partName()
    , isWorksheet( false )
    , hidden( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxWorkbook::CXlsxWorkbook( const SXlsxReadOptions& options )
    : m_options( options )
    , m_date1904( false )
    , m_sheets()
    , m_stringBytes()
    , m_stringOffsets()
    , m_styleKinds()
{GUCEF_TRACE;

    m_stringOffsets.push_back( 0 );
}

/*-------------------------------------------------------------------------*/

CXlsxWorkbook::~CXlsxWorkbook()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static CORE::CString
RelationshipsPartOf( const CORE::CString& part )
{
    const char* text = part.C_String();
    Int32 slash = -1;
    for ( UInt32 i = 0; i < TextLength( part ); ++i )
        if ( '/' == text[ i ] )
            slash = (Int32) i;
    if ( slash < 0 )
        return "_rels/" + part + ".rels";
    return CORE::CString( text, (UInt32) slash + 1 ) + "_rels/" + CORE::CString( text + slash + 1 ) + ".rels";
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxWorkbook::Load( CXlsxPackage& package, CORE::CString& error )
{GUCEF_TRACE;

    m_sheets.clear();
    m_date1904 = false;
    m_styleKinds.clear();
    m_stringBytes.clear();
    m_stringOffsets.clear();
    m_stringOffsets.push_back( 0 );

    // the workbook part, by the relationships of the package
    CORE::CString workbookPart = "xl/workbook.xml";
    if ( package.HasPart( "_rels/.rels" ) )
    {
        CXlsxRelationshipsParser rootRels;
        CORE::TDataTableStatus status = package.ParseXmlPart( "_rels/.rels", rootRels, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        const CXlsxRelationshipsParser::SRelationship* officeDocument = rootRels.FindByType( "/officeDocument" );
        if ( GUCEF_NULL != officeDocument )
        {
            if ( officeDocument->external || !CXlsxPackage::ResolveTarget( "", officeDocument->target, workbookPart ) )
            {
                error = "The relationship of the workbook part targets a part outside of the package: \"" + officeDocument->target + "\"";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
        }
    }
    if ( !package.HasPart( workbookPart ) )
    {
        error = "The package has no workbook part (" + workbookPart + ")";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    CXlsxWorkbookParser workbookParser( m_options.maxSheets );
    CORE::TDataTableStatus status = package.ParseXmlPart( workbookPart, workbookParser, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    m_date1904 = workbookParser.date1904;

    // the relationships of the workbook: where the sheets, the strings and the styles are
    CXlsxRelationshipsParser rels;
    const CORE::CString relsPart = RelationshipsPartOf( workbookPart );
    if ( package.HasPart( relsPart ) )
    {
        status = package.ParseXmlPart( relsPart, rels, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }

    for ( size_t i = 0; i < workbookParser.sheets.size(); ++i )
    {
        const CXlsxWorkbookParser::SSheetEntry& entry = workbookParser.sheets[ i ];
        SXlsxSheetInfo sheet;
        sheet.name = entry.name;
        sheet.hidden = entry.hidden;
        const CXlsxRelationshipsParser::SRelationship* rel = rels.FindById( entry.relationshipId );
        if ( GUCEF_NULL == rel || rel->external )
        {
            error = "The sheet \"" + entry.name + "\" has no relationship to a part of the package";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        if ( !CXlsxPackage::ResolveTarget( workbookPart, rel->target, sheet.partName ) )
        {
            error = "The sheet \"" + entry.name + "\" targets a part outside of the package: \"" + rel->target + "\"";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        sheet.isWorksheet = EndsWith( rel->type, "/worksheet" );
        m_sheets.push_back( sheet );
    }

    const CXlsxRelationshipsParser::SRelationship* strings = rels.FindByType( "/sharedStrings" );
    if ( GUCEF_NULL != strings )
    {
        CORE::CString part;
        if ( strings->external || !CXlsxPackage::ResolveTarget( workbookPart, strings->target, part ) )
        {
            error = "The shared strings target a part outside of the package: \"" + strings->target + "\"";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        CXlsxSharedStringsParser parser( *this, m_options );
        status = package.ParseXmlPart( part, parser, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }

    const CXlsxRelationshipsParser::SRelationship* styles = rels.FindByType( "/styles" );
    if ( GUCEF_NULL != styles )
    {
        CORE::CString part;
        if ( styles->external || !CXlsxPackage::ResolveTarget( workbookPart, styles->target, part ) )
        {
            error = "The styles target a part outside of the package: \"" + styles->target + "\"";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        CXlsxStylesParser parser;
        status = package.ParseXmlPart( part, parser, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        parser.GetKinds( m_styleKinds );
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxWorkbook::IsDate1904( void ) const
{GUCEF_TRACE;

    return m_date1904;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< SXlsxSheetInfo >&
CXlsxWorkbook::GetSheets( void ) const
{GUCEF_TRACE;

    return m_sheets;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxWorkbook::SelectSheet( UInt32& sheetIndex, CORE::CString& error ) const
{GUCEF_TRACE;

    if ( m_options.hasSheetName )
    {
        bool found = false;
        for ( size_t i = 0; i < m_sheets.size() && !found; ++i )
        {
            if ( m_sheets[ i ].name == m_options.sheetName )
            {
                sheetIndex = (UInt32) i;
                found = true;
            }
        }
        if ( !found )
        {
            error = "The workbook has no sheet named \"" + m_options.sheetName + "\"";
            return CORE::DATATABLE_STATUS_NOT_FOUND;
        }
    }
    else
    {
        sheetIndex = m_options.sheetIndex;
        if ( sheetIndex >= m_sheets.size() )
        {
            error = "The workbook has " + CORE::UInt32ToString( (UInt32) m_sheets.size() ) + " sheets: there is no sheet with index " + CORE::UInt32ToString( sheetIndex );
            return CORE::DATATABLE_STATUS_NOT_FOUND;
        }
    }
    if ( !m_sheets[ sheetIndex ].isWorksheet )
    {
        error = "The sheet \"" + m_sheets[ sheetIndex ].name + "\" is no worksheet (a chart sheet, for example) and holds no cells";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TXlsxNumberKind
CXlsxWorkbook::GetStyleKind( UInt32 styleIndex ) const
{GUCEF_TRACE;

    if ( styleIndex >= m_styleKinds.size() )
        return XLSX_NUMBERKIND_PLAIN;
    return (TXlsxNumberKind) m_styleKinds[ styleIndex ];
}

/*-------------------------------------------------------------------------*/

UInt32
CXlsxWorkbook::GetSharedStringCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( m_stringOffsets.size() - 1 );
}

/*-------------------------------------------------------------------------*/

bool
CXlsxWorkbook::GetSharedString( UInt32 index, const char*& text, UInt32& length ) const
{GUCEF_TRACE;

    if ( index >= GetSharedStringCount() )
        return false;
    const UInt32 start = m_stringOffsets[ index ];
    length = m_stringOffsets[ index + 1 ] - start;
    text = m_stringBytes.empty() ? "" : &m_stringBytes[ 0 ] + start;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  tablecodecpluginXLSX_TestApp: tests of the xlsx table codec plugin
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

#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H
#include "tablecodecpluginXLSX_CXlsxTableCodec.h"
#define TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#include "tablecodecpluginXLSX_CXlsxDates.h"
#define TABLECODECPLUGIN_XLSX_CXLSXDATES_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXDATES_H ? */

#include "TestXlsxSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace XLSXTEST {

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool g_pluginsLoaded = false;

bool
AddPlugin( const CString& name )
{
    #ifdef GUCEF_DEBUG_MODE
    const CString first = "$MODULEDIR$/" + name + "_d";
    const CString second = "$MODULEDIR$/" + name;
    #else
    const CString first = "$MODULEDIR$/" + name;
    const CString second = "$MODULEDIR$/" + name + "_d";
    #endif
    CPluginControl& pluginControl = CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( first, "XlsxTestPlugins", true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, "XlsxTestPlugins", true );
}

const char* const MainNamespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const char* const RelationshipNamespace = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
const char* const PackageRelationshipNamespace = "http://schemas.openxmlformats.org/package/2006/relationships";

CString
Rel( const char* id, const char* type, const CString& target )
{
    return CString( "<Relationship Id=\"" ) + id + "\" Type=\"" + RelationshipNamespace + "/" + type + "\" Target=\"" + target + "\"/>";
}

}

/*-------------------------------------------------------------------------*/

void
LoadRuntimePlugins( void )
{
    if ( g_pluginsLoaded )
        return;
    g_pluginsLoaded = true;
    AddPlugin( "vfspluginZIP" );
    AddPlugin( "dstorepluginPARSIFALXML" );
}

/*-------------------------------------------------------------------------*/

bool
ArePluginsAvailable( void )
{
    VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
    VFS::CVfsGlobal::Instance()->GetVfs().GetListOfSupportedArchiveTypes( types );
    CDStoreCodecRegistry::TDStoreCodecPtr xml;
    return types.find( "zip" ) != types.end() && CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetXmlCodec( xml ) && !xml.IsNULL();
}

/*-------------------------------------------------------------------------*/

CITableCodec&
GetXlsxCodec( void )
{
    static TABLECODECPLUGIN::XLSX::CXlsxTableCodec codec;
    return codec;
}

/*-------------------------------------------------------------------------*/

CString
Xml( const CString& text )
{
    CString result;
    for ( UInt32 i = 0; i + 1 < text.ByteSize(); ++i )
    {
        const char c = text.C_String()[ i ];
        if ( '&' == c ) result += "&amp;";
        else if ( '<' == c ) result += "&lt;";
        else if ( '>' == c ) result += "&gt;";
        else if ( '"' == c ) result += "&quot;";
        else result += c;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CString
CellOpen( const char* ref, const char* type, int style )
{
    CString text = CString( "<c r=\"" ) + ref + "\"";
    if ( GUCEF_NULL != type && 0 != type[ 0 ] )
        text += CString( " t=\"" ) + type + "\"";
    if ( style >= 0 )
        text += " s=\"" + Int32ToString( style ) + "\"";
    return text + ">";
}

/*-------------------------------------------------------------------------*/

CString CellShared( const char* ref, UInt32 index, int style ) { return CellOpen( ref, "s", style ) + "<v>" + UInt32ToString( index ) + "</v></c>"; }
CString CellNumber( const char* ref, const char* text, int style ) { return CellOpen( ref, GUCEF_NULL, style ) + "<v>" + text + "</v></c>"; }
CString CellFormulaString( const char* ref, const char* formula, const char* cached ) { return CellOpen( ref, "str", -1 ) + "<f>" + Xml( formula ) + "</f><v>" + Xml( cached ) + "</v></c>"; }
CString CellFormulaNumber( const char* ref, const char* formula, const char* cached, int style ) { return CellOpen( ref, GUCEF_NULL, style ) + "<f>" + Xml( formula ) + "</f><v>" + cached + "</v></c>"; }
CString CellFormulaOnly( const char* ref, const char* formula ) { return CellOpen( ref, GUCEF_NULL, -1 ) + "<f>" + Xml( formula ) + "</f></c>"; }
CString CellBool( const char* ref, bool value ) { return CellOpen( ref, "b", -1 ) + "<v>" + ( value ? "1" : "0" ) + "</v></c>"; }
CString CellInline( const char* ref, const char* text ) { return CellOpen( ref, "inlineStr", -1 ) + "<is><t>" + Xml( text ) + "</t></is></c>"; }
CString CellIsoDate( const char* ref, const char* text ) { return CellOpen( ref, "d", -1 ) + "<v>" + text + "</v></c>"; }
CString CellError( const char* ref, const char* text ) { return CellOpen( ref, "e", -1 ) + "<v>" + Xml( text ) + "</v></c>"; }

/*-------------------------------------------------------------------------*/

CString
Row( UInt32 number, const CString& cells )
{
    return "<row r=\"" + UInt32ToString( number ) + "\">" + cells + "</row>";
}

/*-------------------------------------------------------------------------*/

CString
Sheet( const CString& rows, const CString& behindData )
{
    return CString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"" ) + MainNamespace + "\"><sheetData>" + rows + "</sheetData>" + behindData + "</worksheet>";
}

/*-------------------------------------------------------------------------*/

CString
DateStyles( void )
{
    return CString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"" ) + MainNamespace + "\">"
           "<numFmts count=\"2\"><numFmt numFmtId=\"164\" formatCode=\"yyyy\\-mm\\-dd\\ hh:mm:ss\"/><numFmt numFmtId=\"165\" formatCode=\"#,##0.00&quot; USD, day&quot;\"/></numFmts>"
           "<cellStyleXfs count=\"1\"><xf numFmtId=\"14\"/></cellStyleXfs>"
           "<cellXfs count=\"6\"><xf numFmtId=\"0\"/><xf numFmtId=\"14\"/><xf numFmtId=\"164\"/><xf numFmtId=\"165\"/><xf numFmtId=\"46\"/><xf numFmtId=\"2\"/></cellXfs>"
           "</styleSheet>";
}

/*-------------------------------------------------------------------------*/

bool
Contains( const CString& text, const char* part )
{
    return text.HasSubstr( part, true ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
AddOption( CValueList& options, const char* key, const char* value )
{
    options.SetAllowMultipleValues( true );
    options.Set( CString( key ), CVariant( CString( value ) ) );
}

/*-------------------------------------------------------------------------*/

CString
BigSheet( UInt32 rows )
{
    CString xml;
    xml += Row( 1, CellInline( "A1", "id" ) + CellInline( "B1", "amount" ) );
    for ( UInt32 i = 0; i < rows; ++i )
    {
        const UInt32 row = i + 2;
        const CString rowText = UInt32ToString( row );
        xml += Row( row, CellNumber( ( "A" + rowText ).C_String(), UInt32ToString( i ).C_String() ) + CellNumber( ( "B" + rowText ).C_String(), ( UInt32ToString( i % 1000 ) + ".25" ).C_String() ) );
    }
    return Sheet( xml );
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder::CXlsxBuilder( void )
    : m_sheets()
    , m_parts()
    , m_removed()
    , m_sharedStringsXml()
    , m_stylesXml()
    , m_date1904( false )
{
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::AddSheet( const CString& name, const CString& sheetXml )
{
    SSheet sheet;
    sheet.name = name;
    sheet.xml = sheetXml;
    sheet.isChart = false;
    m_sheets.push_back( sheet );
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::AddChartSheet( const CString& name )
{
    SSheet sheet;
    sheet.name = name;
    sheet.xml = CString( "<chartsheet xmlns=\"" ) + MainNamespace + "\"/>";
    sheet.isChart = true;
    m_sheets.push_back( sheet );
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetSharedStrings( const CString::StringVector& strings )
{
    CString xml = CString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<sst xmlns=\"" ) + MainNamespace + "\" count=\"" +
                  UInt32ToString( (UInt32) strings.size() ) + "\" uniqueCount=\"" + UInt32ToString( (UInt32) strings.size() ) + "\">";
    for ( size_t i = 0; i < strings.size(); ++i )
        xml += "<si><t>" + Xml( strings[ i ] ) + "</t></si>";
    m_sharedStringsXml = xml + "</sst>";
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetSharedStringsXml( const CString& xml )
{
    m_sharedStringsXml = xml;
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetStyles( const CString& xml )
{
    m_stylesXml = xml;
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetDate1904( bool date1904 )
{
    m_date1904 = date1904;
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetPart( const CString& name, const CString& content )
{
    CDynamicBuffer buffer;
    buffer.Append( content.C_String(), content.ByteSize() > 0 ? content.ByteSize() - 1 : 0 );
    return SetBinaryPart( name, buffer );
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::SetBinaryPart( const CString& name, const CDynamicBuffer& content )
{
    for ( size_t i = 0; i < m_parts.size(); ++i )
    {
        if ( m_parts[ i ].name == name )
        {
            m_parts[ i ].content = content;
            return *this;
        }
    }
    SPart part;
    part.name = name;
    part.content = content;
    m_parts.push_back( part );
    return *this;
}

/*-------------------------------------------------------------------------*/

CXlsxBuilder&
CXlsxBuilder::RemovePart( const CString& name )
{
    m_removed.push_back( name );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxBuilder::Build( CDynamicBuffer& xlsx ) const
{
    // the parts of the workbook, then what the test sets
    GUCEF::vector< SPart > parts;
    const CString header = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    SPart part;

    #define XLSXTEST_ADD_PART( partName, text ) { part.name = partName; part.content.Clear(); const CString t = text; part.content.Append( t.C_String(), t.ByteSize() - 1 ); parts.push_back( part ); }

    XLSXTEST_ADD_PART( "[Content_Types].xml", header + "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"xml\" ContentType=\"application/xml\"/></Types>" );
    XLSXTEST_ADD_PART( "_rels/.rels", header + "<Relationships xmlns=\"" + PackageRelationshipNamespace + "\">" + Rel( "rId1", "officeDocument", "xl/workbook.xml" ) + "</Relationships>" );

    CString sheets;
    CString rels;
    for ( size_t i = 0; i < m_sheets.size(); ++i )
    {
        const CString number = UInt32ToString( (UInt32) i + 1 );
        const CString id = "rId" + number;
        sheets += "<sheet name=\"" + Xml( m_sheets[ i ].name ) + "\" sheetId=\"" + number + "\" r:id=\"" + id + "\"/>";
        const CString partName = ( m_sheets[ i ].isChart ? "xl/chartsheets/sheet" : "xl/worksheets/sheet" ) + number + ".xml";
        rels += Rel( id.C_String(), m_sheets[ i ].isChart ? "chartsheet" : "worksheet", ( m_sheets[ i ].isChart ? "chartsheets/sheet" : "worksheets/sheet" ) + number + ".xml" );
        XLSXTEST_ADD_PART( partName, m_sheets[ i ].xml );
    }
    if ( !m_sharedStringsXml.IsNULLOrEmpty() )
    {
        rels += Rel( "rIdStrings", "sharedStrings", "sharedStrings.xml" );
        XLSXTEST_ADD_PART( "xl/sharedStrings.xml", m_sharedStringsXml );
    }
    if ( !m_stylesXml.IsNULLOrEmpty() )
    {
        rels += Rel( "rIdStyles", "styles", "styles.xml" );
        XLSXTEST_ADD_PART( "xl/styles.xml", m_stylesXml );
    }
    XLSXTEST_ADD_PART( "xl/_rels/workbook.xml.rels", header + "<Relationships xmlns=\"" + PackageRelationshipNamespace + "\">" + rels + "</Relationships>" );
    XLSXTEST_ADD_PART( "xl/workbook.xml", header + "<workbook xmlns=\"" + MainNamespace + "\" xmlns:r=\"" + RelationshipNamespace + "\">" +
                                          ( m_date1904 ? "<workbookPr date1904=\"1\"/>" : "<workbookPr/>" ) + "<sheets>" + sheets + "</sheets></workbook>" );
    #undef XLSXTEST_ADD_PART

    for ( size_t i = 0; i < m_parts.size(); ++i )
    {
        bool replaced = false;
        for ( size_t p = 0; p < parts.size() && !replaced; ++p )
        {
            if ( parts[ p ].name == m_parts[ i ].name )
            {
                parts[ p ].content = m_parts[ i ].content;
                replaced = true;
            }
        }
        if ( !replaced )
            parts.push_back( m_parts[ i ] );
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TArchiveWriterPtr writer = vfs.CreateArchiveWriter( "zip" );
    if ( writer.IsNULL() )
        return false;
    xlsx.Clear( true );
    CDynamicBufferAccess output( xlsx, CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    if ( !writer->Open( output ) )
        return false;
    VFS::CArchiveWriterEntrySettings settings;
    settings.compression = VFS::CArchiveWriterEntrySettings::COMPRESSION_DEFLATE;
    for ( size_t i = 0; i < parts.size(); ++i )
    {
        bool removed = false;
        for ( size_t r = 0; r < m_removed.size(); ++r )
            if ( m_removed[ r ] == parts[ i ].name )
                removed = true;
        if ( removed )
            continue;
        CDynamicBufferAccess source( parts[ i ].content );
        if ( !writer->AddFile( parts[ i ].name, source, settings ) )
            return false;
    }
    return writer->Close();
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
ReadXlsx( const CDynamicBuffer& xlsx, const CValueList& options, CString& error, CDataTableSchemaPtr* schema, TBatchVector* batches,
          UInt32 batchRowCount, CITableFileMetaDataPtr* metaDataOut )
{
    CITableCodec& codec = GetXlsxCodec();
    CITableFileMetaDataPtr metaData;
    CDynamicBufferAccess metaSource( xlsx );
    TDataTableStatus status = codec.ReadMetaData( metaSource, options, metaData, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    if ( GUCEF_NULL != schema )
        *schema = metaData->GetSchema();
    if ( GUCEF_NULL != metaDataOut )
        *metaDataOut = metaData;

    CITableReaderPtr reader;
    status = codec.CreateReader( IOAccessPtr( GUCEF_NEW CDynamicBufferAccess( xlsx ) ), metaData, options, reader, error );
    if ( DATATABLE_STATUS_OK != status )
        return status;
    CDataTableReadRequest request;
    request.batchRowCount = batchRowCount;
    status = reader->OpenRowGroup( 0, request );
    if ( DATATABLE_STATUS_OK != status )
    {
        error = reader->GetLastErrorMessage();
        return status;
    }
    for ( ;; )
    {
        CDataTableBatchPtr batch;
        status = reader->ReadNextBatch( batch );
        if ( DATATABLE_STATUS_END == status )
            return DATATABLE_STATUS_OK;
        if ( DATATABLE_STATUS_OK != status )
        {
            error = reader->GetLastErrorMessage();
            return status;
        }
        if ( GUCEF_NULL != batches )
            batches->push_back( batch );
    }
}

/*-------------------------------------------------------------------------*/

CString
DescribeSchema( const CDataTableSchema& schema )
{
    CString text;
    for ( UInt32 i = 0; i < schema.GetColumnCount(); ++i )
    {
        if ( i > 0 )
            text += "|";
        text += schema.GetColumn( i ).name + ":" + schema.GetColumn( i ).GetTypeText();
    }
    return text;
}

/*-------------------------------------------------------------------------*/

static CString
CellText( const CDataTableColumnBatch& column, UInt32 row )
{
    if ( !column.IsValid( row ) )
        return "~";
    switch ( column.GetLogicalType() )
    {
        case DATATABLE_TYPE_BOOL: return 0 != ( column.GetBoolBits()[ row >> 3 ] & ( 1u << ( row & 7u ) ) ) ? "true" : "false";
        case DATATABLE_TYPE_INT64: return Int64ToString( column.GetInt64Data()[ row ] );
        case DATATABLE_TYPE_DECIMAL:
        {
            CDecimal value;
            CDecimal::FromUnscaled( column.GetInt64Data()[ row ], column.GetDecimalScale(), value );
            return value.ToString();
        }
        case DATATABLE_TYPE_DATE: return TABLECODECPLUGIN::XLSX::CXlsxDates::ToIsoText( column.GetInt32Data()[ row ], 0 );
        case DATATABLE_TYPE_TIMESTAMP:
        {
            const Int64 ticks = column.GetInt64Data()[ row ];
            Int64 days = ticks / TABLECODECPLUGIN::XLSX::CXlsxDates::MicrosPerDay;
            Int64 micros = ticks % TABLECODECPLUGIN::XLSX::CXlsxDates::MicrosPerDay;
            if ( micros < 0 )
            {
                micros += TABLECODECPLUGIN::XLSX::CXlsxDates::MicrosPerDay;
                --days;
            }
            return TABLECODECPLUGIN::XLSX::CXlsxDates::ToIsoText( (Int32) days, micros ) + ( micros == 0 ? "T00:00:00" : "" );
        }
        case DATATABLE_TYPE_FLOAT64:
        {
            char buffer[ 64 ];
            sprintf( buffer, "%.17g", column.GetFloat64Data()[ row ] );
            return buffer;
        }
        case DATATABLE_TYPE_UTF8:
        {
            const UInt32* offsets = column.GetOffsets();
            return CString( column.GetBytes() + offsets[ row ], offsets[ row + 1 ] - offsets[ row ] );
        }
        default:
            return "?";
    }
}

/*-------------------------------------------------------------------------*/

CString
DumpRows( const CDataTableSchema& schema, const TBatchVector& batches )
{
    CString text;
    for ( size_t b = 0; b < batches.size(); ++b )
    {
        for ( UInt32 row = 0; row < batches[ b ]->GetRowCount(); ++row )
        {
            for ( UInt32 c = 0; c < batches[ b ]->GetColumnCount(); ++c )
            {
                if ( c > 0 )
                    text += "|";
                text += CellText( batches[ b ]->GetColumn( c ), row );
            }
            text += "\n";
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

UInt64
CountRows( const TBatchVector& batches )
{
    UInt64 rows = 0;
    for ( size_t i = 0; i < batches.size(); ++i )
        rows += batches[ i ]->GetRowCount();
    return rows;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSXTEST */

/*-------------------------------------------------------------------------*/

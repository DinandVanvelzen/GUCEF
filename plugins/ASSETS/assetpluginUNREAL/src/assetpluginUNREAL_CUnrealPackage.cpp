/*
 *  assetpluginUNREAL: An opened Unreal package: summary, tables, soft object paths and trailer
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealPackage::CUnrealPackage( void )
    : m_source()
    , m_limits()
    , m_summary()
    , m_names()
    , m_imports()
    , m_exports()
    , m_exportRangeValid()
    , m_softPaths()
    , m_trailer()
    , m_hasTrailer( false )
    , m_dataResourceCount( -1 )
    , m_error()
    , m_problems()
    , m_bytesFetched( 0 )
    , m_open( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealPackage::~CUnrealPackage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealPackage::Close( void )
{GUCEF_TRACE;

    m_summary.Clear();
    m_names.Clear();
    m_imports.clear();
    m_exports.clear();
    m_exportRangeValid.clear();
    m_softPaths.clear();
    m_trailer.Clear();
    m_hasTrailer = false;
    m_dataResourceCount = -1;
    m_error.Clear();
    m_problems.clear();
    m_bytesFetched = 0;
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::Open( const CORE::CString& vfsPath, const CUnrealReaderLimits& limits )
{GUCEF_TRACE;

    Close();
    if ( !m_source.OpenVfs( vfsPath ) )
    {
        m_error = "The file " + vfsPath + " cannot be opened or is empty";
        return false;
    }
    return Load( limits );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::OpenEmbedded( const CORE::CString& vfsPath, UInt64 offset, UInt64 size, const CUnrealReaderLimits& limits )
{GUCEF_TRACE;

    Close();
    if ( !m_source.OpenVfsRange( vfsPath, offset, size ) )
    {
        m_error = "The range of " + CORE::UInt64ToString( size ) + " bytes at offset " + CORE::UInt64ToString( offset ) + " of the file " + vfsPath + " cannot be opened";
        return false;
    }
    return Load( limits );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::OpenMemory( const void* bytes, UInt32 size, const CORE::CString& name, const CUnrealReaderLimits& limits )
{GUCEF_TRACE;

    Close();
    m_source.OpenMemory( bytes, size, name );
    return Load( limits );
}

/*-------------------------------------------------------------------------*/

void
CUnrealPackage::AddProblem( const CORE::CString& text )
{GUCEF_TRACE;

    m_problems.push_back( text );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::CheckTable( CUnrealArchiveReader& reader, Int32 count, Int32 offset, UInt32 minEntryBytes, const char* what )
{GUCEF_TRACE;

    if ( count < 0 || offset < 0 )
    {
        m_error = CORE::CString( "The " ) + what + " table has a negative count or offset";
        return false;
    }
    if ( 0 == count )
        return true;
    if ( (UInt32) count > m_limits.maxTableEntries )
    {
        m_error = CORE::CString( "The " ) + what + " count " + CORE::Int32ToString( count ) + " exceeds the limit";
        return false;
    }
    if ( (UInt64) offset < m_summary.summarySize || (UInt64) offset >= reader.GetEnd() )
    {
        m_error = CORE::CString( "The " ) + what + " table offset " + CORE::Int32ToString( offset ) + " is outside the package header of " + CORE::UInt64ToString( reader.GetEnd() ) + " bytes";
        return false;
    }
    if ( (UInt64) count > ( reader.GetEnd() - (UInt64) offset ) / minEntryBytes )
    {
        m_error = CORE::CString( "The " ) + what + " table of " + CORE::Int32ToString( count ) + " entries cannot fit in the package header";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::Load( const CUnrealReaderLimits& limits )
{GUCEF_TRACE;

    m_limits = limits;
    const UInt64 packageSize = m_source.GetSize();
    if ( packageSize < 64 )
    {
        m_error = "The file is too small to be a package (" + CORE::UInt64ToString( packageSize ) + " bytes)";
        return false;
    }

    // the summary first: it says how large the header is
    UInt64 probe = packageSize < limits.maxHeaderBytes ? packageSize : limits.maxHeaderBytes;
    CUnrealArchiveReader reader;
    if ( !m_source.OpenRange( 0, probe, reader, 16 * 1024 ) )
    {
        m_error = "The package cannot be read";
        return false;
    }
    CORE::CString problem;
    const bool summaryOk = m_summary.Read( reader, limits, problem );
    AccountFetched( reader );
    if ( !summaryOk )
    {
        m_error = problem;
        return false;
    }
    if ( m_summary.totalHeaderSize <= 0 || (UInt64) m_summary.totalHeaderSize < m_summary.summarySize || (UInt64) m_summary.totalHeaderSize > packageSize )
    {
        m_error = "The header size " + CORE::Int32ToString( m_summary.totalHeaderSize ) + " is not valid for a package of " + CORE::UInt64ToString( packageSize ) + " bytes";
        return false;
    }
    if ( (UInt64) m_summary.totalHeaderSize > limits.maxHeaderBytes )
    {
        m_error = "The header size " + CORE::Int32ToString( m_summary.totalHeaderSize ) + " exceeds the header budget of " + CORE::UInt64ToString( limits.maxHeaderBytes ) + " bytes";
        return false;
    }
    // the whole header is wanted now: one window of its size (within the window limit) instead of the window of the summary
    CUnrealArchiveReader headerReader;
    const UInt32 headerWindow = (UInt32) m_summary.totalHeaderSize < limits.windowBytes ? (UInt32) m_summary.totalHeaderSize : limits.windowBytes;
    if ( !m_source.OpenRange( 0, (UInt64) m_summary.totalHeaderSize, headerReader, headerWindow ) )
    {
        m_error = "The package header cannot be read";
        return false;
    }
    headerReader.Seek( m_summary.summarySize );

    // names
    if ( !CheckTable( headerReader, m_summary.nameCount, m_summary.nameOffset, 8, "name" ) )
        return false;
    if ( m_summary.nameCount > 0 )
    {
        if ( !headerReader.Seek( (UInt64) m_summary.nameOffset ) || !m_names.Read( headerReader, (UInt32) m_summary.nameCount, limits ) )
        {
            m_error = "The name table cannot be read: " + headerReader.GetError();
            return false;
        }
    }

    // imports
    if ( !CheckTable( headerReader, m_summary.importCount, m_summary.importOffset, 16, "import" ) )
        return false;
    if ( m_summary.importCount > 0 )
    {
        headerReader.Seek( (UInt64) m_summary.importOffset );
        m_imports.reserve( (size_t) m_summary.importCount );
        for ( Int32 i=0; i<m_summary.importCount; ++i )
        {
            CUnrealObjectImport object;
            if ( !object.Read( headerReader, m_names, m_summary.versions, m_summary.packageFlags ) )
            {
                m_error = "Import " + CORE::Int32ToString( i ) + " cannot be read: " + headerReader.GetError();
                return false;
            }
            m_imports.push_back( object );
        }
    }

    // exports
    if ( !CheckTable( headerReader, m_summary.exportCount, m_summary.exportOffset, 40, "export" ) )
        return false;
    if ( m_summary.exportCount > 0 )
    {
        headerReader.Seek( (UInt64) m_summary.exportOffset );
        m_exports.reserve( (size_t) m_summary.exportCount );
        for ( Int32 i=0; i<m_summary.exportCount; ++i )
        {
            CUnrealObjectExport object;
            if ( !object.Read( headerReader, m_names, m_summary.versions, m_summary.packageFlags ) )
            {
                m_error = "Export " + CORE::Int32ToString( i ) + " cannot be read: " + headerReader.GetError();
                return false;
            }
            m_exports.push_back( object );
        }
    }

    // soft object path list
    if ( m_summary.versions.IsAtLeast( UE5_ADD_SOFTOBJECTPATH_LIST ) && m_summary.softObjectPathsCount > 0 )
    {
        if ( !CheckTable( headerReader, m_summary.softObjectPathsCount, m_summary.softObjectPathsOffset, 4, "soft object path" ) )
            return false;
        headerReader.Seek( (UInt64) m_summary.softObjectPathsOffset );
        m_softPaths.reserve( (size_t) m_summary.softObjectPathsCount );
        for ( Int32 i=0; i<m_summary.softObjectPathsCount; ++i )
        {
            CUnrealSoftObjectPath path;
            if ( !path.ReadListEntry( headerReader, m_names, m_summary.versions ) )
            {
                m_error = "Soft object path " + CORE::Int32ToString( i ) + " cannot be read: " + headerReader.GetError();
                return false;
            }
            m_softPaths.push_back( path );
        }
    }
    AccountFetched( headerReader );

    // the exports must lie inside the package, and behind the header
    m_exportRangeValid.assign( m_exports.size(), true );
    for ( size_t i=0; i<m_exports.size(); ++i )
    {
        const CUnrealObjectExport& object = m_exports[ i ];
        if ( object.serialOffset < 0 || object.serialSize < 0 || (UInt64) object.serialOffset > packageSize || (UInt64) object.serialSize > packageSize - (UInt64) object.serialOffset )
        {
            m_exportRangeValid[ i ] = false;
            AddProblem( "Export " + CORE::UInt64ToString( i ) + " (" + GetClassName( object ) + ") ends at " + CORE::Int64ToString( object.serialOffset + object.serialSize ) + " beyond the package size " + CORE::UInt64ToString( packageSize ) );
        }
        else
        if ( object.scriptSerializationEndOffset > object.serialSize || object.scriptSerializationStartOffset < 0 || object.scriptSerializationStartOffset > object.scriptSerializationEndOffset )
        {
            m_exportRangeValid[ i ] = false;
            AddProblem( "Export " + CORE::UInt64ToString( i ) + " (" + GetClassName( object ) + ") has a script range outside its data" );
        }
    }

    // the package trailer
    if ( m_summary.versions.IsAtLeast( UE5_PAYLOAD_TOC ) && m_summary.payloadTocOffset > 0 )
    {
        CUnrealArchiveReader header;
        CUnrealArchiveReader footer;
        CORE::CString trailerProblem;
        const UInt64 trailerOffset = (UInt64) m_summary.payloadTocOffset;
        if ( trailerOffset < packageSize && m_source.OpenRange( trailerOffset, packageSize - trailerOffset, header, 64 * 1024 ) && m_source.OpenRange( packageSize - 20, 20, footer, 4096 ) )
        {
            footer.Seek( packageSize - 20 );
            if ( m_trailer.Read( header, footer, trailerOffset, packageSize, limits, trailerProblem ) )
                m_hasTrailer = true;
            else
                AddProblem( "Package trailer: " + trailerProblem );
            AccountFetched( header );
            AccountFetched( footer );
        }
        else
            AddProblem( "The package trailer offset " + CORE::Int64ToString( m_summary.payloadTocOffset ) + " is outside the package" );
    }

    // the data resource table: only its size is read
    if ( m_summary.versions.IsAtLeast( UE5_DATA_RESOURCES ) && m_summary.dataResourceOffset > 0 )
    {
        CUnrealArchiveReader table;
        if ( (UInt64) m_summary.dataResourceOffset + 8 <= packageSize && m_source.OpenRange( (UInt64) m_summary.dataResourceOffset, 8, table, 4096 ) )
        {
            table.ReadUInt32();
            m_dataResourceCount = table.ReadInt32();
            AccountFetched( table );
        }
        else
            AddProblem( "The data resource table is outside the package" );
    }
    m_open = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool CUnrealPackage::IsOpen( void ) const { return m_open; }
const CORE::CString& CUnrealPackage::GetError( void ) const { return m_error; }
const CORE::CString::StringVector& CUnrealPackage::GetProblems( void ) const { return m_problems; }
const CUnrealPackageSource& CUnrealPackage::GetSource( void ) const { return m_source; }
const CUnrealReaderLimits& CUnrealPackage::GetLimits( void ) const { return m_limits; }
const CUnrealPackageSummary& CUnrealPackage::GetSummary( void ) const { return m_summary; }
const CUnrealVersionContext& CUnrealPackage::GetVersions( void ) const { return m_summary.versions; }
const CUnrealNameTable& CUnrealPackage::GetNames( void ) const { return m_names; }
const CUnrealPackage::TImportVector& CUnrealPackage::GetImports( void ) const { return m_imports; }
const CUnrealPackage::TExportVector& CUnrealPackage::GetExports( void ) const { return m_exports; }
const CUnrealSoftObjectPath::TSoftObjectPathVector& CUnrealPackage::GetSoftObjectPaths( void ) const { return m_softPaths; }
bool CUnrealPackage::HasTrailer( void ) const { return m_hasTrailer; }
const CUnrealPackageTrailer& CUnrealPackage::GetTrailer( void ) const { return m_trailer; }
Int32 CUnrealPackage::GetDataResourceCount( void ) const { return m_dataResourceCount; }
UInt64 CUnrealPackage::GetBytesFetched( void ) const { return m_bytesFetched; }

/*-------------------------------------------------------------------------*/

void
CUnrealPackage::AccountFetched( const CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    m_bytesFetched += reader.GetBytesFetched();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPackage::GetObjectName( const CUnrealPackageIndex& index ) const
{GUCEF_TRACE;

    if ( index.IsImport() && index.ToImportIndex() < m_imports.size() )
        return m_imports[ index.ToImportIndex() ].objectName;
    if ( index.IsExport() && index.ToExportIndex() < m_exports.size() )
        return m_exports[ index.ToExportIndex() ].objectName;
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPackage::GetClassName( const CUnrealObjectExport& object ) const
{GUCEF_TRACE;

    if ( object.classIndex.IsNull() )
        return "Class";
    return GetObjectName( object.classIndex );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPackage::GetImportPath( UInt32 importIndex ) const
{GUCEF_TRACE;

    CORE::CString path;
    CUnrealPackageIndex current( -(Int32) ( importIndex + 1 ) );
    for ( UInt32 depth=0; depth<m_limits.maxDepth && current.IsImport() && current.ToImportIndex() < m_imports.size(); ++depth )
    {
        const CUnrealObjectImport& object = m_imports[ current.ToImportIndex() ];
        const bool isPackage = object.outerIndex.IsNull();
        path = object.objectName + ( path.IsNULLOrEmpty() ? CORE::CString() : ( isPackage ? CORE::CString( "." ) : CORE::CString( ":" ) ) + path );
        current = object.outerIndex;
    }
    return path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealPackage::GetExportPath( UInt32 exportIndex ) const
{GUCEF_TRACE;

    CORE::CString path;
    CUnrealPackageIndex current( (Int32) ( exportIndex + 1 ) );
    UInt32 depth = 0;
    for ( ; depth<m_limits.maxDepth && current.IsExport() && current.ToExportIndex() < m_exports.size(); ++depth )
    {
        const CUnrealObjectExport& object = m_exports[ current.ToExportIndex() ];
        path = object.objectName + ( path.IsNULLOrEmpty() ? CORE::CString() : CORE::CString( ":" ) + path );
        current = object.outerIndex;
    }
    if ( depth >= m_limits.maxDepth )
        return path; // a cycle or a chain that is too deep: the part that was followed
    if ( current.IsImport() )
        return GetImportPath( current.ToImportIndex() ) + "." + path;
    return m_summary.packageName.IsNULLOrEmpty() ? path : m_summary.packageName + "." + path;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealPackage::FindExport( const CORE::CString& objectName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_exports.size(); ++i )
        if ( m_exports[ i ].objectName == objectName )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::IsExportRangeValid( UInt32 exportIndex ) const
{GUCEF_TRACE;

    return exportIndex < m_exportRangeValid.size() && m_exportRangeValid[ exportIndex ];
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::OpenExportRange( UInt32 exportIndex, CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    if ( !IsExportRangeValid( exportIndex ) )
        return false;
    const CUnrealObjectExport& object = m_exports[ exportIndex ];
    return m_source.OpenRange( (UInt64) object.serialOffset, (UInt64) object.serialSize, reader, m_limits.windowBytes );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::OpenPropertyRange( UInt32 exportIndex, CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    if ( !IsExportRangeValid( exportIndex ) )
        return false;
    const CUnrealObjectExport& object = m_exports[ exportIndex ];
    UInt64 start = (UInt64) object.serialOffset;
    UInt64 size = (UInt64) object.serialSize;
    if ( m_summary.versions.IsAtLeast( UE5_SCRIPT_SERIALIZATION_OFFSET ) && object.scriptSerializationEndOffset > object.scriptSerializationStartOffset )
    {
        start += (UInt64) object.scriptSerializationStartOffset;
        size = (UInt64) ( object.scriptSerializationEndOffset - object.scriptSerializationStartOffset );
    }
    if ( size > m_limits.maxPropertyBytes )
        return false;
    return m_source.OpenRange( start, size, reader, m_limits.windowBytes );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPackage::OpenPayload( const CUnrealPayloadReference& reference, CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    if ( !reference.IsReadable() )
        return false;
    return m_source.OpenRange( reference.offset, reference.storedSize, reader, m_limits.windowBytes );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "tablecodecpluginXLSX_CXlsxPackage.h"

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

namespace {

/** Uncompressed size from where the expansion ratio is held against the limit */
static const UInt64 RatioCheckMinimumBytes = 4ULL * 1024 * 1024;

MT::CTAtomic< UInt32 >&
OpenCounter( void )
{GUCEF_TRACE;

    static MT::CTAtomic< UInt32 > counter( 0 );
    return counter;
}

MT::CTAtomic< UInt32 >&
PackageCounter( void )
{GUCEF_TRACE;

    static MT::CTAtomic< UInt32 > counter( 0 );
    return counter;
}

}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeUniqueName( void )
{GUCEF_TRACE;

    UInt8 random[ 8 ];
    memset( random, 0, sizeof( random ) );
    CORE::CRandom::GetRandomBytes( random, sizeof( random ) );
    CORE::CString name = "gucefxlsx_";
    static const char hex[] = "0123456789abcdef";
    for ( UInt32 i = 0; i < sizeof( random ); ++i )
    {
        char pair[ 3 ] = { hex[ random[ i ] >> 4 ], hex[ random[ i ] & 0x0F ], '\0' };
        name += pair;
    }
    name += "_" + CORE::UInt32ToString( ++PackageCounter() );
    return name;
}

/*-------------------------------------------------------------------------*/

CXlsxPackage::CXlsxPackage( const SXlsxReadOptions& options )
    : m_options( options )
    , m_mountName()
    , m_filePath()
    , m_mounted( false )
    , m_fileBytes( 0 )
    , m_budget( options.maxUncompressedBytes )
    , m_bytesRead( 0 )
    , m_activeInput( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxPackage::~CXlsxPackage()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CXlsxPackage::Release( void )
{GUCEF_TRACE;

    if ( m_mounted )
    {
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_mountName );
        m_mounted = false;
        --OpenCounter();
    }
    if ( !m_filePath.IsNULLOrEmpty() )
    {
        if ( !CORE::DeleteFile( m_filePath ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "XlsxPackage: Failed to remove the temporary file " << m_filePath );
        m_filePath.Clear();
    }
    m_activeInput = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxPackage::Spill( CORE::CIOAccess& source, CORE::CString& error )
{GUCEF_TRACE;

    if ( 0 != source.Setpos( 0 ) )
    {
        error = "The source cannot be positioned at its start";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    const UInt64 declaredSize = source.GetSize();
    if ( declaredSize > m_options.maxFileBytes )
    {
        error = "The file is " + CORE::UInt64ToString( declaredSize ) + " bytes, more than the limit of " + CORE::UInt64ToString( m_options.maxFileBytes ) + " bytes (maxFileBytes)";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    CORE::CString path = CORE::TempDir();
    CORE::AppendToPath( path, m_mountName + ".xlsx" );
    m_filePath = path;

    CORE::CFileAccess out( m_filePath, "wb" );
    if ( !out.IsValid() )
    {
        error = "The temporary file " + m_filePath + " cannot be written";
        m_filePath.Clear();
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( !CORE::SetOwnerOnlyAccess( m_filePath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "XlsxPackage: Failed to restrict the access to the temporary file " << m_filePath );

    char buffer[ 65536 ];
    m_fileBytes = 0;
    bool first = true;
    for ( ;; )
    {
        const UInt32 got = source.Read( buffer, 1, (UInt32) sizeof( buffer ) );
        if ( 0 == got )
            break;
        if ( first )
        {
            first = false;
            if ( got < 4 || 'P' != buffer[ 0 ] || 'K' != buffer[ 1 ] )
            {
                error = "The file is not an .xlsx workbook: it is no zip archive";
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            }
        }
        m_fileBytes += got;
        if ( m_fileBytes > m_options.maxFileBytes )
        {
            error = "The file is larger than the limit of " + CORE::UInt64ToString( m_options.maxFileBytes ) + " bytes (maxFileBytes)";
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        }
        if ( out.Write( buffer, 1, got ) != got )
        {
            error = "The temporary file " + m_filePath + " cannot be written";
            return CORE::DATATABLE_STATUS_IO_ERROR;
        }
    }
    if ( first )
    {
        error = "The file is empty";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    out.Flush();
    out.Close();
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxPackage::CheckEntries( CORE::CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    VFS::CVFS::TStringVector entries;
    vfs.GetFileList( entries, "/" + m_mountName + "/", true, true, CORE::CString::Empty, m_options.maxEntries + 1 );
    if ( entries.empty() )
    {
        error = "The zip archive has no entries";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( entries.size() > m_options.maxEntries )
    {
        error = "The zip archive has more than " + CORE::UInt32ToString( m_options.maxEntries ) + " entries (maxEntries)";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    // the sizes the entries state: a bomb states what it will expand to
    UInt64 declared = 0;
    for ( size_t i = 0; i < entries.size(); ++i )
    {
        const UInt64 size = vfs.GetFileSize( entries[ i ] );
        if ( size > GUCEF_UINT64MAX - declared )
            declared = GUCEF_UINT64MAX;
        else
            declared += size;
    }
    if ( declared > m_options.maxUncompressedBytes )
    {
        error = "The zip archive states " + CORE::UInt64ToString( declared ) + " bytes of content, more than the limit of " +
                CORE::UInt64ToString( m_options.maxUncompressedBytes ) + " bytes (maxUncompressedBytes): refused as a zip bomb";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }
    if ( 0 != m_options.maxExpansionRatio && declared > RatioCheckMinimumBytes && declared / ( m_fileBytes > 0 ? m_fileBytes : 1 ) > m_options.maxExpansionRatio )
    {
        error = "The zip archive of " + CORE::UInt64ToString( m_fileBytes ) + " bytes states " + CORE::UInt64ToString( declared ) +
                " bytes of content, which is more than " + CORE::UInt32ToString( m_options.maxExpansionRatio ) + " times its size (maxExpansionRatio): refused as a zip bomb";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    if ( !HasPart( "[Content_Types].xml" ) )
    {
        error = "The zip archive is no .xlsx workbook: it has no [Content_Types].xml";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxPackage::Open( CORE::CIOAccess& source, CORE::CString& error )
{GUCEF_TRACE;

    Release();
    m_budget = m_options.maxUncompressedBytes;
    m_bytesRead = 0;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
    vfs.GetListOfSupportedArchiveTypes( types );
    if ( types.find( "zip" ) == types.end() )
    {
        error = "The VFS has no zip archive type: the plugin vfspluginZIP is not loaded";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr xmlCodec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetXmlCodec( xmlCodec ) )
    {
        error = "There is no XML codec: the plugin dstorepluginPARSIFALXML is not loaded";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    m_mountName = MakeUniqueName();
    CORE::TDataTableStatus status = Spill( source, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        Release();
        return status;
    }

    if ( !vfs.MountArchive( m_mountName, m_filePath, "zip", "/" + m_mountName, false, false, m_filePath ) )
    {
        error = "The file is not a readable zip archive";
        Release();
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    m_mounted = true;
    ++OpenCounter();

    status = CheckEntries( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        Release();
        return status;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxPackage::HasPart( const CORE::CString& partName ) const
{GUCEF_TRACE;

    if ( !m_mounted )
        return false;
    return VFS::CVfsGlobal::Instance()->GetVfs().FileExists( "/" + m_mountName + "/" + partName );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxPackage::ParseXmlPart( const CORE::CString& partName, CXlsxXmlCallbacks& callbacks, CORE::CString& error )
{GUCEF_TRACE;

    if ( !m_mounted )
    {
        error = "The package is not open";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = "/" + m_mountName + "/" + partName;
    if ( !vfs.FileExists( path ) )
    {
        error = "The workbook has no part " + partName;
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    const UInt64 declared = vfs.GetFileSize( path );
    if ( declared > m_budget )
    {
        error = "The part " + partName + " states " + CORE::UInt64ToString( declared ) + " bytes, more than the " + CORE::UInt64ToString( m_budget ) +
                " that the limit (maxUncompressedBytes) leaves: refused as a zip bomb";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( path, "rb", false );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
    {
        error = "The part " + partName + " cannot be opened";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr xmlCodec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetXmlCodec( xmlCodec ) )
    {
        error = "There is no XML codec: the plugin dstorepluginPARSIFALXML is not loaded";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    CXlsxLimitedAccess limited( resource->GetAccess(), &m_budget );
    m_activeInput = &limited;
    const bool parsed = xmlCodec->ParseDocument( limited, callbacks );
    m_activeInput = GUCEF_NULL;
    m_bytesRead += limited.GetBytesRead();

    // the parser of the part knows best what is wrong with the content, then the bounds, then the syntax
    if ( callbacks.IsFailed() )
    {
        error = partName + ": " + callbacks.GetError();
        return callbacks.GetStatus();
    }
    if ( limited.IsExceeded() )
    {
        error = "The part " + partName + " holds more than the " + CORE::UInt64ToString( m_options.maxUncompressedBytes ) +
                " bytes that the limit (maxUncompressedBytes) allows for all parts: refused as a zip bomb";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }
    if ( !parsed )
    {
        error = "The part " + partName + " is not well formed XML";
        if ( !callbacks.GetSyntaxError().IsNULLOrEmpty() )
            error += " (" + callbacks.GetSyntaxError() + ")";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CXlsxPackage::AbortParse( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_activeInput )
        m_activeInput->Abort();
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxPackage::GetBytesRead( void ) const
{GUCEF_TRACE;

    return m_bytesRead;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxPackage::GetFileBytes( void ) const
{GUCEF_TRACE;

    return m_fileBytes;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CXlsxPackage::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CXlsxPackage::GetMountName( void ) const
{GUCEF_TRACE;

    return m_mountName;
}

/*-------------------------------------------------------------------------*/

UInt32
CXlsxPackage::GetOpenPackageCount( void )
{GUCEF_TRACE;

    return OpenCounter().Load();
}

/*-------------------------------------------------------------------------*/

bool
CXlsxPackage::ResolveTarget( const CORE::CString& basePart, const CORE::CString& target, CORE::CString& partName )
{GUCEF_TRACE;

    CORE::CString combined;
    const char* targetText = target.C_String();
    if ( target.ByteSize() > 1 && '/' == targetText[ 0 ] )
    {
        combined = target;
    }
    else
    {
        const char* base = basePart.C_String();
        Int32 slash = -1;
        for ( UInt32 i = 0; i + 1 < basePart.ByteSize(); ++i )
            if ( '/' == base[ i ] )
                slash = (Int32) i;
        if ( slash >= 0 )
            combined = CORE::CString( base, (UInt32) slash + 1 );
        combined += target;
    }

    // the segments: "." goes, ".." takes the segment before it, and fails when there is none
    GUCEF::vector< CORE::CString > segments;
    const char* text = combined.C_String();
    const UInt32 length = combined.ByteSize() > 0 ? combined.ByteSize() - 1 : 0;
    UInt32 start = 0;
    for ( UInt32 i = 0; i <= length; ++i )
    {
        if ( i == length || '/' == text[ i ] || '\\' == text[ i ] )
        {
            const UInt32 segmentLength = i - start;
            if ( 0 == segmentLength || ( 1 == segmentLength && '.' == text[ start ] ) )
            {
                // nothing
            }
            else
            if ( 2 == segmentLength && '.' == text[ start ] && '.' == text[ start + 1 ] )
            {
                if ( segments.empty() )
                    return false;
                segments.pop_back();
            }
            else
                segments.push_back( CORE::CString( text + start, segmentLength ) );
            start = i + 1;
        }
    }
    if ( segments.empty() )
        return false;

    partName.Clear();
    for ( size_t i = 0; i < segments.size(); ++i )
    {
        if ( i > 0 )
            partName += "/";
        partName += segments[ i ];
    }
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

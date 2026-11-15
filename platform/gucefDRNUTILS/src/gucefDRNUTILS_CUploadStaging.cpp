/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CUploadStaging.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#include "gucefCORE_CNativeFileSystemAccess.h"
#define GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_CNATIVEFILESYSTEMACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#include "gucefDRNUTILS_CDevicePairingCrypto.h"
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char ManifestName[] = "upload.json";
const char ManifestFormat[] = "gucef.drnutils.Upload";
const char FilesDirectoryName[] = "files";

const char*
StateToString( CUploadStaging::EUploadState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case CUploadStaging::UPLOADSTATE_COMMITTED: return "COMMITTED";
        case CUploadStaging::UPLOADSTATE_ABORTED: return "ABORTED";
        default: return "RECEIVING";
    }
}

/*-------------------------------------------------------------------------*/

CUploadStaging::EUploadState
StateFromString( const CORE::CString& text )
{GUCEF_TRACE;

    if ( text == "COMMITTED" )
        return CUploadStaging::UPLOADSTATE_COMMITTED;
    if ( text == "ABORTED" )
        return CUploadStaging::UPLOADSTATE_ABORTED;
    return CUploadStaging::UPLOADSTATE_RECEIVING;
}

/*-------------------------------------------------------------------------*/

bool
IsHexDigest( const CORE::CString& text )
{GUCEF_TRACE;

    if ( 64 != text.Length() || 65 != text.ByteSize() )
        return false;
    const char* c = text.C_String();
    for ( UInt32 i=0; i<64; ++i )
    {
        char ch = c[ i ];
        if ( !( ( ch >= '0' && ch <= '9' ) || ( ch >= 'a' && ch <= 'f' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsReservedDeviceName( const CORE::CString& segment )
{GUCEF_TRACE;

    // the part before the first dot, without case: CON, PRN, AUX, NUL, COM1-9, LPT1-9
    CORE::CString base = segment;
    Int32 dot = segment.HasChar( '.' );
    if ( dot >= 0 )
        base = segment.SubstrToIndex( (UInt32) dot, true );
    base = base.Trim( false ).Uppercase();
    if ( base == "CON" || base == "PRN" || base == "AUX" || base == "NUL" )
        return true;
    if ( 4 == base.Length() && ( base.SubstrToIndex( 3, true ) == "COM" || base.SubstrToIndex( 3, true ) == "LPT" ) )
    {
        char digit = base.C_String()[ 3 ];
        return digit >= '1' && digit <= '9';
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt64
SystemUtcMs( void )
{GUCEF_TRACE;

    CORE::CSystemUtcClock clock;
    Int64 now = clock.GetUtcTimeInMs();
    return now > 0 ? (UInt64) now : 0;
}

/*-------------------------------------------------------------------------*/

void
Refuse( CORE::CString& detailCode, CORE::CString& message, const char* code, const CORE::CString& text )
{GUCEF_TRACE;

    detailCode = code;
    message = CORE::CString( code ) + ": " + text;
}

}

/*-------------------------------------------------------------------------*/

CUploadStaging::SSettings::SSettings( void )
    : directory()
    , maxBytesPerSubject( (UInt64) 50 * 1024 * 1024 * 1024 )
    , maxTotalBytes( 0 )
    , maxUploadsPerSubject( 1 )
    , maxFilesPerUpload( 100000 )
    , maxFileBytes( 0 )
    , expiryMs( (UInt64) 48 * 60 * 60 * 1000 )
    , blockSize( 262144 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUploadStaging::SFileSpec::SFileSpec( void )
    : relativePath()
    , size( 0 )
    , sha256Hex()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUploadStaging::SUploadInfo::SUploadInfo( void )
    : uploadId()
    , subject()
    , name()
    , state( UPLOADSTATE_RECEIVING )
    , files()
    , received()
    , totalBytes( 0 )
    , receivedBytes( 0 )
    , createdUtcMs( 0 )
    , lastWriteUtcMs( 0 )
    , expiresUtcMs( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUploadStaging::CUploadStaging( void )
    : m_lock()
    , m_settings()
    , m_uploads()
    , m_open( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUploadStaging::~CUploadStaging()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::IsSafeRelativePath( const CORE::CString& relativePath )
{GUCEF_TRACE;

    UInt32 bytes = relativePath.ByteSize() > 0 ? relativePath.ByteSize() - 1 : 0;
    if ( 0 == bytes || bytes > 1024 )
        return false;
    const char* c = relativePath.C_String();
    if ( '/' == c[ 0 ] )
        return false;
    for ( UInt32 i=0; i<bytes; ++i )
    {
        unsigned char ch = (unsigned char) c[ i ];
        if ( ch < 0x20 || 0x7F == ch || '\\' == ch || ':' == ch )
            return false;
    }
    CORE::CString::StringVector segments = relativePath.ParseElements( '/', true );
    if ( segments.empty() )
        return false;
    for ( UInt32 i=0; i<segments.size(); ++i )
    {
        const CORE::CString& segment = segments[ i ];
        UInt32 segmentBytes = segment.ByteSize() > 0 ? segment.ByteSize() - 1 : 0;
        if ( 0 == segmentBytes || segmentBytes > 255 )
            return false;
        if ( segment == "." || segment == ".." )
            return false;
        char last = segment.C_String()[ segmentBytes - 1 ];
        if ( '.' == last || ' ' == last )
            return false;
        if ( IsReservedDeviceName( segment ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUploadStaging::GetUploadDirectory( const CORE::CString& uploadId ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_settings.directory, uploadId );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUploadStaging::GetFilePath( const SUploadInfo& info, UInt32 fileIndex ) const
{GUCEF_TRACE;

    CORE::CString path = CORE::CombinePath( GetUploadDirectory( info.uploadId ), FilesDirectoryName );
    CORE::CString::StringVector segments = info.files[ fileIndex ].relativePath.ParseElements( '/', false );
    for ( UInt32 i=0; i<segments.size(); ++i )
        path = CORE::CombinePath( path, segments[ i ] );
    return path;
}

/*-------------------------------------------------------------------------*/

void
CUploadStaging::RemoveUploadFiles( const CORE::CString& uploadId ) const
{GUCEF_TRACE;

    if ( uploadId.IsNULLOrEmpty() || m_settings.directory.IsNULLOrEmpty() )
        return;
    CORE::CString directory = GetUploadDirectory( uploadId );
    if ( CORE::DirExists( directory ) && !CORE::DeleteDir( directory, true ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: the folder " << directory << " cannot be removed" );
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::SaveManifest( const SUploadInfo& info, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    CORE::CDataNode document( "upload", GUCEF_DATATYPE_OBJECT );
    document.SetAttribute( "format", CORE::CString( ManifestFormat ) );
    document.SetAttribute( "version", (UInt32) 1 );
    document.SetAttribute( "uploadId", info.uploadId );
    document.SetAttribute( "subject", info.subject );
    document.SetAttribute( "name", info.name );
    document.SetAttribute( "state", CORE::CString( StateToString( info.state ) ) );
    document.SetAttribute( "createdUtcMs", info.createdUtcMs );
    document.SetAttribute( "lastWriteUtcMs", info.lastWriteUtcMs );
    document.SetAttribute( "expiresUtcMs", info.expiresUtcMs );
    CORE::CDataNode* files = document.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    for ( UInt32 i=0; i<info.files.size(); ++i )
    {
        CORE::CDataNode* file = files->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        file->SetAttribute( "path", info.files[ i ].relativePath );
        file->SetAttribute( "size", info.files[ i ].size );
        file->SetAttribute( "sha256", info.files[ i ].sha256Hex );
        file->SetAttribute( "received", info.received[ i ].ToString() );
    }
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->StoreDataTree( &document, &access ) )
        {
            error = "the manifest of the upload cannot be encoded";
            return false;
        }
    }
    CORE::CString path = CORE::CombinePath( GetUploadDirectory( info.uploadId ), ManifestName );
    if ( !CORE::WriteFileAtomically( path, buffer ) )
    {
        error = "the manifest " + path + " cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::LoadManifest( const CORE::CString& path, SUploadInfo& info, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    CORE::CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) )
    {
        error = "the manifest " + path + " cannot be read";
        return false;
    }
    CORE::CDataNode document;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->BuildDataTree( &document, &access ) )
        {
            error = "the manifest " + path + " is not valid JSON";
            return false;
        }
    }
    const CORE::CDataNode* root = &document;
    if ( document.GetAttributeValueOrChildValueByName( "format" ).AsString().IsNULLOrEmpty() && 1 == document.GetNrOfChildNodes() )
        root = *document.ConstBegin();
    if ( ManifestFormat != root->GetAttributeValueOrChildValueByName( "format" ).AsString() )
    {
        error = "the manifest " + path + " is not an upload manifest";
        return false;
    }
    SUploadInfo read;
    read.uploadId = root->GetAttributeValueOrChildValueByName( "uploadId" ).AsString();
    read.subject = root->GetAttributeValueOrChildValueByName( "subject" ).AsString();
    read.name = root->GetAttributeValueOrChildValueByName( "name" ).AsString();
    read.state = StateFromString( root->GetAttributeValueOrChildValueByName( "state" ).AsString() );
    read.createdUtcMs = root->GetAttributeValueOrChildValueByName( "createdUtcMs" ).AsUInt64( 0 );
    read.lastWriteUtcMs = root->GetAttributeValueOrChildValueByName( "lastWriteUtcMs" ).AsUInt64( 0 );
    read.expiresUtcMs = root->GetAttributeValueOrChildValueByName( "expiresUtcMs" ).AsUInt64( 0 );
    const CORE::CDataNode* files = root->FindChild( "files" );
    if ( GUCEF_NULL != files )
    {
        CORE::CDataNode::const_iterator i = files->ConstBegin();
        for ( ; i != files->ConstEnd(); ++i )
        {
            SFileSpec spec;
            spec.relativePath = (*i)->GetAttributeValueOrChildValueByName( "path" ).AsString();
            spec.size = (*i)->GetAttributeValueOrChildValueByName( "size" ).AsUInt64( 0 );
            spec.sha256Hex = (*i)->GetAttributeValueOrChildValueByName( "sha256" ).AsString();
            CUploadRangeSet ranges;
            if ( !IsSafeRelativePath( spec.relativePath ) || !IsHexDigest( spec.sha256Hex ) ||
                 !ranges.FromString( (*i)->GetAttributeValueOrChildValueByName( "received" ).AsString() ) )
            {
                error = "the manifest " + path + " holds a malformed file entry";
                return false;
            }
            read.files.push_back( spec );
            read.received.push_back( ranges );
            read.totalBytes += spec.size;
            read.receivedBytes += ranges.GetTotalBytes();
        }
    }
    if ( read.uploadId.IsNULLOrEmpty() || read.subject.IsNULLOrEmpty() )
    {
        error = "the manifest " + path + " has no upload id or subject";
        return false;
    }
    info = read;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Open( const SSettings& settings, CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_uploads.clear();
    m_open = false;
    m_settings = settings;
    if ( 0 == m_settings.blockSize )
        m_settings.blockSize = 262144;
    if ( m_settings.directory.IsNULLOrEmpty() )
    {
        error = "the upload staging has no directory";
        return false;
    }
    if ( !CORE::DirExists( m_settings.directory ) )
    {
        if ( !CORE::CreateDirs( m_settings.directory ) )
        {
            error = "the staging directory " + m_settings.directory + " cannot be made";
            return false;
        }
        CORE::SetOwnerOnlyAccess( m_settings.directory );
    }

    // uploads of an earlier run: kept when their manifest is whole and they did not expire
    const UInt64 now = SystemUtcMs();
    CORE::CNativeFileSystemAccess fileSystem;
    CORE::TFileSystemIteratorPtr iterator;
    CORE::CString::StringVector directories;
    if ( CORE::FILESYSTEMACCESS_SUCCESS == fileSystem.TryGetFileSystemIterator( m_settings.directory, iterator ) && !iterator.IsNULL() &&
         iterator->FindFirst( m_settings.directory ) )
    {
        do
        {
            CORE::CString name = iterator->GetResourceName();
            if ( iterator->IsADirectory() && !iterator->IsSymlink() && name != "." && name != ".." )
                directories.push_back( name );
        }
        while ( iterator->FindNext() );
        iterator->FindClose();
    }
    for ( UInt32 i=0; i<directories.size(); ++i )
    {
        CORE::CString manifestPath = CORE::CombinePath( CORE::CombinePath( m_settings.directory, directories[ i ] ), ManifestName );
        SUploadInfo info;
        CORE::CString loadError;
        if ( !CORE::FileExists( manifestPath ) || !LoadManifest( manifestPath, info, loadError ) || info.uploadId != directories[ i ] )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: removing the unreadable upload folder " << directories[ i ] << ( loadError.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( ": " ) + loadError ) );
            RemoveUploadFiles( directories[ i ] );
            continue;
        }
        if ( UPLOADSTATE_ABORTED == info.state || ( UPLOADSTATE_RECEIVING == info.state && 0 != m_settings.expiryMs && info.expiresUtcMs <= now ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: removing the expired upload " << info.uploadId );
            RemoveUploadFiles( info.uploadId );
            continue;
        }
        m_uploads[ info.uploadId ] = info;
    }
    m_open = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: " << (UInt32) m_uploads.size() << " uploads kept in " << m_settings.directory );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUploadStaging::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_uploads.clear();
    m_open = false;
}

/*-------------------------------------------------------------------------*/

const CUploadStaging::SSettings&
CUploadStaging::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Begin( const CORE::CString& subject, const CORE::CString& name, const TFileSpecVector& files, UInt64 nowUtcMs,
                       CORE::CString& uploadId, CORE::CString& detailCode, CORE::CString& message )
{GUCEF_TRACE;

    if ( subject.IsNULLOrEmpty() )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadUnknown, "an upload needs a subject" );
        return false;
    }
    if ( files.empty() )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::OutOfRange, "an upload holds at least one file" );
        return false;
    }

    UInt64 total = 0;
    CORE::CString::StringSet seen;
    for ( UInt32 i=0; i<files.size(); ++i )
    {
        const SFileSpec& spec = files[ i ];
        if ( !IsSafeRelativePath( spec.relativePath ) )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UnsafePath, "the file " + CORE::ToString( i ) + " has an unsafe relative path" );
            return false;
        }
        if ( !seen.insert( spec.relativePath.Lowercase() ).second )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UnsafePath, "the path of the file " + CORE::ToString( i ) + " is given twice" );
            return false;
        }
        if ( !IsHexDigest( spec.sha256Hex ) )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::OutOfRange, "the file " + CORE::ToString( i ) + " has no SHA-256 of 64 lower case hexadecimal characters" );
            return false;
        }
        if ( 0 != m_settings.maxFileBytes && spec.size > m_settings.maxFileBytes )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::QuotaExceeded, "the file " + CORE::ToString( i ) + " is larger than " + CORE::ToString( m_settings.maxFileBytes ) + " bytes" );
            return false;
        }
        total += spec.size;
    }

    CORE::CString id;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_open )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::StagingFull, "the upload staging is not open" );
            return false;
        }
        if ( 0 != m_settings.maxFilesPerUpload && files.size() > m_settings.maxFilesPerUpload )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::QuotaExceeded, "an upload holds at most " + CORE::ToString( m_settings.maxFilesPerUpload ) + " files" );
            return false;
        }
        UInt32 open = 0;
        UInt64 usedBySubject = 0;
        UInt64 usedInTotal = 0;
        TUploadMap::const_iterator u = m_uploads.begin();
        for ( ; u != m_uploads.end(); ++u )
        {
            usedInTotal += (*u).second.totalBytes;
            if ( (*u).second.subject == subject )
            {
                usedBySubject += (*u).second.totalBytes;
                if ( UPLOADSTATE_RECEIVING == (*u).second.state )
                    ++open;
            }
        }
        if ( 0 != m_settings.maxUploadsPerSubject && open >= m_settings.maxUploadsPerSubject )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadLimit, "at most " + CORE::ToString( m_settings.maxUploadsPerSubject ) + " uploads at a time" );
            return false;
        }
        if ( 0 != m_settings.maxBytesPerSubject && usedBySubject + total > m_settings.maxBytesPerSubject )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::QuotaExceeded, "the staging quota of " + CORE::ToString( m_settings.maxBytesPerSubject ) + " bytes would be exceeded" );
            return false;
        }
        if ( 0 != m_settings.maxTotalBytes && usedInTotal + total > m_settings.maxTotalBytes )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::StagingFull, "the staging is full" );
            return false;
        }
        CORE::CString randomPart;
        if ( !CDevicePairingCrypto::GenerateSessionId( randomPart ) )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::StagingFull, "no randomness for an upload id" );
            return false;
        }
        id = "u-" + randomPart;

        SUploadInfo info;
        info.uploadId = id;
        info.subject = subject;
        info.name = name;
        info.state = UPLOADSTATE_RECEIVING;
        info.files = files;
        info.received.resize( files.size() );
        info.totalBytes = total;
        info.createdUtcMs = nowUtcMs;
        info.lastWriteUtcMs = nowUtcMs;
        info.expiresUtcMs = nowUtcMs + m_settings.expiryMs;
        // reserved now, so that two Begin() calls cannot both take the last of a quota
        m_uploads[ id ] = info;
    }

    // the folder and an empty file per entry, so that blocks can be written at their offsets
    SUploadInfo info;
    {
        MT::CScopeMutex lock( m_lock );
        info = m_uploads[ id ];
    }
    CORE::CString error;
    bool made = CORE::CreateDirs( CORE::CombinePath( GetUploadDirectory( id ), FilesDirectoryName ) );
    for ( UInt32 i=0; made && i<info.files.size(); ++i )
    {
        CORE::CString path = GetFilePath( info, i );
        made = CORE::CreatePathDirectories( path );
        if ( made )
        {
            CORE::CFileAccess file;
            made = file.Open( path, "wb" );
            file.Close();
        }
    }
    if ( made )
        made = SaveManifest( info, error );
    if ( !made )
    {
        RemoveUploadFiles( id );
        MT::CScopeMutex lock( m_lock );
        m_uploads.erase( id );
        Refuse( detailCode, message, CRemoteServiceDetailCodes::StagingFull, "the upload folder cannot be made" + ( error.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( ": " ) + error ) );
        return false;
    }
    uploadId = id;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: upload " << id << " of " << subject << ": " << (UInt32) info.files.size() << " files, " << total << " bytes" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::WriteBlock( const CORE::CString& subject, const CORE::CString& uploadId, UInt32 fileIndex, UInt64 offset,
                            const void* data, UInt32 size, UInt64 nowUtcMs, CORE::CString& detailCode, CORE::CString& message )
{GUCEF_TRACE;

    SUploadInfo info;
    {
        MT::CScopeMutex lock( m_lock );
        TUploadMap::const_iterator i = m_uploads.find( uploadId );
        if ( i == m_uploads.end() || (*i).second.subject != subject )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadUnknown, "there is no upload " + uploadId );
            return false;
        }
        info = (*i).second;
    }
    if ( UPLOADSTATE_RECEIVING != info.state )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::OutOfRange, "the upload " + uploadId + " takes no more blocks" );
        return false;
    }
    if ( fileIndex >= info.files.size() )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::OutOfRange, "the upload has no file " + CORE::ToString( fileIndex ) );
        return false;
    }
    const UInt64 fileSize = info.files[ fileIndex ].size;
    if ( 0 == size || GUCEF_NULL == data || offset > fileSize || size > fileSize - offset || size > m_settings.blockSize )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::OutOfRange, "the block does not fit the file " + CORE::ToString( fileIndex ) );
        return false;
    }

    CORE::CString path = GetFilePath( info, fileIndex );
    bool written = false;
    {
        CORE::CFileAccess file;
        if ( file.Open( path, "r+b" ) && 0 == file.Setpos( offset ) )
            written = 1 == file.Write( data, size, 1 );
        file.Close();
    }
    if ( !written )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::StagingFull, "the block cannot be written to the staging" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );
    TUploadMap::iterator i = m_uploads.find( uploadId );
    if ( i == m_uploads.end() )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadUnknown, "the upload " + uploadId + " ended while the block was written" );
        return false;
    }
    SUploadInfo& current = (*i).second;
    current.received[ fileIndex ].Add( offset, size );
    current.receivedBytes = 0;
    for ( UInt32 f=0; f<current.received.size(); ++f )
        current.receivedBytes += current.received[ f ].GetTotalBytes();
    current.lastWriteUtcMs = nowUtcMs;
    current.expiresUtcMs = nowUtcMs + m_settings.expiryMs;
    CORE::CString error;
    if ( !SaveManifest( current, error ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: " << error );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Query( const CORE::CString& subject, const CORE::CString& uploadId, SUploadInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUploadMap::const_iterator i = m_uploads.find( uploadId );
    if ( i == m_uploads.end() || (*i).second.subject != subject )
        return false;
    info = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Commit( const CORE::CString& subject, const CORE::CString& uploadId, CORE::CString& stagedDirectory,
                        CORE::CString& detailCode, CORE::CString& message )
{GUCEF_TRACE;

    SUploadInfo info;
    {
        MT::CScopeMutex lock( m_lock );
        TUploadMap::const_iterator i = m_uploads.find( uploadId );
        if ( i == m_uploads.end() || (*i).second.subject != subject )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadUnknown, "there is no upload " + uploadId );
            return false;
        }
        info = (*i).second;
    }
    if ( UPLOADSTATE_COMMITTED == info.state )
    {
        stagedDirectory = CORE::CombinePath( GetUploadDirectory( uploadId ), FilesDirectoryName );
        return true;
    }
    for ( UInt32 f=0; f<info.files.size(); ++f )
    {
        if ( !info.received[ f ].IsComplete( info.files[ f ].size ) )
        {
            Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadIncomplete, "the file " + CORE::ToString( f ) + " is not complete" );
            return false;
        }
    }

    // the digests, read in chunks outside the lock
    GUCEF::vector< UInt32 > mismatched;
    CORE::CDynamicBuffer chunk( 65536, false );
    for ( UInt32 f=0; f<info.files.size(); ++f )
    {
        CORE::CSha256 sha;
        CORE::CFileAccess file;
        bool ok = file.Open( GetFilePath( info, f ), "rb" );
        UInt64 remaining = info.files[ f ].size;
        while ( ok && remaining > 0 )
        {
            UInt32 want = remaining > 65536 ? 65536 : (UInt32) remaining;
            UInt32 got = file.Read( chunk.GetBufferPtr(), 1, want );
            if ( got != want )
            {
                ok = false;
                break;
            }
            sha.Update( chunk.GetConstBufferPtr(), got );
            remaining -= got;
        }
        file.Close();
        UInt8 digest[ CORE::CSha256::DigestSize ];
        sha.Final( digest );
        if ( !ok || DRN::CDRNCrypto::ToHex( digest, sizeof( digest ) ).Lowercase() != info.files[ f ].sha256Hex )
            mismatched.push_back( f );
    }

    MT::CScopeMutex lock( m_lock );
    TUploadMap::iterator i = m_uploads.find( uploadId );
    if ( i == m_uploads.end() )
    {
        Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadUnknown, "the upload " + uploadId + " ended during the commit" );
        return false;
    }
    SUploadInfo& current = (*i).second;
    CORE::CString error;
    if ( !mismatched.empty() )
    {
        CORE::CString list;
        for ( UInt32 m=0; m<mismatched.size(); ++m )
        {
            current.received[ mismatched[ m ] ].Clear();
            if ( !list.IsNULLOrEmpty() )
                list += ',';
            list += CORE::ToString( mismatched[ m ] );
        }
        current.receivedBytes = 0;
        for ( UInt32 f=0; f<current.received.size(); ++f )
            current.receivedBytes += current.received[ f ].GetTotalBytes();
        SaveManifest( current, error );
        Refuse( detailCode, message, CRemoteServiceDetailCodes::UploadDigestMismatch, "the SHA-256 of the files " + list + " does not match; send them again" );
        return false;
    }
    current.state = UPLOADSTATE_COMMITTED;
    if ( !SaveManifest( current, error ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: " << error );
    stagedDirectory = CORE::CombinePath( GetUploadDirectory( uploadId ), FilesDirectoryName );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: upload " << uploadId << " committed" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Abort( const CORE::CString& subject, const CORE::CString& uploadId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TUploadMap::iterator i = m_uploads.find( uploadId );
        if ( i == m_uploads.end() || (*i).second.subject != subject )
            return false;
        m_uploads.erase( i );
    }
    RemoveUploadFiles( uploadId );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: upload " << uploadId << " aborted" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUploadStaging::Release( const CORE::CString& uploadId )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TUploadMap::iterator i = m_uploads.find( uploadId );
        if ( i == m_uploads.end() )
            return false;
        m_uploads.erase( i );
    }
    RemoveUploadFiles( uploadId );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStaging::ExpireOld( UInt64 nowUtcMs )
{GUCEF_TRACE;

    CORE::CString::StringVector expired;
    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 == m_settings.expiryMs )
            return 0;
        TUploadMap::iterator i = m_uploads.begin();
        while ( i != m_uploads.end() )
        {
            if ( UPLOADSTATE_COMMITTED != (*i).second.state && (*i).second.expiresUtcMs <= nowUtcMs )
            {
                expired.push_back( (*i).first );
                m_uploads.erase( i++ );
            }
            else
            {
                ++i;
            }
        }
    }
    for ( UInt32 e=0; e<expired.size(); ++e )
    {
        RemoveUploadFiles( expired[ e ] );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "UploadStaging: upload " << expired[ e ] << " expired" );
    }
    return (UInt32) expired.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CUploadStaging::GetUsedBytes( const CORE::CString& subject ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 used = 0;
    TUploadMap::const_iterator i = m_uploads.begin();
    for ( ; i != m_uploads.end(); ++i )
    {
        if ( (*i).second.subject == subject )
            used += (*i).second.totalBytes;
    }
    return used;
}

/*-------------------------------------------------------------------------*/

UInt64
CUploadStaging::GetTotalUsedBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 used = 0;
    TUploadMap::const_iterator i = m_uploads.begin();
    for ( ; i != m_uploads.end(); ++i )
        used += (*i).second.totalBytes;
    return used;
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStaging::GetOpenCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_uploads.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CUploadStaging::GetOpenCount( const CORE::CString& subject ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    TUploadMap::const_iterator i = m_uploads.begin();
    for ( ; i != m_uploads.end(); ++i )
    {
        if ( (*i).second.subject == subject )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

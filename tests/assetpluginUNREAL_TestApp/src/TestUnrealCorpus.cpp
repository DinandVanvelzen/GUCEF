/*
 *  assetpluginUNREAL_TestApp: Tests of the Unreal reader on the local corpus of licensed packages (GUCEF_TEST_ASSET_CORPUS=1)
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

#include <stdio.h>
#include <string.h>
#include <algorithm>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#include "assetpluginUNREAL_CUnrealCompressedBufferHeader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFERHEADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#include "assetpluginUNREAL_CUnrealCustomVersionIds.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#include "TestUnrealCorpus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------*/

static CORE::CString
HexOf( UInt32 value )
{
    char text[ 16 ];
    sprintf( text, "0x%X", (unsigned int) value );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

/** What the golden files record of a package: versions, counts, trailer and the first export classes. No bytes of the asset. */
static CORE::CString
DescribePackage( CUnrealPackage& package, const CORE::CString& entry )
{
    const CUnrealPackageSummary& summary = package.GetSummary();
    CORE::CString text = entry + " | legacy " + CORE::Int32ToString( summary.versions.GetLegacyFileVersion() ) + " ue4 " + CORE::Int32ToString( summary.versions.GetUe4Version() ) + " ue5 " + CORE::Int32ToString( summary.versions.GetUe5Version() )
        + " | flags " + HexOf( summary.packageFlags ) + " | saved by " + summary.savedByEngineVersion.ToString()
        + " | names " + CORE::UInt32ToString( package.GetNames().GetCount() ) + " imports " + CORE::UInt64ToString( package.GetImports().size() ) + " exports " + CORE::UInt64ToString( package.GetExports().size() )
        + " header " + CORE::Int32ToString( summary.totalHeaderSize ) + " custom " + CORE::UInt64ToString( summary.versions.GetCustomVersions().size() ) + " soft " + CORE::UInt64ToString( package.GetSoftObjectPaths().size() )
        + " trailer " + ( package.HasTrailer() ? CORE::UInt64ToString( package.GetTrailer().GetEntries().size() ) : CORE::CString( "none" ) ) + " problems " + CORE::UInt64ToString( package.GetProblems().size() ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

/** The properties of the main export (the first export without outer that is not a package): counts and the first names */
static CORE::CString
DescribeProperties( CUnrealPackage& package, UInt32 maxProperties, UInt32& mismatches )
{
    mismatches = 0;
    CORE::CString text;
    for ( size_t i=0; i<package.GetExports().size(); ++i )
    {
        if ( !package.GetExports()[ i ].outerIndex.IsNull() )
            continue;
        const CORE::CString className = package.GetClassName( package.GetExports()[ i ] );
        CUnrealArchiveReader reader;
        if ( !package.OpenPropertyRange( (UInt32) i, reader ) )
        {
            text += "  export " + package.GetExports()[ i ].objectName + " (" + className + "): properties not readable\n";
            continue;
        }
        CORE::CDataNode root( "root" );
        CUnrealPropertyReader properties( package );
        const bool ok = properties.ReadExport( reader, root, className );
        for ( size_t m=0; m<properties.GetMismatches().size(); ++m )
            if ( properties.GetMismatches()[ m ].HasSubstr( "has a binary form that is not known" ) < 0 )
                ++mismatches;
        text += "  export " + package.GetExports()[ i ].objectName + " (" + className + "): " + ( ok ? "ok" : "failed: " + reader.GetError() ) + ", properties " + CORE::UInt32ToString( properties.GetPropertyCount() )
            + ", mismatches " + CORE::UInt64ToString( properties.GetMismatches().size() ) + ", unknown " + CORE::UInt64ToString( properties.GetUnknownProperties().size() ) + ", end " + CORE::UInt64ToString( reader.GetRemaining() ) + " bytes left\n";
        for ( size_t m=0; m<properties.GetMismatches().size() && m<3; ++m )
            text += "    mismatch: " + properties.GetMismatches()[ m ] + "\n";
        for ( size_t m=0; m<properties.GetUnknownProperties().size() && m<3; ++m )
            text += "    skipped: " + properties.GetUnknownProperties()[ m ] + "\n";
        UInt32 shown = 0;
        CORE::CDataNode::const_iterator c = root.ConstBegin();
        while ( c != root.ConstEnd() && shown < maxProperties )
        {
            const CORE::CDataNode* node = *c;
            CORE::CString value = node->GetValue().AsString();
            if ( value.Length() > 60 )
                value = value.SubstrFromRange( 0, 60 );
            text += "    " + node->GetName() + " : " + node->GetAttributeValue( "type" ).AsString() + " = " + value + "\n";
            ++shown;
            ++c;
        }
        break; // the main export only
    }
    return text;
}

/*-------------------------------------------------------------------------*/

static bool
MountZip( const CORE::CString& archivePath, const CORE::CString& name )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    return vfs.MountArchive( name, archivePath, "zip", name, false, false, archivePath );
}

/*-------------------------------------------------------------------------*/

/** Unmounts an archive when the test case is done: the zip plugin must not be unloaded while it is still mounted */
class CMountGuard
{
    public:

    explicit CMountGuard( const CORE::CString& archiveName ) : m_name( archiveName ) {}

    ~CMountGuard() { VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_name ); }

    private:

    CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

/** One stored entry of an archive: where its bytes are in the archive file */
struct SStoredEntry
{
    CORE::CString name;
    UInt64 offset;
    UInt64 size;
};

/**
 *  The directory of a zip archive with 64 bit sizes (the .mhpkg files), read by range through the VFS. Only stored entries
 *  are listed. The zip plugin of the VFS cannot open these archives: its zip library has no 64 bit directory support.
 */
static bool
ReadZip64Directory( const CORE::CString& vfsPath, GUCEF::vector< SStoredEntry >& entries )
{
    CUnrealPackageSource file;
    if ( !file.OpenVfs( vfsPath ) )
        return false;
    const UInt64 size = file.GetSize();
    CUnrealArchiveReader tail;
    const UInt64 tailSize = size < 70000 ? size : 70000;
    if ( !file.OpenRange( size - tailSize, tailSize, tail, 70000 ) )
        return false;
    GUCEF::vector< UInt8 > bytes( (size_t) tailSize );
    tail.Read( &bytes[ 0 ], tailSize );
    Int64 eocd = -1;
    for ( Int64 i=(Int64) tailSize-22; i>=0; --i )
        if ( 'P' == bytes[ (size_t) i ] && 'K' == bytes[ (size_t) i + 1 ] && 5 == bytes[ (size_t) i + 2 ] && 6 == bytes[ (size_t) i + 3 ] ) { eocd = i; break; }
    if ( eocd < 20 )
        return false;
    UInt64 directoryOffset = 0;
    UInt64 count = 0;
    {
        // the zip64 locator sits in front of the end of directory record; it names the zip64 record
        const UInt8* locator = &bytes[ (size_t) eocd - 20 ];
        if ( 'P' != locator[ 0 ] || 'K' != locator[ 1 ] || 6 != locator[ 2 ] || 7 != locator[ 3 ] )
            return false;
        UInt64 recordOffset = 0;
        memcpy( &recordOffset, locator + 8, 8 );
        CUnrealArchiveReader record;
        if ( !file.OpenRange( recordOffset, 56, record, 4096 ) || 0x06064b50 != record.ReadUInt32() )
            return false;
        record.Skip( 28 );
        count = record.ReadUInt64();
        record.ReadUInt64();
        directoryOffset = record.ReadUInt64();
    }
    CUnrealArchiveReader directory;
    if ( !file.OpenRange( directoryOffset, size - directoryOffset, directory, 1024 * 1024 ) )
        return false;
    for ( UInt64 e=0; e<count; ++e )
    {
        if ( 0x02014b50 != directory.ReadUInt32() )
            return false;
        directory.Skip( 6 );
        const UInt16 method = directory.ReadUInt16();
        directory.Skip( 8 );
        UInt64 compressed = directory.ReadUInt32();
        UInt64 uncompressed = directory.ReadUInt32();
        const UInt16 nameLength = directory.ReadUInt16();
        const UInt16 extraLength = directory.ReadUInt16();
        const UInt16 commentLength = directory.ReadUInt16();
        directory.Skip( 8 );
        UInt64 headerOffset = directory.ReadUInt32();
        GUCEF::vector< char > name( nameLength + 1, 0 );
        directory.Read( &name[ 0 ], nameLength );
        GUCEF::vector< UInt8 > extra( extraLength + 1, 0 );
        directory.Read( &extra[ 0 ], extraLength );
        directory.Skip( commentLength );
        if ( directory.HasError() )
            return false;
        for ( UInt32 at=0; at+4<=extraLength; )
        {
            const UInt16 id = (UInt16) ( extra[ at ] | ( extra[ at + 1 ] << 8 ) );
            const UInt16 length = (UInt16) ( extra[ at + 2 ] | ( extra[ at + 3 ] << 8 ) );
            if ( 1 == id )
            {
                UInt32 used = at + 4;
                if ( 0xFFFFFFFF == uncompressed ) { memcpy( &uncompressed, &extra[ used ], 8 ); used += 8; }
                if ( 0xFFFFFFFF == compressed ) { memcpy( &compressed, &extra[ used ], 8 ); used += 8; }
                if ( 0xFFFFFFFF == headerOffset ) { memcpy( &headerOffset, &extra[ used ], 8 ); }
            }
            at += 4 + length;
        }
        if ( 0 != method )
            continue;
        // the data starts behind the local header, whose name and extra field have their own lengths
        CUnrealArchiveReader local;
        if ( !file.OpenRange( headerOffset, 30, local, 4096 ) || 0x04034b50 != local.ReadUInt32() )
            return false;
        local.Skip( 22 );
        const UInt16 localName = local.ReadUInt16();
        const UInt16 localExtra = local.ReadUInt16();
        SStoredEntry entry;
        entry.name = CORE::CString( &name[ 0 ] );
        entry.offset = headerOffset + 30 + localName + localExtra;
        entry.size = uncompressed;
        entries.push_back( entry );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
CollectPackages( const CORE::CString& mount, CORE::CString::StringVector& packages )
{
    VFS::CVFS::TStringVector files;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( files, mount, true, true, CORE::CString::Empty, 1000000 );
    for ( size_t i=0; i<files.size(); ++i )
    {
        const CORE::CString lower = files[ i ].Lowercase();
        if ( lower.HasSubstr( ".uasset" ) == (Int32) lower.Length() - 7 || lower.HasSubstr( ".umap" ) == (Int32) lower.Length() - 5 )
            packages.push_back( files[ i ] );
    }
    std::sort( packages.begin(), packages.end() );
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealCorpusTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Corpus" );

    GUCEF_TESTFW_TESTCASE( "Corpus: archives of stored entries (.mhpkg): versions per engine release, tables, trailers, payload headers, golden summaries" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString archiveDir = CorpusSetting( "archivedir" );
    if ( archiveDir.IsNULLOrEmpty() || !CORE::DirExists( archiveDir ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no archive folder" );
    try
    {
        CMountGuard archivesGuard( "unreal_corpus_archives" );
        ASSERT_TRUE( VFS::CVfsGlobal::Instance()->GetVfs().MountArchive( "unreal_corpus_archives", archiveDir, VFS::CVFS::FileSystemArchiveTypeName, "storedcorpus", false, false, archiveDir ) );
        VFS::CVFS::TStringVector archiveFiles;
        VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( archiveFiles, "storedcorpus", false, true, CORE::CString::Empty, 100000 );
        std::sort( archiveFiles.begin(), archiveFiles.end() );
        UInt64 totalFetched = 0;
        UInt32 packagesRead = 0;
        UInt32 archivesRead = 0;
        UInt32 releases[ 2 ] = { 0, 0 };
        for ( size_t a=0; a<archiveFiles.size(); ++a )
        {
            if ( archiveFiles[ a ].Lowercase().HasSubstr( ".mhpkg" ) != (Int32) archiveFiles[ a ].Length() - 6 )
                continue;
            const CORE::CString vfsFile = archiveFiles[ a ];
            const CORE::CString archiveName = CORE::ExtractFilename( vfsFile ).SubstrFromRange( 0, CORE::ExtractFilename( vfsFile ).Length() - 6 );
            // the archive is mounted through the VFS like any other (the libarchive plugin reads the 64 bit directory and reads
            // stored entries by range); the own directory reader is the fallback for builds without that plugin
            const CORE::CString mountName = "unreal_mhpkg_" + archiveName;
            CMountGuard mhpkgGuard( mountName );
            const CORE::CString nativePath = CORE::CombinePath( archiveDir, CORE::ExtractFilename( vfsFile ) );
            const bool mounted = VFS::CVfsGlobal::Instance()->GetVfs().MountArchive( mountName, nativePath, "mhpkg", mountName, false, false, nativePath );
            GUCEF::vector< SStoredEntry > packages;
            if ( mounted )
            {
                CORE::CString::StringVector files;
                CollectPackages( mountName, files );
                for ( size_t i=0; i<files.size(); ++i )
                {
                    SStoredEntry entry;
                    // the listing names the files below the mount, with or without a leading separator
                    const Int32 at = files[ i ].HasSubstr( mountName );
                    CORE::CString relative = files[ i ].SubstrFromRange( ( at < 0 ? 0 : (UInt32) at ) + mountName.Length(), files[ i ].Length() );
                    while ( relative.Length() > 0 && '/' == relative[ 0 ] )
                        relative = relative.CutChars( 1, true );
                    entry.name = relative;
                    entry.offset = 0;
                    entry.size = 0;
                    packages.push_back( entry );
                }
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Corpus: the archive " << archiveName << " is not mounted through the VFS (no plugin for .mhpkg): reading its directory directly" );
                GUCEF::vector< SStoredEntry > stored;
                ASSERT_TRUE( ReadZip64Directory( vfsFile, stored ) );
                for ( size_t i=0; i<stored.size(); ++i )
                    if ( stored[ i ].name.Lowercase().HasSubstr( ".uasset" ) == (Int32) stored[ i ].name.Length() - 7 )
                        packages.push_back( stored[ i ] );
            }
            ASSERT_TRUE( packages.size() >= 3 );
            ++archivesRead;

            const UInt64 start = MT::PrecisionTickCount();
            CORE::CString summaryText;
            CORE::CString propertyText;
            UInt32 failures = 0;
            UInt32 mismatchTotal = 0;
            for ( size_t p=0; p<packages.size(); ++p )
            {
                CUnrealPackage package;
                const CORE::CString entry = packages[ p ].name;
                if ( !( mounted ? package.Open( mountName + "/" + entry ) : package.OpenEmbedded( vfsFile, packages[ p ].offset, packages[ p ].size ) ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Corpus package " << entry << " failed: " << package.GetError() );
                    ++failures;
                    continue;
                }
                ++packagesRead;
                totalFetched += package.GetBytesFetched();
                summaryText += DescribePackage( package, entry );
                const CUnrealPackageSummary& summary = package.GetSummary();
                // the file version follows the engine release that saved the package: 5.6 has 1017, 5.7 and 5.8 have 1018
                const Int32 expected = 6 == summary.savedByEngineVersion.minor ? 1017 : 1018;
                ++releases[ 6 == summary.savedByEngineVersion.minor ? 0 : 1 ];
                ASSERT_TRUE( 5 == summary.savedByEngineVersion.major && ( 6 == summary.savedByEngineVersion.minor || summary.savedByEngineVersion.minor >= 7 ) );
                ASSERT_TRUE( expected == summary.versions.GetUe5Version() && 522 == summary.versions.GetUe4Version() && -9 == summary.versions.GetLegacyFileVersion() );
                ASSERT_TRUE( summary.IsEditorPackage() && package.GetProblems().empty() );
                ASSERT_TRUE( package.GetNames().GetCount() == (UInt32) summary.nameCount && package.GetExports().size() == (size_t) summary.exportCount );
                ASSERT_TRUE( 40 == package.GetVersions().GetCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion() ) || !package.GetVersions().HasCustomVersion( CUnrealCustomVersionIds::EditorObjectVersion() ) );
                // every payload of the trailer starts with a compressed buffer header that fits the table
                if ( package.HasTrailer() )
                {
                    const CUnrealPackageTrailer::TEntryVector& entries = package.GetTrailer().GetEntries();
                    for ( size_t e=0; e<entries.size(); ++e )
                    {
                        CUnrealArchiveReader payload;
                        CUnrealCompressedBufferHeader header;
                        const UInt64 offset = package.GetTrailer().GetPayloadOffset( entries[ e ] );
                        const UInt64 peek = entries[ e ].compressedSize < 65536 ? entries[ e ].compressedSize : 65536;
                        const bool headerOk = package.GetSource().OpenRange( offset, peek, payload, 65536 ) && header.Read( payload, entries[ e ].compressedSize );
                        if ( !headerOk || header.rawSize != entries[ e ].rawSize )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Payload " << e << " of " << entry << ": " << payload.GetError() );
                            ++failures;
                        }
                        else
                            summaryText += "  payload " + CORE::UInt64ToString( e ) + " method " + CORE::UInt32ToString( header.method ) + " raw " + CORE::UInt64ToString( header.rawSize ) + " stored " + CORE::UInt64ToString( entries[ e ].compressedSize ) + " blocks " + CORE::UInt32ToString( header.blockCount ) + "\n";
                        package.AccountFetched( payload );
                    }
                }
                // the properties of the main export of the first assets and of a sample of the others
                if ( p < 6 || 0 == p % 7 )
                {
                    UInt32 mismatches = 0;
                    propertyText += entry + "\n" + DescribeProperties( package, 14, mismatches );
                    mismatchTotal += mismatches;
                }
            }
            const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus " << archiveName << ": " << packages.size() << " packages read in " << ms << " ms, " << mismatchTotal << " property mismatches, " << failures << " failures" );
            ASSERT_TRUE( 0 == failures );
            ASSERT_TRUE( 0 == mismatchTotal );
            CORE::CString problem;
            const bool summaryGolden = CheckGolden( CORE::CString( "archive_" ) + archiveName + "_summary", summaryText, problem );
            if ( !summaryGolden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( summaryGolden );
            const bool propertyGolden = CheckGolden( CORE::CString( "archive_" ) + archiveName + "_properties", propertyText, problem );
            if ( !propertyGolden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            ASSERT_TRUE( propertyGolden );
        }
        // the headers only: the gigabytes of the archives are not read
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus archives: " << archivesRead << " archives, " << packagesRead << " packages (" << releases[ 0 ] << " of 5.6, " << releases[ 1 ] << " of 5.7 or 5.8), " << totalFetched << " bytes fetched" );
        ASSERT_TRUE( archivesRead >= 1 && packagesRead >= 3 );
        ASSERT_TRUE( totalFetched < 64 * 1024 * 1024 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corpus: a project archive of UE 5.3 (deflated zip through the zip plugin): versions, classic tag layout, a sample of 100 packages read" )
    if ( !CorpusEnabled() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_ASSET_CORPUS is not set to 1" );
    const CORE::CString projectZip = CorpusSetting( "projectzip" );
    if ( projectZip.IsNULLOrEmpty() || !CORE::FileExists( projectZip ) )
        GUCEF_TESTFW_SKIP( "The corpus manifest names no project archive" );
    try
    {
        CMountGuard projectGuard( "projectcorpus" );
        ASSERT_TRUE( MountZip( projectZip, "projectcorpus" ) );
        CORE::CString::StringVector packages;
        CollectPackages( "projectcorpus", packages );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus project: " << packages.size() << " packages listed" );
        ASSERT_TRUE( packages.size() >= 2500 );
        CORE::CString summaryText;
        CORE::CString propertyText;
        UInt32 read = 0;
        UInt32 classic53 = 0;
        UInt32 mismatchTotal = 0;
        UInt32 failures = 0;
        const UInt64 start = MT::PrecisionTickCount();
        const size_t stride = packages.size() / 100;
        for ( size_t p=0; p<packages.size(); p+=stride )
        {
            CUnrealPackage package;
            if ( !package.Open( packages[ p ] ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Corpus package " << packages[ p ] << " failed: " << package.GetError() );
                ++failures;
                continue;
            }
            ++read;
            const CORE::CString entry = packages[ p ].SubstrFromRange( CORE::CString( "projectcorpus" ).Length(), packages[ p ].Length() );
            summaryText += DescribePackage( package, entry );
            if ( 1009 == package.GetVersions().GetUe5Version() && -8 == package.GetVersions().GetLegacyFileVersion() && 522 == package.GetVersions().GetUe4Version() )
                ++classic53;
            ASSERT_TRUE( package.GetSummary().IsEditorPackage() );
            UInt32 mismatches = 0;
            propertyText += entry + "\n" + DescribeProperties( package, 10, mismatches );
            mismatchTotal += mismatches;
        }
        const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corpus project: " << read << " packages read in " << ms << " ms, " << classic53 << " of version 1009, " << mismatchTotal << " property mismatches, " << failures << " failures" );
        ASSERT_TRUE( 0 == failures && read >= 90 && classic53 >= 80 );
        ASSERT_TRUE( 0 == mismatchTotal );
        CORE::CString problem;
        const bool summaryGolden = CheckGolden( "project_summary", summaryText, problem );
        if ( !summaryGolden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        ASSERT_TRUE( summaryGolden );
        const bool propertyGolden = CheckGolden( "project_properties", propertyText, problem );
        if ( !propertyGolden ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        ASSERT_TRUE( propertyGolden );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"          /* GUCEF CORE library API */
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_VFSFileLoadUnloadTest.h"

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of version v of test file f: small files (below the memory load size) and larger ones, a size per version */
UInt32
ReplacedFileSize( UInt32 file, UInt32 version )
{
    return ( 0 == file % 2 ? 600 : 20000 ) + 16 * version;
}

UInt8
ReplacedFileByte( UInt32 file, UInt32 version, UInt32 offset )
{
    return (UInt8) ( file * 31 + version * 7 + offset );
}

bool
MakeReplacedFile( UInt32 file, UInt32 version, CORE::CDynamicBuffer& content )
{
    const UInt32 size = ReplacedFileSize( file, version );
    content.SetDataSize( size );
    UInt8* bytes = static_cast< UInt8* >( content.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        bytes[ i ] = ReplacedFileByte( file, version, i );
    return true;
}

/** Whether the bytes are one whole version of the file, and which */
bool
IsWholeVersion( UInt32 file, const CORE::CDynamicBuffer& content, UInt32 maxVersion, UInt32& version )
{
    const UInt32 base = ReplacedFileSize( file, 0 );
    const UInt32 size = content.GetDataSize();
    if ( size < base || 0 != ( size - base ) % 16 || ( size - base ) / 16 > maxVersion )
        return false;
    version = ( size - base ) / 16;
    const UInt8* bytes = static_cast< const UInt8* >( content.GetConstBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
        if ( bytes[ i ] != ReplacedFileByte( file, version, i ) )
            return false;
    return true;
}

/** Checks, opens and reads files of a mount over and over, as the scans of a table do while its files are replaced */
class CVfsReplacedFilesReader : public MT::CActiveObject
{
    public:

    CVfsReplacedFilesReader( const CORE::CString& vfsDir, UInt32 fileCount, UInt32 maxVersion, UInt32 seed )
        : MT::CActiveObject(), m_vfsDir( vfsDir ), m_fileCount( fileCount ), m_maxVersion( maxVersion ), m_next( seed ),
          m_stop( 0 ), m_reads( 0 ), m_failures( 0 )
    {GUCEF_TRACE;
    }

    void Stop( void ) { m_stop.Store( 1 ); }
    UInt32 GetReads( void ) const { return m_reads.Load(); }
    UInt32 GetFailures( void ) const { return m_failures.Load(); }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( 0 != m_stop.Load() )
            return true;

        // the same files as the other readers and different ones, in an order of its own
        const UInt32 file = m_next % m_fileCount;
        m_next = m_next * 7 + 3;
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const CORE::CString path = m_vfsDir + "/f" + CORE::ToString( file ) + ".bin";
        bool ok = vfs.FileExists( path );
        VFS::TBasicVfsResourcePtr resource = vfs.GetFile( path, "rb", false );
        CORE::IOAccessPtr access = resource.IsNULL() ? CORE::IOAccessPtr() : resource->GetAccess();
        if ( access.IsNULL() )
        {
            ok = false;
        }
        else
        {
            CORE::CDynamicBuffer content;
            const UInt64 size = access->GetSize();
            content.SetDataSize( (UInt32) size );
            UInt32 version = 0;
            ok = ok && 0 == access->Setpos( 0 ) && size == access->Read( content.GetBufferPtr(), 1, (UInt32) size ) &&
                 IsWholeVersion( file, content, m_maxVersion, version );
        }
        m_reads.FetchAdd( 1 );
        if ( !ok )
            m_failures.FetchAdd( 1 );
        return false;
    }

    private:

    CORE::CString m_vfsDir;
    UInt32 m_fileCount;
    UInt32 m_maxVersion;
    UInt32 m_next;
    MT::CTAtomic< UInt32 > m_stop;
    MT::CTAtomic< UInt32 > m_reads;
    MT::CTAtomic< UInt32 > m_failures;
};

}

/*-------------------------------------------------------------------------*/

void
PerformVFSFileLoadUnloadTest( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING VFS file load/unload TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "VFSFileLoadUnload" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "Test: Files of a mounted directory can be listed, loaded and released" )
        try
        {
            // The module directory holds at least the test app itself
            ASSERT_TRUE( vfs.AddRoot( "$MODULEDIR$", "LoadUnloadModuleDir", false, false, "moduledir" ) );

            VFS::CVFS::TStringVector fileList;
            ASSERT_TRUE( vfs.GetFileList( fileList, "moduledir", false, true, CORE::CString::EmptyStringSet ) );
            ASSERT_FALSE( fileList.empty() );

            // Load a number of files at the same time
            UInt32 maxFiles = (UInt32) fileList.size() > 5 ? 5 : (UInt32) fileList.size();
            GUCEF::vector< VFS::TBasicVfsResourcePtr > fileHandles;
            VFS::CVFS::TStringVector::iterator n = fileList.begin();
            for ( UInt32 i=0; i<maxFiles; ++i, ++n )
            {
                VFS::TBasicVfsResourcePtr filePtr = vfs.GetFile( (*n), "rb", false );
                ASSERT_FALSE( filePtr.IsNULL() );
                if ( !filePtr.IsNULL() )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Loaded file " + filePtr->GetFilename() );
                    fileHandles.push_back( filePtr );
                }
            }
            ASSERT_TRUE( maxFiles == fileHandles.size() );

            // Releasing the handles unloads the files
            while ( !fileHandles.empty() )
                fileHandles.pop_back();

            ASSERT_TRUE( vfs.UnmountArchiveByName( "LoadUnloadModuleDir" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A file written through the VFS reads back identically" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_LoadUnload" );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            ASSERT_TRUE( vfs.AddRoot( dir, "LoadUnloadTempDir", true, false, "tempdir" ) );

            CORE::CString newFilename = "tempdir/DummyVFSTestFile.tmp";
            vfs.DeleteFile( newFilename, true );
            ASSERT_FALSE( vfs.FileExists( newFilename ) );

            // Create a buffer with test values
            char buffer[ 1024 ];
            UInt8 testValue = 0;
            for ( UInt32 n=0; n<1024; ++n )
                buffer[ n ] = (char) testValue++;

            VFS::TBasicVfsResourcePtr filePtr = vfs.GetFile( newFilename, "wb", true );
            ASSERT_FALSE( filePtr.IsNULL() );
            if ( !filePtr.IsNULL() )
            {
                CORE::IOAccessPtr access = filePtr->GetAccess();
                ASSERT_FALSE( access.IsNULL() );
                if ( !access.IsNULL() )
                {
                    access->Open();
                    bool allWritten = true;
                    for ( UInt32 n=0; n<1024 && allWritten; ++n )
                        allWritten = 1024 == access->Write( buffer, 1, 1024 );
                    ASSERT_TRUE( allWritten );
                    access->Close();
                }
            }

            // Make sure there is no outstanding reference to our test file and load it again
            filePtr = VFS::TBasicVfsResourcePtr();
            filePtr = vfs.GetFile( newFilename, "rb", false );
            ASSERT_FALSE( filePtr.IsNULL() );
            if ( !filePtr.IsNULL() )
            {
                CORE::IOAccessPtr access = filePtr->GetAccess();
                ASSERT_FALSE( access.IsNULL() );
                if ( !access.IsNULL() )
                {
                    access->Open();
                    char readBuffer[ 1024 ];
                    bool allRead = true;
                    bool allMatch = true;
                    for ( UInt32 n=0; n<1024 && allRead && allMatch; ++n )
                    {
                        allRead = 1024 == access->Read( readBuffer, 1, 1024 );
                        allMatch = 0 == memcmp( buffer, readBuffer, 1024 );
                    }
                    ASSERT_TRUE( allRead );
                    ASSERT_TRUE( allMatch );
                    access->Close();
                }
            }

            filePtr = VFS::TBasicVfsResourcePtr();
            ASSERT_TRUE( vfs.DeleteFile( newFilename, false ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "LoadUnloadTempDir" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Regression: a small file (at most the memory load size) was kept in the archive's memory cache under a key the
    // release of its handle never found, so every later read gave the first contents, also after the file changed
    GUCEF_TESTFW_TESTCASE( "Test: Small files read through the VFS give their current contents after a change or a replacement" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_SmallFiles" );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            ASSERT_TRUE( vfs.AddRoot( dir, "SmallFilesTempDir", true, false, "smallfiles" ) );
            CORE::CString fsPath = CORE::CombinePath( dir, "small.txt" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( fsPath, "first" ) );
            ASSERT_TRUE( vfs.GetMemloadSize() >= 6 );

            CORE::CDynamicBuffer contents;
            ASSERT_TRUE( vfs.LoadFile( contents, "smallfiles/small.txt", "rb" ) );
            ASSERT_TRUE( 5 == contents.GetDataSize() && 0 == memcmp( contents.GetConstBufferPtr(), "first", 5 ) );

            // changed on disk, outside the VFS, after the handle was released
            ASSERT_TRUE( CORE::WriteStringAsTextFile( fsPath, "second" ) );
            contents.Clear();
            ASSERT_TRUE( vfs.LoadFile( contents, "smallfiles/small.txt", "rb" ) );
            ASSERT_TRUE( 6 == contents.GetDataSize() && 0 == memcmp( contents.GetConstBufferPtr(), "second", 6 ) );

            // replaced through the VFS while a handle is open: the open handle keeps its copy, a new read gets the new file
            VFS::TBasicVfsResourcePtr held = vfs.GetFile( "smallfiles/small.txt", "rb", false );
            ASSERT_FALSE( held.IsNULL() );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "small.txt.new" ), "third" ) );
            ASSERT_TRUE( vfs.MoveFile( "smallfiles/small.txt.new", "smallfiles/small.txt", true ) );
            contents.Clear();
            ASSERT_TRUE( vfs.LoadFile( contents, "smallfiles/small.txt", "rb" ) );
            ASSERT_TRUE( 5 == contents.GetDataSize() && 0 == memcmp( contents.GetConstBufferPtr(), "third", 5 ) );
            if ( !held.IsNULL() )
            {
                char oldBytes[ 8 ];
                CORE::IOAccessPtr access = held->GetAccess();
                ASSERT_TRUE( 0 == access->Setpos( 0 ) && 6 == access->Read( oldBytes, 1, 8 ) && 0 == memcmp( oldBytes, "second", 6 ) );
            }
            held.Unlink();

            // deleted through the VFS: gone, also for reads that hit the cache before
            ASSERT_TRUE( vfs.DeleteFile( "smallfiles/small.txt", false ) );
            ASSERT_TRUE( vfs.GetFile( "smallfiles/small.txt", "rb", false ).IsNULL() );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "SmallFilesTempDir" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Files are checked, opened and read on many threads at once, the same and different ones, while they are replaced: every read gives one whole version, the last one afterwards" )
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_ConcurrentReplace" );
            CORE::DeleteDir( dir, true );
            ASSERT_TRUE( CORE::CreateDirs( dir ) );
            ASSERT_TRUE( vfs.AddRoot( dir, "ConcurrentReplaceDir", true, false, "concurrentreplace" ) );
            const UInt32 fileCount = 6;
            const UInt32 versions = 40;
            CORE::CDynamicBuffer content;
            for ( UInt32 f=0; f<fileCount; ++f )
            {
                MakeReplacedFile( f, 0, content );
                ASSERT_TRUE( content.WriteContentToFile( CORE::CombinePath( dir, "f" + CORE::ToString( f ) + ".bin" ), 0, true ) );
            }

            const UInt32 readerCount = 8;
            CVfsReplacedFilesReader* readers[ readerCount ];
            for ( UInt32 r=0; r<readerCount; ++r )
            {
                readers[ r ] = GUCEF_NEW CVfsReplacedFilesReader( "concurrentreplace", fileCount, versions, r );
                ASSERT_TRUE( readers[ r ]->Activate( GUCEF_NULL, 0, 0 ) );
            }

            // the writer: each version of each file is written next to it and put in place in one rename
            UInt32 replaced = 0;
            for ( UInt32 v=1; v<=versions; ++v )
            {
                for ( UInt32 f=0; f<fileCount; ++f )
                {
                    const CORE::CString name = "f" + CORE::ToString( f ) + ".bin";
                    MakeReplacedFile( f, v, content );
                    if ( content.WriteContentToFile( CORE::CombinePath( dir, name + ".tmp" ), 0, true ) &&
                         vfs.MoveFileAtomically( "concurrentreplace/" + name + ".tmp", "concurrentreplace/" + name, true ) )
                        ++replaced;
                }
                MT::PrecisionDelay( 1 );
            }
            ASSERT_TRUE( fileCount * versions == replaced );

            UInt32 reads = 0;
            UInt32 failures = 0;
            for ( UInt32 r=0; r<readerCount; ++r )
            {
                readers[ r ]->Stop();
                readers[ r ]->WaitForThreadToFinish( 60000 );
                reads += readers[ r ]->GetReads();
                failures += readers[ r ]->GetFailures();
                GUCEF_DELETE readers[ r ];
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "VFS concurrent replace: " << reads << " reads on " << readerCount << " threads, " << failures << " failed, " << replaced << " replaces" );
            ASSERT_TRUE( reads > readerCount && 0 == failures );

            // afterwards every file reads as its last version, also the small ones a reader may have held in memory meanwhile
            for ( UInt32 f=0; f<fileCount; ++f )
            {
                CORE::CDynamicBuffer loaded;
                UInt32 version = 0;
                ASSERT_TRUE( vfs.LoadFile( loaded, "concurrentreplace/f" + CORE::ToString( f ) + ".bin", "rb" ) );
                ASSERT_TRUE( IsWholeVersion( f, loaded, versions, version ) && versions == version );
            }
            ASSERT_TRUE( vfs.UnmountArchiveByName( "ConcurrentReplaceDir" ) );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: MoveFileAtomically replaces a file in one rename or not at all: never a copy, a refused rename leaves the target and its readers as they were" )
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        GUCEF_TESTFW_SKIP_IF( 0 == ::geteuid(), "the super user renames in a read only directory" );
        #endif
        try
        {
            CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_AtomicMove" );
            CORE::CString otherDir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_AtomicMoveOther" );
            CORE::DeleteDir( dir, true );
            CORE::DeleteDir( otherDir, true );
            ASSERT_TRUE( CORE::CreateDirs( dir ) && CORE::CreateDirs( otherDir ) );
            ASSERT_TRUE( vfs.AddRoot( dir, "AtomicMoveDir", true, false, "atomicmove" ) );
            ASSERT_TRUE( vfs.AddRoot( otherDir, "AtomicMoveOtherDir", true, false, "atomicother" ) );

            // larger than the memory load size: read from the file, not from a copy in memory
            CORE::CString oldText;
            CORE::CString newText;
            for ( UInt32 i=0; i<512; ++i )
            {
                oldText += "OLD-";
                newText += "NEW+";
            }
            const CORE::CString target = CORE::CombinePath( dir, "part.bin" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( target, oldText ) );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "part.bin.tmp-1" ), newText ) );

            // across archives there is no atomic move: refused, nothing copied
            ASSERT_FALSE( vfs.MoveFileAtomically( "atomicmove/part.bin.tmp-1", "atomicother/part.bin", true ) );
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( otherDir, "part.bin" ) ) );
            ASSERT_TRUE( CORE::FileExists( CORE::CombinePath( dir, "part.bin.tmp-1" ) ) );

            // without overwrite an existing target stays
            ASSERT_FALSE( vfs.MoveFileAtomically( "atomicmove/part.bin.tmp-1", "atomicmove/part.bin", false ) );

            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            // the rename is refused (the directory takes no new names) while the target file itself stays writable and a
            // reader holds it: the target is not rewritten, the reader keeps the old bytes
            VFS::TBasicVfsResourcePtr held = vfs.GetFile( "atomicmove/part.bin", "rb", false );
            ASSERT_FALSE( held.IsNULL() );
            ASSERT_TRUE( 0 == ::chmod( dir.C_String(), 0555 ) );
            const bool movedWhileRefused = vfs.MoveFileAtomically( "atomicmove/part.bin.tmp-1", "atomicmove/part.bin", true );
            ::chmod( dir.C_String(), 0755 );
            ASSERT_FALSE( movedWhileRefused );
            CORE::CString content;
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) && content == oldText );
            if ( !held.IsNULL() )
            {
                CORE::CDynamicBuffer bytes;
                CORE::IOAccessPtr access = held->GetAccess();
                bytes.SetDataSize( oldText.Length() );
                ASSERT_TRUE( 0 == access->Setpos( 0 ) && oldText.Length() == access->Read( bytes.GetBufferPtr(), 1, oldText.Length() ) );
                ASSERT_TRUE( 0 == memcmp( bytes.GetConstBufferPtr(), oldText.C_String(), oldText.Length() ) );
            }

            // allowed again: one rename, the reader keeps the old file, the path gives the new one with the target's mode
            ASSERT_TRUE( 0 == ::chmod( target.C_String(), 0600 ) && 0 == ::chmod( CORE::CombinePath( dir, "part.bin.tmp-1" ).C_String(), 0644 ) );
            ASSERT_TRUE( vfs.MoveFileAtomically( "atomicmove/part.bin.tmp-1", "atomicmove/part.bin", true ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( target, content, false ) && content == newText );
            struct stat info;
            ASSERT_TRUE( 0 == ::stat( target.C_String(), &info ) && 0600 == ( info.st_mode & 07777 ) );
            if ( !held.IsNULL() )
            {
                char first[ 4 ];
                CORE::IOAccessPtr access = held->GetAccess();
                ASSERT_TRUE( 0 == access->Setpos( 0 ) && 4 == access->Read( first, 1, 4 ) && 0 == memcmp( first, "OLD-", 4 ) );
            }
            held.Unlink();
            #else
            ASSERT_TRUE( vfs.MoveFileAtomically( "atomicmove/part.bin.tmp-1", "atomicmove/part.bin", true ) );
            #endif
            ASSERT_FALSE( CORE::FileExists( CORE::CombinePath( dir, "part.bin.tmp-1" ) ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "AtomicMoveDir" ) && vfs.UnmountArchiveByName( "AtomicMoveOtherDir" ) );
            CORE::DeleteDir( dir, true );
            CORE::DeleteDir( otherDir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL VFS file load/unload TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/

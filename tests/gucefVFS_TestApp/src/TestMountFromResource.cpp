/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestMountFromResource.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEST ARCHIVE                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Minimal archive that can only be loaded from a resource: it exposes the entire
 *  content of that resource as a single file called "content.txt".
 */
class CTestResourceArchive : public VFS::CArchive
{
    public:

    static const VFS::CString TypeName;

    CTestResourceArchive( void ) : VFS::CArchive(), m_writeable( false ) {}
    virtual ~CTestResourceArchive() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( vfsResource.IsNULL() || vfsResource->GetAccess().IsNULL() )
            return false;

        CORE::IOAccessPtr access = vfsResource->GetAccess();
        access->Setpos( 0 );
        UInt64 size = access->GetSize();
        m_content.SetDataSize( (UInt32) size );
        if ( size > 0 && access->Read( m_content.GetBufferPtr(), (UInt32) size, 1 ) != 1 )
            return false;

        m_archiveName = archiveName;
        m_writeable = writeableRequest;
        return true;
    }

    virtual bool LoadArchive( const VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE { m_content.Clear(); return true; }

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file ,
                                               const char* mode         ,
                                               const VFS::UInt32 memLoadSize ,
                                               const bool overwrite     ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !FileExists( file ) )
            return VFS::TBasicVfsResourcePtr();

        CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( m_content ) );
        return VFS::TBasicVfsResourcePtr( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, "content.txt", file ) ) );
    }

    virtual bool StoreAsFile( const CORE::CString& filepath, const CORE::CDynamicBuffer& data, const CORE::UInt64 offset, const bool overwrite ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual void GetFileList( TStringVector& outputList, const VFS::CString& mountLocation, const VFS::CString& archiveLocation, bool recursive, bool includePathInFilename,
                              const VFS::CString::StringSet& nameFilters, VFS::UInt32 maxListEntries ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( archiveLocation.IsNULLOrEmpty() || archiveLocation == "/" )
            outputList.push_back( includePathInFilename ? "/content.txt" : "content.txt" );
    }

    virtual void GetDirList( TStringVector& outputList, const VFS::CString& mountLocation, const VFS::CString& archiveLocation, bool recursive, bool includeParentDirInName,
                             const VFS::CString::StringSet& nameFilters, VFS::UInt32 maxListEntries ) const GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool DeleteFile( const VFS::CString& filePath ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool MoveFile( const VFS::CString& oldFilePath, const VFS::CString& newFilePath, const bool overwrite ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool FileExists( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return filePath == "content.txt" || filePath == "/content.txt";
    }

    virtual bool DirExists( const VFS::CString& dirPath ) const GUCEF_VIRTUAL_OVERRIDE { return dirPath.IsNULLOrEmpty() || dirPath == "/"; }
    virtual VFS::UInt64 GetFileSize( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE { return FileExists( filePath ) ? m_content.GetDataSize() : 0; }
    virtual bool GetFileMetaData( const VFS::CString& filePath, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual VFS::CString GetFileHash( const VFS::CString& file ) const GUCEF_VIRTUAL_OVERRIDE { return VFS::CString::Empty; }
    virtual CORE::CDateTime GetFileModificationTime( const VFS::CString& filePath ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CDateTime(); }
    virtual const VFS::CString& GetArchiveName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_archiveName; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_writeable; }
    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TypeName; }

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE { delete objectToBeDestroyed; }

    protected:

    VFS::CString m_archiveName;
    CORE::CDynamicBuffer m_content;
    bool m_writeable;
};

const VFS::CString CTestResourceArchive::TypeName = "testresourcearchive";

typedef CORE::CTFactory< VFS::CArchive, CTestResourceArchive, MT::CMutex > TTestResourceArchiveFactory;

/*-------------------------------------------------------------------------*/

/**
 *  As the archive above, but like archives of real formats it keeps the resource it was loaded from while it
 *  is mounted, and the files it hands out are destroyed by the archive itself. It notes when it is destroyed
 *  while files it handed out still exist: their destruction would then call a destroyed archive.
 */
class CTestKeepingArchive : public CTestResourceArchive
{
    public:

    static const VFS::CString TypeName;
    static bool destroyedWhileFilesExist;

    CTestKeepingArchive( void ) : CTestResourceArchive(), m_source(), m_filesOut( 0 ) {}

    virtual ~CTestKeepingArchive() GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_filesOut > 0 )
            destroyedWhileFilesExist = true;
    }

    virtual bool LoadArchive( const VFS::CString& archiveName       ,
                              VFS::TBasicVfsResourcePtr vfsResource ,
                              const bool writeableRequest           ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !CTestResourceArchive::LoadArchive( archiveName, vfsResource, writeableRequest ) )
            return false;
        m_source = vfsResource;
        return true;
    }

    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_source.Unlink();
        return CTestResourceArchive::UnloadArchive();
    }

    virtual VFS::TBasicVfsResourcePtr GetFile( const VFS::CString& file ,
                                               const char* mode         ,
                                               const VFS::UInt32 memLoadSize ,
                                               const bool overwrite     ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !FileExists( file ) )
            return VFS::TBasicVfsResourcePtr();
        CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( m_content ) );
        ++m_filesOut;
        return VFS::TBasicVfsResourcePtr( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, "content.txt", file ), this ) );
    }

    virtual const VFS::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TypeName; }

    virtual void DestroyObject( VFS::CVFSHandle* objectToBeDestroyed ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == objectToBeDestroyed )
            return;
        --m_filesOut;
        delete objectToBeDestroyed;
    }

    private:

    VFS::TBasicVfsResourcePtr m_source;
    mutable Int32 m_filesOut;
};

const VFS::CString CTestKeepingArchive::TypeName = "testkeepingarchive";
bool CTestKeepingArchive::destroyedWhileFilesExist = false;

typedef CORE::CTFactory< VFS::CArchive, CTestKeepingArchive, MT::CMutex > TTestKeepingArchiveFactory;

/*-------------------------------------------------------------------------*/

void
PerformMountFromResourceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Mount from resource TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "MountFromResource" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    TTestResourceArchiveFactory factory;
    vfs.RegisterArchiveFactory( CTestResourceArchive::TypeName, factory );
    TTestKeepingArchiveFactory keepingFactory;
    vfs.RegisterArchiveFactory( CTestKeepingArchive::TypeName, keepingFactory );

    const char* payload = "archive payload that came from a resource";
    CORE::CDynamicBuffer payloadBuffer;
    payloadBuffer.CopyFrom( (UInt32) strlen( payload ), payload );

    GUCEF_TESTFW_TESTCASE( "Test: An archive is mounted from a resource and its content is readable through the VFS" )
        try
        {
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( payloadBuffer ) );
            VFS::TBasicVfsResourcePtr resource( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, "payload.bin", "payload.bin" ) ) );

            ASSERT_TRUE( vfs.MountArchive( "mountedFromResource", resource, CTestResourceArchive::TypeName, "/fromresource", false ) );
            ASSERT_TRUE( vfs.IsMountedArchive( "/fromresource" ) );
            ASSERT_TRUE( vfs.FileExists( "/fromresource/content.txt" ) );
            ASSERT_TRUE( vfs.GetFileSize( "/fromresource/content.txt" ) == strlen( payload ) );

            CORE::CDynamicBuffer loaded;
            ASSERT_TRUE( vfs.LoadFile( loaded, "/fromresource/content.txt", "rb" ) );
            ASSERT_TRUE( loaded.GetDataSize() == strlen( payload ) );
            ASSERT_TRUE( 0 == memcmp( loaded.GetConstBufferPtr(), payload, strlen( payload ) ) );

            ASSERT_TRUE( vfs.UnmountArchiveByName( "mountedFromResource" ) );
            ASSERT_FALSE( vfs.IsMountedArchive( "/fromresource" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The IOAccessPtr convenience overload mounts the same way" )
        try
        {
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( payloadBuffer ) );
            ASSERT_TRUE( vfs.MountArchive( "mountedFromAccess", access, CTestResourceArchive::TypeName, "/fromaccess", false ) );
            ASSERT_TRUE( vfs.FileExists( "/fromaccess/content.txt" ) );
            ASSERT_TRUE( vfs.GetFileSize( "/fromaccess/content.txt" ) == strlen( payload ) );
            ASSERT_TRUE( vfs.UnmountArchiveByName( "mountedFromAccess" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Mounting from a resource fails cleanly for missing resources and unknown archive types" )
        try
        {
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( payloadBuffer ) );
            ASSERT_FALSE( vfs.MountArchive( "nullAccess", CORE::IOAccessPtr(), CTestResourceArchive::TypeName, "/nullaccess", false ) );
            ASSERT_FALSE( vfs.MountArchive( "nullResource", VFS::TBasicVfsResourcePtr(), CTestResourceArchive::TypeName, "/nullresource", false ) );
            ASSERT_FALSE( vfs.MountArchive( "unknownType", access, "thisarchivetypedoesnotexist", "/unknowntype", false ) );
            ASSERT_FALSE( vfs.IsMountedArchive( "/unknowntype" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unmounting everything lets an archive mounted from a file of another archive go first" )
        try
        {
            // the outer archive from memory, the inner one from the file of the outer one, as a disc image holds the archives of a game
            CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( payloadBuffer ) );
            VFS::TBasicVfsResourcePtr resource( VFS::TVfsResourcePtr( GUCEF_NEW VFS::CVFSHandle( access, "payload.bin", "payload.bin" ) ) );
            ASSERT_TRUE( vfs.MountArchive( "outerArchive", resource, CTestKeepingArchive::TypeName, "/outer", false ) );
            resource.Unlink();
            VFS::TBasicVfsResourcePtr inner = vfs.GetFile( "/outer/content.txt", "rb", false );
            ASSERT_FALSE( inner.IsNULL() );
            ASSERT_TRUE( vfs.MountArchive( "innerArchive", inner, CTestKeepingArchive::TypeName, "/inner", false ) );
            inner.Unlink();
            ASSERT_TRUE( vfs.FileExists( "/inner/content.txt" ) );

            CTestKeepingArchive::destroyedWhileFilesExist = false;
            vfs.UnmountAllArchives();
            ASSERT_FALSE( vfs.IsMountedArchive( "/outer" ) || vfs.IsMountedArchive( "/inner" ) );
            ASSERT_FALSE( CTestKeepingArchive::destroyedWhileFilesExist );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    vfs.UnregisterArchiveFactory( CTestResourceArchive::TypeName );
    vfs.UnregisterArchiveFactory( CTestKeepingArchive::TypeName );
}

/*-------------------------------------------------------------------------*/

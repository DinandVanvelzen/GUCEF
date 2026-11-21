/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsConfig.h"

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A folder with one file, for an archive of the config */
static CORE::CString
MakeConfigTestDir( const char* name )
{GUCEF_TRACE;

    CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_Config" ), name );
    CORE::CreateDirs( dir );
    CORE::WriteStringAsTextFile( CORE::CombinePath( dir, CORE::CString( name ) + ".txt" ), name );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** Archive settings in the form of a config file */
static void
AddArchiveSettings( CORE::CDataNode& archives, const CORE::CString& name, const CORE::CString& dir, const CORE::CString& mountPath )
{GUCEF_TRACE;

    CORE::CDataNode* settings = archives.AddChild( "ArchiveSettings" );
    settings->SetAttribute( "archiveName", name );
    settings->SetAttribute( "actualArchivePath", dir );
    settings->SetAttribute( "archiveType", VFS::CVFS::FileSystemArchiveTypeName );
    settings->SetAttribute( "mountPath", mountPath );
    settings->SetAttribute( "writeable", false );
}

/*-------------------------------------------------------------------------*/

/** The names of the archive settings saved under VFS/Archives */
static CORE::CString::StringSet
SavedArchiveNames( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CString::StringSet names;
    const CORE::CDataNode* vfsNode = tree.FindChild( "VFS" );
    const CORE::CDataNode* archives = GUCEF_NULL != vfsNode ? vfsNode->FindChild( "Archives" ) : GUCEF_NULL;
    if ( GUCEF_NULL != archives )
    {
        CORE::CDataNode::TConstDataNodeSet entries = archives->FindChildrenOfType( "ArchiveSettings" );
        CORE::CDataNode::TConstDataNodeSet::iterator i = entries.begin();
        while ( i != entries.end() )
        {
            names.insert( (*i)->GetAttributeValue( "archiveName" ).AsString() );
            ++i;
        }
    }
    return names;
}

/*-------------------------------------------------------------------------*/

/** Counts what the VFS logs about mounting the filesystem roots, per kind of message, while attached to the log manager */
class CMountRootsLogCapture : public CORE::CILogger
{
    public:

    CMountRootsLogCapture( void ) : CORE::CILogger(), m_errors( 0 ), m_warnings( 0 ), m_lock()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this );
    }

    virtual ~CMountRootsLogCapture()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this );
    }

    UInt32 GetErrors( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_errors;
    }

    UInt32 GetWarnings( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_warnings;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( logMessage.HasSubstr( "VFS:MountFileSystemRoots" ) < 0 )
            return;
        MT::CScopeMutex lock( m_lock );
        if ( CORE::LOG_ERROR == logMsgType )
            ++m_errors;
        else
        if ( CORE::LOG_WARNING == logMsgType )
            ++m_warnings;
    }

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage, threadId, timestamp );
    }

    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    {
        Log( logMsgType, logLevel, logMessage.ToString(), threadId, timestamp );
    }

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    UInt32 m_errors;
    UInt32 m_warnings;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/


void
PerformVfsConfigTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VfsConfig" );

    GUCEF_TESTFW_TESTCASE( "VfsConfig: The archives of the config are mounted and saved back as configured, without the archives mounted at run time" )
        try
        {
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            const CORE::CString dirA = MakeConfigTestDir( "cfga" );
            const CORE::CString dirB = MakeConfigTestDir( "cfgb" );
            const CORE::CString dirC = MakeConfigTestDir( "cfgc" );

            // a config as a config file has it: the system settings and a list of archives
            CORE::CDataNode config( "Config" );
            CORE::CDataNode* archives = config.Structure( "VFS/Archives", '/' );
            ASSERT_TRUE( GUCEF_NULL != archives );
            AddArchiveSettings( *archives, "VfsConfigTestA", dirA, "vfsconfigtest/a" );
            AddArchiveSettings( *archives, "VfsConfigTestB", dirB, "vfsconfigtest/b" );
            ASSERT_TRUE( vfs.LoadConfig( config ) );
            ASSERT_TRUE( vfs.FileExists( "vfsconfigtest/a/cfga.txt" ) && vfs.FileExists( "vfsconfigtest/b/cfgb.txt" ) );

            // an archive mounted by the program, a disc for example
            ASSERT_TRUE( vfs.MountArchive( "VfsConfigTestRuntime", dirC, VFS::CVFS::FileSystemArchiveTypeName, "vfsconfigtest/c", false, false, dirC ) );

            // saved: the system settings and the configured archives, not the one of the program
            CORE::CDataNode saved( "Config" );
            ASSERT_TRUE( vfs.SaveConfig( saved ) );
            CORE::CString::StringSet names = SavedArchiveNames( saved );
            ASSERT_TRUE( 2 == names.size() && names.find( "VfsConfigTestA" ) != names.end() && names.find( "VfsConfigTestB" ) != names.end() );
            ASSERT_TRUE( saved.FindChildrenOfType( "vfsroot", true, false ).empty() );
            const CORE::CDataNode* vfsNode = saved.FindChild( "VFS" );
            ASSERT_TRUE( GUCEF_NULL != vfsNode && GUCEF_NULL != vfsNode->GetAttribute( "maxmemload" ) && GUCEF_NULL != vfsNode->GetAttribute( "autoMountFsVolumes" ) );

            // and what was saved loads as it was configured
            ASSERT_TRUE( vfs.UnmountArchiveByName( "VfsConfigTestA" ) && vfs.UnmountArchiveByName( "VfsConfigTestB" ) );
            ASSERT_FALSE( vfs.FileExists( "vfsconfigtest/a/cfga.txt" ) );
            ASSERT_TRUE( vfs.LoadConfig( saved ) );
            ASSERT_TRUE( vfs.FileExists( "vfsconfigtest/a/cfga.txt" ) && vfs.FileExists( "vfsconfigtest/b/cfgb.txt" ) && vfs.FileExists( "vfsconfigtest/c/cfgc.txt" ) );
            CORE::CDataNode savedAgain( "Config" );
            ASSERT_TRUE( vfs.SaveConfig( savedAgain ) && 2 == SavedArchiveNames( savedAgain ).size() );

            vfs.UnmountArchiveByName( "VfsConfigTestA" );
            vfs.UnmountArchiveByName( "VfsConfigTestB" );
            vfs.UnmountArchiveByName( "VfsConfigTestRuntime" );
            CORE::DeleteDir( CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_Config" ), true );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

void
PerformVfsMountRootsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VfsMountRoots" );

    // Where the system has storage volumes they are mounted without an error. Where it has none, or cannot list them
    // (a container has no block devices) the VFS says so with a warning: that is not an error, and the result stays false
    // for a failure to list. The same test runs on both kinds of system
    GUCEF_TESTFW_TESTCASE( "VfsMountRoots: A system without storage volumes is no error, a system with them mounts them" )
        try
        {
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

            CORE::CString::StringSet volumeIds;
            const bool listed = CORE::GetAllFileSystemStorageVolumes( volumeIds );

            CMountRootsLogCapture capture;
            const bool mounted = vfs.MountFileSystemRoots( "mountrootstest/" );

            if ( !listed )
            {
                ASSERT_FALSE( mounted );
                ASSERT_TRUE( 1 == capture.GetWarnings() );
                ASSERT_TRUE( 0 == capture.GetErrors() );
            }
            else
            if ( volumeIds.empty() )
            {
                ASSERT_TRUE( mounted );
                ASSERT_TRUE( 0 == capture.GetWarnings() && 0 == capture.GetErrors() );
            }
            else
            {
                ASSERT_TRUE( 0 == capture.GetWarnings() && 0 == capture.GetErrors() );
            }
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

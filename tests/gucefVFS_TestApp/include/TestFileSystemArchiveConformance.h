/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTFILESYSTEMARCHIVECONFORMANCE_H
#define GUCEF_VFS_TESTAPP_TESTFILESYSTEMARCHIVECONFORMANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestArchiveConformanceFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The conformance fixture of the file system archive (VFS::CFileSystemArchive), the reference implementation of the
 *  archive interface: the reference tree is written to a directory of the local file system and the archive is loaded
 *  on that directory.
 *
 *  The directory is made below GUCEF_TEST_SCRATCH_DIR when that environment variable is set and below the temporary
 *  directory of the system otherwise, and removed again by Teardown().
 */
class CFileSystemArchiveConformanceFixture : public CArchiveConformanceFixture
{
    public:

    /** @param writable whether the archive is loaded as a writable one, which also runs the write cases */
    explicit CFileSystemArchiveConformanceFixture( bool writable );

    virtual ~CFileSystemArchiveConformanceFixture() GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Setup( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Teardown( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::VFS::CArchive* GetArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CArchiveConformanceTree& GetTree( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CArchiveConformanceCapabilities& GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PutFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RemoveFile( const GUCEF::CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetPathOutsideArchive( GUCEF::CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;

    /** The directory of the file system that holds the archive, empty before Setup() */
    const GUCEF::CORE::CString& GetRootDir( void ) const;

    private:

    CFileSystemArchiveConformanceFixture( const CFileSystemArchiveConformanceFixture& src );
    CFileSystemArchiveConformanceFixture& operator=( const CFileSystemArchiveConformanceFixture& src );

    GUCEF::CORE::CString GetFileSystemPath( const GUCEF::CORE::CString& archivePath ) const;

    bool WriteTreeEntry( const CArchiveConformanceTree::SEntry& entry ) const;

    private:

    bool m_writable;
    GUCEF::CORE::CString m_scratchDir;
    GUCEF::CORE::CString m_rootDir;
    CArchiveConformanceTree m_tree;
    CArchiveConformanceCapabilities m_capabilities;
    GUCEF::VFS::CArchive* m_archive;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Runs the archive conformance suite (PerformArchiveConformanceSuite()) on the file system archive: once as a
 *  writable archive, with the write cases, and once as a read only archive.
 */
void
PerformFileSystemArchiveConformanceTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTFILESYSTEMARCHIVECONFORMANCE_H ? */

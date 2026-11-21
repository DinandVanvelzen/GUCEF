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

#ifndef GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCEFIXTURE_H
#define GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCEFIXTURE_H

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

#include "TestArchiveConformanceCapabilities.h"
#include "TestArchiveConformanceTree.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a backend gives the archive conformance suite (PerformArchiveConformanceSuite()): a loaded archive that holds
 *  the reference tree (CArchiveConformanceTree), what that archive can do, and optionally a way to change the content
 *  behind it.
 *
 *  A fixture for a backend that is not a local directory (a share on the other side of a connection, a package file)
 *  implements this interface and runs the same suite: it makes no assumption about local disk semantics. Everything
 *  the suite asks of the archive goes through the CArchive interface, everything it asks of what is behind the archive
 *  goes through this interface.
 *
 *  Use: Setup(), run the suite, Teardown(). The suite does that itself.
 */
class CArchiveConformanceFixture
{
    public:

    virtual ~CArchiveConformanceFixture();

    /** A name for the results, like "FileSystem" */
    virtual GUCEF::CORE::CString GetName( void ) const = 0;

    /**
     *  Makes the backend and puts the tree in it, then loads the archive, so that GetArchive() gives an archive that
     *  holds exactly the tree and is ready to use. The capabilities may depend on what the setup found.
     *  @return false when that failed, the suite does not run then and the failure is recorded
     */
    virtual bool Setup( void ) = 0;

    /** Removes what Setup() made. Safe to call when Setup() failed or was not called, and more than once */
    virtual void Teardown( void ) = 0;

    /** The archive under test, valid between a successful Setup() and Teardown(). The fixture keeps the ownership */
    virtual GUCEF::VFS::CArchive* GetArchive( void ) = 0;

    /** The tree the archive holds */
    virtual const CArchiveConformanceTree& GetTree( void ) const = 0;

    virtual const CArchiveConformanceCapabilities& GetCapabilities( void ) const = 0;

    /**
     *  Puts a file with the given content behind the archive, replacing an existing one, as another party would: not
     *  through the archive under test. For the change detection cases; only used when canMutateTree is set.
     *  The default does nothing and fails.
     *  @param path relative to the root of the archive
     */
    virtual bool PutFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CDynamicBuffer& content );

    /** Removes a file from behind the archive, as PutFile() does. The default does nothing and fails */
    virtual bool RemoveFile( const GUCEF::CORE::CString& path );

    /**
     *  A path relative to the root of the archive that, when a ".." is followed out of the archive, leads to an
     *  existing file that is not part of the archive. For the case that checks that an archive stays within its root;
     *  only used when canProvideOutsidePath is set. The default fails.
     */
    virtual bool GetPathOutsideArchive( GUCEF::CORE::CString& path );
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCEFIXTURE_H ? */

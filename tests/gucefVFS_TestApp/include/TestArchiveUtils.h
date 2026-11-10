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

#ifndef GUCEF_VFS_TESTAPP_TESTARCHIVEUTILS_H
#define GUCEF_VFS_TESTAPP_TESTARCHIVEUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Shared helpers for the archive tests
 */
namespace TestArchiveUtils {

/** loads the VFS plugin with the given name from the module directory, debug or release build */
bool LoadVfsPlugin( const GUCEF::CORE::CString& pluginName );

/**
 *  The tests with entries of many gigabytes take minutes, which is too long for every run. They only run when asked:
 *  start the application with -large or set the environment variable GUCEF_TEST_LARGE_FILES to 1.
 */
bool LargeFileTestsEnabled( void );

/**
 *  A source of zero bytes of a given size which takes no memory or disk space,
 *  optionally pretending its size is unknown like a stream would
 */
class CZeroSource : public GUCEF::CORE::CIOAccess
{
    public:

    CZeroSource( GUCEF::UInt64 size, bool sizeKnown );

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 Read( void* dest, GUCEF::UInt32 esize, GUCEF::UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::Int32 Seek( GUCEF::Int64 offset, GUCEF::Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::UInt64 m_size;
    GUCEF::UInt64 m_pos;
    bool m_sizeKnown;
};

/**
 *  Runs an external program and waits for it
 *  @return its exit code, or -1 when it could not be run or did not finish in time
 */
GUCEF::Int32 RunExternalProgram( const GUCEF::CORE::CString& exe                     ,
                                 const GUCEF::CORE::CProcess::TStringVector& args   ,
                                 GUCEF::UInt32 timeoutInMs = 600000                  );

/** whether the given program can be run, checked by running it with the given argument */
bool IsToolAvailable( const GUCEF::CORE::CString& exe, const GUCEF::CORE::CString& versionArg );

/**
 *  Runs the given Python script with the given arguments
 *  @return true if the script succeeded or Python is not available, in which case a warning is logged
 */
bool RunPythonCheck( const GUCEF::CORE::CString& scriptPath, const GUCEF::CORE::CProcess::TStringVector& args );

/** reads the entire content of a file via the VFS */
bool ReadVfsFile( GUCEF::VFS::CVFS& vfs, const GUCEF::CORE::CString& path, GUCEF::CORE::CDynamicBuffer& content );

}; /* namespace TestArchiveUtils */

#endif /* GUCEF_VFS_TESTAPP_TESTARCHIVEUTILS_H ? */

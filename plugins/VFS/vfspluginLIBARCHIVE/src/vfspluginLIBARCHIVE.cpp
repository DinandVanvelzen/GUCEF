/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#include "archive.h"

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "vfspluginLIBARCHIVE_CLibArchiveArchive.h"
#include "vfspluginLIBARCHIVE_CLibArchiveWriter.h"
#include "vfspluginLIBARCHIVE.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< VFS::CArchive, CLibArchiveArchive, MT::CMutex > TArchiveFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CTLibArchiveFormatWriter< CLibArchiveWriter::FORMAT_TAR >, MT::CMutex > TTarWriterFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CTLibArchiveFormatWriter< CLibArchiveWriter::FORMAT_TAR_GZIP >, MT::CMutex > TTarGzipWriterFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CTLibArchiveFormatWriter< CLibArchiveWriter::FORMAT_ZIP >, MT::CMutex > TZipWriterFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CTLibArchiveFormatWriter< CLibArchiveWriter::FORMAT_7ZIP >, MT::CMutex > T7ZipWriterFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CTLibArchiveFormatWriter< CLibArchiveWriter::FORMAT_CPIO >, MT::CMutex > TCpioWriterFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The archive types which can be mounted, libarchive detects the actual format from the content */
static const char* ReadableArchiveTypes[] = { "tar", "tgz", "tar.gz", "zip", "pk3", "jar", "7z", "rar", "iso", "cpio", "cab", "lha", "lzh", "xar", "ar" };

// Created when the plugin is loaded and destroyed when it is unloaded:
// as static objects they could be destroyed at process exit while still registered with the VFS
static TArchiveFactory* archiveFactory = GUCEF_NULL;
static TTarWriterFactory* tarWriterFactory = GUCEF_NULL;
static TTarGzipWriterFactory* tarGzipWriterFactory = GUCEF_NULL;
static TZipWriterFactory* zipWriterFactory = GUCEF_NULL;
static T7ZipWriterFactory* sevenZipWriterFactory = GUCEF_NULL;
static TCpioWriterFactory* cpioWriterFactory = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    archiveFactory = GUCEF_NEW TArchiveFactory();
    tarWriterFactory = GUCEF_NEW TTarWriterFactory();
    tarGzipWriterFactory = GUCEF_NEW TTarGzipWriterFactory();
    zipWriterFactory = GUCEF_NEW TZipWriterFactory();
    sevenZipWriterFactory = GUCEF_NEW T7ZipWriterFactory();
    cpioWriterFactory = GUCEF_NEW TCpioWriterFactory();

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 i=0; i<sizeof( ReadableArchiveTypes ) / sizeof( ReadableArchiveTypes[ 0 ] ); ++i )
    {
        vfs.RegisterArchiveFactory( ReadableArchiveTypes[ i ], *archiveFactory );
        vfs.SetArchiveTypeFileExtensions( ReadableArchiveTypes[ i ], CORE::CString( ReadableArchiveTypes[ i ] ).ParseUniqueElements( ';', false ) );
    }

    vfs.RegisterArchiveWriterFactory( "tar", *tarWriterFactory );
    vfs.RegisterArchiveWriterFactory( "tgz", *tarGzipWriterFactory );
    vfs.RegisterArchiveWriterFactory( "tar.gz", *tarGzipWriterFactory );
    vfs.RegisterArchiveWriterFactory( "zip", *zipWriterFactory );
    vfs.RegisterArchiveWriterFactory( "7z", *sevenZipWriterFactory );
    vfs.RegisterArchiveWriterFactory( "cpio", *cpioWriterFactory );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "vfspluginLIBARCHIVE: Loaded, using " + CORE::CString( archive_version_details() ) );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // Note: no static objects of this module other than plain data are used here since at process exit
    // they can already be destroyed by the time the plugin is unloaded
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( UInt32 i=0; i<sizeof( ReadableArchiveTypes ) / sizeof( ReadableArchiveTypes[ 0 ] ); ++i )
        vfs.UnregisterArchiveFactory( ReadableArchiveTypes[ i ] );

    vfs.UnregisterArchiveWriterFactory( "tar" );
    vfs.UnregisterArchiveWriterFactory( "tgz" );
    vfs.UnregisterArchiveWriterFactory( "tar.gz" );
    vfs.UnregisterArchiveWriterFactory( "zip" );
    vfs.UnregisterArchiveWriterFactory( "7z" );
    vfs.UnregisterArchiveWriterFactory( "cpio" );

    GUCEF_DELETE archiveFactory;
    archiveFactory = GUCEF_NULL;
    GUCEF_DELETE tarWriterFactory;
    tarWriterFactory = GUCEF_NULL;
    GUCEF_DELETE tarGzipWriterFactory;
    tarGzipWriterFactory = GUCEF_NULL;
    GUCEF_DELETE zipWriterFactory;
    zipWriterFactory = GUCEF_NULL;
    GUCEF_DELETE sevenZipWriterFactory;
    sevenZipWriterFactory = GUCEF_NULL;
    GUCEF_DELETE cpioWriterFactory;
    cpioWriterFactory = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, LGPL license version 3. Uses libarchive, BSD 2-clause license";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for VFS archives using libarchive: reading tar, zip, 7z, rar, iso, cpio and more, writing tar, tgz, zip, 7z and cpio";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefVFS";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

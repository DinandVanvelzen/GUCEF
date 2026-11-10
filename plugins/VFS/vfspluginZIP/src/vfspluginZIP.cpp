/*
 *  vfspluginZIP: Generic GUCEF VFS plugin for ZIP archives
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H
#include "vfspluginZIP_CZIPArchive.h"
#define GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_ZIP_CZIPARCHIVE_H ? */

#ifndef VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H
#include "vfspluginZIP_CZipArchiveWriter.h"
#define VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H
#endif /* VFSPLUGIN_ZIP_CZIPARCHIVEWRITER_H ? */

#include "vfspluginZIP.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ZIP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< VFS::CArchive, CZIPArchive, MT::CMutex > TZIPArchiveFactory;
typedef CORE::CTFactory< VFS::CIArchiveWriter, CZipArchiveWriter, MT::CMutex > TZipArchiveWriterFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

// Created when the plugin is loaded and destroyed when it is unloaded:
// as static objects they could be destroyed at process exit while still registered with the VFS
static TZIPArchiveFactory* zipArchiveFactory = GUCEF_NULL;
static TZipArchiveWriterFactory* zipArchiveWriterFactory = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == zipArchiveFactory )
        zipArchiveFactory = GUCEF_NEW TZIPArchiveFactory();
    if ( GUCEF_NULL == zipArchiveWriterFactory )
        zipArchiveWriterFactory = GUCEF_NEW TZipArchiveWriterFactory();

    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveFactory( CZIPArchive::ZipArchiveTypeName, *zipArchiveFactory );
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveFactory( "pk3", *zipArchiveFactory );
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveFactory( "jar", *zipArchiveFactory );

    // the files each type opens, for applications that offer every archive the VFS can open
    VFS::CVfsGlobal::Instance()->GetVfs().SetArchiveTypeFileExtensions( CZIPArchive::ZipArchiveTypeName, CORE::CString( "zip" ).ParseUniqueElements( ';', false ) );
    VFS::CVfsGlobal::Instance()->GetVfs().SetArchiveTypeFileExtensions( "pk3", CORE::CString( "pk3" ).ParseUniqueElements( ';', false ) );
    VFS::CVfsGlobal::Instance()->GetVfs().SetArchiveTypeFileExtensions( "jar", CORE::CString( "jar" ).ParseUniqueElements( ';', false ) );
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveWriterFactory( CZIPArchive::ZipArchiveTypeName, *zipArchiveWriterFactory );
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveWriterFactory( "pk3", *zipArchiveWriterFactory );
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveWriterFactory( "jar", *zipArchiveWriterFactory );

    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // Note: no static objects of this module are used here since at process exit they can already be destroyed
    // by the time the plugin is unloaded, so the type names are spelled out
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveFactory( "zip" );
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveFactory( "pk3" );
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveFactory( "jar" );
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveWriterFactory( "zip" );
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveWriterFactory( "pk3" );
    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveWriterFactory( "jar" );

    GUCEF_DELETE zipArchiveFactory;
    zipArchiveFactory = GUCEF_NULL;
    GUCEF_DELETE zipArchiveWriterFactory;
    zipArchiveWriterFactory = GUCEF_NULL;
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
    
    return "Copyright (C) Dinand Vanvelzen, LGPL license version 3";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for VFS \"ZIP\" archives using zziplib and zlib";
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

}; /* namespace ZIP */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

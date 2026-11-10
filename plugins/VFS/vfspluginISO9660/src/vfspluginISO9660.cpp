/*
 *  vfspluginISO9660: Generic GUCEF VFS plugin for ISO 9660 disc images
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

#ifndef GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H
#include "vfspluginISO9660_CISO9660Archive.h"
#define GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_ISO9660_CISO9660ARCHIVE_H ? */

#include "vfspluginISO9660.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace ISO9660 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< VFS::CArchive, CISO9660Archive, MT::CMutex > TISO9660ArchiveFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

// Created when the plugin is loaded and destroyed when it is unloaded:
// as a static object it could be destroyed at process exit while still registered with the VFS
static TISO9660ArchiveFactory* archiveFactory = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on VFS plugin ISO9660" );

    // Note that the type name is a literal and not a static object of this module: such an object can
    // already be destroyed by the time the plugin is unloaded at process exit.
    // This plugin only registers "iso9660", the "iso" type name is left to the plugins that wrap other libraries.
    archiveFactory = GUCEF_NEW TISO9660ArchiveFactory();
    VFS::CVfsGlobal::Instance()->GetVfs().RegisterArchiveFactory( "iso9660", *archiveFactory );
    VFS::CVfsGlobal::Instance()->GetVfs().SetArchiveTypeFileExtensions( "iso9660", CORE::CString( "iso" ).ParseUniqueElements( ';', false ) );

    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on VFS plugin ISO9660" );

    VFS::CVfsGlobal::Instance()->GetVfs().UnregisterArchiveFactory( "iso9660" );

    GUCEF_DELETE archiveFactory;
    archiveFactory = GUCEF_NULL;
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

    return "Copyright (C) Dinand Vanvelzen, LGPL license version 2.1";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Generic GUCEF plugin for VFS ISO 9660 disc images with Joliet and Rock Ridge support";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefMT,gucefCORE,gucefVFS";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ISO9660 */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

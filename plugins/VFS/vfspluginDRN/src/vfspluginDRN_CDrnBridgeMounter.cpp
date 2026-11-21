/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCENAME_H
#include "gucefDRN_CDRNResourceName.h"
#define GUCEF_DRN_CDRNRESOURCENAME_H
#endif /* GUCEF_DRN_CDRNRESOURCENAME_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#include "vfspluginDRN_CDrnBridgeGlobal.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEGLOBAL_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#include "vfspluginDRN_CDrnBridgeArchive.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEARCHIVE_H ? */

#include "vfspluginDRN_CDrnBridgeMounter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeMounter::CDrnBridgeMounter( VFS::CVFS& vfs )
    : m_vfs( vfs )
    , m_lock()
    , m_mounted( false )
    , m_mountPoint()
    , m_archiveName()
    , m_address()
    , m_archive( GUCEF_NULL )
{GUCEF_TRACE;

    // the archive type is made known to the VFS by the global of the plugin: an application that links the plugin needs no other call
    CDrnBridgeGlobal::Instance();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMounter::~CDrnBridgeMounter()
{GUCEF_TRACE;

    Unmount();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMounter::MakeAddress( const CORE::CString& sessionName, const CORE::CString& member, const CORE::CString& share )
{GUCEF_TRACE;

    const CORE::CString address = "drn://" + sessionName + "/" + member + "/" + share;

    // what the archive makes of the address has to be the parts that went into it
    CORE::CString parsedSession;
    CORE::CString parsedMember;
    CORE::CString parsedShare;
    CORE::CString why;
    if ( !CDrnBridgeArchive::ParseDrnAddress( address, parsedSession, parsedMember, parsedShare, why ) )
        return CORE::CString::Empty;
    if ( parsedSession != sessionName || parsedMember != member || parsedShare != share )
        return CORE::CString::Empty;
    if ( !DRN::CDRNResourceName::IsValidProviderName( share ) )
        return CORE::CString::Empty;
    return address;
}

/*-------------------------------------------------------------------------*/

VFS::CVFS::TArchivePtr
CDrnBridgeMounter::FindMountedArchive( void ) const
{GUCEF_TRACE;

    if ( !m_mounted )
        return VFS::CVFS::TArchivePtr();

    // the name is what the VFS knows the archive by; when it is another archive than the one that was mounted it is not ours
    VFS::CVFS::TArchivePtr archive = m_vfs.GetMountedArchive( m_archiveName );
    if ( archive.IsNULL() || archive.GetPointerAlways() != m_archive )
        return VFS::CVFS::TArchivePtr();
    return archive;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMountResult
CDrnBridgeMounter::Mount( const CORE::CString& sessionName ,
                          const CORE::CString& member      ,
                          const CORE::CString& share       ,
                          const CORE::CString& mountPoint  ,
                          const CDrnBridgeSettings* settings ,
                          const CORE::CString& archiveName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !FindMountedArchive().IsNULL() )
        return CDrnBridgeMountResult::Refused( "this mounter has mounted \"" + m_mountPoint + "\" already: unmount that first" );
    m_mounted = false;

    // the address
    const CORE::CString address = MakeAddress( sessionName, member, share );
    if ( address.IsNULLOrEmpty() )
    {
        const CORE::CString why = "\"drn://" + sessionName + "/" + member + "/" + share + "\" is not a usable address: the session name, the member (a node id or sequencer) and the share name are needed, "
                                  "none of them may hold a separator, and the share name must be one that a node can share under";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: Cannot mount: " << why );
        return CDrnBridgeMountResult::Refused( why );
    }

    // the mount point: not the root, not one that is in use
    CORE::CString trimmed = mountPoint;
    trimmed = trimmed.ReplaceChar( '\\', '/' );
    if ( trimmed.IsNULLOrEmpty() || trimmed.ParseElements( '/', false ).empty() )
    {
        const CORE::CString why = "the mount point is empty or the root: a share is mounted at a path of its own";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: Cannot mount \"" << address << "\": " << why );
        return CDrnBridgeMountResult::Refused( why );
    }
    if ( m_vfs.IsMountedArchive( mountPoint ) )
    {
        const CORE::CString why = "the mount point \"" + mountPoint + "\" is in use by another archive: mounting there would overlay it";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: Cannot mount \"" << address << "\": " << why );
        return CDrnBridgeMountResult::Refused( why );
    }

    // the archive type: known to the VFS when the plugin's global exists
    VFS::CVFS::TAbstractArchiveFactory::TKeySet types;
    m_vfs.GetListOfSupportedArchiveTypes( types );
    if ( types.find( CDrnBridgeGlobal::ArchiveTypeName ) == types.end() )
    {
        const CORE::CString why = "the VFS does not know the archive type \"" + CDrnBridgeGlobal::ArchiveTypeName + "\": the plugin is not loaded";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: Cannot mount \"" << address << "\": " << why );
        return CDrnBridgeMountResult::Refused( why );
    }

    // the name the VFS knows the archive by is what unmounting goes by, so it has to be a name of its own
    const CORE::CString name = archiveName.IsNULLOrEmpty() ? "drn:" + mountPoint : archiveName;
    if ( !m_vfs.GetMountedArchive( name ).IsNULL() )
    {
        const CORE::CString why = "an archive named \"" + name + "\" is mounted already";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: Cannot mount \"" << address << "\": " << why );
        return CDrnBridgeMountResult::Refused( why );
    }

    // the settings of the archive, in the form a configuration holds them
    CORE::CDataNode config;
    config.SetAttribute( "archiveName", name );
    config.SetAttribute( "archiveType", CDrnBridgeGlobal::ArchiveTypeName );
    config.SetAttribute( "actualArchivePath", address );
    config.SetAttribute( "path", address );
    config.SetAttribute( "mountPath", mountPoint );
    config.SetAttribute( "writeable", false );
    if ( GUCEF_NULL != settings )
        settings->SaveToArchiveConfig( config );
    VFS::CArchiveSettings archiveSettings;
    archiveSettings.LoadConfig( config );
    archiveSettings.SetActualArchivePath( address );

    if ( !m_vfs.MountArchive( archiveSettings ) )
    {
        const CORE::CString why = "the VFS refused the mount of \"" + address + "\" at \"" + mountPoint + "\"";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DrnBridgeMounter: " << why );
        return CDrnBridgeMountResult::Refused( why );
    }

    VFS::CVFS::TArchivePtr archive = m_vfs.GetMountedArchive( name );
    if ( archive.IsNULL() )
        return CDrnBridgeMountResult::Refused( "the archive \"" + name + "\" is not in the VFS after it was mounted" );

    m_mounted = true;
    m_mountPoint = mountPoint;
    m_archiveName = name;
    m_address = address;
    m_archive = archive.GetPointerAlways();

    CDrnBridgeMountResult result;
    result.mounted = true;
    result.mountPoint = mountPoint;
    result.archiveName = name;
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMounter::Unmount( void )
{GUCEF_TRACE;

    CORE::CString name;
    {
        MT::CScopeMutex lock( m_lock );
        const bool stillThere = !FindMountedArchive().IsNULL();
        name = m_archiveName;
        m_mounted = false;
        m_mountPoint.Clear();
        m_archiveName.Clear();
        m_address.Clear();
        m_archive = GUCEF_NULL;
        if ( !stillThere )
            return false;
    }

    // outside the lock: the VFS notifies observers of the unmount
    return m_vfs.UnmountArchiveByName( name );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMounter::IsMounted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !FindMountedArchive().IsNULL();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMounter::EState
CDrnBridgeMounter::GetState( void ) const
{GUCEF_TRACE;

    VFS::CVFS::TArchivePtr archive;
    {
        MT::CScopeMutex lock( m_lock );
        archive = FindMountedArchive();
    }
    if ( archive.IsNULL() )
        return STATE_NOT_MOUNTED;
    return archive->IsConnected() ? STATE_ONLINE : STATE_OFFLINE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMounter::GetMountPoint( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mountPoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMounter::GetArchiveName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_archiveName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMounter::GetAddress( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_address;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#include "vfspluginDRN_CDrnBridgeSettings.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H
#include "vfspluginDRN_CDrnBridgeMountResult.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTRESULT_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Mounts one share of a DRN session into a VFS, for an application that has (the name of) a session: the form of
 *  "drn://<session name>/<member>/<share>" without writing the address or the archive settings by hand, with the checks
 *  that the VFS does not make itself.
 *
 *  The session is found by its name in the DRN session registry (CDRNGlobal::GetSessionRegistry()) when it is used, so the
 *  session need not exist at the time of the mount, and it may be replaced by another of the same name afterwards (see the README of
 *  the plugin, late session binding): the mount stays and is not reachable meanwhile. The application, or the service kit, registers the
 *  session. The mounter does not register sessions.
 *
 *  A mount is refused, with a reason, when the archive type "drn" is unknown to the VFS (the plugin is not loaded), the address is not a usable
 *  one, the mount point is empty or the root, the mount point is in use by an archive already (the VFS resolves mounts in order, so a silent overlay is
 *  the risk this check removes), the archive name is in use, or the VFS refuses the mount.
 *
 *  The mounter unmounts what it mounted in its destructor, and only what it mounted. It makes sure the plugin's global exists
 *  (CDrnBridgeGlobal::Instance()), which registers the archive type, so an application that links the plugin needs no extra call.
 *
 *  The methods may be called from any thread. The VFS must outlive the mounter.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeMounter
{
    public:

    enum EState
    {
        STATE_NOT_MOUNTED = 0   ,   /**< nothing is mounted (never was, unmounted, or the archive was unmounted by someone else) */
        STATE_ONLINE            ,   /**< mounted, and the share's serving member can be reached now */
        STATE_OFFLINE               /**< mounted, the share cannot be reached now (no session, no link): the mount answers from what it has cached */
    };

    explicit CDrnBridgeMounter( VFS::CVFS& vfs );

    /** Unmounts what this mounter mounted */
    ~CDrnBridgeMounter();

    /**
     *  @param sessionName  the name the session has in the DRN session registry
     *  @param member       a node id of a member of the session, or "sequencer" for the host of a client-server session
     *  @param share        the name the serving member shares it under
     *  @param mountPoint   the VFS path to mount at
     *  @param settings     the settings of the archive, NULL for the defaults
     *  @param archiveName  the name of the archive in the VFS (what unmounting goes by), empty for "drn:" plus the mount point
     */
    CDrnBridgeMountResult Mount( const CORE::CString& sessionName          ,
                                 const CORE::CString& member               ,
                                 const CORE::CString& share                ,
                                 const CORE::CString& mountPoint           ,
                                 const CDrnBridgeSettings* settings = GUCEF_NULL ,
                                 const CORE::CString& archiveName = CORE::CString::Empty );

    /** Unmounts the share; false when nothing was mounted by this mounter, also when it is unmounted by someone else already. Safe to call twice */
    bool Unmount( void );

    /** The share this mounter mounted is mounted still */
    bool IsMounted( void ) const;

    EState GetState( void ) const;

    /** The mount point as it was given, empty when nothing is mounted */
    CORE::CString GetMountPoint( void ) const;

    /** The name the archive is mounted under, empty when nothing is mounted */
    CORE::CString GetArchiveName( void ) const;

    /** The address that was mounted, empty when nothing is mounted */
    CORE::CString GetAddress( void ) const;

    /** The address of a share: drn://<session name>/<member>/<share>; empty when one of the parts cannot be part of such an address */
    static CORE::CString MakeAddress( const CORE::CString& sessionName, const CORE::CString& member, const CORE::CString& share );

    private:

    CDrnBridgeMounter( const CDrnBridgeMounter& src );                  /**< not implemented */
    CDrnBridgeMounter& operator=( const CDrnBridgeMounter& src );       /**< not implemented */

    /** The archive this mounter mounted, as the VFS has it now; a NULL pointer when it is not there (any more) or the name is another archive's */
    VFS::CVFS::TArchivePtr FindMountedArchive( void ) const;

    VFS::CVFS& m_vfs;
    mutable MT::CMutex m_lock;
    bool m_mounted;
    CORE::CString m_mountPoint;
    CORE::CString m_archiveName;
    CORE::CString m_address;
    const VFS::CArchive* m_archive;     /**< identity of the archive that was mounted, never used to call */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMOUNTER_H ? */

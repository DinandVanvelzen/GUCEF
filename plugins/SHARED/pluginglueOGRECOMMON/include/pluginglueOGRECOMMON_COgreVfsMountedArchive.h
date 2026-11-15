/*
 *  pluginglueOGRECOMMON: Header only adapters between GUCEF and the Ogre 1.x API, compiled into the plugins that use them
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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreVfsArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRECOMMON {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An Ogre archive over a file of the VFS that is itself an archive, for resource locations that name an archive
 *  file ("/data/models.zip") as the Ogre ZIP archive does. The name of the archive is the VFS path of the file.
 *
 *  When Ogre loads the archive, the file is mounted in the VFS as an archive of the type that handles its file
 *  extension (a plugin of gucefVFS, for ZIP files the ZIP archive plugin) at a mount point of its own, from the
 *  resource of the file or, for archive types that do not read from a resource, from the file the VFS has for the path, and the
 *  archive then answers like the archive over a directory of the VFS does for that mount point. When Ogre unloads
 *  the archive the mount is taken out of the VFS again. Loading fails with an Ogre exception when there is no such
 *  file or no archive type for it.
 */
class COgreVfsMountedArchive : public COgreVfsArchive
{
    public:

    COgreVfsMountedArchive( const Ogre::String& name     ,
                            const Ogre::String& archType ,
                            VFS::CVFS& vfs               )
        : COgreVfsArchive( name, archType, vfs )
        , m_mountName()
        , m_resource()
    {GUCEF_TRACE;
    }

    virtual ~COgreVfsMountedArchive() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        Unmount();
    }

    virtual void load( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Unmount();

        VFS::CVFS& vfs = GetVfs();
        const CORE::CString filePath = CORE::CString( "/" ) + COgreArchiveListing::Normalize( mName ).c_str();
        m_resource = vfs.GetFile( filePath, "rb" );
        if ( m_resource.IsNULL() )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_FILE_NOT_FOUND, "The file " + mName + " is not in the VFS", "COgreVfsMountedArchive::load" );
        }

        CORE::CString archiveType;
        if ( !vfs.TryGetArchiveTypeForFile( filePath, archiveType ) )
            archiveType = "zip";

        const CORE::CString mountPoint = CORE::CString( "/ogre-archives/" ) + CORE::PointerToString( this );
        const CORE::CString mountName = CORE::CString( "OgreArchive:" ) + filePath + ":" + CORE::PointerToString( this );
        // archive types that cannot read from a resource (the ZIP plugin) are mounted from the file the VFS has for the path
        if ( !vfs.MountArchive( mountName, m_resource, archiveType, mountPoint, false ) &&
             !vfs.MountArchive( mountName, filePath, archiveType, mountPoint, false, false ) )
        {
            m_resource.Unlink();
            OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "The file " + mName + " could not be mounted as an archive of type " +
                Ogre::String( archiveType.C_String() ), "COgreVfsMountedArchive::load" );
        }

        m_mountName = mountName;
        SetRootPath( mountPoint.C_String() );
        COgreVfsArchive::load();
    }

    virtual void unload( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        COgreVfsArchive::unload();
        Unmount();
    }

    private:

    void Unmount( void )
    {GUCEF_TRACE;

        if ( !m_mountName.IsNULLOrEmpty() )
        {
            GetVfs().UnmountArchiveByName( m_mountName );
            m_mountName.Clear();
        }
        m_resource.Unlink();
    }

    private:

    CORE::CString m_mountName;
    VFS::TBasicVfsResourcePtr m_resource;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRECOMMON */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H ? */

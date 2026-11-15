/*
 *  OgrePlugin_GucefVFS: Ogre plugin that registers the archive factories which read through gucefVFS
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

#ifndef GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H
#define GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgrePlugin.h"

#ifndef GUCEF_OGREPLUGIN_GUCEFVFS_MACROS_H
#include "OgrePlugin_GucefVFS_macros.h"
#define GUCEF_OGREPLUGIN_GUCEFVFS_MACROS_H
#endif /* GUCEF_OGREPLUGIN_GUCEFVFS_MACROS_H ? */

namespace GUCEF { namespace PLUGINGLUE { namespace OGRECOMMON {
class COgreVfsArchiveFactory;
class COgreIOAccessArchiveFactory;
} } }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Ogre plugin that registers the archive factories which read through gucefVFS:
 *
 *      "GucefVFS"        the resource location is a directory of the VFS (the type that resource groups should name)
 *      "GucefIOAccess"   named resources that the application holds in memory as CIOAccess objects
 *      "FileSystem"      as "GucefVFS", under the name existing resource configurations use
 *      "Zip"             a resource location that is an archive file of the VFS, mounted while Ogre has it loaded
 *
 *  Ogre itself registers no archive factories that read from disk when it is built with
 *  GUCEF_OGRE_NO_NATIVE_ARCHIVES, so a "FileSystem" or "Zip" resource location reads through the VFS. If the type is
 *  taken already by another plugin the factory is not registered and a warning is logged.
 */
class GUCEF_OGREPLUGIN_GUCEFVFS_EXPORT_CPP CGucefVfsOgrePlugin : public Ogre::Plugin
{
    public:

    static const char* const VfsArchiveType;        /**< "GucefVFS" */
    static const char* const IOAccessArchiveType;   /**< "GucefIOAccess" */
    static const char* const FileSystemArchiveType; /**< "FileSystem" */
    static const char* const ZipArchiveType;        /**< "Zip" */

    CGucefVfsOgrePlugin( void );

    virtual ~CGucefVfsOgrePlugin() GUCEF_VIRTUAL_OVERRIDE;

    virtual const Ogre::String& getName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void install( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void initialise( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void shutdown( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void uninstall( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGucefVfsOgrePlugin( const CGucefVfsOgrePlugin& src );              /**< not implemented */
    CGucefVfsOgrePlugin& operator=( const CGucefVfsOgrePlugin& src );   /**< not implemented */

    void AddFactory( Ogre::ArchiveFactory* factory, bool onlyIfFree );

    private:

    enum { MaxFactories = 4 };

    Ogre::ArchiveFactory* m_factories[ MaxFactories ];
    bool m_registered[ MaxFactories ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGREPLUGIN_GUCEFVFS_CGUCEFVFSOGREPLUGIN_H ? */

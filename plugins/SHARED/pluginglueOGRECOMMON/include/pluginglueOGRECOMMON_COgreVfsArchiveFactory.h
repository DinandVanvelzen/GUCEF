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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreArchiveFactory.h"

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreVfsArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H
#include "pluginglueOGRECOMMON_COgreVfsMountedArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSMOUNTEDARCHIVE_H ? */

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
 *  Makes the archives of Ogre that read through the VFS. One factory has one archive type name:
 *
 *      "GucefVFS"    the location is a directory of the VFS
 *      "FileSystem"  the same, under the name of the Ogre archive type that resource configuration already names
 *      "Zip"         the location is an archive file of the VFS, which is mounted for the time Ogre has it loaded
 */
class COgreVfsArchiveFactory : public Ogre::ArchiveFactory
{
    public:

    enum EArchiveKind
    {
        ARCHIVEKIND_DIRECTORY ,
        ARCHIVEKIND_ARCHIVEFILE
    };
    typedef enum EArchiveKind TArchiveKind;

    COgreVfsArchiveFactory( VFS::CVFS& vfs, const Ogre::String& typeName, TArchiveKind kind )
        : Ogre::ArchiveFactory()
        , m_vfs( vfs )
        , m_typeName( typeName )
        , m_kind( kind )
    {GUCEF_TRACE;
    }

    virtual ~COgreVfsArchiveFactory() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual const Ogre::String& getType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_typeName;
    }

    using Ogre::ArchiveFactory::createInstance;

    /** The archives are read only whatever is asked: the VFS archives that Ogre loads are only read */
    virtual Ogre::Archive* createInstance( const Ogre::String& name, bool readOnly ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( ARCHIVEKIND_ARCHIVEFILE == m_kind )
            return new COgreVfsMountedArchive( name, m_typeName, m_vfs );
        return new COgreVfsArchive( name, m_typeName, m_vfs );
    }

    virtual void destroyInstance( Ogre::Archive* archive ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        delete archive;
    }

    private:

    COgreVfsArchiveFactory( const COgreVfsArchiveFactory& src );              /**< not implemented */
    COgreVfsArchiveFactory& operator=( const COgreVfsArchiveFactory& src );   /**< not implemented */

    private:

    VFS::CVFS& m_vfs;
    Ogre::String m_typeName;
    TArchiveKind m_kind;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H ? */

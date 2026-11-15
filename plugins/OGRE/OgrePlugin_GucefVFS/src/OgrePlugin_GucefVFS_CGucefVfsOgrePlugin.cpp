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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreArchiveManager.h"
#include "OgreArchiveFactory.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H
#include "pluginglueOGRECOMMON_COgreVfsArchiveFactory.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVEFACTORY_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H
#include "pluginglueOGRECOMMON_COgreIOAccessArchiveFactory.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H ? */

#include "OgrePlugin_GucefVFS_CGucefVfsOgrePlugin.h"

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CGucefVfsOgrePlugin::VfsArchiveType = "GucefVFS";
const char* const CGucefVfsOgrePlugin::IOAccessArchiveType = "GucefIOAccess";
const char* const CGucefVfsOgrePlugin::FileSystemArchiveType = "FileSystem";
const char* const CGucefVfsOgrePlugin::ZipArchiveType = "Zip";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefVfsOgrePlugin::CGucefVfsOgrePlugin( void )
    : Ogre::Plugin()
{GUCEF_TRACE;

    for ( int i=0; i<MaxFactories; ++i )
    {
        m_factories[ i ] = GUCEF_NULL;
        m_registered[ i ] = false;
    }
}

/*-------------------------------------------------------------------------*/

CGucefVfsOgrePlugin::~CGucefVfsOgrePlugin()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const Ogre::String&
CGucefVfsOgrePlugin::getName( void ) const
{GUCEF_TRACE;

    static const Ogre::String name = "GucefVFS Archives";
    return name;
}

/*-------------------------------------------------------------------------*/

void
CGucefVfsOgrePlugin::AddFactory( Ogre::ArchiveFactory* factory, bool onlyIfFree )
{GUCEF_TRACE;

    int slot = 0;
    while ( slot < MaxFactories && GUCEF_NULL != m_factories[ slot ] )
        ++slot;
    if ( slot >= MaxFactories )
    {
        delete factory;
        return;
    }
    m_factories[ slot ] = factory;

    Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
    if ( onlyIfFree && archiveManager.hasArchiveFactory( factory->getType() ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OgrePlugin_GucefVFS: An Ogre archive factory for the type "
            << factory->getType() << " is registered already, that one stays" );
        return;
    }

    archiveManager.addArchiveFactory( factory );
    m_registered[ slot ] = true;
}

/*-------------------------------------------------------------------------*/

void
CGucefVfsOgrePlugin::install( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    AddFactory( new OGRECOMMON::COgreVfsArchiveFactory( vfs, VfsArchiveType, OGRECOMMON::COgreVfsArchiveFactory::ARCHIVEKIND_DIRECTORY ), false );
    AddFactory( new OGRECOMMON::COgreIOAccessArchiveFactory( IOAccessArchiveType ), false );

    // the names of the archive types that existing resource configuration uses: they read through the VFS as well
    AddFactory( new OGRECOMMON::COgreVfsArchiveFactory( vfs, FileSystemArchiveType, OGRECOMMON::COgreVfsArchiveFactory::ARCHIVEKIND_DIRECTORY ), true );
    AddFactory( new OGRECOMMON::COgreVfsArchiveFactory( vfs, ZipArchiveType, OGRECOMMON::COgreVfsArchiveFactory::ARCHIVEKIND_ARCHIVEFILE ), true );
}

/*-------------------------------------------------------------------------*/

void
CGucefVfsOgrePlugin::initialise( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGucefVfsOgrePlugin::shutdown( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGucefVfsOgrePlugin::uninstall( void )
{GUCEF_TRACE;

    Ogre::ArchiveManager* archiveManager = Ogre::ArchiveManager::getSingletonPtr();

    // Ogre keeps the archives of a resource location loaded when the location or its group goes, until the archive
    // manager is destroyed. That is after this plugin is, and the archives are code of this plugin: they go first
    if ( GUCEF_NULL != archiveManager )
    {
        GUCEF::vector< Ogre::String > leftOver;
        Ogre::ArchiveManager::ArchiveMapIterator archives = archiveManager->getArchiveIterator();
        while ( archives.hasMoreElements() )
        {
            const Ogre::String& type = archives.peekNextValue()->getType();
            for ( int i=0; i<MaxFactories; ++i )
            {
                if ( m_registered[ i ] && GUCEF_NULL != m_factories[ i ] && type == m_factories[ i ]->getType() )
                {
                    leftOver.push_back( archives.peekNextKey() );
                    break;
                }
            }
            archives.moveNext();
        }
        for ( size_t i=0; i<leftOver.size(); ++i )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OgrePlugin_GucefVFS: unloading the archive " << leftOver[ i ] << " that Ogre still has loaded" );
            archiveManager->unload( leftOver[ i ] );
        }
    }

    for ( int i=0; i<MaxFactories; ++i )
    {
        if ( GUCEF_NULL != m_factories[ i ] )
        {
            if ( m_registered[ i ] && GUCEF_NULL != archiveManager )
                archiveManager->removeArchiveFactory( m_factories[ i ] );
            delete m_factories[ i ];
            m_factories[ i ] = GUCEF_NULL;
            m_registered[ i ] = false;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

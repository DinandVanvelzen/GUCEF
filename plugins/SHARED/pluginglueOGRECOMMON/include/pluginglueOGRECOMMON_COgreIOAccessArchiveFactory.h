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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreArchiveFactory.h"

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreIOAccessArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H ? */

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
 *  Makes the archives of named in-memory resources. The application gets hold of an archive that Ogre made for a
 *  resource location with
 *
 *      COgreIOAccessArchive::FromArchive( Ogre::ArchiveManager::getSingleton().load( name, type, true ), type )
 */
class COgreIOAccessArchiveFactory : public Ogre::ArchiveFactory
{
    public:

    explicit COgreIOAccessArchiveFactory( const Ogre::String& typeName )
        : Ogre::ArchiveFactory()
        , m_typeName( typeName )
    {GUCEF_TRACE;
    }

    virtual ~COgreIOAccessArchiveFactory() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual const Ogre::String& getType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_typeName;
    }

    using Ogre::ArchiveFactory::createInstance;

    virtual Ogre::Archive* createInstance( const Ogre::String& name, bool readOnly ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return new COgreIOAccessArchive( name, m_typeName );
    }

    virtual void destroyInstance( Ogre::Archive* archive ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        delete archive;
    }

    private:

    COgreIOAccessArchiveFactory( const COgreIOAccessArchiveFactory& src );              /**< not implemented */
    COgreIOAccessArchiveFactory& operator=( const COgreIOAccessArchiveFactory& src );   /**< not implemented */

    private:

    Ogre::String m_typeName;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVEFACTORY_H ? */

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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <time.h>

#include "OgreArchive.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H
#include "pluginglueOGRECOMMON_COgreArchiveListing.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#include "pluginglueOGRECOMMON_CIOAccessToOgreDataStream.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H ? */

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
 *  An Ogre archive of named resources that the application holds in memory, as CIOAccess objects: generated
 *  data, data that was received from somewhere, data that another library decoded. The application registers the
 *  resources with AddResource() and Ogre finds them by name in the resource group that has this archive as a
 *  location.
 *
 *  The archive shares the IO access objects with the application, so they stay valid for as long as the archive or
 *  an open stream needs them. A resource name can have '/' in it, the directories of the archive follow from the
 *  names. Opening a resource moves the IO access object to its start; the streams that are open on one resource
 *  share its position, so a resource is read by one stream at a time.
 */
class COgreIOAccessArchive : public Ogre::Archive
{
    public:

    COgreIOAccessArchive( const Ogre::String& name     ,
                          const Ogre::String& archType )
        : Ogre::Archive( name, archType )
        , m_lock()
        , m_resources()
        , m_listing()
        , m_listingValid( false )
    {GUCEF_TRACE;
        mReadOnly = true;
    }

    virtual ~COgreIOAccessArchive() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    /** The archive of an Ogre archive pointer, NULL when it is not an archive of this class (by its type name) */
    static COgreIOAccessArchive* FromArchive( Ogre::Archive* archive, const Ogre::String& typeName )
    {GUCEF_TRACE;

        if ( GUCEF_NULL == archive || archive->getType() != typeName )
            return GUCEF_NULL;
        return static_cast< COgreIOAccessArchive* >( archive );
    }

    /**
     *  Registers a resource under a name, replacing the resource of that name.
     *  The modification time is what getModifiedTime() answers, the time of the call when it is 0
     */
    bool AddResource( const Ogre::String& filename, const CORE::IOAccessPtr& access, time_t modified = 0 )
    {GUCEF_TRACE;

        const Ogre::String name = COgreArchiveListing::Normalize( filename );
        if ( name.empty() || access.IsNULL() || COgreArchiveListing::HasParentReference( filename ) )
            return false;

        SResource resource;
        resource.access = access;
        resource.modified = 0 == modified ? time( GUCEF_NULL ) : modified;

        MT::CScopeMutex lock( m_lock );
        m_resources[ name ] = resource;
        m_listingValid = false;
        return true;
    }

    bool RemoveResource( const Ogre::String& filename )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        const bool removed = 0 != m_resources.erase( COgreArchiveListing::Normalize( filename ) );
        m_listingValid = m_listingValid && !removed;
        return removed;
    }

    void ClearResources( void )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        m_resources.clear();
        m_listingValid = false;
    }

    UInt32 GetResourceCount( void ) const
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        return (UInt32) m_resources.size();
    }

    virtual bool isCaseSensitive( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual void load( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        // the resources are added by the application at any time, nothing is read
    }

    virtual void unload( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        // the resources stay with the application, the archive only shares them
    }

    virtual Ogre::DataStreamPtr open( const Ogre::String& filename, bool readOnly = true ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::IOAccessPtr access;
        {
            MT::CScopeMutex lock( m_lock );
            TResourceMap::const_iterator i = m_resources.find( COgreArchiveListing::Normalize( filename ) );
            if ( i == m_resources.end() )
                return Ogre::DataStreamPtr();
            access = (*i).second.access;
        }

        if ( 0 != access->Setpos( 0 ) )
            return Ogre::DataStreamPtr();
        return Ogre::DataStreamPtr( new CIOAccessToOgreDataStream( access, filename ) );
    }

    virtual Ogre::StringVectorPtr list( bool recursive = true, bool dirs = false ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Ogre::StringVectorPtr result( new Ogre::StringVector() );
        MT::CScopeMutex lock( m_lock );
        EnsureListing();
        m_listing.List( recursive, dirs, *result );
        return result;
    }

    virtual Ogre::FileInfoListPtr listFileInfo( bool recursive = true, bool dirs = false ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Ogre::FileInfoListPtr result( new Ogre::FileInfoList() );
        MT::CScopeMutex lock( m_lock );
        EnsureListing();
        m_listing.ListInfo( recursive, dirs, this, *result );
        return result;
    }

    virtual Ogre::StringVectorPtr find( const Ogre::String& pattern, bool recursive = true, bool dirs = false ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Ogre::StringVectorPtr result( new Ogre::StringVector() );
        MT::CScopeMutex lock( m_lock );
        EnsureListing();
        m_listing.Find( pattern, recursive, dirs, isCaseSensitive(), *result );
        return result;
    }

    virtual Ogre::FileInfoListPtr findFileInfo( const Ogre::String& pattern, bool recursive = true, bool dirs = false ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        Ogre::FileInfoListPtr result( new Ogre::FileInfoList() );
        MT::CScopeMutex lock( m_lock );
        EnsureListing();
        m_listing.FindInfo( pattern, recursive, dirs, isCaseSensitive(), this, *result );
        return result;
    }

    virtual bool exists( const Ogre::String& filename ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        return m_resources.find( COgreArchiveListing::Normalize( filename ) ) != m_resources.end();
    }

    virtual time_t getModifiedTime( const Ogre::String& filename ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        TResourceMap::const_iterator i = m_resources.find( COgreArchiveListing::Normalize( filename ) );
        return i == m_resources.end() ? 0 : (*i).second.modified;
    }

    private:

    struct SResource
    {
        CORE::IOAccessPtr access;
        time_t modified;
    };
    typedef GUCEF::map< Ogre::String, SResource >   TResourceMap;

    /** Needs the lock */
    void EnsureListing( void ) const
    {GUCEF_TRACE;

        if ( m_listingValid )
            return;

        m_listing.Clear();
        TResourceMap::const_iterator i = m_resources.begin();
        while ( i != m_resources.end() )
        {
            m_listing.AddFile( (*i).first, (*i).second.access->GetSize() );
            ++i;
        }
        m_listingValid = true;
    }

    private:

    COgreIOAccessArchive( const COgreIOAccessArchive& src );              /**< not implemented */
    COgreIOAccessArchive& operator=( const COgreIOAccessArchive& src );   /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    TResourceMap m_resources;
    mutable COgreArchiveListing m_listing;
    mutable bool m_listingValid;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H ? */

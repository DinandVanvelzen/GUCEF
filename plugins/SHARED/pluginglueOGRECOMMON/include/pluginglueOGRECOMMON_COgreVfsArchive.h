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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <time.h>

#include "OgreArchive.h"
#include "OgreException.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H
#include "pluginglueOGRECOMMON_COgreArchiveListing.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H
#include "pluginglueOGRECOMMON_CVfsResourceToOgreDataStream.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H ? */

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
 *  An Ogre archive over a directory of the VFS. The name of the archive is the VFS path of its root, for example
 *  "/assets", which is what a resource group location names.
 *
 *  Every query is relative to that root: open(), exists() and getModifiedTime() all prefix it. The archive is read
 *  only. Opening a file for writing gives the file for reading, as the archive cannot write; create() and remove()
 *  are not supported.
 *
 *  The files and directories below the root are listed once, when the first query needs them, and the queries of
 *  Ogre are answered from that listing. The listing is built again after the VFS mounted or unmounted an archive,
 *  and when InvalidateListing() is called, which an application does after it added files to a writeable mount that
 *  Ogre should see. Names that are not in the listing do not exist for Ogre.
 *
 *  The sizes of the files are asked of the VFS when a query needs them (listFileInfo(), findFileInfo()).
 *  The archive can be used from several threads.
 */
class COgreVfsArchive : public Ogre::Archive ,
                        public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< COgreVfsArchive >  TEventCallback;

    enum { MaxDirectoryDepth = 64 };

    COgreVfsArchive( const Ogre::String& name     ,
                     const Ogre::String& archType ,
                     VFS::CVFS& vfs               )
        : Ogre::Archive( name, archType )
        , CORE::CObserver()
        , m_vfs( vfs )
        , m_root( NormalizeRoot( name ) )
        , m_lock()
        , m_listing()
        , m_listingBuilt( false )
        , m_sizesLoaded( false )
        , m_generation( 0 )
        , m_builtGeneration( 0 )
        , m_buildCount( 0 )
        , m_warnedAboutWrite( false )
    {GUCEF_TRACE;
        mReadOnly = true;
    }

    virtual ~COgreVfsArchive() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        // the VFS does not call us while we are being taken apart
        SignalUpcomingObserverDestruction();
    }

    virtual bool isCaseSensitive( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual void load( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // the root can be mounted later than the archive is made, so nothing is read here
        TEventCallback mountedCallback( this, &COgreVfsArchive::OnVfsMountChanged );
        SubscribeTo( &m_vfs, VFS::CVFS::ArchiveMountedEvent, mountedCallback );
        TEventCallback unmountedCallback( this, &COgreVfsArchive::OnVfsMountChanged );
        SubscribeTo( &m_vfs, VFS::CVFS::ArchiveUnmountedEvent, unmountedCallback );
        InvalidateListing();
    }

    virtual void unload( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        MT::CScopeMutex lock( m_lock );
        m_listing.Clear();
        m_listingBuilt = false;
        m_sizesLoaded = false;
    }

    virtual Ogre::DataStreamPtr open( const Ogre::String& filename, bool readOnly = true ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !readOnly )
            WarnAboutWriteRequest( filename );

        CORE::CString path;
        if ( !ResolvePath( filename, path ) )
            return Ogre::DataStreamPtr();

        // always "rb": the mode "rw" is not a mode of the VFS and the archive is read only
        VFS::TBasicVfsResourcePtr resource = m_vfs.GetFile( path, "rb" );
        if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "COgreVfsArchive: file " << path << " could not be opened" );
            return Ogre::DataStreamPtr();
        }
        return Ogre::DataStreamPtr( new CVfsResourceToOgreDataStream( resource, filename ) );
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
        EnsureSizes();
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
        EnsureSizes();
        m_listing.FindInfo( pattern, recursive, dirs, isCaseSensitive(), this, *result );
        return result;
    }

    virtual bool exists( const Ogre::String& filename ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COgreArchiveListing::HasParentReference( filename ) )
            return false;

        MT::CScopeMutex lock( m_lock );
        EnsureListing();
        return m_listing.HasFile( filename );
    }

    virtual time_t getModifiedTime( const Ogre::String& filename ) GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString path;
        if ( !ResolvePath( filename, path ) )
            return 0;

        return ToTimeT( m_vfs.GetFileModificationTime( path ) );
    }

    /** The listing is built again at the next query */
    void InvalidateListing( void )
    {GUCEF_TRACE;
        ++m_generation;
    }

    /** How many times the VFS was asked for the listing, for tests and diagnostics */
    UInt32 GetListingBuildCount( void ) const
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        return m_buildCount;
    }

    /** The VFS path of the root of the archive, without trailing '/'. Empty for the root of the VFS */
    const Ogre::String& GetRootPath( void ) const
    {
        return m_root;
    }

    /** The VFS path of a file of the archive, false for names that leave the root */
    bool ResolvePath( const Ogre::String& filename, CORE::CString& vfsPath ) const
    {GUCEF_TRACE;

        if ( COgreArchiveListing::HasParentReference( filename ) )
            return false;

        const Ogre::String relative = COgreArchiveListing::Normalize( filename );
        if ( relative.empty() )
            return false;

        vfsPath = ( m_root + "/" + relative ).c_str();
        return true;
    }

    static time_t ToTimeT( const CORE::CDateTime& dt )
    {GUCEF_TRACE;

        // a date time that was never set has no year
        if ( dt.GetDate().GetYear() < 1970 )
            return 0;
        return (time_t) ( dt.ToUTC().ToUnixEpochBasedTicksInMillisecs() / 1000 );
    }

    protected:

    /** For archives that are mounted by the archive itself, which then root the archive at the mount point */
    void SetRootPath( const Ogre::String& vfsRoot )
    {GUCEF_TRACE;

        m_root = NormalizeRoot( vfsRoot );
        InvalidateListing();
    }

    VFS::CVFS& GetVfs( void ) const
    {
        return m_vfs;
    }

    static Ogre::String NormalizeRoot( const Ogre::String& name )
    {GUCEF_TRACE;

        // "/" and "" are the root of the VFS: the root is kept without trailing '/', so a path is root + "/" + file
        const Ogre::String clean = COgreArchiveListing::Normalize( name );
        if ( clean.empty() )
            return Ogre::String();
        return "/" + clean;
    }

    private:

    void OnVfsMountChanged( CORE::CNotifier* notifier   ,
                            const CORE::CEvent& eventId ,
                            CORE::CICloneable* evenData )
    {GUCEF_TRACE;

        // never takes the lock of the listing: the VFS can notify while it holds its own locks, which the
        // listing takes in the opposite order when it asks the VFS for the files
        InvalidateListing();
    }

    void WarnAboutWriteRequest( const Ogre::String& filename ) const
    {GUCEF_TRACE;

        if ( !m_warnedAboutWrite )
        {
            m_warnedAboutWrite = true;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COgreVfsArchive: " << mName << " is read only, file "
                << filename << " is opened for reading" );
        }
    }

    /** Needs the lock of the listing */
    void EnsureListing( void ) const
    {GUCEF_TRACE;

        const UInt32 generation = m_generation.Load();
        if ( m_listingBuilt && generation == m_builtGeneration )
            return;

        m_listing.Clear();
        m_sizesLoaded = false;

        // the directories one level at a time, their names come back relative to the directory that was listed
        CollectDirectories( Ogre::String(), 0 );

        const CORE::CString root = m_root.empty() ? CORE::CString( "/" ) : CORE::CString( m_root.c_str() );
        VFS::CVFS::TStringVector files;
        m_vfs.GetFileList( files, root, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
        VFS::CVFS::TStringVector::const_iterator i = files.begin();
        while ( i != files.end() )
        {
            const Ogre::String rel = RelativeToRoot( (*i).C_String() );
            if ( !rel.empty() )
                m_listing.AddFile( rel, 0 );
            ++i;
        }

        m_listingBuilt = true;
        m_builtGeneration = generation;
        ++m_buildCount;
    }

    /** Needs the lock of the listing. Adds the directories below the directory of the archive, and what is below those */
    void CollectDirectories( const Ogre::String& directory, UInt32 depth ) const
    {GUCEF_TRACE;

        // a bound for archives that list a directory below itself
        if ( depth >= MaxDirectoryDepth )
            return;

        const Ogre::String location = directory.empty() ? m_root : m_root + "/" + directory;
        VFS::CVFS::TStringVector children;
        m_vfs.GetDirList( children, location.empty() ? CORE::CString( "/" ) : CORE::CString( location.c_str() ), false, false, CORE::CString::Empty, GUCEF_UINT32MAX );

        VFS::CVFS::TStringVector::const_iterator i = children.begin();
        while ( i != children.end() )
        {
            // only the name of the directory is of use: archives write it with or without the path that was listed
            Ogre::String name = COgreArchiveListing::Normalize( (*i).C_String() );
            const size_t slash = name.rfind( '/' );
            if ( Ogre::String::npos != slash )
                name.erase( 0, slash + 1 );
            if ( !name.empty() && ".." != name )
            {
                const Ogre::String relative = directory.empty() ? name : directory + "/" + name;
                if ( m_listing.AddDirectory( relative ) )
                    CollectDirectories( relative, depth + 1 );
            }
            ++i;
        }
    }

    /** Needs the lock of the listing */
    void EnsureSizes( void ) const
    {GUCEF_TRACE;

        if ( m_sizesLoaded )
            return;

        COgreArchiveListing::TEntryVector& entries = m_listing.GetEntries();
        COgreArchiveListing::TEntryVector::iterator i = entries.begin();
        while ( i != entries.end() )
        {
            if ( !(*i).isDirectory )
                (*i).size = m_vfs.GetFileSize( ( m_root + "/" + (*i).path ).c_str() );
            ++i;
        }
        m_sizesLoaded = true;
    }

    /** The path the VFS listed, as a path relative to the root of the archive */
    Ogre::String RelativeToRoot( const Ogre::String& listed ) const
    {GUCEF_TRACE;

        const Ogre::String clean = COgreArchiveListing::Normalize( listed );
        const Ogre::String rootClean = m_root.empty() ? Ogre::String() : m_root.substr( 1 );
        if ( !rootClean.empty() && 0 == clean.compare( 0, rootClean.size() + 1, rootClean + "/" ) )
            return clean.substr( rootClean.size() + 1 );
        return clean;
    }

    private:

    COgreVfsArchive( const COgreVfsArchive& src );              /**< not implemented */
    COgreVfsArchive& operator=( const COgreVfsArchive& src );   /**< not implemented */

    private:

    VFS::CVFS& m_vfs;
    Ogre::String m_root;
    mutable MT::CMutex m_lock;
    mutable COgreArchiveListing m_listing;
    mutable bool m_listingBuilt;
    mutable bool m_sizesLoaded;
    MT::CTAtomic< UInt32 > m_generation;
    mutable UInt32 m_builtGeneration;
    mutable UInt32 m_buildCount;
    mutable bool m_warnedAboutWrite;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREVFSARCHIVE_H ? */

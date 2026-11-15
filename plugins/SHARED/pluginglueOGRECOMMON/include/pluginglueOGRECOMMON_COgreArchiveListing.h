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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreArchive.h"
#include "OgreStringVector.h"
#include "OgreString.h"

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#include "pluginglueOGRECOMMON_macros.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H ? */

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
 *  The files and directories of an archive in memory, in the form the Ogre archive interface answers with.
 *
 *  Paths are relative to the root of the archive, use '/' and have no leading or trailing '/'. The archive
 *  classes of this module fill a listing once and answer list(), find() and exists() from it, so one query of
 *  Ogre is not one query of the file system behind the archive.
 *
 *  The queries follow the file system archive of Ogre: a pattern can start with a directory ("sub/dir/*.png")
 *  and the wildcard mask is matched against the file name only; the names that come back are relative to the root
 *  of the archive.
 */
class COgreArchiveListing
{
    public:

    struct SEntry
    {
        Ogre::String path;      /**< relative to the root of the archive, no leading or trailing '/' */
        Ogre::String directory; /**< the path up to and including the last '/', empty for entries in the root */
        Ogre::String name;      /**< the last part of the path */
        UInt64 size;            /**< bytes, 0 for directories and for sizes that are not known (yet) */
        bool isDirectory;
    };
    typedef GUCEF::vector< SEntry >                 TEntryVector;
    typedef GUCEF::map< Ogre::String, UInt32 >      TPathIndexMap;

    COgreArchiveListing( void )
        : m_entries()
        , m_index()
    {GUCEF_TRACE;
    }

    void Clear( void )
    {GUCEF_TRACE;
        m_entries.clear();
        m_index.clear();
    }

    /** Adds a file, and the directories above it that are not there yet. Returns false for a path that is already there */
    bool AddFile( const Ogre::String& path, UInt64 size )
    {GUCEF_TRACE;

        Ogre::String clean = Normalize( path );
        if ( clean.empty() || m_index.find( clean ) != m_index.end() )
            return false;

        AddParentDirectories( clean );
        return AddEntry( clean, size, false );
    }

    /** Adds a directory, and the directories above it that are not there yet. Returns false for a path that is already there */
    bool AddDirectory( const Ogre::String& path )
    {GUCEF_TRACE;

        Ogre::String clean = Normalize( path );
        if ( clean.empty() || m_index.find( clean ) != m_index.end() )
            return false;

        AddParentDirectories( clean );
        return AddEntry( clean, 0, true );
    }

    UInt32 GetCount( void ) const
    {
        return (UInt32) m_entries.size();
    }

    TEntryVector& GetEntries( void )
    {
        return m_entries;
    }

    const TEntryVector& GetEntries( void ) const
    {
        return m_entries;
    }

    const SEntry* FindEntry( const Ogre::String& path ) const
    {GUCEF_TRACE;

        TPathIndexMap::const_iterator i = m_index.find( Normalize( path ) );
        if ( i == m_index.end() )
            return GUCEF_NULL;
        return &m_entries[ i->second ];
    }

    /** True when the path is a file of the listing */
    bool HasFile( const Ogre::String& path ) const
    {GUCEF_TRACE;

        const SEntry* entry = FindEntry( path );
        return GUCEF_NULL != entry && !entry->isDirectory;
    }

    /** The names of the files (or of the directories) in the root, or in the whole tree when recursive */
    void List( bool recursive, bool dirs, Ogre::StringVector& names ) const
    {GUCEF_TRACE;

        TEntryVector::const_iterator i = m_entries.begin();
        while ( i != m_entries.end() )
        {
            if ( (*i).isDirectory == dirs && ( recursive || (*i).directory.empty() ) )
                names.push_back( (*i).path );
            ++i;
        }
    }

    void ListInfo( bool recursive, bool dirs, const Ogre::Archive* owner, Ogre::FileInfoList& infos ) const
    {GUCEF_TRACE;

        TEntryVector::const_iterator i = m_entries.begin();
        while ( i != m_entries.end() )
        {
            if ( (*i).isDirectory == dirs && ( recursive || (*i).directory.empty() ) )
                infos.push_back( ToFileInfo( *i, owner ) );
            ++i;
        }
    }

    /** The names that match the pattern, see the description of the class */
    void Find( const Ogre::String& pattern, bool recursive, bool dirs, bool caseSensitive, Ogre::StringVector& names ) const
    {GUCEF_TRACE;

        Ogre::String directory;
        Ogre::String mask;
        SplitPattern( pattern, directory, mask );

        TEntryVector::const_iterator i = m_entries.begin();
        while ( i != m_entries.end() )
        {
            if ( Matches( *i, directory, mask, recursive, dirs, caseSensitive ) )
                names.push_back( (*i).path );
            ++i;
        }
    }

    void FindInfo( const Ogre::String& pattern, bool recursive, bool dirs, bool caseSensitive, const Ogre::Archive* owner, Ogre::FileInfoList& infos ) const
    {GUCEF_TRACE;

        Ogre::String directory;
        Ogre::String mask;
        SplitPattern( pattern, directory, mask );

        TEntryVector::const_iterator i = m_entries.begin();
        while ( i != m_entries.end() )
        {
            if ( Matches( *i, directory, mask, recursive, dirs, caseSensitive ) )
                infos.push_back( ToFileInfo( *i, owner ) );
            ++i;
        }
    }

    /**
     *  Makes a path of the form the listing uses: '\' becomes '/', "." parts and doubled or surrounding '/' go.
     *  A ".." part is kept, it names nothing in a listing, so no query can match it
     */
    static Ogre::String Normalize( const Ogre::String& path )
    {GUCEF_TRACE;

        Ogre::String result;
        result.reserve( path.size() );
        size_t start = 0;
        while ( start <= path.size() )
        {
            size_t end = start;
            while ( end < path.size() && '/' != path[ end ] && '\\' != path[ end ] )
                ++end;
            const Ogre::String part = path.substr( start, end - start );
            if ( !part.empty() && "." != part )
            {
                if ( !result.empty() )
                    result += '/';
                result += part;
            }
            start = end + 1;
        }
        return result;
    }

    /** True when a path has a ".." part, which would leave the root of an archive */
    static bool HasParentReference( const Ogre::String& path )
    {GUCEF_TRACE;

        size_t start = 0;
        while ( start <= path.size() )
        {
            size_t end = start;
            while ( end < path.size() && '/' != path[ end ] && '\\' != path[ end ] )
                ++end;
            if ( ".." == path.substr( start, end - start ) )
                return true;
            start = end + 1;
        }
        return false;
    }

    private:

    static void SplitPattern( const Ogre::String& pattern, Ogre::String& directory, Ogre::String& mask )
    {GUCEF_TRACE;

        size_t pos = pattern.find_last_of( "/\\" );
        if ( Ogre::String::npos == pos )
        {
            directory.clear();
            mask = pattern;
        }
        else
        {
            // a directory as the pattern has, in the form of the entries
            directory = Normalize( pattern.substr( 0, pos + 1 ) );
            if ( !directory.empty() )
                directory += '/';
            mask = pattern.substr( pos + 1 );
        }
    }

    static bool Matches( const SEntry& entry, const Ogre::String& directory, const Ogre::String& mask, bool recursive, bool dirs, bool caseSensitive )
    {GUCEF_TRACE;

        if ( entry.isDirectory != dirs )
            return false;

        if ( recursive )
        {
            if ( 0 != entry.directory.compare( 0, directory.size(), directory ) )
                return false;
        }
        else
        {
            if ( entry.directory != directory )
                return false;
        }
        return Ogre::StringUtil::match( entry.name, mask, caseSensitive );
    }

    static Ogre::FileInfo ToFileInfo( const SEntry& entry, const Ogre::Archive* owner )
    {GUCEF_TRACE;

        Ogre::FileInfo info;
        info.archive = owner;
        info.filename = entry.path;
        info.path = entry.directory;
        info.basename = entry.name;
        info.compressedSize = (size_t) entry.size;
        info.uncompressedSize = (size_t) entry.size;
        return info;
    }

    void AddParentDirectories( const Ogre::String& path )
    {GUCEF_TRACE;

        size_t pos = path.find( '/' );
        while ( Ogre::String::npos != pos )
        {
            const Ogre::String parent = path.substr( 0, pos );
            if ( m_index.find( parent ) == m_index.end() )
                AddEntry( parent, 0, true );
            pos = path.find( '/', pos + 1 );
        }
    }

    bool AddEntry( const Ogre::String& path, UInt64 size, bool isDirectory )
    {GUCEF_TRACE;

        SEntry entry;
        entry.path = path;
        const size_t pos = path.rfind( '/' );
        if ( Ogre::String::npos == pos )
        {
            entry.name = path;
        }
        else
        {
            entry.directory = path.substr( 0, pos + 1 );
            entry.name = path.substr( pos + 1 );
        }
        entry.size = size;
        entry.isDirectory = isDirectory;

        m_index[ path ] = (UInt32) m_entries.size();
        m_entries.push_back( entry );
        return true;
    }

    private:

    TEntryVector m_entries;
    TPathIndexMap m_index;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREARCHIVELISTING_H ? */

/*
 *  gucef common header: header-only writer of small ISO 9660 disc images for tests
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*
 *  Test support: writes small ISO 9660 images in memory, with optional Joliet and Rock Ridge names, files stored
 *  in several extents and long Rock Ridge names that continue in another block. This is for testing readers of
 *  disc images without having to ship binary images or depend on external tools.
 *
 *  Usage:
 *    GUCEF::TESTFW::ISO::SNode root = GUCEF::TESTFW::ISO::Dir( "", "" );
 *    root.children.push_back( GUCEF::TESTFW::ISO::File( "ReadMe.txt", "README.TXT;1", "content" ) );
 *    GUCEF::TESTFW::ISO::SOptions options; options.joliet = true;
 *    GUCEF::TESTFW::ISO::CImageWriter writer( root, options, "VOLUME" );
 *    std::vector< unsigned char > image = writer.Write();
 *
 *  Names: every node has a name for Joliet and Rock Ridge (long, mixed case, UTF-8) and a plain ISO 9660 name
 *  (upper case, files with a ";1" version suffix) which the writer uses as given.
 */

#ifndef GUCEF_TEST_ISOIMAGEWRITER_H
#define GUCEF_TEST_ISOIMAGEWRITER_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <map>
#include <string.h>

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

namespace GUCEF {
namespace TESTFW {
namespace ISO {

static const UInt32 SectorSize = 2048;

typedef std::vector< UInt8 > TBytes;

/** One file or directory of the image to write */
struct SNode
{
    std::string longName;           /**< the name for Joliet and Rock Ridge, UTF-8 */
    std::string isoName;            /**< the plain ISO 9660 name, for files including the ";1" */
    bool isDir;
    std::string content;
    UInt32 splitAt;                 /**< if not 0 the file is written as two records, the first holds this many bytes (a multiple of 2048) */
    std::vector< SNode > children;

    UInt32 isoExtent;
    UInt32 jolietExtent;
    UInt32 fileExtent;
    UInt32 isoDirSize;
    UInt32 jolietDirSize;
    UInt32 ceExtent;

    SNode( void ) : isDir( false ), splitAt( 0 ), isoExtent( 0 ), jolietExtent( 0 ), fileExtent( 0 ), isoDirSize( 0 ), jolietDirSize( 0 ), ceExtent( 0 ) {}
};

struct SOptions
{
    bool joliet;
    bool rockRidge;
    SOptions( void ) : joliet( false ), rockRidge( false ) {}
};

inline SNode
File( const std::string& longName, const std::string& isoName, const std::string& content )
{
    SNode n;
    n.longName = longName;
    n.isoName = isoName;
    n.content = content;
    return n;
}

inline SNode
Dir( const std::string& longName, const std::string& isoName )
{
    SNode n;
    n.longName = longName;
    n.isoName = isoName;
    n.isDir = true;
    return n;
}

inline void
Put16Both( TBytes& b, size_t pos, UInt16 v )
{
    b[ pos ] = (UInt8) ( v & 0xFF ); b[ pos+1 ] = (UInt8) ( v >> 8 );
    b[ pos+2 ] = (UInt8) ( v >> 8 ); b[ pos+3 ] = (UInt8) ( v & 0xFF );
}

inline void
Put32Both( TBytes& b, size_t pos, UInt32 v )
{
    b[ pos ] = (UInt8) v; b[ pos+1 ] = (UInt8) ( v >> 8 ); b[ pos+2 ] = (UInt8) ( v >> 16 ); b[ pos+3 ] = (UInt8) ( v >> 24 );
    b[ pos+4 ] = (UInt8) ( v >> 24 ); b[ pos+5 ] = (UInt8) ( v >> 16 ); b[ pos+6 ] = (UInt8) ( v >> 8 ); b[ pos+7 ] = (UInt8) v;
}

inline UInt32
BlocksFor( UInt64 bytes )
{
    return (UInt32) ( ( bytes + SectorSize - 1 ) / SectorSize );
}

/** UTF-8 to UCS-2 big endian, enough for the test names (basic multilingual plane only) */
inline std::string
ToUcs2Be( const std::string& utf8 )
{
    std::string out;
    size_t i = 0;
    while ( i < utf8.size() )
    {
        UInt32 cp = 0;
        UInt8 c = (UInt8) utf8[ i ];
        if ( c < 0x80 ) { cp = c; i += 1; }
        else if ( ( c & 0xE0 ) == 0xC0 ) { cp = ( ( c & 0x1F ) << 6 ) | ( utf8[ i+1 ] & 0x3F ); i += 2; }
        else { cp = ( ( c & 0x0F ) << 12 ) | ( ( utf8[ i+1 ] & 0x3F ) << 6 ) | ( utf8[ i+2 ] & 0x3F ); i += 3; }
        out += (char) ( cp >> 8 );
        out += (char) ( cp & 0xFF );
    }
    return out;
}

/** Appends one directory record to the data of a directory */
inline void
AppendRecord( TBytes& dirData, const std::string& name, UInt32 extent, UInt32 size, bool isDir, bool continues, const TBytes& systemUse )
{
    const size_t nameLen = name.size();
    const size_t sua = 33 + nameLen + ( 0 == ( nameLen % 2 ) ? 1 : 0 );
    size_t recLen = sua + systemUse.size();
    if ( recLen % 2 )
        ++recLen;

    // records never cross a sector boundary
    const size_t used = dirData.size() % SectorSize;
    if ( used + recLen > SectorSize )
        dirData.resize( dirData.size() + ( SectorSize - used ), 0 );

    const size_t pos = dirData.size();
    dirData.resize( pos + recLen, 0 );
    dirData[ pos ] = (UInt8) recLen;
    Put32Both( dirData, pos + 2, extent );
    Put32Both( dirData, pos + 10, size );
    // recorded 1995-06-15 12:30:45 UTC
    dirData[ pos+18 ] = 95; dirData[ pos+19 ] = 6; dirData[ pos+20 ] = 15; dirData[ pos+21 ] = 12; dirData[ pos+22 ] = 30; dirData[ pos+23 ] = 45; dirData[ pos+24 ] = 0;
    dirData[ pos+25 ] = (UInt8) ( ( isDir ? 0x02 : 0x00 ) | ( continues ? 0x80 : 0x00 ) );
    Put16Both( dirData, pos + 28, 1 );
    dirData[ pos+32 ] = (UInt8) nameLen;
    memcpy( &dirData[ pos+33 ], name.data(), nameLen );
    if ( !systemUse.empty() )
        memcpy( &dirData[ pos + sua ], &systemUse[ 0 ], systemUse.size() );
}

class CImageWriter
{
    public:

    CImageWriter( const SNode& root, const SOptions& options, const std::string& volumeId )
        : m_root( root ), m_options( options ), m_volumeId( volumeId ), m_ceData()
    {
    }

    TBytes Write( void )
    {
        std::vector< SNode* > dirs;
        std::vector< SNode* > parents;
        Flatten( &m_root, &m_root, dirs, parents );

        // first the sizes of the directories, which do not depend on where they end up
        for ( size_t i = 0; i < dirs.size(); ++i )
        {
            TBytes data;
            BuildDirectory( *dirs[ i ], *parents[ i ], i == 0, false, data, false );
            dirs[ i ]->isoDirSize = (UInt32) data.size();
            data.clear();
            BuildDirectory( *dirs[ i ], *parents[ i ], i == 0, true, data, false );
            dirs[ i ]->jolietDirSize = (UInt32) data.size();
        }

        // then the places on the disc: system area, descriptors (primary, supplementary, terminator), directories, files
        UInt32 cursor = 16 + ( m_options.joliet ? 3 : 2 );
        for ( size_t i = 0; i < dirs.size(); ++i )
        {
            dirs[ i ]->isoExtent = cursor;
            cursor += dirs[ i ]->isoDirSize / SectorSize;
        }
        if ( m_options.joliet )
        {
            for ( size_t i = 0; i < dirs.size(); ++i )
            {
                dirs[ i ]->jolietExtent = cursor;
                cursor += dirs[ i ]->jolietDirSize / SectorSize;
            }
        }
        AssignFileExtents( m_root, cursor );

        TBytes image( (size_t) cursor * SectorSize, 0 );

        // and now the directories for real, with the extents known
        for ( size_t i = 0; i < dirs.size(); ++i )
        {
            TBytes data;
            BuildDirectory( *dirs[ i ], *parents[ i ], i == 0, false, data, true );
            memcpy( &image[ (size_t) dirs[ i ]->isoExtent * SectorSize ], &data[ 0 ], data.size() );
            if ( m_options.joliet )
            {
                data.clear();
                BuildDirectory( *dirs[ i ], *parents[ i ], i == 0, true, data, true );
                memcpy( &image[ (size_t) dirs[ i ]->jolietExtent * SectorSize ], &data[ 0 ], data.size() );
            }
        }
        WriteFiles( m_root, image );
        for ( std::map< UInt32, TBytes >::const_iterator ce = m_ceData.begin(); ce != m_ceData.end(); ++ce )
            memcpy( &image[ (size_t) (*ce).first * SectorSize ], &(*ce).second[ 0 ], (*ce).second.size() );

        WriteDescriptors( image, m_root );
        return image;
    }

    private:

    void Flatten( SNode* dir, SNode* parent, std::vector< SNode* >& dirs, std::vector< SNode* >& parents )
    {
        dirs.push_back( dir );
        parents.push_back( parent );
        for ( size_t i = 0; i < dir->children.size(); ++i )
            if ( dir->children[ i ].isDir )
                Flatten( &dir->children[ i ], dir, dirs, parents );
    }

    void AssignFileExtents( SNode& dir, UInt32& cursor )
    {
        for ( size_t i = 0; i < dir.children.size(); ++i )
        {
            SNode& c = dir.children[ i ];
            if ( c.isDir )
            {
                AssignFileExtents( c, cursor );
            }
            else
            {
                // names that do not fit in the record get a block for their name entries
                if ( m_options.rockRidge && c.longName.size() > 100 )
                    c.ceExtent = cursor++;
                c.fileExtent = cursor;
                cursor += BlocksFor( c.content.size() );
            }
        }
    }

    void WriteFiles( SNode& dir, TBytes& image )
    {
        for ( size_t i = 0; i < dir.children.size(); ++i )
        {
            SNode& c = dir.children[ i ];
            if ( c.isDir )
                WriteFiles( c, image );
            else if ( !c.content.empty() )
                memcpy( &image[ (size_t) c.fileExtent * SectorSize ], c.content.data(), c.content.size() );
        }
    }

    /** Rock Ridge system use entries for a record: the alternate name, using a continuation area when it is long */
    TBytes RockRidgeEntries( SNode& node, bool isRootDot, bool writing )
    {
        TBytes su;
        if ( !m_options.rockRidge )
            return su;
        if ( isRootDot )
        {
            const UInt8 sp[ 7 ] = { 'S', 'P', 7, 1, 0xBE, 0xEF, 0 };
            su.insert( su.end(), sp, sp + 7 );
            return su;
        }

        const std::string& name = node.longName;
        if ( name.size() <= 100 )
        {
            su.push_back( 'N' ); su.push_back( 'M' ); su.push_back( (UInt8) ( 5 + name.size() ) ); su.push_back( 1 ); su.push_back( 0 );
            su.insert( su.end(), name.begin(), name.end() );
        }
        else
        {
            su.resize( 28, 0 );
            su[ 0 ] = 'C'; su[ 1 ] = 'E'; su[ 2 ] = 28; su[ 3 ] = 1;
            TBytes ce;
            size_t done = 0;
            while ( done < name.size() )
            {
                const size_t chunk = name.size() - done > 100 ? 100 : name.size() - done;
                const bool more = done + chunk < name.size();
                ce.push_back( 'N' ); ce.push_back( 'M' ); ce.push_back( (UInt8) ( 5 + chunk ) ); ce.push_back( 1 ); ce.push_back( more ? 1 : 0 );
                ce.insert( ce.end(), name.begin() + done, name.begin() + done + chunk );
                done += chunk;
            }
            if ( writing )
            {
                Put32Both( su, 4, node.ceExtent );
                Put32Both( su, 12, 0 );
                Put32Both( su, 20, (UInt32) ce.size() );
                m_ceData[ node.ceExtent ] = ce;
            }
        }
        return su;
    }

    void BuildDirectory( SNode& dir, SNode& parent, bool isRoot, bool joliet, TBytes& out, bool writing )
    {
        const TBytes none;
        const UInt32 selfExtent = joliet ? dir.jolietExtent : dir.isoExtent;
        const UInt32 selfSize = joliet ? dir.jolietDirSize : dir.isoDirSize;
        const UInt32 parentExtent = joliet ? parent.jolietExtent : parent.isoExtent;
        const UInt32 parentSize = joliet ? parent.jolietDirSize : parent.isoDirSize;

        AppendRecord( out, std::string( 1, '\0' ), selfExtent, selfSize, true, false, joliet ? none : RockRidgeEntries( dir, isRoot, writing ) );
        AppendRecord( out, std::string( 1, '\1' ), parentExtent, parentSize, true, false, none );

        for ( size_t i = 0; i < dir.children.size(); ++i )
        {
            SNode& c = dir.children[ i ];
            const std::string name = joliet ? ToUcs2Be( c.longName ) : c.isoName;
            const TBytes su = joliet ? none : RockRidgeEntries( c, false, writing );
            if ( c.isDir )
            {
                AppendRecord( out, name, joliet ? c.jolietExtent : c.isoExtent, joliet ? c.jolietDirSize : c.isoDirSize, true, false, su );
            }
            else if ( c.splitAt > 0 && c.splitAt < c.content.size() )
            {
                AppendRecord( out, name, c.fileExtent, c.splitAt, false, true, su );
                AppendRecord( out, name, c.fileExtent + c.splitAt / SectorSize, (UInt32) c.content.size() - c.splitAt, false, false, su );
            }
            else
            {
                AppendRecord( out, name, c.fileExtent, (UInt32) c.content.size(), false, false, su );
            }
        }
        out.resize( BlocksFor( out.size() ) * SectorSize, 0 );
    }

    void WriteDescriptor( TBytes& image, UInt32 sector, UInt8 type, bool joliet, UInt32 volumeSpaceBlocks )
    {
        TBytes d( SectorSize, 0 );
        d[ 0 ] = type;
        memcpy( &d[ 1 ], "CD001", 5 );
        d[ 6 ] = 1;
        // identifiers are space padded text
        for ( size_t i = 8; i < 813; ++i )
            d[ i ] = ' ';
        if ( joliet )
        {
            d[ 7 ] = 0; // flags
            // Joliet identifiers are UCS-2: pad with spaces in that encoding
            for ( size_t i = 8; i < 813; i += 2 )
            {
                d[ i ] = 0; d[ i+1 ] = ' ';
            }
            const std::string vol = ToUcs2Be( m_volumeId );
            memcpy( &d[ 40 ], vol.data(), vol.size() > 32 ? 32 : vol.size() );
            d[ 88 ] = '%'; d[ 89 ] = '/'; d[ 90 ] = 'E';
            for ( size_t i = 91; i < 120; ++i )
                d[ i ] = 0;
        }
        else
        {
            memcpy( &d[ 40 ], m_volumeId.data(), m_volumeId.size() > 32 ? 32 : m_volumeId.size() );
        }
        Put32Both( d, 80, volumeSpaceBlocks );
        Put16Both( d, 120, 1 );
        Put16Both( d, 124, 1 );
        Put16Both( d, 128, (UInt16) SectorSize );
        // the root directory record
        const size_t r = 156;
        d[ r ] = 34;
        Put32Both( d, r + 2, joliet ? m_root.jolietExtent : m_root.isoExtent );
        Put32Both( d, r + 10, joliet ? m_root.jolietDirSize : m_root.isoDirSize );
        d[ r+18 ] = 95; d[ r+19 ] = 6; d[ r+20 ] = 15; d[ r+21 ] = 12; d[ r+22 ] = 30; d[ r+23 ] = 45;
        d[ r+25 ] = 0x02;
        Put16Both( d, r + 28, 1 );
        d[ r+32 ] = 1;
        d[ r+33 ] = 0;
        // dates: not specified
        for ( size_t i = 813; i < 881; i += 17 )
        {
            memset( &d[ i ], '0', 16 );
            d[ i+16 ] = 0;
        }
        d[ 881 ] = 1;
        memcpy( &image[ (size_t) sector * SectorSize ], &d[ 0 ], SectorSize );
    }

    void WriteDescriptors( TBytes& image, SNode& root )
    {
        (void) root;
        const UInt32 total = (UInt32) ( image.size() / SectorSize );
        WriteDescriptor( image, 16, 1, false, total );
        UInt32 next = 17;
        if ( m_options.joliet )
            WriteDescriptor( image, next++, 2, true, total );
        TBytes term( SectorSize, 0 );
        term[ 0 ] = 255;
        memcpy( &term[ 1 ], "CD001", 5 );
        term[ 6 ] = 1;
        memcpy( &image[ (size_t) next * SectorSize ], &term[ 0 ], SectorSize );
    }

    SNode m_root;
    SOptions m_options;
    std::string m_volumeId;
    std::map< UInt32, TBytes > m_ceData;
};


}; /* namespace ISO */
}; /* namespace TESTFW */
}; /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_ISOIMAGEWRITER_H ? */

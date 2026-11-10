/*
 *  gucefVIDEO_TestApp: GUCEF test application for the VIDEO module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#ifndef GUCEFVIDEO_H
#include "gucefVIDEO.h"
#define GUCEFVIDEO_H
#endif /* GUCEFVIDEO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVideoSmacker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* PluginGroup = "videocodecplugin";

typedef GUCEF::vector< UInt8 > TBytes;
typedef GUCEF::vector< UInt8 > TCode;          /**< Huffman code bits, first bit first */

/*-------------------------------------------------------------------------*/

bool
LoadSmkPlugin( void )
{
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = "$MODULEDIR$/videocodecpluginSMK_d";
    const CORE::CString second = "$MODULEDIR$/videocodecpluginSMK";
    #else
    const CORE::CString first = "$MODULEDIR$/videocodecpluginSMK";
    const CORE::CString second = "$MODULEDIR$/videocodecpluginSMK_d";
    #endif

    VIDEO::TVideoCodecPtr codec;
    if ( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( "smk", codec ) )
        return true;
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, PluginGroup, true ) || pluginControl.AddPluginFromDir( second, PluginGroup, true );
}

/*-------------------------------------------------------------------------*/

VIDEO::TVideoCodecPtr
FindCodec( void )
{
    VIDEO::TVideoCodecPtr codec;
    VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodec( "smk", codec );
    return codec;
}

/*-------------------------------------------------------------------------*/

void
AppendLE32( TBytes& bytes, UInt32 value )
{
    bytes.push_back( (UInt8) value );
    bytes.push_back( (UInt8) ( value >> 8 ) );
    bytes.push_back( (UInt8) ( value >> 16 ) );
    bytes.push_back( (UInt8) ( value >> 24 ) );
}

/*-------------------------------------------------------------------------*/

void
Append( TBytes& bytes, const TBytes& more )
{
    bytes.insert( bytes.end(), more.begin(), more.end() );
}

/*-------------------------------------------------------------------------*/

/** Writes bits least significant first, as Smacker reads them */
class CBitWriter
{
    public:

    CBitWriter( void ) : bytes(), bitCount( 0 ) {}

    void Bit( UInt32 bit )
    {
        if ( 0 == bitCount % 8 )
            bytes.push_back( 0 );
        if ( bit )
            bytes.back() |= (UInt8) ( 1 << ( bitCount % 8 ) );
        ++bitCount;
    }

    void Bits( UInt32 value, UInt32 count )
    {
        for ( UInt32 i=0; i<count; ++i )
            Bit( ( value >> i ) & 1 );
    }

    void Code( const TCode& code )
    {
        for ( size_t i=0; i<code.size(); ++i )
            Bit( code[ i ] );
    }

    TBytes bytes;
    UInt32 bitCount;
};

/*-------------------------------------------------------------------------*/

/** An 8 bit Huffman tree over a set of values, balanced; no values makes an absent tree, which decodes to 0 */
class CSmallTree
{
    public:

    explicit CSmallTree( const GUCEF::vector< UInt32 >& values )
        : m_values()
        , m_codes()
    {
        GUCEF::set< UInt32 > distinct( values.begin(), values.end() );
        m_values.assign( distinct.begin(), distinct.end() );
        if ( !m_values.empty() )
            Shape( GUCEF_NULL, 0, m_values.size(), TCode() );
    }

    void Write( CBitWriter& writer ) const
    {
        if ( m_values.empty() )
        {
            writer.Bit( 0 );
            return;
        }
        writer.Bit( 1 );
        Shape( &writer, 0, m_values.size(), TCode() );
        writer.Bit( 0 );
    }

    void Encode( CBitWriter& writer, UInt32 value ) const
    {
        if ( m_values.empty() )
            return;
        writer.Code( m_codes.find( value )->second );
    }

    private:

    void Shape( CBitWriter* writer, size_t first, size_t end, const TCode& prefix ) const
    {
        if ( 1 == end - first )
        {
            if ( GUCEF_NULL != writer )
            {
                writer->Bit( 0 );
                writer->Bits( m_values[ first ], 8 );
            }
            else
                const_cast< CSmallTree* >( this )->m_codes[ m_values[ first ] ] = prefix;
            return;
        }
        if ( GUCEF_NULL != writer )
            writer->Bit( 1 );
        const size_t middle = first + ( end - first ) / 2;
        TCode zero( prefix ); zero.push_back( 0 );
        TCode one( prefix ); one.push_back( 1 );
        Shape( writer, first, middle, zero );
        Shape( writer, middle, end, one );
    }

    GUCEF::vector< UInt32 > m_values;
    GUCEF::map< UInt32, TCode > m_codes;
};

/*-------------------------------------------------------------------------*/

/**
 *  A 16 bit Huffman tree as Smacker has them: leaves with fixed values and leaves for the escapes, which stand for
 *  the cache of the three last values. Encoding prefers an escape leaf whose cache holds the value, so the tests
 *  go through the cache.
 */
class CBigTree
{
    public:

    CBigTree( void ) : m_leaves(), m_escapeLeaf(), m_escapes(), m_codes(), m_low( GUCEF::vector< UInt32 >() ), m_high( GUCEF::vector< UInt32 >() ), m_present( false )
    {
        for ( UInt32 k=0; k<3; ++k )
        {
            m_escapes[ k ] = 0xFFF0 + k;
            m_hasEscapeLeaf[ k ] = false;
            cache[ k ] = 0;
        }
    }

    void AddValue( UInt32 value ) { m_leaves.push_back( value ); m_escapeLeaf.push_back( -1 ); m_present = true; }

    void AddEscape( UInt32 k, UInt32 escapeValue )
    {
        m_escapes[ k ] = escapeValue;
        m_hasEscapeLeaf[ k ] = true;
        m_leaves.push_back( escapeValue );
        m_escapeLeaf.push_back( (Int32) k );
        m_present = true;
    }

    void Finish( void )
    {
        GUCEF::vector< UInt32 > lows, highs;
        for ( size_t i=0; i<m_leaves.size(); ++i )
        {
            lows.push_back( m_leaves[ i ] & 0xFF );
            highs.push_back( m_leaves[ i ] >> 8 );
        }
        m_low = CSmallTree( lows );
        m_high = CSmallTree( highs );
        if ( !m_leaves.empty() )
            Shape( GUCEF_NULL, 0, m_leaves.size(), TCode() );
    }

    void Write( CBitWriter& writer ) const
    {
        if ( !m_present )
        {
            writer.Bit( 0 );
            return;
        }
        writer.Bit( 1 );
        m_low.Write( writer );
        m_high.Write( writer );
        for ( UInt32 k=0; k<3; ++k )
            writer.Bits( m_escapes[ k ], 16 );
        Shape( &writer, 0, m_leaves.size(), TCode() );
        writer.Bit( 0 );
    }

    void ResetCache( void ) { cache[ 0 ] = cache[ 1 ] = cache[ 2 ] = 0; }

    /** False when the tree cannot make the value */
    bool Encode( CBitWriter& writer, UInt32 value )
    {
        if ( !m_present )
        {
            if ( 0 != value )
                return false;
        }
        else
        {
            Int32 leaf = -1;
            for ( UInt32 k=0; k<3 && leaf<0; ++k )
                if ( m_hasEscapeLeaf[ k ] && cache[ k ] == value )
                    for ( size_t i=0; i<m_leaves.size(); ++i )
                        if ( m_escapeLeaf[ i ] == (Int32) k )
                            leaf = (Int32) i;
            for ( size_t i=0; i<m_leaves.size() && leaf<0; ++i )
                if ( m_escapeLeaf[ i ] < 0 && m_leaves[ i ] == value )
                    leaf = (Int32) i;
            if ( leaf < 0 )
                return false;
            writer.Code( m_codes[ (size_t) leaf ] );
        }
        if ( value != cache[ 0 ] )
        {
            cache[ 2 ] = cache[ 1 ];
            cache[ 1 ] = cache[ 0 ];
            cache[ 0 ] = value;
        }
        return true;
    }

    UInt32 cache[ 3 ];

    private:

    void Shape( CBitWriter* writer, size_t first, size_t end, const TCode& prefix ) const
    {
        if ( 1 == end - first )
        {
            if ( GUCEF_NULL != writer )
            {
                writer->Bit( 0 );
                m_low.Encode( *writer, m_leaves[ first ] & 0xFF );
                m_high.Encode( *writer, m_leaves[ first ] >> 8 );
            }
            else
            {
                CBigTree* self = const_cast< CBigTree* >( this );
                if ( self->m_codes.size() < m_leaves.size() )
                    self->m_codes.resize( m_leaves.size() );
                self->m_codes[ first ] = prefix;
            }
            return;
        }
        if ( GUCEF_NULL != writer )
            writer->Bit( 1 );
        const size_t middle = first + ( end - first ) / 2;
        TCode zero( prefix ); zero.push_back( 0 );
        TCode one( prefix ); one.push_back( 1 );
        Shape( writer, first, middle, zero );
        Shape( writer, middle, end, one );
    }

    GUCEF::vector< UInt32 > m_leaves;
    GUCEF::vector< Int32 > m_escapeLeaf;    /**< per leaf: the escape it stands for, -1 for a fixed value */
    UInt32 m_escapes[ 3 ];
    bool m_hasEscapeLeaf[ 3 ];
    GUCEF::vector< TCode > m_codes;
    CSmallTree m_low;
    CSmallTree m_high;
    bool m_present;
};

/*-------------------------------------------------------------------------*/

enum { MMAP = 0, MCLR = 1, FULL = 2, TYPE = 3 };

/** The type code of a run of blocks */
UInt32
TypeCode( UInt32 kind, UInt32 run, UInt32 colour = 0 )
{
    UInt32 index = run - 1;
    if ( run >= 128 )
        index = 59 + ( 128 == run ? 0 : ( 256 == run ? 1 : ( 512 == run ? 2 : ( 1024 == run ? 3 : 4 ) ) ) );
    return kind | ( index << 2 ) | ( colour << 8 );
}

/*-------------------------------------------------------------------------*/

/** An audio chunk with compressed sound, made as the format describes it */
TBytes
PackAudio( const GUCEF::vector< Int32 >& samples, bool stereo, bool sixteen )
{
    const UInt32 channels = stereo ? 2 : 1;
    const UInt32 treeCount = ( sixteen ? 2 : 1 ) * channels;

    // the deltas per channel, as the bytes the trees hand out
    GUCEF::vector< UInt32 > treeValues[ 4 ];
    GUCEF::vector< UInt32 > codes;          // per sample after the start values: tree value(s)
    Int32 previous[ 2 ] = { 0, 0 };
    for ( size_t i=0; i<samples.size(); ++i )
    {
        const UInt32 c = (UInt32) ( i % channels );
        if ( i >= channels )
        {
            const UInt32 delta = (UInt32) ( samples[ i ] - previous[ c ] );
            if ( sixteen )
            {
                treeValues[ c * 2 ].push_back( delta & 0xFF );
                treeValues[ c * 2 + 1 ].push_back( ( delta >> 8 ) & 0xFF );
            }
            else
            {
                treeValues[ c ].push_back( delta & 0xFF );
            }
        }
        previous[ c ] = samples[ i ];
    }

    GUCEF::vector< CSmallTree > trees;
    for ( UInt32 t=0; t<treeCount; ++t )
        trees.push_back( CSmallTree( treeValues[ t ] ) );

    CBitWriter writer;
    writer.Bit( 1 );
    writer.Bit( stereo ? 1 : 0 );
    writer.Bit( sixteen ? 1 : 0 );
    for ( UInt32 t=0; t<treeCount; ++t )
        trees[ t ].Write( writer );
    for ( Int32 c=(Int32)channels-1; c>=0; --c )
    {
        if ( sixteen )
        {
            const UInt32 v = (UInt32) samples[ c ] & 0xFFFF;
            writer.Bits( ( ( v & 0xFF ) << 8 ) | ( v >> 8 ), 16 );
        }
        else
        {
            writer.Bits( (UInt32) samples[ c ], 8 );
        }
    }
    previous[ 0 ] = samples[ 0 ];
    previous[ 1 ] = stereo ? samples[ 1 ] : 0;
    for ( size_t i=channels; i<samples.size(); ++i )
    {
        const UInt32 c = (UInt32) ( i % channels );
        const UInt32 delta = (UInt32) ( samples[ i ] - previous[ c ] );
        if ( sixteen )
        {
            trees[ c * 2 ].Encode( writer, delta & 0xFF );
            trees[ c * 2 + 1 ].Encode( writer, ( delta >> 8 ) & 0xFF );
        }
        else
        {
            trees[ c ].Encode( writer, delta & 0xFF );
        }
        previous[ c ] = samples[ i ];
    }

    TBytes chunk;
    AppendLE32( chunk, (UInt32) samples.size() * ( sixteen ? 2 : 1 ) );
    Append( chunk, writer.bytes );
    return chunk;
}

/*-------------------------------------------------------------------------*/

struct SSmkFrame
{
    TBytes palette;                 /**< palette ops, without the length byte */
    bool hasPalette;
    TBytes audio[ 7 ];              /**< chunk contents, without the size */
    bool hasAudio[ 7 ];
    TBytes video;
    UInt32 sizeFlags;               /**< the low 2 bits of the frame size, which the reader must ignore */

    SSmkFrame( void ) : palette(), hasPalette( false ), video(), sizeFlags( 0 )
    {
        for ( UInt32 t=0; t<7; ++t )
            hasAudio[ t ] = false;
    }
};

/** A whole file. 'frames' includes the ring frame when the flags say there is one */
TBytes
MakeSmk( const char* signature, UInt32 width, UInt32 height, Int32 frameRate, UInt32 flags, const UInt32 audioRates[ 7 ],
         const TBytes& trees, const GUCEF::vector< SSmkFrame >& frames )
{
    const UInt32 frameCount = (UInt32) frames.size() - ( ( flags & 1 ) ? 1 : 0 );
    GUCEF::vector< TBytes > bodies;
    for ( size_t f=0; f<frames.size(); ++f )
    {
        TBytes body;
        if ( frames[ f ].hasPalette )
        {
            TBytes chunk( 1, 0 );
            Append( chunk, frames[ f ].palette );
            while ( 0 != chunk.size() % 4 )
                chunk.push_back( 0 );
            chunk[ 0 ] = (UInt8) ( chunk.size() / 4 );
            Append( body, chunk );
        }
        for ( UInt32 t=0; t<7; ++t )
        {
            if ( frames[ f ].hasAudio[ t ] )
            {
                AppendLE32( body, (UInt32) frames[ f ].audio[ t ].size() + 4 );
                Append( body, frames[ f ].audio[ t ] );
            }
        }
        Append( body, frames[ f ].video );
        while ( 0 != body.size() % 4 )
            body.push_back( 0 );
        bodies.push_back( body );
    }

    TBytes file( signature, signature + 4 );
    AppendLE32( file, width );
    AppendLE32( file, height );
    AppendLE32( file, frameCount );
    AppendLE32( file, (UInt32) frameRate );
    AppendLE32( file, flags );
    for ( UInt32 t=0; t<7; ++t )
        AppendLE32( file, 0x10000 );        // the largest audio chunk, not needed to decode
    AppendLE32( file, (UInt32) trees.size() );
    for ( UInt32 t=0; t<4; ++t )
        AppendLE32( file, 1024 );           // tree sizes, not needed to decode
    for ( UInt32 t=0; t<7; ++t )
        AppendLE32( file, audioRates[ t ] );
    AppendLE32( file, 0 );
    for ( size_t f=0; f<bodies.size(); ++f )
        AppendLE32( file, (UInt32) bodies[ f ].size() | frames[ f ].sizeFlags );
    for ( size_t f=0; f<frames.size(); ++f )
    {
        UInt8 type = frames[ f ].hasPalette ? 1 : 0;
        for ( UInt32 t=0; t<7; ++t )
            if ( frames[ f ].hasAudio[ t ] )
                type |= (UInt8) ( 2 << t );
        file.push_back( type );
    }
    Append( file, trees );
    for ( size_t f=0; f<bodies.size(); ++f )
        Append( file, bodies[ f ] );
    return file;
}

/*-------------------------------------------------------------------------*/

/** A new palette colour op: 6 bit components */
void
NewColour( TBytes& ops, UInt8 r, UInt8 g, UInt8 b )
{
    ops.push_back( r );
    ops.push_back( g );
    ops.push_back( b );
}

UInt8
Expand6( UInt8 v )
{
    return (UInt8) ( ( v << 2 ) | ( v >> 4 ) );
}

/*-------------------------------------------------------------------------*/

/** The picture the test expects, drawn with the block rules of the format */
class CExpectedPicture
{
    public:

    CExpectedPicture( UInt32 width = 16, UInt32 height = 8 ) : w( width ), h( height ), pixels( width * height, 0 ) {}

    UInt8* Block( UInt32 block ) { return &pixels[ ( block / ( w / 4 ) ) * 4 * w + ( block % ( w / 4 ) ) * 4 ]; }

    void Fill( UInt32 block, UInt8 colour )
    {
        UInt8* p = Block( block );
        for ( UInt32 r=0; r<4; ++r )
            memset( p + r * w, colour, 4 );
    }

    void Mono( UInt32 block, UInt32 colours, UInt32 map )
    {
        UInt8* p = Block( block );
        for ( UInt32 i=0; i<16; ++i )
            p[ ( i / 4 ) * w + ( i % 4 ) ] = ( ( map >> i ) & 1 ) ? (UInt8) ( colours >> 8 ) : (UInt8) ( colours & 0xFF );
    }

    /** rows: per row the two pair values in the order they are coded */
    void Full( UInt32 block, const UInt32 rows[ 8 ] )
    {
        UInt8* p = Block( block );
        for ( UInt32 r=0; r<4; ++r )
        {
            p[ r * w + 2 ] = (UInt8) ( rows[ r * 2 ] & 0xFF );
            p[ r * w + 3 ] = (UInt8) ( rows[ r * 2 ] >> 8 );
            p[ r * w + 0 ] = (UInt8) ( rows[ r * 2 + 1 ] & 0xFF );
            p[ r * w + 1 ] = (UInt8) ( rows[ r * 2 + 1 ] >> 8 );
        }
    }

    UInt32 w, h;
    GUCEF::vector< UInt8 > pixels;
};

/*-------------------------------------------------------------------------*/

/** Collects what a source hands to its audio sink */
class CAudioCollector : public VIDEO::IVideoAudioSink
{
    public:

    CAudioCollector( void ) : VIDEO::IVideoAudioSink(), calls( 0 ) {}

    virtual void OnVideoAudio( UInt32 track, const void* frames, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++calls;
        if ( track >= 3 )
            return;
        const UInt32 frameSize = 0 == track ? 1 : ( 1 == track ? 4 : 2 );
        const UInt8* bytes = static_cast< const UInt8* >( frames );
        data[ track ].insert( data[ track ].end(), bytes, bytes + frameCount * frameSize );
    }

    /** The 16 bit samples of a track */
    GUCEF::vector< Int32 > Samples16( UInt32 track ) const
    {
        GUCEF::vector< Int32 > samples;
        for ( size_t i=0; i+1<data[ track ].size(); i+=2 )
        {
            Int16 s;
            memcpy( &s, &data[ track ][ i ], 2 );
            samples.push_back( s );
        }
        return samples;
    }

    void Clear( void ) { for ( UInt32 t=0; t<3; ++t ) data[ t ].clear(); calls = 0; }

    TBytes data[ 3 ];
    UInt32 calls;
};

/*-------------------------------------------------------------------------*/

/**
 *  The test video: SMK2, 16x8 pixels (4x2 blocks), 15 frames per second, 3 frames and a ring frame, with three
 *  audio tracks in the Smacker track slots 0 (compressed 8 bit mono), 1 (compressed 16 bit stereo) and 3 (plain
 *  16 bit mono). The expected pictures, palettes and sound are filled in as the file is made.
 */
struct STestVideo
{
    TBytes file;
    CExpectedPicture pictures[ 3 ];
    UInt8 palettes[ 3 ][ 768 ];
    GUCEF::vector< Int32 > sound8;          /**< all frames of track 0 */
    GUCEF::vector< Int32 > sound16;         /**< track 1, interleaved */
    GUCEF::vector< Int32 > soundPlain;      /**< track 3 */

    STestVideo( void ) : file(), sound8(), sound16(), soundPlain() {}
};

bool
MakeTestVideo( STestVideo& video )
{
    CBigTree trees[ 4 ];
    trees[ MMAP ].AddValue( 0xA5A5 );
    trees[ MMAP ].AddValue( 0x0F0F );
    trees[ MMAP ].AddEscape( 0, 0x1111 );
    trees[ MCLR ].AddValue( 0x0203 );
    trees[ MCLR ].AddValue( 0x0405 );
    trees[ MCLR ].AddEscape( 0, 0x7777 );
    trees[ MCLR ].AddEscape( 1, 0x8888 );
    trees[ FULL ].AddValue( 0x0102 );
    trees[ FULL ].AddValue( 0x0304 );
    trees[ FULL ].AddValue( 0x0506 );
    trees[ FULL ].AddValue( 0x0708 );
    const UInt32 fill1run2 = TypeCode( 3, 2, 1 ), mono1 = TypeCode( 0, 1 ), full1 = TypeCode( 1, 1 ), fill0run4 = TypeCode( 3, 4, 0 );
    const UInt32 skip2 = TypeCode( 2, 2 ), fill5 = TypeCode( 3, 1, 5 ), mono2 = TypeCode( 0, 2 ), skipAll = TypeCode( 2, 2048 );
    trees[ TYPE ].AddValue( fill1run2 );
    trees[ TYPE ].AddValue( full1 );
    trees[ TYPE ].AddValue( fill0run4 );
    trees[ TYPE ].AddValue( skip2 );
    trees[ TYPE ].AddValue( fill5 );
    trees[ TYPE ].AddValue( mono2 );
    trees[ TYPE ].AddValue( skipAll );
    trees[ TYPE ].AddEscape( 2, 0xABCD );        // mono1 is 0: only the cache, which starts at 0 in every frame, makes it
    CBitWriter treeWriter;
    for ( UInt32 t=0; t<4; ++t )
    {
        trees[ t ].Finish();
        trees[ t ].Write( treeWriter );
    }

    GUCEF::vector< SSmkFrame > frames( 4 );
    bool ok = true;

    // frame 0: four new colours and 252 kept; two filled blocks, a two colour block, a full block, four filled blocks
    NewColour( frames[ 0 ].palette, 63, 0, 0 );
    NewColour( frames[ 0 ].palette, 0, 63, 0 );
    NewColour( frames[ 0 ].palette, 0, 0, 63 );
    NewColour( frames[ 0 ].palette, 10, 20, 30 );
    frames[ 0 ].palette.push_back( 0x80 | 127 );
    frames[ 0 ].palette.push_back( 0x80 | 123 );
    frames[ 0 ].hasPalette = true;
    frames[ 0 ].sizeFlags = 1;
    {
        CBitWriter w;
        for ( UInt32 t=0; t<4; ++t ) trees[ t ].ResetCache();
        ok = ok && trees[ TYPE ].Encode( w, fill1run2 );
        ok = ok && trees[ TYPE ].Encode( w, mono1 );
        ok = ok && trees[ MCLR ].Encode( w, 0x0203 ) && trees[ MMAP ].Encode( w, 0xA5A5 );
        ok = ok && trees[ TYPE ].Encode( w, full1 );
        const UInt32 rows[ 8 ] = { 0x0102, 0x0304, 0x0506, 0x0708, 0x0102, 0x0708, 0x0506, 0x0304 };
        for ( UInt32 i=0; i<8; ++i )
            ok = ok && trees[ FULL ].Encode( w, rows[ i ] );
        ok = ok && trees[ TYPE ].Encode( w, fill0run4 );
        frames[ 0 ].video = w.bytes;

        CExpectedPicture& p = video.pictures[ 0 ];
        p.Fill( 0, 1 ); p.Fill( 1, 1 );
        p.Mono( 2, 0x0203, 0xA5A5 );
        p.Full( 3, rows );
        for ( UInt32 b=4; b<8; ++b ) p.Fill( b, 0 );
    }
    memset( video.palettes[ 0 ], 0, 768 );
    video.palettes[ 0 ][ 0 ] = 255;
    video.palettes[ 0 ][ 4 ] = 255;
    video.palettes[ 0 ][ 8 ] = 255;
    video.palettes[ 0 ][ 9 ] = Expand6( 10 ); video.palettes[ 0 ][ 10 ] = Expand6( 20 ); video.palettes[ 0 ][ 11 ] = Expand6( 30 );

    GUCEF::vector< Int32 > s8a; s8a.push_back( 128 ); s8a.push_back( 130 ); s8a.push_back( 127 ); s8a.push_back( 127 ); s8a.push_back( 140 );
    frames[ 0 ].audio[ 0 ] = PackAudio( s8a, false, false );
    frames[ 0 ].hasAudio[ 0 ] = true;
    video.sound8.insert( video.sound8.end(), s8a.begin(), s8a.end() );
    const Int32 stereoA[ 6 ] = { 1000, -5, 1100, -5, 900, 300 };
    GUCEF::vector< Int32 > s16a( stereoA, stereoA + 6 );
    frames[ 0 ].audio[ 1 ] = PackAudio( s16a, true, true );
    frames[ 0 ].hasAudio[ 1 ] = true;
    video.sound16.insert( video.sound16.end(), s16a.begin(), s16a.end() );
    const Int16 plainA[ 3 ] = { 1, -2, 32767 };
    for ( UInt32 i=0; i<3; ++i )
    {
        frames[ 0 ].audio[ 3 ].push_back( (UInt8) ( (UInt16) plainA[ i ] & 0xFF ) );
        frames[ 0 ].audio[ 3 ].push_back( (UInt8) ( (UInt16) plainA[ i ] >> 8 ) );
        video.soundPlain.push_back( plainA[ i ] );
    }
    frames[ 0 ].hasAudio[ 3 ] = true;

    // frame 1: colour 0 changes, then entry 4 and 5 are copied from 0 and 1 of the palette as it was before this frame,
    // entry 6 from 3; skip, a filled block, two two-colour blocks (the second through the caches), the rest skipped
    NewColour( frames[ 1 ].palette, 5, 5, 5 );
    frames[ 1 ].palette.push_back( 0x80 | 2 );
    frames[ 1 ].palette.push_back( 0x40 | 1 );
    frames[ 1 ].palette.push_back( 0 );
    frames[ 1 ].palette.push_back( 0x40 | 0 );
    frames[ 1 ].palette.push_back( 3 );
    frames[ 1 ].palette.push_back( 0x80 | 127 );
    frames[ 1 ].palette.push_back( 0x80 | 120 );
    frames[ 1 ].hasPalette = true;
    frames[ 1 ].sizeFlags = 2;
    {
        CBitWriter w;
        for ( UInt32 t=0; t<4; ++t ) trees[ t ].ResetCache();
        ok = ok && trees[ TYPE ].Encode( w, skip2 );
        ok = ok && trees[ TYPE ].Encode( w, fill5 );
        ok = ok && trees[ TYPE ].Encode( w, mono2 );
        ok = ok && trees[ MCLR ].Encode( w, 0x0405 ) && trees[ MMAP ].Encode( w, 0x0F0F );
        ok = ok && trees[ MCLR ].Encode( w, 0x0405 ) && trees[ MMAP ].Encode( w, 0x0F0F );
        ok = ok && trees[ TYPE ].Encode( w, skipAll );
        frames[ 1 ].video = w.bytes;

        CExpectedPicture& p = video.pictures[ 1 ];
        p = video.pictures[ 0 ];
        p.Fill( 2, 5 );
        p.Mono( 3, 0x0405, 0x0F0F );
        p.Mono( 4, 0x0405, 0x0F0F );
    }
    memcpy( video.palettes[ 1 ], video.palettes[ 0 ], 768 );
    video.palettes[ 1 ][ 0 ] = video.palettes[ 1 ][ 1 ] = video.palettes[ 1 ][ 2 ] = Expand6( 5 );
    memcpy( video.palettes[ 1 ] + 12, video.palettes[ 0 ] + 0, 6 );
    memcpy( video.palettes[ 1 ] + 18, video.palettes[ 0 ] + 9, 3 );

    // a compressed chunk without sound, and a plain one
    frames[ 1 ].audio[ 0 ].push_back( 0 ); frames[ 1 ].audio[ 0 ].push_back( 0 ); frames[ 1 ].audio[ 0 ].push_back( 0 ); frames[ 1 ].audio[ 0 ].push_back( 0 );
    frames[ 1 ].audio[ 0 ].push_back( 0 );
    frames[ 1 ].hasAudio[ 0 ] = true;
    frames[ 1 ].audio[ 3 ].push_back( 7 );
    frames[ 1 ].audio[ 3 ].push_back( 0 );
    frames[ 1 ].hasAudio[ 3 ] = true;
    video.soundPlain.push_back( 7 );

    // frame 2: no palette; three two-colour blocks: the type comes from the cache that starts at 0, the colours of the
    // third block from the second cache entry
    {
        CBitWriter w;
        for ( UInt32 t=0; t<4; ++t ) trees[ t ].ResetCache();
        ok = ok && trees[ TYPE ].Encode( w, mono1 );
        ok = ok && trees[ MCLR ].Encode( w, 0x0203 ) && trees[ MMAP ].Encode( w, 0xA5A5 );
        ok = ok && trees[ TYPE ].Encode( w, mono1 );
        ok = ok && trees[ MCLR ].Encode( w, 0x0405 ) && trees[ MMAP ].Encode( w, 0xA5A5 );
        ok = ok && trees[ TYPE ].Encode( w, mono1 );
        ok = ok && trees[ MCLR ].Encode( w, 0x0203 ) && trees[ MMAP ].Encode( w, 0x0F0F );
        ok = ok && trees[ TYPE ].Encode( w, skipAll );
        frames[ 2 ].video = w.bytes;

        CExpectedPicture& p = video.pictures[ 2 ];
        p = video.pictures[ 1 ];
        p.Mono( 0, 0x0203, 0xA5A5 );
        p.Mono( 1, 0x0405, 0xA5A5 );
        p.Mono( 2, 0x0203, 0x0F0F );
    }
    memcpy( video.palettes[ 2 ], video.palettes[ 1 ], 768 );
    GUCEF::vector< Int32 > s8b; s8b.push_back( 50 ); s8b.push_back( 50 ); s8b.push_back( 49 );
    frames[ 2 ].audio[ 0 ] = PackAudio( s8b, false, false );
    frames[ 2 ].hasAudio[ 0 ] = true;
    video.sound8.insert( video.sound8.end(), s8b.begin(), s8b.end() );
    const Int32 stereoB[ 4 ] = { 0, 0, 1, -1 };
    GUCEF::vector< Int32 > s16b( stereoB, stereoB + 4 );
    frames[ 2 ].audio[ 1 ] = PackAudio( s16b, true, true );
    frames[ 2 ].hasAudio[ 1 ] = true;
    video.sound16.insert( video.sound16.end(), s16b.begin(), s16b.end() );

    // the ring frame: frame 0 again, never handed out
    frames[ 3 ] = frames[ 0 ];

    const UInt32 rates[ 7 ] = { 0xC00003E8, 0xF00007D0, 0, 0x600001F4, 0, 0, 0 };
    video.file = MakeSmk( "SMK2", 16, 8, -6666, 1, rates, treeWriter.bytes, frames );
    return ok;
}

/*-------------------------------------------------------------------------*/

VIDEO::TVideoSourcePtr
OpenBytes( const TBytes& bytes, CORE::CValueList options = CORE::CValueList() )
{
    CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer();
    buffer->Append( &bytes[ 0 ], (UInt32) bytes.size() );
    CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
    VIDEO::TVideoSourcePtr source;
    VIDEO::TVideoCodecPtr codec = FindCodec();
    if ( codec.IsNULL() || !codec->OpenSource( access, source, options ) )
        return VIDEO::TVideoSourcePtr();
    return source;
}

/*-------------------------------------------------------------------------*/

bool
SamePicture( const VIDEO::CVideoFrame& frame, const CExpectedPicture& expected )
{
    if ( frame.GetWidth() != expected.w || frame.GetHeight() != expected.h )
        return false;
    return 0 == memcmp( frame.GetPicture().GetDataPtr(), &expected.pixels[ 0 ], expected.pixels.size() );
}

/*-------------------------------------------------------------------------*/

UInt32
Crc32( UInt32 crc, const void* data, size_t size )
{
    static UInt32 table[ 256 ];
    static bool made = false;
    if ( !made )
    {
        for ( UInt32 i=0; i<256; ++i )
        {
            UInt32 c = i;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? 0xEDB88320U ^ ( c >> 1 ) : c >> 1;
            table[ i ] = c;
        }
        made = true;
    }
    const UInt8* p = static_cast< const UInt8* >( data );
    crc = ~crc;
    while ( size-- > 0 )
        crc = table[ ( crc ^ *p++ ) & 0xFF ] ^ ( crc >> 8 );
    return ~crc;
}

/*-------------------------------------------------------------------------*/

/** Running checksums of the sound of each track, for the comparison with a reference */
class CAudioChecksums : public VIDEO::IVideoAudioSink
{
    public:

    CAudioChecksums( const VIDEO::IVideoSource& source ) : VIDEO::IVideoAudioSink()
    {
        for ( UInt32 t=0; t<7; ++t )
        {
            crc[ t ] = 0;
            frames[ t ] = 0;
            AUDIO::CAudioFormat format;
            frameSize[ t ] = source.GetAudioTrackFormat( t, format ) ? format.GetBytesPerFrame() : 0;
        }
    }

    virtual void OnVideoAudio( UInt32 track, const void* data, UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( track >= 7 )
            return;
        crc[ track ] = Crc32( crc[ track ], data, (size_t) frameCount * frameSize[ track ] );
        frames[ track ] += frameCount;
    }

    UInt32 crc[ 7 ];
    UInt64 frames[ 7 ];
    UInt32 frameSize[ 7 ];
};

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVideoSmackerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Smacker video codec plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefVIDEO.Smacker" );

    const bool loaded = LoadSmkPlugin();

    GUCEF_TESTFW_TESTCASE( "Smacker: The plugin loads through the C interface and registers its codec type" )
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    if ( !loaded )
        GUCEF_TESTFW_SKIP( "plugins are not loaded at run time in this WebAssembly build" );
    #endif
    try
    {
        ASSERT_TRUE( loaded );
        VIDEO::TVideoCodecPtr codec = FindCodec();
        ASSERT_FALSE( codec.IsNULL() );
        ASSERT_TRUE( codec->GetType() == "smk" );

        VIDEO::TVideoCodecPtr byName;
        ASSERT_TRUE( VIDEO::CVideoGlobal::Instance()->GetVideoCodecRegistry().TryFindCodecForFilename( "anims/LOGO.SMK", byName ) );

        VIDEO::CVideoCodecPluginManager::TVideoCodecCPluginPtr plugin = VIDEO::CVideoGlobal::Instance()->GetVideoCodecPluginManager().GetPluginForCodecType( "SMK" );
        ASSERT_FALSE( plugin.IsNULL() );
        ASSERT_TRUE( plugin->IsLinked() );
        ASSERT_TRUE( plugin->GetName().HasSubstr( "Smacker" ) >= 0 );
        ASSERT_TRUE( 1 == plugin->GetRegisteredCodecTypes().size() && 1 == plugin->GetVersion().minor );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    if ( !loaded )
        return;

    STestVideo video;
    const bool made = MakeTestVideo( video );

    GUCEF_TESTFW_TESTCASE( "Smacker: Every block type, the Huffman caches and the palette changes decode as the format describes" )
    try
    {
        ASSERT_TRUE( made );
        VIDEO::TVideoSourcePtr source = OpenBytes( video.file );
        ASSERT_FALSE( source.IsNULL() );
        const VIDEO::CVideoFormat& format = source->GetFormat();
        ASSERT_TRUE( 16 == format.GetWidth() && 8 == format.GetHeight() && format.IsPalettized() );
        ASSERT_TRUE( 3 == format.GetFrameCount() );                                    // the ring frame is not counted
        ASSERT_TRUE( 100000 == format.GetFrameRateNumerator() && 6666 == format.GetFrameRateDenominator() );

        VIDEO::CVideoFrame frame;
        CAudioCollector sound;
        for ( UInt32 n=0; n<3; ++n )
        {
            ASSERT_TRUE( source->DecodeNextFrame( frame, &sound ) );
            ASSERT_TRUE( n == frame.GetFrameIndex() && 66660 * n == frame.GetTimeInMicroSeconds() );
            ASSERT_TRUE( SamePicture( frame, video.pictures[ n ] ) );
            ASSERT_TRUE( 0 == memcmp( frame.GetPalette(), video.palettes[ n ], 768 ) );
            ASSERT_TRUE( ( n < 2 ) == frame.IsPaletteChanged() );
        }
        ASSERT_FALSE( source->DecodeNextFrame( frame, &sound ) );
        ASSERT_TRUE( source->IsEndOfStream() && !source->HasFailed() );
        ASSERT_TRUE( 3 == source->GetPositionInFrames() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: Compressed 8 and 16 bit sound, mono and stereo, and plain sound, per track" )
    try
    {
        VIDEO::TVideoSourcePtr source = OpenBytes( video.file );
        ASSERT_FALSE( source.IsNULL() );
        ASSERT_TRUE( 3 == source->GetAudioTrackCount() );
        AUDIO::CAudioFormat format;
        ASSERT_TRUE( source->GetAudioTrackFormat( 0, format ) );
        ASSERT_TRUE( 1000 == format.GetSampleRate() && 1 == format.GetChannelCount() && AUDIO::SAMPLETYPE_UINT8 == format.GetSampleType() );
        ASSERT_TRUE( source->GetAudioTrackFormat( 1, format ) );
        ASSERT_TRUE( 2000 == format.GetSampleRate() && 2 == format.GetChannelCount() && AUDIO::SAMPLETYPE_INT16 == format.GetSampleType() );
        ASSERT_TRUE( source->GetAudioTrackFormat( 2, format ) );                       // Smacker track 3, the gap is closed
        ASSERT_TRUE( 500 == format.GetSampleRate() && 1 == format.GetChannelCount() && AUDIO::SAMPLETYPE_INT16 == format.GetSampleType() );
        ASSERT_FALSE( source->GetAudioTrackFormat( 3, format ) );

        VIDEO::CVideoFrame frame;
        CAudioCollector sound;
        while ( source->DecodeNextFrame( frame, &sound ) ) {}

        ASSERT_TRUE( video.sound8.size() == sound.data[ 0 ].size() );
        for ( size_t i=0; i<video.sound8.size() && i<sound.data[ 0 ].size(); ++i )
            ASSERT_TRUE( video.sound8[ i ] == (Int32) sound.data[ 0 ][ i ] );
        GUCEF::vector< Int32 > stereo = sound.Samples16( 1 );
        ASSERT_TRUE( stereo == video.sound16 );
        GUCEF::vector< Int32 > plain = sound.Samples16( 2 );
        ASSERT_TRUE( plain == video.soundPlain );

        // without a sink the sound is dropped, the pictures are the same
        VIDEO::TVideoSourcePtr quiet = OpenBytes( video.file );
        VIDEO::CVideoFrame quietFrame;
        for ( UInt32 n=0; n<3; ++n )
        {
            ASSERT_TRUE( quiet->DecodeNextFrame( quietFrame, GUCEF_NULL ) && SamePicture( quietFrame, video.pictures[ n ] ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: Seeking decodes up to the frame, which then comes completely drawn with its palette" )
    try
    {
        VIDEO::TVideoSourcePtr source = OpenBytes( video.file );
        ASSERT_TRUE( source->IsSeekable() );
        VIDEO::CVideoFrame frame;
        ASSERT_TRUE( source->SeekToFrame( 2 ) && 2 == source->GetPositionInFrames() );
        ASSERT_TRUE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
        ASSERT_TRUE( 2 == frame.GetFrameIndex() && SamePicture( frame, video.pictures[ 2 ] ) );
        ASSERT_TRUE( frame.IsPaletteChanged() && 0 == memcmp( frame.GetPalette(), video.palettes[ 2 ], 768 ) );

        // back to the start after the end
        ASSERT_FALSE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
        ASSERT_TRUE( source->SeekToFrame( 0 ) && !source->IsEndOfStream() );
        ASSERT_TRUE( source->DecodeNextFrame( frame, GUCEF_NULL ) && 0 == frame.GetFrameIndex() && SamePicture( frame, video.pictures[ 0 ] ) );
        ASSERT_FALSE( source->SeekToFrame( 4 ) );                                      // beyond the end
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: Doubled lines, SMK4 full blocks, and the full block modes that are not supported" )
    try
    {
        // 4x4, SMK4, Y doubled, two frames: a plain full block, then a full block in the "double" mode
        CBigTree trees[ 4 ];
        trees[ FULL ].AddValue( 0x1122 );
        trees[ FULL ].AddValue( 0x3344 );
        trees[ TYPE ].AddValue( TypeCode( 1, 1 ) );
        CBitWriter treeWriter;
        for ( UInt32 t=0; t<4; ++t )
        {
            trees[ t ].Finish();
            trees[ t ].Write( treeWriter );
        }
        GUCEF::vector< SSmkFrame > frames( 2 );
        CBitWriter w0;
        trees[ TYPE ].Encode( w0, TypeCode( 1, 1 ) );
        w0.Bit( 0 );
        w0.Bit( 0 );                                    // SMK4: the plain full block
        const UInt32 rows[ 8 ] = { 0x1122, 0x3344, 0x3344, 0x1122, 0x1122, 0x1122, 0x3344, 0x3344 };
        for ( UInt32 i=0; i<8; ++i )
            trees[ FULL ].Encode( w0, rows[ i ] );
        frames[ 0 ].video = w0.bytes;
        CBitWriter w1;
        trees[ TYPE ].ResetCache();
        trees[ TYPE ].Encode( w1, TypeCode( 1, 1 ) );
        w1.Bit( 1 );                                    // the "double" mode
        frames[ 1 ].video = w1.bytes;
        const UInt32 noAudio[ 7 ] = { 0, 0, 0, 0, 0, 0, 0 };
        TBytes file = MakeSmk( "SMK4", 4, 4, 100, 4, noAudio, treeWriter.bytes, frames );

        VIDEO::TVideoSourcePtr source = OpenBytes( file );
        ASSERT_FALSE( source.IsNULL() );
        ASSERT_TRUE( 4 == source->GetFormat().GetWidth() && 8 == source->GetFormat().GetHeight() );        // doubled
        ASSERT_TRUE( 1000 == source->GetFormat().GetFrameRateNumerator() && 100 == source->GetFormat().GetFrameRateDenominator() );
        ASSERT_TRUE( 0 == source->GetAudioTrackCount() );

        CExpectedPicture stored( 4, 4 );
        stored.Full( 0, rows );
        VIDEO::CVideoFrame frame;
        ASSERT_TRUE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
        const UInt8* lines = static_cast< const UInt8* >( frame.GetPicture().GetDataPtr() );
        for ( UInt32 y=0; y<8; ++y )
            ASSERT_TRUE( 0 == memcmp( lines + y * 4, &stored.pixels[ ( y / 2 ) * 4 ], 4 ) );

        ASSERT_FALSE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
        ASSERT_TRUE( source->HasFailed() && source->IsEndOfStream() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: Damaged files are refused or end as failed, they never crash" )
    try
    {
        // not a Smacker file, a header that is too short, a size that is not plausible
        TBytes text( 200, 'x' );
        ASSERT_TRUE( OpenBytes( text ).IsNULL() );
        TBytes shortHeader( video.file.begin(), video.file.begin() + 50 );
        ASSERT_TRUE( OpenBytes( shortHeader ).IsNULL() );
        TBytes huge( video.file );
        huge[ 4 ] = 0xFF; huge[ 5 ] = 0xFF; huge[ 6 ] = 0xFF;
        ASSERT_TRUE( OpenBytes( huge ).IsNULL() );

        // cut in the middle of the last frame (the ring frame comes after it, its size is the fourth in the table)
        const UInt32 ringSize = ( video.file[ 116 ] | ( video.file[ 117 ] << 8 ) | ( video.file[ 118 ] << 16 ) | ( video.file[ 119 ] << 24 ) ) & ~3U;
        TBytes cut( video.file.begin(), video.file.end() - ringSize - 4 );
        VIDEO::TVideoSourcePtr source = OpenBytes( cut );
        ASSERT_FALSE( source.IsNULL() );
        VIDEO::CVideoFrame frame;
        UInt32 decoded = 0;
        while ( source->DecodeNextFrame( frame, GUCEF_NULL ) )
            ++decoded;
        ASSERT_TRUE( 2 == decoded );
        ASSERT_TRUE( source->HasFailed() );

        // every length of the file: open or not, decode or not, but no crash
        for ( size_t length=0; length<video.file.size(); length+=3 )
        {
            TBytes part( video.file.begin(), video.file.begin() + length );
            if ( part.empty() )
                part.push_back( 0 );
            VIDEO::TVideoSourcePtr partSource = OpenBytes( part );
            if ( !partSource.IsNULL() )
                while ( partSource->DecodeNextFrame( frame, GUCEF_NULL ) ) {}
        }

        // flipped bits everywhere in the frame data
        for ( size_t i=150; i<video.file.size(); i+=5 )
        {
            TBytes damaged( video.file );
            damaged[ i ] ^= 0x5A;
            VIDEO::TVideoSourcePtr damagedSource = OpenBytes( damaged );
            CAudioCollector sound;
            if ( !damagedSource.IsNULL() )
                while ( damagedSource->DecodeNextFrame( frame, &sound ) ) {}
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: The meta data tells the format, length and tracks without decoding" )
    try
    {
        CORE::CDynamicBuffer buffer;
        buffer.Append( &video.file[ 0 ], (UInt32) video.file.size() );
        CORE::CDynamicBufferAccess access( buffer );
        CORE::CValueList meta;
        ASSERT_TRUE( FindCodec()->DecodeMetaData( access, meta ) );
        ASSERT_TRUE( "16" == meta.GetValueAlways( "width" ).AsString() && "8" == meta.GetValueAlways( "height" ).AsString() );
        ASSERT_TRUE( "palette8" == meta.GetValueAlways( "pixelFormat" ).AsString() );
        ASSERT_TRUE( "3" == meta.GetValueAlways( "frames" ).AsString() && "199" == meta.GetValueAlways( "durationInMs" ).AsString() );
        ASSERT_TRUE( "3" == meta.GetValueAlways( "audioTracks" ).AsString() );
        ASSERT_TRUE( "2" == meta.GetValueAlways( "audioTrack1.channels" ).AsString() && "2000" == meta.GetValueAlways( "audioTrack1.sampleRate" ).AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: A video plays through the player with its sound on the null driver" )
    try
    {
        AUDIO::CAudioEngineParams params;
        params.SetDriverName( "null" );
        params.SetThreadingMode( AUDIO::AUDIOTHREADING_COOPERATIVE );
        AUDIO::CAudioEngine engine;
        ASSERT_TRUE( engine.Initialize( params ) );

        CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer();
        buffer->Append( &video.file[ 0 ], (UInt32) video.file.size() );
        CORE::IOAccessPtr access( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
        VIDEO::CVideoPlayer player;
        ASSERT_TRUE( player.Open( "smk", access ) );
        ASSERT_TRUE( player.Start( &engine, GUCEF_NULL, 1 ) );                         // the stereo track
        ASSERT_TRUE( player.IsAudioClock() && SamePicture( player.GetCurrentFrame(), video.pictures[ 0 ] ) );
        ASSERT_FALSE( player.Open( "nosuchcodec", access ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // A real Smacker file, when one is given: GUCEF_TEST_SMK_FILE is decoded to its end. With GUCEF_TEST_SMK_REFERENCE,
    // a text file with a line "F <frame> <crc32 of the indices> <crc32 of the palette>" per frame and a line
    // "A <track> <crc32 of the sound> <frames of sound>" per audio track, every frame and the sound are compared
    GUCEF_TESTFW_TESTCASE( "Smacker: A real file decodes to its end (and matches its reference)" )
        GUCEF_TESTFW_REQUIRE_ENV( path, "GUCEF_TEST_SMK_FILE" );
        try
        {
            CORE::IOAccessPtr file( GUCEF_NEW CORE::CFileAccess( CORE::CString( path.c_str() ), "rb" ) );
            ASSERT_TRUE( file->Opened() );
            VIDEO::TVideoSourcePtr source;
            ASSERT_TRUE( FindCodec()->OpenSource( file, source ) );

            GUCEF::vector< CORE::CString > lines;
            const ::std::string referencePath = GUCEF_TESTFW_GETENV( "GUCEF_TEST_SMK_REFERENCE" );
            if ( !referencePath.empty() )
            {
                FILE* reference = fopen( referencePath.c_str(), "r" );
                ASSERT_TRUE( GUCEF_NULL != reference );
                char line[ 256 ];
                while ( GUCEF_NULL != reference && GUCEF_NULL != fgets( line, sizeof( line ), reference ) )
                {
                    CORE::CString text( line );
                    lines.push_back( text.RemoveChar( '\n' ).RemoveChar( '\r' ) );
                }
                if ( GUCEF_NULL != reference )
                    fclose( reference );
            }

            VIDEO::CVideoFrame frame;
            CAudioChecksums sound( *source );
            UInt32 frames = 0;
            UInt32 mismatches = 0;
            const UInt32 size = source->GetFormat().GetWidth() * source->GetFormat().GetHeight();
            while ( source->DecodeNextFrame( frame, &sound ) )
            {
                char made[ 64 ];
                sprintf( made, "F %u %08x %08x", frames, Crc32( 0, frame.GetPicture().GetDataPtr(), size ), Crc32( 0, frame.GetPalette(), 768 ) );
                if ( !lines.empty() && ( frames >= lines.size() || lines[ frames ] != made ) )
                    ++mismatches;
                ++frames;
            }
            ASSERT_FALSE( source->HasFailed() );
            ASSERT_TRUE( (UInt64) frames == source->GetFormat().GetFrameCount() );
            ASSERT_TRUE( 0 == mismatches );
            for ( UInt32 t=0; t<source->GetAudioTrackCount() && !lines.empty(); ++t )
            {
                char made[ 64 ];
                sprintf( made, "A %u %08x %llu", t, sound.crc[ t ], (unsigned long long) sound.frames[ t ] );
                ASSERT_TRUE( (size_t) frames + t < lines.size() && lines[ frames + t ] == made );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Smacker: Unloading the plugin with a video open ends the video instead of calling into the gone module" )
    try
    {
        VIDEO::TVideoSourcePtr source = OpenBytes( video.file );
        ASSERT_FALSE( source.IsNULL() );
        VIDEO::CVideoFrame frame;
        ASSERT_TRUE( source->DecodeNextFrame( frame, GUCEF_NULL ) );

        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( PluginGroup ) );
        ASSERT_TRUE( FindCodec().IsNULL() );
        ASSERT_FALSE( source->DecodeNextFrame( frame, GUCEF_NULL ) );
        ASSERT_TRUE( source->IsEndOfStream() );
        source.Unlink();

        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetPluginControl().LoadPluginGroup( PluginGroup ) );
        ASSERT_FALSE( FindCodec().IsNULL() );
        VIDEO::TVideoSourcePtr again = OpenBytes( video.file );
        ASSERT_TRUE( !again.IsNULL() && again->DecodeNextFrame( frame, GUCEF_NULL ) && SamePicture( frame, video.pictures[ 0 ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

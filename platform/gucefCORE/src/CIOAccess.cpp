/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#include "CIOAccess.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_open( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }

    static_cast< CIOAccess* >( access->privdata )->Open();
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_opened( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }

    return static_cast< CIOAccess* >( access->privdata )->Opened() ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_close( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }

    static_cast< CIOAccess* >( access->privdata )->Close();
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_readl( struct SIOAccess* access ,
          char **dest              ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access || GUCEF_NULL == dest )
    {
        return 0;
    }

    CString str = static_cast< CIOAccess* >( access->privdata )->ReadLine();
    *dest = GUCEF_NEW char[ str.Length()+1 ];
    memcpy( *dest, str.C_String(), str.Length() );
    return str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_reads( struct SIOAccess* access ,
          char **dest              ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access || GUCEF_NULL == dest )
    {
        return 0;
    }

    CString str = static_cast< CIOAccess* >( access->privdata )->ReadString();
    *dest = GUCEF_NEW char[ str.Length()+1 ];
    memcpy( *dest, str.C_String(), str.Length() );
    return str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_write( struct SIOAccess* access ,
          const void *srcData      ,
          UInt32 esize             ,
          UInt32 elements          ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access || GUCEF_NULL == srcData )
    {
        return 0;
    }
    return static_cast< CIOAccess* >( access->privdata )->Write( srcData, esize, elements );
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_read( struct SIOAccess* access ,
         void *dest               ,
         UInt32 esize             ,
         UInt32 elements          ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access || GUCEF_NULL == dest )
    {
        return 0;
    }
    return static_cast< CIOAccess* >( access->privdata )->Read( dest, esize, elements );
}

/*-------------------------------------------------------------------------*/

UInt64 GUCEF_CALLSPEC_PREFIX
fa_tell( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }
    return static_cast< CIOAccess* >( access->privdata )->Tell();
}

/*-------------------------------------------------------------------------*/

Int32 GUCEF_CALLSPEC_PREFIX
fa_seek( struct SIOAccess* access ,
         Int64 offset             ,
         Int32 origin             ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }    
    return static_cast< CIOAccess* >( access->privdata )->Seek( offset, origin );
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_CALLSPEC_PREFIX
fa_setpos( struct SIOAccess* access ,
           UInt64 pos               ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }    
    return static_cast< CIOAccess* >( access->privdata )->Setpos( pos );
}

/*-------------------------------------------------------------------------*/

Int32 GUCEF_CALLSPEC_PREFIX
fa_getc( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }    
    return static_cast< CIOAccess* >( access->privdata )->GetChar();
}

/*-------------------------------------------------------------------------*/

Int32 GUCEF_CALLSPEC_PREFIX
fa_eof( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }
    return static_cast< CIOAccess* >( access->privdata )->Eof();
}

/*-------------------------------------------------------------------------*/

UInt64 GUCEF_CALLSPEC_PREFIX
fa_size( struct SIOAccess* access ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( GUCEF_NULL == access )
    {
        return 0;
    }
    return static_cast< CIOAccess* >( access->privdata )->GetSize();
}

/*-------------------------------------------------------------------------*/

void GUCEF_CALLSPEC_PREFIX
fa_free( void* mem ) GUCEF_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_DELETE []((char*)mem);
}

/*-------------------------------------------------------------------------*/

CIOAccess::CIOAccess( void )
{GUCEF_TRACE;

    LinkCStyleAccess();
}

/*-------------------------------------------------------------------------*/

CIOAccess::CIOAccess( const CIOAccess& src )
{GUCEF_TRACE;

    LinkCStyleAccess();
}

/*-------------------------------------------------------------------------*/

CIOAccess::~CIOAccess()
{GUCEF_TRACE;

    memset( &m_cStyleAccess, 0, sizeof( TIOAccess ) );
}

/*-------------------------------------------------------------------------*/

CIOAccess&
CIOAccess::operator=( const CIOAccess& src )
{
    if ( this != &src )
    {

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CIOAccess::LinkCStyleAccess( void )
{GUCEF_TRACE;

    memset( &m_cStyleAccess, 0, sizeof( TIOAccess ) );

    m_cStyleAccess.close = fa_close;
    m_cStyleAccess.open = fa_open;
    m_cStyleAccess.write = fa_write;
    m_cStyleAccess.close = fa_close;
    m_cStyleAccess.opened = fa_opened;
    m_cStyleAccess.readl = fa_readl;
    m_cStyleAccess.reads = fa_reads;
    m_cStyleAccess.read = fa_read;
    m_cStyleAccess.tell = fa_tell;
    m_cStyleAccess.seek = fa_seek;
    m_cStyleAccess.setpos = fa_setpos;
    m_cStyleAccess.getc = fa_getc;
    m_cStyleAccess.eof = fa_eof;
    m_cStyleAccess.size = fa_size;
    m_cStyleAccess.memfree = fa_free;
    m_cStyleAccess.privdata = this;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::ReadUntill( void *dest            ,
                       UInt32 bsize          ,
                       const void* delimiter ,
                       UInt32 delimsize      )
{GUCEF_TRACE;

        //DEBUGOUTPUT( "CIOAccess::ReadUntill()" );

        GUCEF_BEGIN;
        if ( !bsize )
        {
                GUCEF_END;
                return 0;
        }
        if ( !delimsize )
        {
                GUCEF_END_RET( Read( dest  ,
                                     bsize ,
                                     1     ) );
        }

        char* buffer = GUCEF_NEW char[ delimsize ];
        char* bufwritepos = buffer;
        char* destbuf = static_cast<char*>( dest );
        UInt32 i;
        for ( i=0; i<bsize; ++i )
        {
                if ( !Eof() )
                {
                        /*
                         *      Add a byte to our read buffer
                         */
                        if ( bufwritepos - buffer < (Int32)delimsize-1 )
                        {
                                *bufwritepos = GetChar();
                                ++bufwritepos;
                        }
                        else
                        {
                                for ( UInt32 n=0; n<delimsize-1; ++n )
                                {
                                        buffer[ n ] = buffer[ n+1 ];
                                }
                                *bufwritepos = GetChar();
                        }

                        /*
                         *      Have we reached the delimiter ?
                         */
                        if ( memcmp( buffer, delimiter, delimsize ) == 0 )
                        {
                                GUCEF_DELETE []buffer;
                                destbuf[ i ] = '\0';
                                GUCEF_END;
                                return i;

                        }
                        destbuf[ i ] = *bufwritepos;
                }
                else
                {
                        GUCEF_DELETE []buffer;
                        destbuf[ i ] = '\0';
                        GUCEF_END;
                        return i;
                }
        }
        GUCEF_DELETE []buffer;
        destbuf[ bsize-i ] = '\0';
        GUCEF_END;
        return i;
}

/*-------------------------------------------------------------------------*/

/**
 *      Skips bytes untill the delimiter is reached or
 *      untill the end of the file is reached.
 *      the actual number of bytes skipped is returned.
 *
 *      @param delimiter the delimiter bytes
 *      @param delimsize size of the delimiter segment
 *      @return the actual number of bytes skipped.
 */
UInt32
CIOAccess::SkipUntill( const void* delimiter ,
                       UInt32 delimsize      )
{GUCEF_TRACE;

    if ( !delimsize )
    {
        Seek( 0, SEEK_END );
    }

    char* buffer = GUCEF_NEW char[ delimsize ];
    char* bufwritepos = buffer;
    UInt64 pos = Tell();
    while( !Eof() )
    {
        /*
            *      Add a byte to our read buffer
            */
        if ( bufwritepos - buffer < (Int32)delimsize-1 )
        {
            *bufwritepos = GetChar();
            ++bufwritepos;
        }
        else
        {
            for ( UInt32 n=0; n<delimsize-1; ++n )
            {
                buffer[ n ] = buffer[ n+1 ];
            }
            *bufwritepos = GetChar();
        }

        /*
            *      Have we reached the delimiter ?
            */
        if ( memcmp( buffer, delimiter, delimsize ) == 0 )
        {
            GUCEF_DELETE []buffer;
            return (UInt32) ( Tell() - pos );
        }
    }
    GUCEF_DELETE []buffer;
    return (UInt32) ( Tell() - pos );
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Read( CDynamicBuffer& dest ,
                 UInt32 esize         ,
                 UInt32 elements      )
{GUCEF_TRACE;

    // First we ensure that we have enough space in the buffer to copy our data
    // into the buffer manually
    dest.SetBufferSize( dest.GetDataSize() + ( esize * elements ) ,
                        false                                     );

    // Perform the actual read
    UInt32 nrOfBytesRead = Read( static_cast< UInt8* >( dest.GetBufferPtr() ) + dest.GetDataSize() ,
                                 esize                                                             ,
                                 elements                                                          );

    // Correct the data delimiter in the buffer
    dest.SetDataSize( dest.GetDataSize() + nrOfBytesRead );

    return nrOfBytesRead;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Read( CDynamicBuffer& dest ,
                 UInt32 esize         )
{GUCEF_TRACE;

    UInt64 currentOffset = Tell();
    Seek( 0, SEEK_END );
    UInt64 endOffset = Tell();
    Setpos( currentOffset );

    UInt64 bytesToRead = endOffset - currentOffset;
    UInt32 elementsToRead = (UInt32) ( bytesToRead / esize );

    return Read( dest, esize, elementsToRead );
}

/*-------------------------------------------------------------------------*/

bool 
CIOAccess::ReadByteSizePrefixedString( CString& str )
{GUCEF_TRACE;

    UInt32 strByteSize = 0;
    if ( ReadValue( strByteSize ) )
    {
        char* strBuffer = str.Reserve( strByteSize );
        if ( Read( strBuffer, strByteSize, 1 ) )
        {
            str.DetermineLength();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CIOAccess::WriteByteSizePrefixedString( const CString& str )
{GUCEF_TRACE;

    UInt32 strByteSize = str.ByteSize();
    if ( WriteValue( strByteSize ) )
    {
        return 1 == Write( str.C_String(), strByteSize, 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TIOAccess*
CIOAccess::CStyleAccess( void )
{GUCEF_TRACE;

    return &m_cStyleAccess;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Write( const void* srcdata ,
                  UInt32 esize        ,
                  UInt32 elements     )
{GUCEF_TRACE;

    // Dummy at this level, override to implement
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Write( CDynamicBuffer& sourceData ,
                  UInt32 esize               ,
                  UInt32 elements            )
{GUCEF_TRACE;

    return Write( sourceData.GetConstBufferPtr() ,
                  esize                          ,
                  elements                       );
}

/*-------------------------------------------------------------------------*/

UInt32 
CIOAccess::Write( CDynamicBuffer& sourceData )
{GUCEF_TRACE;

    if ( 1 == Write( sourceData.GetConstBufferPtr() ,
                     sourceData.GetDataSize()       ,
                     1                              ) )
    {
        return sourceData.GetDataSize();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CIOAccess::Write( CIOAccess& sourceData ,
                  Int64 bytesToWrite    )
{GUCEF_TRACE;

    UInt64 byteCount=0, totalByteCount=0;
    CDynamicBuffer readBuffer( 1024, true );
    readBuffer.SetDataSize( 1024 );
    Int64 bytesLeftToWrite = bytesToWrite;

    while ( !sourceData.Eof() )
    {
        UInt32 bytesToRead = 1024;
        if ( bytesLeftToWrite > 0 && bytesLeftToWrite < 1024 )
            bytesToRead = (UInt32) bytesLeftToWrite;

        byteCount = sourceData.Read( readBuffer.GetBufferPtr(), 1, bytesToRead );
        if ( byteCount > 0 )
        {
            Write( readBuffer.GetBufferPtr(), 1, (UInt32) byteCount );
            totalByteCount += byteCount;
            bytesLeftToWrite -= byteCount;
        }
        else
        {
            return totalByteCount;
        }
    }

    return totalByteCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Write( const CString& string ,
                  bool addEol           )
{GUCEF_TRACE;

    UInt32 bytesWritten = 0;
    
    if ( !string.IsNULLOrEmpty() )
    {
        bytesWritten = Write( string.C_String(), 1, string.ByteSize()-1 );
        if ( bytesWritten == string.ByteSize()-1 && addEol ) 
        {
            bytesWritten += Write( GUCEF_EOL, 1, GUCEF_EOL_LENGTH );
        }
    }
    else
    {
        if ( addEol ) 
        {
            bytesWritten += Write( GUCEF_EOL, 1, GUCEF_EOL_LENGTH );
        }
    }
    return bytesWritten;
}

/*-------------------------------------------------------------------------*/

UInt32 
CIOAccess::Write( const CVariant& var, bool includeNullTermIfAny )
{GUCEF_TRACE;

    if ( !var.IsNULLOrEmpty() )
    {
        return Write( var.AsVoidPtr(), var.ByteSize( includeNullTermIfAny ), 1 );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CIOAccess::GetSize( void ) const
{GUCEF_TRACE;

    UInt64 pos = Tell();
    const_cast< CIOAccess* >( this )->Seek( 0, SEEK_END );
    UInt64 size = Tell();
    const_cast< CIOAccess* >( this )->Seek( pos, SEEK_SET );
    return size;
}

/*-------------------------------------------------------------------------*/

UInt32
CIOAccess::Setpos( UInt64 position )
{GUCEF_TRACE;

    return Seek( position, SEEK_SET );
}

/*-------------------------------------------------------------------------*/

CString 
CIOAccess::ReadLine( void )
{GUCEF_TRACE;
    
    return CString::ReadLine( this );
}

/*-------------------------------------------------------------------------*/

CString 
CIOAccess::ReadString( void )
{GUCEF_TRACE;

    return CString::ReadString( this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

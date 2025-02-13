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

#include <functional>           /* needed here for std::min() */
#include <stdlib.h>             /* needed here for realloc() */
#include <string.h>             /* needed for memcpy() */
#include <assert.h>

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#include "CDynamicBuffer.h"     /* class definition */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

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

GUCEF_IMPLEMENT_MSGEXCEPTION( CDynamicBuffer, EIllegalCast );

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( void ) GUCEF_NOEXCEPT
    : CICloneable()
    , _buffer( NULL )              
    , _bsize( 0 )                 
    , _autoenlarge( true )        
    , m_dataSize( 0 )             
    , m_linked( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( bool autoenlarge ) 
    : CICloneable()
    , _buffer( NULL )              
    , _bsize( 0 )                 
    , _autoenlarge( autoenlarge ) 
    , m_dataSize( 0 )             
    , m_linked( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( CIOAccess& ioAccess )
    : CICloneable()
    , _buffer( NULL )              
    , _bsize( 0 )                 
    , _autoenlarge( true )        
    , m_dataSize( 0 )             
    , m_linked( false )
{GUCEF_TRACE;
    
    Append( ioAccess );
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( UInt32 initialsize ,
                                bool autoenlarge   )
    : CICloneable()
    , _autoenlarge( autoenlarge ) 
    , _buffer( NULL )             
    , _bsize( 0 )                 
    , m_dataSize( 0 )             
    , m_linked( false )
{GUCEF_TRACE;

    if ( initialsize > 0 )
    {
        _buffer = (Int8*) malloc( initialsize );
        if ( GUCEF_NULL != _buffer )
        {
            _bsize = initialsize;               
        }
    }
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( const CDynamicBuffer &src ,
                                bool shrinkToDataSize     )
    : CICloneable( src )
    , _buffer( NULL )                  
    , _bsize( 0 )                      
    , m_dataSize( 0 )                  
    , _autoenlarge( src._autoenlarge ) 
    , m_linked( false )
{GUCEF_TRACE;

    m_dataSize = src.m_dataSize;
    
    if ( shrinkToDataSize )
    {
        _bsize = src.m_dataSize;
        if ( _bsize > 0 )
        {
            _buffer = (Int8*) malloc( src.m_dataSize );
            if ( GUCEF_NULL != _buffer )
            {
                memcpy( _buffer, src._buffer, m_dataSize ); 
            }
            else
            {
                _bsize = m_dataSize = 0;
            }
        }
    }
    else
    {
        _bsize = src._bsize;
        if ( _bsize > 0 )
        {
            _buffer = (Int8*) malloc( src._bsize );    
            if ( GUCEF_NULL != _buffer )
            {
                memcpy( _buffer, src._buffer, m_dataSize ); 
            }
            else
            {
                _bsize = m_dataSize = 0;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer::CDynamicBuffer( const char* externalBuffer    ,
                                const UInt32 bufferSize       ,
                                bool autoenlarge /* = true */ )
    : CICloneable()
    , _buffer( NULL )      
    , _bsize( 0 )          
    , m_dataSize( 0 )      
    , _autoenlarge( true ) 
    , m_linked( false )
{GUCEF_TRACE;

    CopyFrom( bufferSize     ,
              externalBuffer );
    _autoenlarge = autoenlarge;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

CDynamicBuffer::CDynamicBuffer( CDynamicBuffer&& src ) GUCEF_NOEXCEPT
    : CICloneable()
    , _buffer( src._buffer )      
    , _bsize( src._bsize )          
    , m_dataSize( src.m_dataSize )      
    , _autoenlarge( src._autoenlarge ) 
    , m_linked( src.m_linked )
{GUCEF_TRACE;

    // reset source to default constructor values
    src._buffer = GUCEF_NULL;
    src._bsize = 0;
    src.m_dataSize = 0;
    src._autoenlarge = true;
    src.m_linked = false;
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

CDynamicBuffer::~CDynamicBuffer()
{GUCEF_TRACE;
        
    Clear( false );
}

/*-------------------------------------------------------------------------*/

UInt8&
CDynamicBuffer::GetUByte( UInt32 index )
{GUCEF_TRACE;

    SecureLinkBeforeMutation();
    
    if ( index+1 > m_dataSize )
    {
            m_dataSize = index+1;
    }
    return *((UInt8*)_buffer+index);
}

/*-------------------------------------------------------------------------*/

void
CDynamicBuffer::SetUByte( UInt32 index ,
                          UInt8 data   )
{GUCEF_TRACE;
        
    SecureLinkBeforeMutation();
    
    if ( index+1 > m_dataSize )
    {
        m_dataSize = index+1;
    }
    _buffer[ index ] = (char)data;
}

/*-------------------------------------------------------------------------*/

void 
CDynamicBuffer::SetBytes( UInt8 byteInitValue )
{GUCEF_TRACE;
        
    SecureLinkBeforeMutation();
    
    if ( GUCEF_NULL != _buffer && 0 < _bsize )
        memset( _buffer, byteInitValue, (size_t) _bsize ); 
}

/*-------------------------------------------------------------------------*/

char&
CDynamicBuffer::GetChar( UInt32 index )
{GUCEF_TRACE;

    SecureLinkBeforeMutation();
    
    if ( index+1 > m_dataSize )
    {
        m_dataSize = index+1;
    }
    return reinterpret_cast< char* >( _buffer )[ index ];
}

/*-------------------------------------------------------------------------*/

/**
 *      Sets the signed byte located at the index given.
 */
void
CDynamicBuffer::SetChar( UInt32 index ,
                         char data    )
{GUCEF_TRACE;

    SecureLinkBeforeMutation();
    
    if ( index+1 > m_dataSize )
    {
        m_dataSize = index+1;
    }
    _buffer[ index ] = data;
}

/*-------------------------------------------------------------------------*/

/**
 *      Returns the signed byte located at the index given.
 */
char&
CDynamicBuffer::operator[]( UInt32 index )
{GUCEF_TRACE;
        SecureLinkBeforeMutation();
        
        if ( index+1 > m_dataSize )
        {
                SetBufferSize( index+1, false );
                m_dataSize = index+1;
        }
        return reinterpret_cast< char* >( _buffer )[ index ];
}
/*-------------------------------------------------------------------------*/

/**
 *      Returns the signed byte located at the index given.
 */
char
CDynamicBuffer::operator[]( UInt32 index ) const
{GUCEF_TRACE;

    if ( index < m_dataSize && index < _bsize )
        return _buffer[ index ];
    return '\0';
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::operator==( const CDynamicBuffer& other ) const
{GUCEF_TRACE;
    
    if ( m_dataSize == other.m_dataSize )
    {
        return memcmp( _buffer, other._buffer, m_dataSize ) == 0;
    }       
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::operator!=( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    if ( m_dataSize != other.m_dataSize )
        return true;

    return memcmp( _buffer, other._buffer, m_dataSize ) != 0;
}

/*-------------------------------------------------------------------------*/

/**
 *      Turns the current dynamic buffer into a copy of the source
 *      dynamic buffer and returns the result.
 */
CDynamicBuffer&
CDynamicBuffer::operator=( const CDynamicBuffer &src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        if ( 0 != src._bsize )
        {
            Clear( true );
        
            _buffer = (Int8*) realloc( _buffer, src._bsize );
            memcpy( _buffer, src._buffer, src._bsize );
            _bsize = src._bsize;
            m_dataSize = src.m_dataSize;
            m_linked = false;                 
        }
        else
        {
            Clear( false );
        }
    }
    return *this;                        
}

/*-------------------------------------------------------------------------*/

/**
 *      Sets the new buffer size.
 */
bool
CDynamicBuffer::SetBufferSize( const UInt32 newSize                   ,
                               const bool allowreduction              ,
                               const bool resultOnDisallowedReduction )
{GUCEF_TRACE;

    try
    {
        SecureLinkBeforeMutation();
    
        if ( _bsize == newSize ) 
            return true;  // already at the desired size
        if ( newSize < _bsize && !allowreduction )
            return resultOnDisallowedReduction; // we do not allow size reductions

        if ( 0 == newSize )
        {
            Clear( false );
            return true;
        }

        Int8* newBuffer = (Int8*) realloc( _buffer, newSize );
        assert( GUCEF_NULL != newBuffer );
        if ( GUCEF_NULL != newBuffer )
        {
            _buffer = newBuffer;
            _bsize = newSize;
            if ( m_dataSize > newSize )
            {
                m_dataSize = newSize;
            }
            return true;
        }
    }
    catch ( const std::bad_alloc& )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DynamicBuffer:SetBufferSize: bad_alloc exception trying to set buffer size to " + ToString( newSize ) + " bytes" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DynamicBuffer:SetBufferSize: exception trying to set buffer size to " + ToString( newSize ) + " bytes: " + ToString( e.what() ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::GetBufferSize( void ) const
{GUCEF_TRACE;
        return _bsize;
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::GetUnusedBufferSize( void ) const
{GUCEF_TRACE;
    
    return _bsize - m_dataSize;
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::GetRemainingBufferSize( UInt32 offset ) const
{GUCEF_TRACE;
    
    if ( offset < _bsize )
        return _bsize - offset;
    return 0;
}

/*-------------------------------------------------------------------------*/

void 
CDynamicBuffer::SetAutoEnlarge( bool autoenlarge )
{GUCEF_TRACE;
        _autoenlarge = autoenlarge;
}

/*-------------------------------------------------------------------------*/
        
bool 
CDynamicBuffer::GetAutoEnlarge( void ) const
{GUCEF_TRACE;
        return _autoenlarge;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::LoadContentFromFile( const CString& filePath   ,
                                     const UInt32 offsetInFile ,
                                     const Int32 bytesToRead   )
{GUCEF_TRACE;

    // Set the buffer to be the same size as the file
    Clear( true );    
    
    // Zero size file load equals successfull load
    if ( 0 == bytesToRead )
        return true;
    
    // Load the entire file into memory
    // Note that the file size can changed between action and as such asking for file size
    // is just done as a performance optimization. while in most cases it will be the same
    // wrt bytes read vs reported file size and thus efficient without reallocations
    // it will not be true 100% of the time for all files everywhere.

    CDynamicBufferAccess bufferAccess( *this );
    CFileAccess fileAccess;

    if ( fileAccess.Open( filePath, "rb" ) )
    {
        // this section is purely performance optimization to reduce reallocs
        // it can be an extra filesystem op in the case of calling FileSize
        // but the buffer to hold a file can be of substantial size thus a difficult realloc
        if ( bytesToRead < 0 )
        {
            UInt64 fileSize = fileAccess.GetSize();
            UInt64 expectedBytesToRead = fileSize - offsetInFile;
            SetBufferSize( (UInt32) expectedBytesToRead, false );
        }
        else
        {
            SetBufferSize( (UInt32) bytesToRead, true );
        }

        // now perform the actual read
        if ( 0 != offsetInFile )
            fileAccess.Seek( offsetInFile, SEEK_SET );

        UInt64 totalBytesRead = bufferAccess.Write( fileAccess, bytesToRead );

        return true;
    }
    else
    {
        // Failed to open the file for loading
        Clear( false );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CDynamicBuffer::WriteContentToFile( const CORE::CString& filepath ,
                                    const CORE::UInt64 offset     ,
                                    const bool overwrite          ) const
{GUCEF_TRACE;
    
    const char* mode = overwrite ? "wb" : "rb+";

    FILE* fptr = fopen( filepath.C_String(), mode );
    if ( NULL != fptr )
    {
        fseek( fptr, (long) offset, SEEK_SET );
        bool success = 1 == fwrite( _buffer, m_dataSize, 1, fptr );
        fclose( fptr );
        if ( !success )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "DynamicBuffer::WriteContentToFile: Failed to write " + ToString( m_dataSize )  +
                    " bytes as file content at offset " + ToString( offset ) + " to file at " + filepath );
            if ( overwrite )
            {
                // Dont leave behind 0 length files as a result of a failed write
                DeleteFile( filepath );
            }
        }
        return success;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDynamicBuffer::AppendContentToFile( const CORE::CString& filepath ) const
{GUCEF_TRACE;
    
    FILE* fptr = fopen( filepath.C_String(), "ab" );
    if ( NULL != fptr )
    {
        bool success = 1 == fwrite( _buffer, m_dataSize, 1, fptr );
        fclose( fptr );
        return success;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::CopyFrom( UInt32 destinationOffset   ,
                          UInt32 size                ,
                          const void* src            )
{GUCEF_TRACE;

    if ( 0 == size || GUCEF_NULL == src )
        return 0;
    
    SecureLinkBeforeMutation();

    if ( _bsize < destinationOffset+size )         
    {
        if ( _autoenlarge )
        {
            if ( !SetBufferSize( destinationOffset+size ) )
            {
                return 0;
            }
        }
        else
        {       
            Int32 max = (Int32)_bsize - (Int32)destinationOffset - (Int32)size;
            if ( max < 0 ) 
            {
                return 0;
            }                                
            size = max;
        }                
    }                                                
    
    if ( destinationOffset + size > m_dataSize )
    {
        m_dataSize = destinationOffset + size;
    }
    memcpy( _buffer+destinationOffset, src, size );
    return size;
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyFrom( const CString& source, UInt32 offset, bool includeNullTerminator )
{GUCEF_TRACE;

    if ( !source.IsNULLOrEmpty() )
    {
        if ( includeNullTerminator )
            return CopyFrom( offset, source.ByteSize(), source.C_String() );
        else
            return CopyFrom( offset, source.ByteSize()-1, source.C_String() );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyFrom( UInt32 size     ,
                          const void* src )
{GUCEF_TRACE;
    
    return CopyFrom( 0, size, src );
}                          

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyFrom( const CDynamicBuffer& source, UInt32 destinationOffset, UInt32 nrOfBytes, UInt32 sourceOffset )
{GUCEF_TRACE;

    return CopyFrom( destinationOffset, nrOfBytes, source.GetConstBufferPtr( sourceOffset ) );
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyFrom( const CDynamicBuffer& source, UInt32 destinationOffset )
{GUCEF_TRACE;

    return CopyFrom( destinationOffset, source.GetDataSize(), source.GetConstBufferPtr() );
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyFrom( const CVariant& source, UInt32 destinationOffset )
{GUCEF_TRACE;

    return CopyFrom( destinationOffset, source.ByteSize(), source.AsVoidPtr() );
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyAndDecodeBase64From( const CString& source, UInt32 destinationOffset )
{GUCEF_TRACE;

    // Since the base 64 data does not get larger when decoded but will be equal size or smaller 
    // we can use the source string size to pre-allocate the output buffer size
    if ( SetBufferSize( source.ByteSize(), false ) )
    {
        UInt32 bytesUsed = 0;
        if ( Base64Decode( source, GetBufferPtr(), GetBufferSize(), bytesUsed ) )
        {
            SetDataSize( bytesUsed );
            return bytesUsed;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/**
 *      Copys size number of bytes from the buffer to src at the offset
 *      given.
 */
UInt32
CDynamicBuffer::CopyTo( UInt32 offset ,
                        UInt32 size   ,
                        void *dest    ) const
{GUCEF_TRACE;

    if ( offset+size > m_dataSize )
    {
        Int32 maxPossible = (Int32)m_dataSize - (Int32)offset;
        if ( maxPossible > 0 )
        {
            memcpy( dest, _buffer+offset, maxPossible );
            return maxPossible;
        }
        return 0;
    }
    memcpy( dest, _buffer+offset, size );
    return size;
}

/*-------------------------------------------------------------------------*/

/**
 *      Copys all data from the buffer to dest
 */
UInt32 
CDynamicBuffer::CopyTo( void *dest ) const
{GUCEF_TRACE;

    memcpy( dest, _buffer, m_dataSize );
    return m_dataSize;
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::CopyTo( CDynamicBuffer& dest ) const
{GUCEF_TRACE;

    dest.CopyFrom( m_dataSize, _buffer );
    return m_dataSize;        
}

/*-------------------------------------------------------------------------*/

void* 
CDynamicBuffer::GetBufferPtr( const UInt32 offset /* = 0 */ )
{GUCEF_TRACE;
        SecureLinkBeforeMutation();
        
        return _buffer + offset;
}

/*-------------------------------------------------------------------------*/
       
const void* 
CDynamicBuffer::GetConstBufferPtr( const UInt32 offset /* = 0 */ ) const
{GUCEF_TRACE;

    return _buffer + offset;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::RelinquishDataOwnership( void*& data, UInt32& dataSize )
{GUCEF_TRACE;

    SecureLinkBeforeMutation();

    // yield ownership
    data = _buffer;
    dataSize = m_dataSize;

    // release our references
    _buffer = GUCEF_NULL;
    m_dataSize = 0;
    _bsize = 0;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::RelinquishDataOwnership( CVariant& newDataOwner )
{GUCEF_TRACE;

    // this is just syntax sugar, we call the reverse
    return newDataOwner.TransferOwnershipFrom( *this );
}

/*-------------------------------------------------------------------------*/

UInt32 
CDynamicBuffer::Append( const void* data                            ,
                        const UInt32 size                           ,
                        const bool appendToLogicalData /* = true */ )
{GUCEF_TRACE;

    SecureLinkBeforeMutation();

    if ( appendToLogicalData )
    {
        if ( SetBufferSize( m_dataSize + size, false, true ) )
        {
            memcpy( _buffer + m_dataSize, data, size );
            m_dataSize += size;
            return size;
        }
    }
    else
    {
        UInt32 oldBufferSize = _bsize;
        if ( SetBufferSize( oldBufferSize + size, false, true ) )
        {
            memcpy( _buffer + oldBufferSize, data, size );
            m_dataSize = _bsize;
            return size;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::Append( const CDynamicBuffer& data     ,
                        const bool appendToLogicalData )
{GUCEF_TRACE;

    return Append( data._buffer, data.m_dataSize, appendToLogicalData );
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::Append( CIOAccess& data                ,
                        const bool appendToLogicalData )
{GUCEF_TRACE;

    // Try to use seeking to learn remainder bytes available
    // This is an optimimization, if seeking is not supported we just read in blocks
    Int64 inputSize = -1;    
    UInt64 currentPosition = data.Tell();
    if ( 0 == data.Seek( 0, SEEK_END ) )
    {
        inputSize = (Int64) ( data.Tell() - currentPosition );
        data.Setpos( currentPosition );
    }

    if ( inputSize > -1 )
    {
        // Copy the recource data all at once
        if ( SetBufferSize( (UInt32) inputSize, false, true ) )
        {
            UInt32 bytesAppended = data.Read( *this     ,
                                              1         ,
                                              (UInt32)inputSize );
            return bytesAppended;
        }
        return 0;
    }
    else
    {
        // read blocks till we hit the end of the recource
        UInt32 bytesAppended = 0;
        while ( !data.Eof() )
        {
            UInt32 newBytesAppended = data.Read( *this ,
                                                 1     ,
                                                 1024  );
            bytesAppended += newBytesAppended;
            
            if ( newBytesAppended < 1024 )
            {
                break;
            }
        }
        return bytesAppended;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CDynamicBuffer::Find( const void* data        ,
                      const UInt32 size       ,
                      UInt32 offset /* = 0 */ ) const
{GUCEF_TRACE;
    
    if ( offset+size > m_dataSize )
        return -1;

    UInt32 max = m_dataSize - size;
    for ( UInt32 i=offset; i<max; ++i )
    {
        if ( 0 == memcmp( _buffer+i, data, size ) )
            return (Int32) i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringVector
CDynamicBuffer::ParseUtf8StringElements( UInt32 seperatorCodePoint ,
                                         bool addEmptyElements     ) const
{GUCEF_TRACE;

    return CUtf8String::ParseElements( ( const UInt8* ) _buffer, m_dataSize, seperatorCodePoint, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringSet
CDynamicBuffer::ParseUniqueUtf8StringElements( UInt32 seperatorCodePoint ,
                                               bool addEmptyElements     ) const
{GUCEF_TRACE;

    return CUtf8String::ParseUniqueElements( ( const UInt8* ) _buffer, m_dataSize, seperatorCodePoint, addEmptyElements );
}
/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::Downshift( const UInt32 bytesToWipe )
{GUCEF_TRACE;

    if ( 0 == bytesToWipe )
        return true;

    SecureLinkBeforeMutation();
    
    if ( bytesToWipe > m_dataSize )
        return false;

    UInt32 offset = 0;
    UInt32 remnant = m_dataSize;
    while ( remnant > 0 )
    {
        UInt32 toWipe = bytesToWipe;
        if ( toWipe > remnant )
            toWipe = remnant;

        memcpy( _buffer+offset, _buffer+offset+bytesToWipe, toWipe );

        remnant -= toWipe;
        offset += toWipe;    
    }
    m_dataSize -= bytesToWipe;

    return true;
}

/*-------------------------------------------------------------------------*/

void 
CDynamicBuffer::Clear( const bool logicalClearOnly /* = true */ )
{GUCEF_TRACE;

    if ( m_linked )
    {
        // Nothing to deallocate, we don't have ownership of the data
        _buffer = GUCEF_NULL;
        _bsize = 0;
        m_dataSize = 0;
        m_linked = false;
    }
    else
    {
        
        if ( !logicalClearOnly )
        {
            // Cleanup our toys
            if ( GUCEF_NULL != _buffer )
                free( _buffer );
            _buffer = GUCEF_NULL;
            _bsize = 0;
        } 
        // else: reset carat only
        
        m_dataSize = 0;        
    }
}

/*-------------------------------------------------------------------------*/

void
CDynamicBuffer::SetDataSize( const UInt32 newSize )
{GUCEF_TRACE;
        SecureLinkBeforeMutation();
        
        SetBufferSize( newSize, false );
        m_dataSize = newSize;        
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::GetDataSize( void ) const
{GUCEF_TRACE;

    return m_dataSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CDynamicBuffer::GetRemainingDataSize( UInt32 offset ) const
{GUCEF_TRACE;

    if ( offset < m_dataSize )
        return m_dataSize - offset;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::operator<( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return memcmp( _buffer, other._buffer, SMALLEST( GetDataSize(), other.GetDataSize() ) ) < 0;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::operator>( const CDynamicBuffer& other ) const
{GUCEF_TRACE;

    return memcmp( _buffer, other._buffer, SMALLEST( GetDataSize(), other.GetDataSize() ) ) > 0;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer&
CDynamicBuffer::LinkTo( const void* externalBuffer ,
                        UInt32 bufferSize          )
{GUCEF_TRACE;

    // Always clear first for safety,.. we don't want memory leaks or invalid pointers
    Clear( false );
    
    /*  Link to the external buffer
     *
     *  We cast away constness but constness is preserved/guarded by the
     *  SecureLinkBeforeMutation() mechanism so it's not as bad as it looks :)
     */
    _bsize = bufferSize;
    m_dataSize = bufferSize;
    _buffer = static_cast< Int8* >( const_cast< void* >( externalBuffer ) );
    m_linked = true;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDynamicBuffer::IsLinked( void ) const
{GUCEF_TRACE;
    
    return m_linked;
}

/*-------------------------------------------------------------------------*/

void
CDynamicBuffer::SecureLinkBeforeMutation( void )
{GUCEF_TRACE;

    if ( m_linked )
    {
        /*  Buffer mutation requested but the buffer is linked to an
         *  external resource for which this object does not have ownership.
         *  We will copy the data of the external buffer and make it private.
         *  The makes the mutation a safe operation.
         *
         *  First we copy our link locally
         */
        Int8* externalBuffer = _buffer;
        UInt32 extBufferSize = _bsize;
        UInt32 dataSize = m_dataSize;
        const bool autoEnlarge = _autoenlarge;
        
        /*
         *  Now we unlink
         */
        Clear( false );
        m_linked = false; 
        
        /*
         *  And then we simply copy from the external buffer
         *  After the copy it is safe to mutate the buffer because we have ownership
         */
        _autoenlarge = true; 
        CopyFrom( extBufferSize  ,
                  externalBuffer );
        _autoenlarge = autoEnlarge;
        m_dataSize = dataSize;
    }
}

/*-------------------------------------------------------------------------*/

CICloneable* 
CDynamicBuffer::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDynamicBuffer( *this );
}

/*-------------------------------------------------------------------------*/

CVariant 
CDynamicBuffer::AsVariant( UInt32 bufferOffset, UInt8 varType ) const
{GUCEF_TRACE;

    const void* buffer = GetConstBufferPtr( bufferOffset );
    if ( GUCEF_NULL != buffer )
    {        
        return CVariant( buffer, GetDataSize() - bufferOffset, varType );
    }
    return CVariant::Empty;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer& 
CDynamicBuffer::LinkTo( const CVariant& src )
{GUCEF_TRACE;

    return LinkTo( src.AsVoidPtr(), src.ByteSize() );
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer& 
CDynamicBuffer::LinkTo( const CAsciiString& src, bool includeNullTerm )
{GUCEF_TRACE;

    if ( src.IsNULLOrEmpty() )
    {
        Clear( true );
        return *this;
    }
    else
    {
        if ( includeNullTerm )
        {
            return LinkTo( src.C_String(), src.ByteSize() );
        }
        else
        {
            return LinkTo( src.C_String(), src.ByteSize()-1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer& 
CDynamicBuffer::LinkTo( const CUtf8String& src, bool includeNullTerm )
{GUCEF_TRACE;

    if ( src.IsNULLOrEmpty() )
    {
        Clear( true );
        return *this;
    }
    else
    {
        if ( includeNullTerm )
        {
            return LinkTo( src.C_String(), src.ByteSize() );
        }
        else
        {
            return LinkTo( src.C_String(), src.ByteSize()-1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer& 
CDynamicBuffer::LinkTo( const std::string& src, bool includeNullTerm )
{GUCEF_TRACE;

    if ( src.empty() )
    {
        Clear( true );
        return *this;
    }
    else
    {
        if ( includeNullTerm )
        {
            return LinkTo( src.c_str(), (UInt32) src.size() );
        }
        else
        {
            return LinkTo( src.c_str(), (UInt32) src.size() - 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer& 
CDynamicBuffer::LinkTo( const CDynamicBuffer& src )
{GUCEF_TRACE;

    return LinkTo( src.GetConstBufferPtr(), src.GetDataSize() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

 

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
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#ifndef GUCEF_CORE_CMEMORYBLOCKPOOL_H
#include "CMemoryBlockPool.h"   
#define GUCEF_CORE_CMEMORYBLOCKPOOL_H
#endif /* GUCEF_CORE_CMEMORYBLOCKPOOL_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CMemoryBlockPool::CMemoryBlockPool( void )
        : _pool( GUCEF_NULL )   ,
          _capacity( 0 )  ,
          _blocksize( 0 )
{
        GUCEF_BEGIN;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CMemoryBlockPool::CMemoryBlockPool( const CMemoryBlockPool& src )
{
        GUCEF_BEGIN;
        *this = src;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CMemoryBlockPool&
CMemoryBlockPool::operator=( const CMemoryBlockPool& src )
{
        GUCEF_BEGIN;
        GUCEF_CHECKMEM( _pool, _capacity*_blocksize );
        _capacity = src._capacity;
        _blocksize = src._blocksize;
        free( _pool );
        _pool = (UInt8*) malloc( _capacity*_blocksize );
        memcpy( _pool, src._pool, _capacity*_blocksize );
        GUCEF_CHECKMEM( _pool, _capacity*_blocksize );
        GUCEF_END;
        return *this;
}

/*-------------------------------------------------------------------------*/

CMemoryBlockPool::CMemoryBlockPool( UInt32 capacity  ,
                                    UInt32 blocksize )
        : _capacity( capacity )  ,
          _blocksize( blocksize )
{
        GUCEF_BEGIN;
        _pool = (UInt8*) malloc( capacity*blocksize );
        GUCEF_CHECKMEM( _pool, capacity*blocksize );
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CMemoryBlockPool::~CMemoryBlockPool()
{
        GUCEF_BEGIN;
        GUCEF_CHECKMEM( _pool, _capacity*_blocksize );
        free( _pool );
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

void
CMemoryBlockPool::SetBlock( UInt32 blockindex ,
                            const void* dest  ,
                            UInt32 size       )
{
        GUCEF_BEGIN;
        GUCEF_CHECKMEMSEG( _pool, _pool+(blockindex*_blocksize), size );
        memcpy( _pool+(blockindex*_blocksize) ,
                dest                          ,
                size                          );
        GUCEF_END;                
}

/*-------------------------------------------------------------------------*/

void
CMemoryBlockPool::SetBlock( UInt32 blockindex ,
                            const void* dest  )
{
        GUCEF_BEGIN;
        GUCEF_CHECKMEMSEG( _pool, _pool+(blockindex*_blocksize), _blocksize );
        memcpy( _pool+(blockindex*_blocksize) ,
                dest                          ,
                _blocksize                    );
        GUCEF_END;                
}

/*-------------------------------------------------------------------------*/

void*
CMemoryBlockPool::GetBlock( UInt32 blockindex ) const
{
        GUCEF_BEGIN;
        GUCEF_CHECKMEMSEG( _pool, (_pool+(blockindex*_blocksize)), _blocksize );
        GUCEF_END;        
        return (_pool+(blockindex*_blocksize));
}

/*-------------------------------------------------------------------------*/

void
CMemoryBlockPool::SetCapacity( UInt32 capacity )
{
        GUCEF_BEGIN;
        if ( capacity == _capacity )
        {
                GUCEF_END; 
                return;
        }                
        _pool = (UInt8*) realloc( _pool, capacity*_blocksize );
        GUCEF_CHECKMEM( _pool, capacity*_blocksize );
        _capacity = capacity;        
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

UInt32
CMemoryBlockPool::GetCapacity( void ) const
{
        GUCEF_BEGIN;
        GUCEF_END;
        return _capacity;
}

/*-------------------------------------------------------------------------*/

bool
CMemoryBlockPool::SetBlockSize( UInt32 blocksize ,
                                bool movedata    )
{
        GUCEF_BEGIN;
        if ( blocksize == _blocksize ) 
        {
                GUCEF_END;
                return true;
        }                
        if ( blocksize > _blocksize )
        {                
                if ( movedata )
                {
                        UInt8* pool = _pool, *newpool = (UInt8*) malloc( _capacity*blocksize );
                        GUCEF_CHECKMEM( _pool, _capacity*_blocksize );
                        GUCEF_CHECKMEM( newpool, _capacity*blocksize );
                        for ( UInt32 i=0; i<_capacity; ++i )
                        {
                                memcpy( newpool, pool, _blocksize );
                                newpool += blocksize;
                                pool += _blocksize;
                        }
                        free( _pool );
                        _pool = newpool;                        
                }  
                else
                {
                        _pool = (UInt8*) realloc( _pool, _capacity*blocksize );
                        GUCEF_CHECKMEM( _pool, _capacity*blocksize );
                }
                _blocksize = blocksize;
                GUCEF_END;
                return true;                      
        }
        
        if ( movedata ) 
        {
                GUCEF_END;
                return false;
        }                
        _pool = (UInt8*) realloc( _pool, _capacity*blocksize );
        GUCEF_CHECKMEM( _pool, _capacity*blocksize );
        _blocksize = blocksize;
        GUCEF_END;
        return true;                                
}

/*-------------------------------------------------------------------------*/

UInt32
CMemoryBlockPool::GetBlockSize( void ) const
{
        GUCEF_BEGIN;
        GUCEF_END;
        return _blocksize;
}

/*-------------------------------------------------------------------------*/

void
CMemoryBlockPool::ShiftBlocksUp( void )
{
        GUCEF_BEGIN;
        if ( _capacity > 1 )
        {
                UInt8* i = (_pool+((_capacity-2)*_blocksize));
                while ( i >= _pool )
                {
                        GUCEF_CHECKMEMSEG( _pool, i, _blocksize );
                        GUCEF_CHECKMEMSEG( _pool, i+_blocksize, _blocksize );
                        memcpy( i+_blocksize ,
                                i            ,
                                _blocksize   );
                        i -= _blocksize;
                }
        }
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

void
CMemoryBlockPool::ShiftBlocksDown( void )
{
        GUCEF_BEGIN;
        if ( _capacity > 1 )
        {
                UInt8* i = _pool+((_capacity-2)*_blocksize);
                while ( i >= _pool )
                {
                        GUCEF_CHECKMEMSEG( _pool, i, _blocksize );
                        GUCEF_CHECKMEMSEG( _pool, i+_blocksize, _blocksize );                
                        memcpy( i            ,
                                i+_blocksize ,
                                _blocksize   );
                        i -= _blocksize;
                }
        }
        GUCEF_END;        
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
 
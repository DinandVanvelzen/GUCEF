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

#include "CBlockStack.h"

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

CBlockStack::CBlockStack( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBlockStack::CBlockStack( UInt32 capacity  ,
                          UInt32 blocksize )
        : _pool( capacity  ,
                 blocksize ) ,
          _itemcount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBlockStack::~CBlockStack()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CBlockStack::Push( const void *srcdata )
{GUCEF_TRACE;                           
       
        if ( _itemcount == _pool.GetCapacity() )
        {
                _pool.SetCapacity( _pool.GetCapacity()+5 );
        }
        _pool.ShiftBlocksUp();
        _pool.SetBlock( 0, srcdata );
        ++_itemcount;
}

/*-------------------------------------------------------------------------*/

bool
CBlockStack::Pop( void* datadest )
{GUCEF_TRACE;

        if ( _itemcount )
        {
                memcpy( datadest             ,
                        _pool.GetBlock( 0 )  ,
                        _pool.GetBlockSize() );
                _pool.ShiftBlocksDown();
                --_itemcount;
                return true;
        }
        return false;
}

/*-------------------------------------------------------------------------*/

bool
CBlockStack::GetTop( void* datadest )
{GUCEF_TRACE;

        if ( _itemcount )
        {
                memcpy( datadest                       ,
                        _pool.GetBlock( _itemcount-1 ) ,
                        _pool.GetBlockSize()           );
                --_itemcount;
                return true;
        }
        return false;
}

/*-------------------------------------------------------------------------*/

void 
CBlockStack::SetTop( const void *srcdata )
{GUCEF_TRACE;

        if ( _itemcount == _pool.GetCapacity() )
        {
                _pool.SetCapacity( _pool.GetCapacity()+5 );
        }
        _pool.SetBlock( _itemcount, srcdata );
        ++_itemcount;
}

/*-------------------------------------------------------------------------*/

void* 
CBlockStack::PeekTop( void ) const
{GUCEF_TRACE;

        if ( _itemcount )
        {
                return _pool.GetBlock( _itemcount-1 );
        }
        return GUCEF_NULL;                
}

/*-------------------------------------------------------------------------*/       

void* 
CBlockStack::Peek( void ) const
{GUCEF_TRACE;

        if ( _itemcount )
        {                
                return _pool.GetBlock( 0 );
        }
        return GUCEF_NULL;                 
}

/*-------------------------------------------------------------------------*/

UInt32
CBlockStack::GetItemCount( void ) const
{GUCEF_TRACE;

        return _itemcount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


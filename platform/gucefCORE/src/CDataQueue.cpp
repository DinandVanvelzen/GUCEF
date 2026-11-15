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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "CDataQueue.h"

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

CDataQueue::CDataQueue( void )
        : _queue( 0, sizeof( CDynamicBuffer* ) )       ,
          _freebuffers( 0, sizeof( CDynamicBuffer* ) )
{
        GUCEF_BEGIN;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CDataQueue::~CDataQueue()
{
        GUCEF_BEGIN;
        CDynamicBuffer* buffer;
        while ( _queue.Pop( &buffer ) ) 
        {
                GUCEF_DELETE buffer;
        }
        while ( _freebuffers.Pop( &buffer ) ) 
        {
                GUCEF_DELETE buffer;
        }
        GUCEF_END;                
}
        
/*-------------------------------------------------------------------------*/        

void 
CDataQueue::Set( const void* src ,
                 UInt32 datasize )
{
        GUCEF_BEGIN;
        CDynamicBuffer* buffer;
        if ( !_freebuffers.Pop( &buffer ) )
        {
                // no more free buffers are available
                buffer = GUCEF_NEW CDynamicBuffer( true );                              
        }
        
        buffer->CopyFrom( datasize, src );        
        _queue.SetTop( &buffer );
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/                 
                                 
void 
CDataQueue::Get( const void** dataptr ,
                 UInt32* datasize     )
{
        GUCEF_BEGIN;
        CDynamicBuffer* buffer = static_cast<CDynamicBuffer*>(_queue.Peek());
        
        if ( buffer )
        {
                *dataptr = buffer->GetBufferPtr();
                *datasize = buffer->GetBufferSize();
                
                _freebuffers.Push( &buffer );
                return;                
        }
        
        *dataptr = GUCEF_NULL;
        *datasize = 0;
        GUCEF_END;      
}

/*-------------------------------------------------------------------------*/

void 
CDataQueue::Drop( void )
{
        GUCEF_BEGIN;
        CDynamicBuffer* buffer;
        if ( _queue.Pop( &buffer ) )
        {
                _freebuffers.Push( &buffer );        
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
                  
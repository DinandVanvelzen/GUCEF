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

#include "gucefCORE_CLinkedTransferBuffer.h"

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

CLinkedTransferBuffer::CLinkedTransferBuffer( void )
    : CTLinkedCloneableObj< CDynamicBuffer >() ,
      m_transferProgress( 0 )                  ,
      m_additionalData( GUCEF_NULL )                 ,
      m_totalSize( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkedTransferBuffer::CLinkedTransferBuffer( const CDynamicBuffer* buffer                   ,
                                              const Float32 transferProgress /* = 0 */       ,
                                              const Int64 totalSize /* = -1 */               ,
                                              const CICloneable* additionalData /* = NULL */ )
    : CTLinkedCloneableObj< CDynamicBuffer >() ,
      m_transferProgress( transferProgress )   ,
      m_additionalData( GUCEF_NULL )                 ,
      m_totalSize( totalSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL != additionalData )
    {
        m_additionalData = additionalData->Clone(); 
    }
}

/*-------------------------------------------------------------------------*/

CLinkedTransferBuffer::CLinkedTransferBuffer( const CLinkedTransferBuffer& src )
    : CTLinkedCloneableObj< CDynamicBuffer >( src ) ,
      m_transferProgress( src.m_transferProgress )  ,
      m_additionalData( GUCEF_NULL )                      ,
      m_totalSize( src.m_totalSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL != src.m_additionalData )
    {
        m_additionalData = src.m_additionalData->Clone(); 
    }
}

/*-------------------------------------------------------------------------*/

CLinkedTransferBuffer::~CLinkedTransferBuffer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkedTransferBuffer&
CLinkedTransferBuffer::operator=( const CLinkedTransferBuffer& src )
{GUCEF_TRACE;

    CTLinkedCloneableObj< CDynamicBuffer >::operator=( src );
    
    m_transferProgress = src.m_transferProgress;
    m_totalSize = src.m_totalSize;
    
    if ( GUCEF_NULL != src.m_additionalData )
    {
        m_additionalData = src.m_additionalData->Clone(); 
    }
    else
    {
        m_additionalData = GUCEF_NULL;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/
    
CICloneable*
CLinkedTransferBuffer::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLinkedTransferBuffer( *this );
}

/*-------------------------------------------------------------------------*/

void
CLinkedTransferBuffer::SetTotalSize( const Int64 totalSize )
{GUCEF_TRACE;

    m_totalSize = totalSize;
}

/*-------------------------------------------------------------------------*/
    
Int64
CLinkedTransferBuffer::GetTotalSize( void ) const
{GUCEF_TRACE;
    
    return m_totalSize;
}

/*-------------------------------------------------------------------------*/

void
CLinkedTransferBuffer::SetTransferProgress( const Float32 transferProgress )
{GUCEF_TRACE;

    m_transferProgress = transferProgress;
}

/*-------------------------------------------------------------------------*/
    
Float32
CLinkedTransferBuffer::GetTransferProgress( void ) const
{GUCEF_TRACE;

    return m_transferProgress;
}

/*-------------------------------------------------------------------------*/
    
const CICloneable*
CLinkedTransferBuffer::GetAdditionalData( void ) const
{GUCEF_TRACE;

    return m_additionalData;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
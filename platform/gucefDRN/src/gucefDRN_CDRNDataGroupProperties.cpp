/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNDataGroupProperties.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {
    
/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNDataGroupProperties::CDRNDataGroupProperties( void )
    : m_emitAsGroup( false )                 ,
      m_acceptNewStreamerItems( true )       ,
      m_acceptStreamerUpdates( true )        ,
      m_acceptNewPeerItems( false )          ,
      m_allowUnreliableTransmission( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGroupProperties::CDRNDataGroupProperties( const CDRNDataGroupProperties& src )
    : m_emitAsGroup( src.m_emitAsGroup )                                 ,
      m_acceptNewStreamerItems( src.m_acceptNewStreamerItems )           ,
      m_acceptStreamerUpdates( src.m_acceptStreamerUpdates )             ,
      m_acceptNewPeerItems( src.m_acceptNewPeerItems )                   ,
      m_allowUnreliableTransmission( src.m_allowUnreliableTransmission )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDRNDataGroupProperties::~CDRNDataGroupProperties()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CDRNDataGroupProperties&
CDRNDataGroupProperties::operator=( const CDRNDataGroupProperties& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_emitAsGroup = src.m_emitAsGroup;
        m_acceptNewStreamerItems = src.m_acceptNewStreamerItems;
        m_acceptStreamerUpdates = src.m_acceptStreamerUpdates;
        m_acceptNewPeerItems = src.m_acceptNewPeerItems;
        m_allowUnreliableTransmission = src.m_allowUnreliableTransmission;       
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGroupProperties::SetEmitEntireGroupOnChange( const bool emitEntireGroup )
{GUCEF_TRACE;

    m_emitAsGroup = emitEntireGroup;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGroupProperties::GetEmitEntireGroupOnChange( void ) const
{GUCEF_TRACE;

    return m_emitAsGroup;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGroupProperties::SetAcceptNewStreamerItems( const bool acceptNewStreamerItems )
{GUCEF_TRACE;

    m_acceptNewStreamerItems = acceptNewStreamerItems;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGroupProperties::GetAcceptNewStreamerItems( void ) const
{GUCEF_TRACE;

    return m_acceptNewStreamerItems;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGroupProperties::SetAcceptUpdatesFromStreamers( const bool acceptStreamerUpdates )
{GUCEF_TRACE;

    m_acceptStreamerUpdates = acceptStreamerUpdates;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGroupProperties::GetAcceptUpdatesFromStreamers( void ) const
{GUCEF_TRACE;

    return m_acceptStreamerUpdates;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGroupProperties::SetAcceptNewPeerItems( const bool acceptNewPeerItems )
{GUCEF_TRACE;

    m_acceptNewPeerItems = acceptNewPeerItems;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGroupProperties::GetAcceptNewPeerItems( void ) const
{GUCEF_TRACE;

    return m_acceptNewPeerItems;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGroupProperties::SetAllowUnreliableTransmission( const bool allowUnreliableTransmission )
{GUCEF_TRACE;

    m_allowUnreliableTransmission = allowUnreliableTransmission;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGroupProperties::GetAllowUnreliableTransmission( void ) const
{GUCEF_TRACE;

    return m_allowUnreliableTransmission;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#include "gucefDRN_CDRNDataStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNDataStream::DataReceivedEvent = "GUCEF::DRN::CDRNDataStream::DataReceivedEvent";
const CORE::CEvent CDRNDataStream::DataTransmittedEvent = "GUCEF::DRN::CDRNDataStream::DataTransmittedEvent";
    
/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/
   
CDRNDataStream::CDRNDataStream( const CORE::CString& streamName )
    : CObservingNotifier()      ,
      m_streamName( streamName )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/
    
CDRNDataStream::~CDRNDataStream()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::RegisterEvents( void )
{GUCEF_TRACE;

    DataReceivedEvent.Initialize();
    DataTransmittedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnNotify( CORE::CNotifier* notifier                 ,
                          const CORE::CEvent& eventid               ,
                          CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // Call base-class implementation,.. mandatory
    CObservingNotifier::OnNotify( notifier  ,
                                  eventid   ,
                                  eventdata );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::SendData( const void* dataSource                   ,
                          const UInt16 dataSize                    ,
                          const bool allowUnreliable /* = false */ )
{GUCEF_TRACE;

    // First we link up a buffer for safe emission of the event data
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( dataSource, dataSize );
    
    // Emit the data transmission event
    DataTransmittedEventData eData( &buffer );    
    NotifyObservers( DataTransmittedEvent, &eData );
    return true;
}

/*-------------------------------------------------------------------------*/
    
const CORE::CString&
CDRNDataStream::GetName( void ) const
{GUCEF_TRACE;

    return m_streamName;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnDataReceived( const char* data      ,
                                const UInt32 dataSize )
{GUCEF_TRACE;

    // First we link up a buffer for safe emission of the event data
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( data, dataSize );
    
    // Emit the data received event
    DataReceivedEventData eData( &buffer );    
    NotifyObservers( DataReceivedEvent, &eData );    
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

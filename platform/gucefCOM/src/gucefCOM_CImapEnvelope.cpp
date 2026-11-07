/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CImapEnvelope.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
ReadAddressList( const CImapValue& list, CEMailAddress::TEMailAddressVector& addresses )
{
    if ( !list.IsList() )
        return;
    for ( UInt32 i=0; i<list.GetListSize() && addresses.size() < 500; ++i )
    {
        const CImapValue& entry = list.GetElement( i );
        if ( !entry.IsList() || entry.GetListSize() < 4 )
            continue;
        // personal name, route, mailbox, host: a NIL host is the start or the end of a group
        if ( entry.GetElement( 3 ).IsNil() || entry.GetElement( 2 ).IsNil() )
            continue;
        CEMailAddress address;
        address.displayName = CMimeCodec::DecodeHeaderText( entry.GetElement( 0 ).GetString() );
        address.localPart = entry.GetElement( 2 ).GetString();
        address.domain = entry.GetElement( 3 ).GetString();
        if ( address.IsValid( true ) )
            addresses.push_back( address );
    }
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapEnvelope::CImapEnvelope( void )
    : date()
    , subject()
    , from()
    , sender()
    , replyTo()
    , to()
    , cc()
    , bcc()
    , inReplyTo()
    , messageId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapEnvelope::FromValue( const CImapValue& value, CImapEnvelope& envelope )
{GUCEF_TRACE;

    envelope = CImapEnvelope();
    if ( !value.IsList() || value.GetListSize() < 10 )
        return false;
    envelope.date = value.GetElement( 0 ).GetString();
    envelope.subject = CMimeCodec::DecodeHeaderText( value.GetElement( 1 ).GetString() );
    ReadAddressList( value.GetElement( 2 ), envelope.from );
    ReadAddressList( value.GetElement( 3 ), envelope.sender );
    ReadAddressList( value.GetElement( 4 ), envelope.replyTo );
    ReadAddressList( value.GetElement( 5 ), envelope.to );
    ReadAddressList( value.GetElement( 6 ), envelope.cc );
    ReadAddressList( value.GetElement( 7 ), envelope.bcc );
    envelope.inReplyTo = value.GetElement( 8 ).GetString();
    envelope.messageId = value.GetElement( 9 ).GetString();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

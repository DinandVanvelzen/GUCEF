/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CChatMessage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChatMessage::CChatMessage( void )
    : CORE::CIDataNodeSerializable()
    , channel()
    , isDirect( false )
    , from()
    , to()
    , text()
    , time()
    , id()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatMessage::CChatMessage( const CChatMessage& src )
    : CORE::CIDataNodeSerializable( src )
    , channel( src.channel )
    , isDirect( src.isDirect )
    , from( src.from )
    , to( src.to )
    , text( src.text )
    , time( src.time )
    , id( src.id )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatMessage::~CChatMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChatMessage&
CChatMessage::operator=( const CChatMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        channel = src.channel;
        isDirect = src.isDirect;
        from = src.from;
        to = src.to;
        text = src.text;
        time = src.time;
        id = src.id;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CChatMessage::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "channel", channel );
    domRootNode.SetAttribute( "isDirect", isDirect );
    domRootNode.SetAttribute( "from", from );
    domRootNode.SetAttribute( "to", to );
    domRootNode.SetAttribute( "text", text );
    domRootNode.SetAttribute( "time", time.ToIso8601DateTimeString( true, true ) );
    domRootNode.SetAttribute( "id", id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChatMessage::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    channel = domRootNode.GetAttributeValueOrChildValueByName( "channel" ).AsString( CORE::CString::Empty, true );
    isDirect = domRootNode.GetAttributeValueOrChildValueByName( "isDirect" ).AsBool( false, true );
    from = domRootNode.GetAttributeValueOrChildValueByName( "from" ).AsString( CORE::CString::Empty, true );
    to = domRootNode.GetAttributeValueOrChildValueByName( "to" ).AsString( CORE::CString::Empty, true );
    text = domRootNode.GetAttributeValueOrChildValueByName( "text" ).AsString( CORE::CString::Empty, true );
    CORE::CString timeStr = domRootNode.GetAttributeValueOrChildValueByName( "time" ).AsString( CORE::CString::Empty, true );
    if ( !timeStr.IsNULLOrEmpty() )
        time.FromIso8601DateTimeString( timeStr );
    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString( CORE::CString::Empty, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CChatMessage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CChatMessage( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CChatMessage::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CChatMessage";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

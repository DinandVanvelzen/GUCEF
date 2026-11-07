/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#include "matchprovidermail_CEMailRefusal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
GetText( const CORE::CDataNode& node, const char* name )
{GUCEF_TRACE;

    return node.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailRefusal::CEMailRefusal( void ) : CORE::CIDataNodeSerializable(), revision( 0 ), sha256(), code(), reason(), at( 0 ), file() {}
CEMailRefusal::CEMailRefusal( const CEMailRefusal& src ) : CORE::CIDataNodeSerializable( src ), revision( src.revision ), sha256( src.sha256 ), code( src.code ), reason( src.reason ), at( src.at ), file( src.file ) {}
CEMailRefusal::~CEMailRefusal() {}
CEMailRefusal& CEMailRefusal::operator=( const CEMailRefusal& src ) { if ( &src != this ) { revision = src.revision; sha256 = src.sha256; code = src.code; reason = src.reason; at = src.at; file = src.file; } return *this; }
bool
CEMailRefusal::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "sha256", sha256 );
    domRootNode.SetAttribute( "code", code );
    domRootNode.SetAttribute( "reason", reason );
    domRootNode.SetAttribute( "at", at );
    domRootNode.SetAttribute( "file", file );
    return true;
}
bool
CEMailRefusal::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    sha256 = GetText( domRootNode, "sha256" );
    code = GetText( domRootNode, "code" );
    reason = GetText( domRootNode, "reason" );
    at = domRootNode.GetAttributeValueOrChildValueByName( "at" ).AsInt64( 0, true );
    file = GetText( domRootNode, "file" );
    return !code.IsNULLOrEmpty();
}
CORE::CICloneable* CEMailRefusal::Clone( void ) const { return GUCEF_NEW CEMailRefusal( *this ); }
const CORE::CString& CEMailRefusal::GetClassTypeName( void ) const { static const CORE::CString n = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailRefusal"; return n; }
const char* CEMailRefusal::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CEMailRefusal::CloneableTypeName( void ) { return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailRefusal"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

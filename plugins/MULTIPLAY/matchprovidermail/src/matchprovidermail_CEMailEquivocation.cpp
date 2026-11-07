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

#include "matchprovidermail_CEMailEquivocation.h"

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

CEMailEquivocation::CEMailEquivocation( void ) : CORE::CIDataNodeSerializable(), revision( 0 ), issuerKey(), issuerUserId(), firstSha256(), secondSha256(), file() {}
CEMailEquivocation::CEMailEquivocation( const CEMailEquivocation& src ) : CORE::CIDataNodeSerializable( src ), revision( src.revision ), issuerKey( src.issuerKey ), issuerUserId( src.issuerUserId ), firstSha256( src.firstSha256 ), secondSha256( src.secondSha256 ), file( src.file ) {}
CEMailEquivocation::~CEMailEquivocation() {}
CEMailEquivocation& CEMailEquivocation::operator=( const CEMailEquivocation& src ) { if ( &src != this ) { revision = src.revision; issuerKey = src.issuerKey; issuerUserId = src.issuerUserId; firstSha256 = src.firstSha256; secondSha256 = src.secondSha256; file = src.file; } return *this; }
bool
CEMailEquivocation::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "revision", revision );
    domRootNode.SetAttribute( "issuerKey", issuerKey );
    domRootNode.SetAttribute( "issuerUserId", issuerUserId );
    domRootNode.SetAttribute( "firstSha256", firstSha256 );
    domRootNode.SetAttribute( "secondSha256", secondSha256 );
    domRootNode.SetAttribute( "file", file );
    return true;
}
bool
CEMailEquivocation::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt32( 0, true );
    issuerKey = GetText( domRootNode, "issuerKey" );
    issuerUserId = GetText( domRootNode, "issuerUserId" );
    firstSha256 = GetText( domRootNode, "firstSha256" );
    secondSha256 = GetText( domRootNode, "secondSha256" );
    file = GetText( domRootNode, "file" );
    return !issuerKey.IsNULLOrEmpty();
}
CORE::CICloneable* CEMailEquivocation::Clone( void ) const { return GUCEF_NEW CEMailEquivocation( *this ); }
const CORE::CString& CEMailEquivocation::GetClassTypeName( void ) const { static const CORE::CString n = "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailEquivocation"; return n; }
const char* CEMailEquivocation::GetCloneableTypeName( void ) const { return CloneableTypeName(); }
const char* CEMailEquivocation::CloneableTypeName( void ) { return "GUCEF::MULTIPLAYPLUGIN::MAIL::CEMailEquivocation"; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

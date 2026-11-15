/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#include "gucefASSETSTORE_CAssetSignInPrompt.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetSignInPrompt::CAssetSignInPrompt( void )
    : CORE::CIDataNodeSerializable()
    , promptId()
    , storeId()
    , kind()
    , text()
    , url()
    , code()
    , expiresUtcMs( 0 )
    , sensitiveAnswer( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSignInPrompt::~CAssetSignInPrompt()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSignInPrompt::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSignInPrompt();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSignInPrompt::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "promptId", promptId ) && success;
    success = domRootNode.SetAttribute( "storeId", storeId ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetAttribute( "text", text ) && success;
    success = domRootNode.SetAttribute( "url", url ) && success;
    success = domRootNode.SetAttribute( "code", code ) && success;
    success = domRootNode.SetAttribute( "expiresUtcMs", expiresUtcMs ) && success;
    success = domRootNode.SetAttribute( "sensitiveAnswer", sensitiveAnswer ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSignInPrompt::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    promptId = domRootNode.GetAttributeValueOrChildValueByName( "promptId" ).AsString( promptId );
    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString( storeId );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( kind );
    text = domRootNode.GetAttributeValueOrChildValueByName( "text" ).AsString( text );
    url = domRootNode.GetAttributeValueOrChildValueByName( "url" ).AsString( url );
    code = domRootNode.GetAttributeValueOrChildValueByName( "code" ).AsString( code );
    expiresUtcMs = domRootNode.GetAttributeValueOrChildValueByName( "expiresUtcMs" ).AsInt64( expiresUtcMs, true );
    sensitiveAnswer = domRootNode.GetAttributeValueOrChildValueByName( "sensitiveAnswer" ).AsBool( sensitiveAnswer, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSignInPrompt::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSignInPrompt( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSignInPrompt::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSignInPrompt::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSignInPrompt";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSignInPrompt::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSignInPrompt";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

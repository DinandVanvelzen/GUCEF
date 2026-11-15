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
#include "gucefASSETSTORE_CAssetCategoryRule.h"

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

CAssetCategoryRule::CAssetCategoryRule( void )
    : CORE::CIDataNodeSerializable()
    , ruleId()
    , matchOn()
    , pattern()
    , categoryId()
    , kind()
    , themes()
    , artStyles()
    , priority( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetCategoryRule::~CAssetCategoryRule()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetCategoryRule::Clear( void )
{GUCEF_TRACE;

    *this = CAssetCategoryRule();
}

/*-------------------------------------------------------------------------*/

bool
CAssetCategoryRule::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "ruleId", ruleId ) && success;
    success = domRootNode.SetAttribute( "matchOn", matchOn ) && success;
    success = domRootNode.SetAttribute( "pattern", pattern ) && success;
    success = domRootNode.SetAttribute( "categoryId", categoryId ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetValuesOfChildByName( "themes", themes ) && success;
    success = domRootNode.SetValuesOfChildByName( "artStyles", artStyles ) && success;
    success = domRootNode.SetAttribute( "priority", priority ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCategoryRule::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    ruleId = domRootNode.GetAttributeValueOrChildValueByName( "ruleId" ).AsString( ruleId );
    matchOn = domRootNode.GetAttributeValueOrChildValueByName( "matchOn" ).AsString( matchOn );
    pattern = domRootNode.GetAttributeValueOrChildValueByName( "pattern" ).AsString( pattern );
    categoryId = domRootNode.GetAttributeValueOrChildValueByName( "categoryId" ).AsString( categoryId );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( kind );
    domRootNode.GetValuesOfChildByName( "themes", themes );
    domRootNode.GetValuesOfChildByName( "artStyles", artStyles );
    priority = domRootNode.GetAttributeValueOrChildValueByName( "priority" ).AsInt64( priority, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetCategoryRule::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetCategoryRule( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetCategoryRule::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetCategoryRule::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetCategoryRule";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetCategoryRule::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetCategoryRule";
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

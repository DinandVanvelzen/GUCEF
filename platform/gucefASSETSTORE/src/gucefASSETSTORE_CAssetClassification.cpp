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
#include "gucefASSETSTORE_CAssetClassification.h"

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

CAssetClassification::CAssetClassification( void )
    : CORE::CIDataNodeSerializable()
    , categories()
    , kind()
    , themes()
    , artStyles()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetClassification::~CAssetClassification()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetClassification::Clear( void )
{GUCEF_TRACE;

    *this = CAssetClassification();
}

/*-------------------------------------------------------------------------*/

bool
CAssetClassification::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetValuesOfChildByName( "categories", categories ) && success;
    success = domRootNode.SetAttribute( "kind", kind ) && success;
    success = domRootNode.SetValuesOfChildByName( "themes", themes ) && success;
    success = domRootNode.SetValuesOfChildByName( "artStyles", artStyles ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetClassification::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetClassification defaults;
    categories.clear();
    domRootNode.GetValuesOfChildByName( "categories", categories );
    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( defaults.kind );
    themes.clear();
    domRootNode.GetValuesOfChildByName( "themes", themes );
    artStyles.clear();
    domRootNode.GetValuesOfChildByName( "artStyles", artStyles );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetClassification::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetClassification( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetClassification::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetClassification";
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

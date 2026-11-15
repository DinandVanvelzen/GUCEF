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
#include "gucefASSETSTORE_CAssetVocabularyInfo.h"

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

CAssetVocabularyInfo::CAssetVocabularyInfo( void )
    : CORE::CIDataNodeSerializable()
    , revision( 0 )
    , kinds()
    , categories()
    , themes()
    , styles()
    , licenceClasses()
    , restrictionFlags()
    , mappingCountsBySource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetVocabularyInfo::~CAssetVocabularyInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetVocabularyInfo::Clear( void )
{GUCEF_TRACE;

    *this = CAssetVocabularyInfo();
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabularyInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    success = domRootNode.SetValuesOfChildByName( "kinds", kinds ) && success;
    success = domRootNode.SetValuesOfChildByName( "categories", categories ) && success;
    success = domRootNode.SetValuesOfChildByName( "themes", themes ) && success;
    success = domRootNode.SetValuesOfChildByName( "styles", styles ) && success;
    success = domRootNode.SetValuesOfChildByName( "licenceClasses", licenceClasses ) && success;
    success = domRootNode.SetValuesOfChildByName( "restrictionFlags", restrictionFlags ) && success;
    CORE::CDataNode* mappingCountsBySourceNode = domRootNode.AddChild( "mappingCountsBySource", GUCEF_DATATYPE_OBJECT );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator mappingCountsBySourceEntry = mappingCountsBySource.begin();
    while ( GUCEF_NULL != mappingCountsBySourceNode && mappingCountsBySourceEntry != mappingCountsBySource.end() )
    {
        success = mappingCountsBySourceNode->SetAttribute( (*mappingCountsBySourceEntry).first, (*mappingCountsBySourceEntry).second ) && success;
        ++mappingCountsBySourceEntry;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabularyInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( revision, true );
    domRootNode.GetValuesOfChildByName( "kinds", kinds );
    domRootNode.GetValuesOfChildByName( "categories", categories );
    domRootNode.GetValuesOfChildByName( "themes", themes );
    domRootNode.GetValuesOfChildByName( "styles", styles );
    domRootNode.GetValuesOfChildByName( "licenceClasses", licenceClasses );
    domRootNode.GetValuesOfChildByName( "restrictionFlags", restrictionFlags );
    const CORE::CDataNode* mappingCountsBySourceNode = domRootNode.FindChild( "mappingCountsBySource" );
    for ( UInt32 a=0; GUCEF_NULL != mappingCountsBySourceNode && a<mappingCountsBySourceNode->GetAttCount(); ++a )
        mappingCountsBySource[ mappingCountsBySourceNode->GetAttribute( a )->first ] = mappingCountsBySourceNode->GetAttribute( a )->second.AsUInt32( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetVocabularyInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetVocabularyInfo( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetVocabularyInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetVocabularyInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetVocabularyInfo";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetVocabularyInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetVocabularyInfo";
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

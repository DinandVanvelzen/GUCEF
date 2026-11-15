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
#include "gucefASSETSTORE_CAssetConversionJobRequest.h"

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

CAssetConversionJobRequest::CAssetConversionJobRequest( void )
    : CORE::CIDataNodeSerializable()
    , packId()
    , assetIds()
    , allPending( false )
    , cookProfile()
    , keep()
    , force( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionJobRequest::~CAssetConversionJobRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetConversionJobRequest::Clear( void )
{GUCEF_TRACE;

    *this = CAssetConversionJobRequest();
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionJobRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                      const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetValuesOfChildByName( "assetIds", assetIds ) && success;
    success = domRootNode.SetAttribute( "allPending", allPending ) && success;
    success = domRootNode.SetAttribute( "cookProfile", cookProfile ) && success;
    success = domRootNode.SetAttribute( "keep", keep ) && success;
    success = domRootNode.SetAttribute( "force", force ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionJobRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    domRootNode.GetValuesOfChildByName( "assetIds", assetIds );
    allPending = domRootNode.GetAttributeValueOrChildValueByName( "allPending" ).AsBool( allPending, true );
    cookProfile = domRootNode.GetAttributeValueOrChildValueByName( "cookProfile" ).AsString( cookProfile );
    keep = domRootNode.GetAttributeValueOrChildValueByName( "keep" ).AsString( keep );
    force = domRootNode.GetAttributeValueOrChildValueByName( "force" ).AsBool( force, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetConversionJobRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetConversionJobRequest( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetConversionJobRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetConversionJobRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetConversionJobRequest";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetConversionJobRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetConversionJobRequest";
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

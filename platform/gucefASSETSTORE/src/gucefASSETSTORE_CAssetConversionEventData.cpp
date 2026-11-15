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

#include "gucefASSETSTORE_CAssetConversionEventData.h"

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

CAssetConversionEventData::CAssetConversionEventData( void )
    : CORE::CIDataNodeSerializable()
    , jobId()
    , packId()
    , forIngest( false )
    , done( 0 )
    , total( 0 )
    , assetPath()
    , status()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionEventData::~CAssetConversionEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetConversionEventData::Clear( void )
{GUCEF_TRACE;

    *this = CAssetConversionEventData();
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionEventData::Serialize( CORE::CDataNode& domRootNode                        ,
                                      const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "jobId", jobId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "forIngest", forIngest ) && success;
    success = domRootNode.SetAttribute( "done", done ) && success;
    success = domRootNode.SetAttribute( "total", total ) && success;
    success = domRootNode.SetAttribute( "assetPath", assetPath ) && success;
    success = domRootNode.SetAttribute( "status", status ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionEventData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetConversionEventData defaults;
    jobId = domRootNode.GetAttributeValueOrChildValueByName( "jobId" ).AsString( defaults.jobId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( defaults.packId );
    forIngest = domRootNode.GetAttributeValueOrChildValueByName( "forIngest" ).AsBool( defaults.forIngest, true );
    done = domRootNode.GetAttributeValueOrChildValueByName( "done" ).AsUInt32( defaults.done, true );
    total = domRootNode.GetAttributeValueOrChildValueByName( "total" ).AsUInt32( defaults.total, true );
    assetPath = domRootNode.GetAttributeValueOrChildValueByName( "assetPath" ).AsString( defaults.assetPath );
    status = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( defaults.status );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( defaults.message );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetConversionEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetConversionEventData( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetConversionEventData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetConversionEventData";
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

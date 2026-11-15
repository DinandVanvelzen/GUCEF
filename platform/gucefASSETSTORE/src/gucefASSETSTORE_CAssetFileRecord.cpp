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
#include "gucefASSETSTORE_CAssetFileRecord.h"

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

CAssetFileRecord::CAssetFileRecord( void )
    : CORE::CIDataNodeSerializable()
    , path()
    , sizeBytes( 0 )
    , digest()
    , role( "main" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetFileRecord::~CAssetFileRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetFileRecord::Clear( void )
{GUCEF_TRACE;

    *this = CAssetFileRecord();
}

/*-------------------------------------------------------------------------*/

bool
CAssetFileRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "path", path ) && success;
    success = domRootNode.SetAttribute( "sizeBytes", sizeBytes ) && success;
    success = domRootNode.SetAttribute( "digest", digest ) && success;
    success = domRootNode.SetAttribute( "role", role ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetFileRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetFileRecord defaults;
    path = domRootNode.GetAttributeValueOrChildValueByName( "path" ).AsString( defaults.path );
    sizeBytes = domRootNode.GetAttributeValueOrChildValueByName( "sizeBytes" ).AsInt64( defaults.sizeBytes, true );
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString( defaults.digest );
    role = domRootNode.GetAttributeValueOrChildValueByName( "role" ).AsString( defaults.role );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetFileRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetFileRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetFileRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetFileRecord";
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

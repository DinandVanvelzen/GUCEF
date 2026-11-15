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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#include "gucefASSETSTORE_CAssetThumbnailKey.h"

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

CAssetThumbnailKey::CAssetThumbnailKey( void )
    : CORE::CIDataNodeSerializable()
    , digest()
    , rendererId()
    , rendererVersion()
    , size( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetThumbnailKey::~CAssetThumbnailKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetThumbnailKey::Clear( void )
{GUCEF_TRACE;

    *this = CAssetThumbnailKey();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetThumbnailKey::ToString( void ) const
{GUCEF_TRACE;

    return digest + "-" + rendererId + "-" + rendererVersion + "-" + CORE::UInt32ToString( size );
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailKey::IsEmpty( void ) const
{GUCEF_TRACE;

    return digest.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailKey::operator==( const CAssetThumbnailKey& other ) const
{GUCEF_TRACE;

    return digest == other.digest && rendererId == other.rendererId && rendererVersion == other.rendererVersion && size == other.size;
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailKey::operator<( const CAssetThumbnailKey& other ) const
{GUCEF_TRACE;

    if ( digest != other.digest )
        return digest < other.digest;
    if ( rendererId != other.rendererId )
        return rendererId < other.rendererId;
    if ( rendererVersion != other.rendererVersion )
        return rendererVersion < other.rendererVersion;
    return size < other.size;
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailKey::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "digest", digest ) && success;
    success = domRootNode.SetAttribute( "rendererId", rendererId ) && success;
    success = domRootNode.SetAttribute( "rendererVersion", rendererVersion ) && success;
    success = domRootNode.SetAttribute( "size", size ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetThumbnailKey::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString( digest );
    rendererId = domRootNode.GetAttributeValueOrChildValueByName( "rendererId" ).AsString( rendererId );
    rendererVersion = domRootNode.GetAttributeValueOrChildValueByName( "rendererVersion" ).AsString( rendererVersion );
    size = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsUInt32( size, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetThumbnailKey::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetThumbnailKey( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetThumbnailKey::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetThumbnailKey::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetThumbnailKey";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetThumbnailKey::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetThumbnailKey";
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

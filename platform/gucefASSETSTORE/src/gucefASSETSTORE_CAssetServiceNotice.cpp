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
#include "gucefASSETSTORE_CAssetServiceNotice.h"

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

CAssetServiceNotice::CAssetServiceNotice( void )
    : CORE::CIDataNodeSerializable()
    , level()
    , code()
    , text()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceNotice::~CAssetServiceNotice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetServiceNotice::Clear( void )
{GUCEF_TRACE;

    *this = CAssetServiceNotice();
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceNotice::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "level", level ) && success;
    success = domRootNode.SetAttribute( "code", code ) && success;
    success = domRootNode.SetAttribute( "text", text ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceNotice::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    level = domRootNode.GetAttributeValueOrChildValueByName( "level" ).AsString( level );
    code = domRootNode.GetAttributeValueOrChildValueByName( "code" ).AsString( code );
    text = domRootNode.GetAttributeValueOrChildValueByName( "text" ).AsString( text );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetServiceNotice::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetServiceNotice( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceNotice::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceNotice::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetServiceNotice";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetServiceNotice::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetServiceNotice";
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

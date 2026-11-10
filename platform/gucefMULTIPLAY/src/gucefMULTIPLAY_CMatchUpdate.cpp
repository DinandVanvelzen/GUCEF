/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CMatchUpdate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchUpdate::CMatchUpdate( void )
    : CORE::CIDataNodeSerializable()
    , preview()
    , info()
    , hasInfo( false )
    , cursor()
    , providerName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate::CMatchUpdate( const CMatchUpdate& src )
    : CORE::CIDataNodeSerializable( src )
    , preview( src.preview )
    , info( src.info )
    , hasInfo( src.hasInfo )
    , cursor( src.cursor )
    , providerName( src.providerName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate::~CMatchUpdate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchUpdate&
CMatchUpdate::operator=( const CMatchUpdate& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        preview = src.preview;
        info = src.info;
        hasInfo = src.hasInfo;
        cursor = src.cursor;
        providerName = src.providerName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMatchUpdate::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "cursor", cursor );
    domRootNode.SetAttribute( "providerName", providerName );
    preview.Serialize( *domRootNode.AddChild( "preview", GUCEF_DATATYPE_OBJECT ), settings );
    if ( hasInfo )
        info.Serialize( *domRootNode.AddChild( "info", GUCEF_DATATYPE_OBJECT ), settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchUpdate::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    cursor = domRootNode.GetAttributeValueOrChildValueByName( "cursor" ).AsString( CORE::CString::Empty, true );
    providerName = domRootNode.GetAttributeValueOrChildValueByName( "providerName" ).AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* previewNode = domRootNode.FindChild( "preview" );
    if ( GUCEF_NULL == previewNode || !preview.Deserialize( *previewNode, settings ) )
        return false;
    const CORE::CDataNode* infoNode = domRootNode.FindChild( "info" );
    hasInfo = GUCEF_NULL != infoNode && info.Deserialize( *infoNode, settings );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchUpdate::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchUpdate( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchUpdate::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CMatchUpdate";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CMatchUpdate::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchUpdate::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAY::CMatchUpdate";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

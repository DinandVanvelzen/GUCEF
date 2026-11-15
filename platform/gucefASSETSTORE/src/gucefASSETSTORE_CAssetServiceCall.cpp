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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#include "gucefASSETSTORE_CAssetServiceCall.h"
#define GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICECALL_H ? */

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

CAssetServiceCall::CAssetServiceCall( void )
    : CORE::CIDataNodeSerializable()
    , operation()
    , commandId()
    , args( "args", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceCall::CAssetServiceCall( const CORE::CString& operationName, const CORE::CString& commandIdOfCall )
    : CORE::CIDataNodeSerializable()
    , operation( operationName )
    , commandId( commandIdOfCall )
    , args( "args", GUCEF_DATATYPE_OBJECT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetServiceCall::~CAssetServiceCall()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::Clear( void )
{GUCEF_TRACE;

    operation.Clear();
    commandId.Clear();
    args.Clear();
    args.SetName( "args" );
    args.SetNodeType( GUCEF_DATATYPE_OBJECT );
}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::SetText( const CORE::CString& name, const CORE::CString& value )
{GUCEF_TRACE;

    args.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetServiceCall::GetText( const CORE::CString& name ) const
{GUCEF_TRACE;

    return args.GetAttributeValueOrChildValueByName( name ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::SetUInt32( const CORE::CString& name, UInt32 value )
{GUCEF_TRACE;

    args.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetServiceCall::GetUInt32( const CORE::CString& name, UInt32 defaultValue ) const
{GUCEF_TRACE;

    return args.GetAttributeValueOrChildValueByName( name ).AsUInt32( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::SetUInt64( const CORE::CString& name, UInt64 value )
{GUCEF_TRACE;

    args.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetServiceCall::GetUInt64( const CORE::CString& name, UInt64 defaultValue ) const
{GUCEF_TRACE;

    return args.GetAttributeValueOrChildValueByName( name ).AsUInt64( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::SetBool( const CORE::CString& name, bool value )
{GUCEF_TRACE;

    args.SetAttribute( name, value );
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceCall::GetBool( const CORE::CString& name, bool defaultValue ) const
{GUCEF_TRACE;

    return args.GetAttributeValueOrChildValueByName( name ).AsBool( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetServiceCall::SetTexts( const CORE::CString& name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    args.SetValuesOfChildByName( name, values );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetServiceCall::GetTexts( const CORE::CString& name ) const
{GUCEF_TRACE;

    CORE::CString::StringVector values;
    args.GetValuesOfChildByName( name, values );
    return values;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceCall::SetRecord( const CORE::CString& name, const CORE::CIDataNodeSerializable& record )
{GUCEF_TRACE;

    args.DelChild( name );
    CORE::CDataNode* child = args.AddChild( name, GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == child )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    return record.Serialize( *child, settings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceCall::GetRecord( const CORE::CString& name, CORE::CIDataNodeSerializable& record ) const
{GUCEF_TRACE;

    const CORE::CDataNode* child = args.FindChild( name );
    if ( GUCEF_NULL == child )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    return record.Deserialize( *child, settings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceCall::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "operation", operation );
    success = domRootNode.SetAttribute( "commandId", commandId ) && success;
    domRootNode.DelChild( "args" );
    return GUCEF_NULL != domRootNode.AddChild( args ) && success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetServiceCall::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    operation = domRootNode.GetAttributeValueOrChildValueByName( "operation" ).AsString( CORE::CString::Empty, true );
    commandId = domRootNode.GetAttributeValueOrChildValueByName( "commandId" ).AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* argsNode = domRootNode.FindChild( "args" );
    if ( GUCEF_NULL != argsNode )
    {
        args.Copy( *argsNode );
        args.SetName( "args" );
    }
    return !operation.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetServiceCall::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetServiceCall( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceCall::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetServiceCall::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetServiceCall";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetServiceCall::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetServiceCall";
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

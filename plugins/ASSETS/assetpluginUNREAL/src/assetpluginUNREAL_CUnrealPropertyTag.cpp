/*
 *  assetpluginUNREAL: One property tag in either layout
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H
#include "assetpluginUNREAL_CUnrealPropertyTag.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealPropertyTag::CUnrealPropertyTag( void )
    : name()
    , type()
    , size( 0 )
    , arrayIndex( 0 )
    , flags( 0 )
    , hasPropertyGuid( false )
    , propertyGuid()
    , boolValue( false )
    , hasBinaryOrNativeSerialize( false )
    , skippedSerialize( false )
    , structName()
    , structGuid()
    , enumName()
    , extension( 0 )
    , isClassic( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealPropertyTag::Clear( void )
{GUCEF_TRACE;

    *this = CUnrealPropertyTag();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyTag::ReadExtensions( CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    extension = reader.ReadUInt8();
    if ( extension & UE_TAGEXT_OVERRIDABLE_INFORMATION )
    {
        reader.ReadUInt8();  // the overridden operation
        reader.ReadUInt8();  // the experimental overridable logic flag
    }
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyTag::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealReaderLimits& limits, bool& isNone )
{GUCEF_TRACE;

    Clear();
    isNone = false;
    if ( !names.ReadName( reader, name ) )
        return false;
    if ( name == "None" )
    {
        isNone = true;
        return true;
    }

    if ( versions.IsAtLeast( UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME ) )
    {
        // complete type name layout: name, type tree, size, flags, optional array index, guid and extensions
        if ( !type.Read( reader, names, limits.maxDepth * 8 ) )
            return false;
        size = reader.ReadInt32();
        flags = reader.ReadUInt8();
        if ( flags & UE_TAGFLAG_HAS_ARRAY_INDEX )
            arrayIndex = reader.ReadInt32();
        if ( flags & UE_TAGFLAG_HAS_PROPERTY_GUID )
        {
            hasPropertyGuid = true;
            reader.ReadGuid( propertyGuid );
        }
        if ( flags & UE_TAGFLAG_HAS_PROPERTY_EXTENSIONS )
            ReadExtensions( reader );
        boolValue = 0 != ( flags & UE_TAGFLAG_BOOL_TRUE );
        hasBinaryOrNativeSerialize = 0 != ( flags & UE_TAGFLAG_HAS_BINARY_OR_NATIVE );
        skippedSerialize = 0 != ( flags & UE_TAGFLAG_SKIPPED_SERIALIZE );
        const CORE::CString& kind = type.GetName();
        if ( kind == "StructProperty" )
            structName = type.GetParameterName( 0 );
        else
        if ( kind == "ByteProperty" || kind == "EnumProperty" )
            enumName = type.GetParameterName( 0 );
    }
    else
    {
        isClassic = true;
        CORE::CString kind;
        if ( !names.ReadName( reader, kind ) )
            return false;
        size = reader.ReadInt32();
        arrayIndex = reader.ReadInt32();
        if ( reader.HasError() )
            return false;
        if ( kind == "StructProperty" )
        {
            names.ReadName( reader, structName );
            if ( versions.IsAtLeast( UE4_STRUCT_GUID_IN_PROPERTY_TAG ) )
                reader.ReadGuid( structGuid );
            type.AddNode( kind, 1 );
            type.AddNode( structName, 0 );
        }
        else
        if ( kind == "BoolProperty" )
        {
            boolValue = reader.ReadBool8();
            type.SetSimple( kind );
        }
        else
        if ( kind == "ByteProperty" || kind == "EnumProperty" )
        {
            names.ReadName( reader, enumName );
            if ( enumName == "None" )
                enumName.Clear();
            type.AddNode( kind, enumName.IsNULLOrEmpty() ? 0 : 1 );
            if ( !enumName.IsNULLOrEmpty() )
                type.AddNode( enumName, 0 );
        }
        else
        if ( kind == "ArrayProperty" || kind == "SetProperty" )
        {
            CORE::CString inner;
            names.ReadName( reader, inner );
            type.AddNode( kind, 1 );
            type.AddNode( inner, 0 );
        }
        else
        if ( kind == "MapProperty" )
        {
            CORE::CString key;
            CORE::CString value;
            names.ReadName( reader, key );
            names.ReadName( reader, value );
            type.AddNode( kind, 2 );
            type.AddNode( key, 0 );
            type.AddNode( value, 0 );
        }
        else
            type.SetSimple( kind );

        if ( versions.IsAtLeast( UE4_PROPERTY_GUID_IN_PROPERTY_TAG ) )
        {
            hasPropertyGuid = reader.ReadBool8();
            if ( hasPropertyGuid )
                reader.ReadGuid( propertyGuid );
        }
        if ( versions.IsAtLeast( UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION ) )
            ReadExtensions( reader );
    }
    if ( reader.HasError() )
        return false;
    if ( size < 0 )
        return reader.SetError( "Property " + name + " has a negative size" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

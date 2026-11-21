/*
 *  assetpluginUNREAL: Reads the tagged properties of an export into a data node tree
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

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

CUnrealPropertyReader::CUnrealPropertyReader( const CUnrealPackage& package )
    : m_package( &package )
    , m_names( &package.GetNames() )
    , m_versions( &package.GetVersions() )
    , m_softPaths( package.GetSummary().softObjectPathsCount > 0 ? &package.GetSoftObjectPaths() : GUCEF_NULL )
    , m_limits( package.GetLimits() )
    , m_className()
    , m_mismatches()
    , m_unknown()
    , m_reported()
    , m_elements( 0 )
    , m_propertyCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealPropertyReader::CUnrealPropertyReader( const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealSoftObjectPath::TSoftObjectPathVector* softPaths, const CUnrealReaderLimits& limits )
    : m_package( GUCEF_NULL )
    , m_names( &names )
    , m_versions( &versions )
    , m_softPaths( softPaths )
    , m_limits( limits )
    , m_className()
    , m_mismatches()
    , m_unknown()
    , m_reported()
    , m_elements( 0 )
    , m_propertyCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealPropertyReader::~CUnrealPropertyReader()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector& CUnrealPropertyReader::GetMismatches( void ) const { return m_mismatches; }
const CORE::CString::StringVector& CUnrealPropertyReader::GetUnknownProperties( void ) const { return m_unknown; }
UInt32 CUnrealPropertyReader::GetPropertyCount( void ) const { return m_propertyCount; }

/*-------------------------------------------------------------------------*/

CUnrealPropertyReader::EKind
CUnrealPropertyReader::GetKind( const CORE::CString& typeName )
{GUCEF_TRACE;

    struct SKindName { const char* name; EKind kind; };
    static const SKindName kinds[] =
    {
        { "BoolProperty", K_BOOL }, { "ByteProperty", K_BYTE }, { "Int8Property", K_INT8 }, { "Int16Property", K_INT16 },
        { "IntProperty", K_INT32 }, { "Int64Property", K_INT64 }, { "UInt16Property", K_UINT16 }, { "UInt32Property", K_UINT32 },
        { "UInt64Property", K_UINT64 }, { "FloatProperty", K_FLOAT }, { "DoubleProperty", K_DOUBLE }, { "StrProperty", K_STR },
        { "NameProperty", K_NAME }, { "TextProperty", K_TEXT }, { "ObjectProperty", K_OBJECT }, { "ClassProperty", K_OBJECT },
        { "WeakObjectProperty", K_OBJECT }, { "InterfaceProperty", K_OBJECT }, { "LazyObjectProperty", K_LAZY },
        { "SoftObjectProperty", K_SOFTOBJECT }, { "SoftClassProperty", K_SOFTOBJECT }, { "AssetObjectProperty", K_SOFTOBJECT },
        { "AssetClassProperty", K_SOFTOBJECT }, { "EnumProperty", K_ENUM }, { "StructProperty", K_STRUCT },
        { "ArrayProperty", K_ARRAY }, { "SetProperty", K_SET }, { "MapProperty", K_MAP }, { "OptionalProperty", K_OPTIONAL },
        { "FieldPathProperty", K_FIELDPATH }, { "DelegateProperty", K_DELEGATE }, { "MulticastDelegateProperty", K_MULTICAST },
        { "MulticastInlineDelegateProperty", K_MULTICAST }, { "MulticastSparseDelegateProperty", K_MULTICAST },
        { GUCEF_NULL, K_UNKNOWN }
    };
    for ( const SKindName* entry = kinds; GUCEF_NULL != entry->name; ++entry )
        if ( typeName == entry->name )
            return entry->kind;
    return K_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealPropertyReader::MinElementBytes( EKind kind )
{GUCEF_TRACE;

    switch ( kind )
    {
        case K_BOOL: case K_BYTE: case K_INT8: return 1;
        case K_INT16: case K_UINT16: return 2;
        case K_ARRAY: case K_SET: case K_MAP: case K_OPTIONAL: case K_TEXT: case K_STR: case K_INT32: case K_UINT32: case K_FLOAT: case K_OBJECT: case K_SOFTOBJECT: case K_STRUCT: case K_MULTICAST: case K_FIELDPATH: return 4;
        case K_LAZY: return 16;
        default: return 8;
    }
}

/*-------------------------------------------------------------------------*/

void
CUnrealPropertyReader::AddMismatch( const CORE::CString& property, const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString key = m_className + "|" + property;
    if ( m_reported.find( key ) != m_reported.end() )
        return;
    m_reported.insert( key );
    m_mismatches.push_back( "Property " + property + " of class " + m_className + ": " + text );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::CountElements( CUnrealArchiveReader& reader, UInt64 count )
{GUCEF_TRACE;

    m_elements += count;
    if ( m_elements > m_limits.maxElements )
        return reader.SetError( "The containers of the properties hold more elements than the limit of " + CORE::UInt64ToString( m_limits.maxElements ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadExport( CUnrealArchiveReader& reader, CORE::CDataNode& out, const CORE::CString& className )
{GUCEF_TRACE;

    m_className = className;
    m_elements = 0;
    if ( m_versions->IsAtLeast( UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION ) )
    {
        const UInt8 control = reader.ReadUInt8();
        if ( control & UE_TAGEXT_OVERRIDABLE_INFORMATION )
            reader.ReadUInt8();
    }
    if ( reader.HasError() )
        return false;
    return ReadTags( reader, out, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadTagList( CUnrealArchiveReader& reader, CORE::CDataNode& parent )
{GUCEF_TRACE;

    m_elements = 0;
    return ReadTags( reader, parent, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadTags( CUnrealArchiveReader& reader, CORE::CDataNode& parent, UInt32 depth )
{GUCEF_TRACE;

    if ( depth > m_limits.maxDepth )
        return reader.SetError( "The nesting of the properties exceeds the limit of " + CORE::UInt32ToString( m_limits.maxDepth ) );

    CUnrealPropertyTag tag;
    while ( reader.GetRemaining() > 0 )
    {
        bool isNone = false;
        if ( !tag.Read( reader, *m_names, *m_versions, m_limits, isNone ) )
            return false;
        if ( isNone )
            return true;
        ++m_propertyCount;
        if ( (UInt64) tag.size > reader.GetRemaining() )
            return reader.SetError( "Property " + tag.name + " declares " + CORE::Int32ToString( tag.size ) + " bytes but only " + CORE::UInt64ToString( reader.GetRemaining() ) + " remain" );

        CORE::CDataNode* node = parent.AddChild( tag.name );
        node->SetAttribute( CORE::CString( "type" ), CORE::CVariant( tag.type.ToString() ) );
        if ( 0 != tag.arrayIndex )
            node->SetAttribute( CORE::CString( "index" ), CORE::CVariant( tag.arrayIndex ) );

        const UInt64 dataEnd = reader.GetPosition() + (UInt64) tag.size;
        const UInt64 previousEnd = reader.NarrowEnd( dataEnd );
        const EKind kind = GetKind( tag.type.GetName() );
        const bool unknownBinaryStruct = K_STRUCT == kind && tag.hasBinaryOrNativeSerialize && !tag.isClassic && !CUnrealNativeStructs::IsNative( tag.structName );
        if ( K_UNKNOWN == kind || tag.skippedSerialize || unknownBinaryStruct )
        {
            if ( K_UNKNOWN == kind || unknownBinaryStruct )
            {
                m_unknown.push_back( m_className + "." + tag.name + ": " + tag.type.ToString() );
                node->SetAttribute( CORE::CString( "skipped" ), CORE::CVariant( true ) );
            }
        }
        else
        {
            const bool ok = ReadValue( reader, &tag, tag.type, kind, false, tag.hasBinaryOrNativeSerialize, *node, depth );
            if ( !ok || reader.HasError() )
            {
                AddMismatch( tag.name, "the value could not be read (" + reader.GetError() + ") and is skipped" );
                node->SetAttribute( CORE::CString( "unread" ), CORE::CVariant( true ) );
                reader.ClearError();
            }
            else
            if ( reader.GetPosition() != dataEnd )
            {
                AddMismatch( tag.name, "value read " + CORE::UInt64ToString( reader.GetPosition() - ( dataEnd - (UInt64) tag.size ) ) + " bytes, tag size " + CORE::Int32ToString( tag.size ) + " (skipped)" );
                node->SetAttribute( CORE::CString( "mismatch" ), CORE::CVariant( true ) );
            }
        }
        reader.RestoreEnd( previousEnd );
        reader.Seek( dataEnd );
        if ( reader.HasError() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadObjectIndex( CUnrealArchiveReader& reader, CORE::CDataNode& node )
{GUCEF_TRACE;

    const Int32 index = reader.ReadInt32();
    if ( reader.HasError() )
        return false;
    node.SetValue( CORE::CVariant( index ) );
    if ( GUCEF_NULL != m_package )
    {
        CORE::CString path;
        if ( index < 0 && CUnrealPackageIndex( index ).ToImportIndex() < m_package->GetImports().size() )
            path = m_package->GetImportPath( CUnrealPackageIndex( index ).ToImportIndex() );
        else
        if ( index > 0 && CUnrealPackageIndex( index ).ToExportIndex() < m_package->GetExports().size() )
            path = m_package->GetExportPath( CUnrealPackageIndex( index ).ToExportIndex() );
        if ( !path.IsNULLOrEmpty() )
            node.SetAttribute( CORE::CString( "path" ), CORE::CVariant( path ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadText( CUnrealArchiveReader& reader, CORE::CDataNode& node )
{GUCEF_TRACE;

    const UInt32 flags = reader.ReadUInt32();
    const Int8 history = reader.ReadInt8();
    node.SetAttribute( CORE::CString( "flags" ), CORE::CVariant( flags ) );
    if ( reader.HasError() )
        return false;
    CORE::CString text;
    if ( history == -1 )
    {
        // no history: an optional culture invariant string
        if ( reader.ReadBool32() )
            reader.ReadString( text );
    }
    else
    if ( 0 == history )
    {
        CORE::CString ns;
        CORE::CString key;
        reader.ReadString( ns );
        reader.ReadString( key );
        reader.ReadString( text );
        node.SetAttribute( CORE::CString( "namespace" ), CORE::CVariant( ns ) );
        node.SetAttribute( CORE::CString( "key" ), CORE::CVariant( key ) );
    }
    else
    if ( 11 == history )
    {
        CORE::CString table;
        CORE::CString key;
        m_names->ReadName( reader, table );
        reader.ReadString( key );
        node.SetAttribute( CORE::CString( "table" ), CORE::CVariant( table ) );
        node.SetAttribute( CORE::CString( "key" ), CORE::CVariant( key ) );
    }
    else
        return reader.SetError( "Text history type " + CORE::Int32ToString( history ) + " is not supported" );
    node.SetValue( CORE::CVariant( text ) );
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadStruct( CUnrealArchiveReader& reader, const CORE::CString& structName, bool native, Int64 declaredSize, CORE::CDataNode& node, UInt32 depth )
{GUCEF_TRACE;

    node.SetAttribute( CORE::CString( "struct" ), CORE::CVariant( structName ) );
    if ( native )
    {
        if ( !CUnrealNativeStructs::IsNative( structName ) )
            return reader.SetError( "Struct " + structName + " has a binary form that is not known" );
        return CUnrealNativeStructs::Read( reader, structName, *m_names, *m_versions, m_softPaths, declaredSize, node );
    }
    return ReadTags( reader, node, depth + 1 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadValue( CUnrealArchiveReader& reader, const CUnrealPropertyTag* tag, const CUnrealPropertyTypeName& type, EKind kind, bool byteIsName, bool nativeHint, CORE::CDataNode& node, UInt32 depth )
{GUCEF_TRACE;

    if ( depth > m_limits.maxDepth )
        return reader.SetError( "The nesting of the properties exceeds the limit of " + CORE::UInt32ToString( m_limits.maxDepth ) );

    switch ( kind )
    {
        case K_BOOL:
            node.SetValue( CORE::CVariant( GUCEF_NULL != tag ? tag->boolValue : 0 != reader.ReadUInt8() ) );
            return !reader.HasError();
        case K_BYTE:
        {
            const bool asName = GUCEF_NULL != tag ? 8 == tag->size : ( byteIsName || type.GetParameterCount() > 0 );
            if ( asName )
            {
                CORE::CString name;
                m_names->ReadName( reader, name );
                node.SetValue( CORE::CVariant( name ) );
            }
            else
                node.SetValue( CORE::CVariant( reader.ReadUInt8() ) );
            return !reader.HasError();
        }
        case K_ENUM:
        {
            CORE::CString name;
            m_names->ReadName( reader, name );
            node.SetValue( CORE::CVariant( name ) );
            return !reader.HasError();
        }
        case K_INT8: node.SetValue( CORE::CVariant( reader.ReadInt8() ) ); return !reader.HasError();
        case K_INT16: node.SetValue( CORE::CVariant( reader.ReadInt16() ) ); return !reader.HasError();
        case K_INT32: node.SetValue( CORE::CVariant( reader.ReadInt32() ) ); return !reader.HasError();
        case K_INT64: node.SetValue( CORE::CVariant( reader.ReadInt64() ) ); return !reader.HasError();
        case K_UINT16: node.SetValue( CORE::CVariant( reader.ReadUInt16() ) ); return !reader.HasError();
        case K_UINT32: node.SetValue( CORE::CVariant( reader.ReadUInt32() ) ); return !reader.HasError();
        case K_UINT64: node.SetValue( CORE::CVariant( reader.ReadUInt64() ) ); return !reader.HasError();
        case K_FLOAT: node.SetValue( CORE::CVariant( reader.ReadFloat32() ) ); return !reader.HasError();
        case K_DOUBLE: node.SetValue( CORE::CVariant( reader.ReadFloat64() ) ); return !reader.HasError();
        case K_STR:
        {
            CORE::CString text;
            reader.ReadString( text, m_limits.maxNameLength );
            node.SetValue( CORE::CVariant( text ) );
            return !reader.HasError();
        }
        case K_NAME:
        {
            CORE::CString name;
            m_names->ReadName( reader, name );
            node.SetValue( CORE::CVariant( name ) );
            return !reader.HasError();
        }
        case K_TEXT: return ReadText( reader, node );
        case K_OBJECT: return ReadObjectIndex( reader, node );
        case K_LAZY:
        {
            CUnrealGuid guid;
            reader.ReadGuid( guid );
            node.SetValue( CORE::CVariant( guid.ToString() ) );
            return !reader.HasError();
        }
        case K_SOFTOBJECT:
        {
            CUnrealSoftObjectPath path;
            if ( !path.Read( reader, *m_names, *m_versions, m_softPaths ) )
                return false;
            node.SetValue( CORE::CVariant( path.ToString() ) );
            return true;
        }
        case K_FIELDPATH:
        {
            const Int32 count = reader.ReadInt32();
            if ( count < 0 || !reader.CheckCount( (UInt64) count, 8, "field path names" ) )
                return reader.SetError( "Field path with a count that is not valid" );
            CORE::CString text;
            for ( Int32 i=0; i<count; ++i )
            {
                CORE::CString part;
                m_names->ReadName( reader, part );
                text += ( i > 0 ? "." : "" ) + part;
            }
            reader.ReadInt32(); // the owner
            node.SetValue( CORE::CVariant( text ) );
            return !reader.HasError();
        }
        case K_DELEGATE:
        {
            CORE::CString function;
            ReadObjectIndex( reader, node );
            m_names->ReadName( reader, function );
            node.SetAttribute( CORE::CString( "function" ), CORE::CVariant( function ) );
            return !reader.HasError();
        }
        case K_MULTICAST:
        {
            const Int32 count = reader.ReadInt32();
            if ( count < 0 || !reader.CheckCount( (UInt64) count, 12, "delegates" ) || !CountElements( reader, (UInt64) count ) )
                return reader.SetError( "Multicast delegate with a count that is not valid" );
            node.SetNodeType( GUCEF_DATATYPE_ARRAY );
            for ( Int32 i=0; i<count; ++i )
            {
                CORE::CDataNode* entry = node.AddChild( CORE::Int32ToString( i ) );
                CORE::CString function;
                ReadObjectIndex( reader, *entry );
                m_names->ReadName( reader, function );
                entry->SetAttribute( CORE::CString( "function" ), CORE::CVariant( function ) );
            }
            return !reader.HasError();
        }
        case K_STRUCT:
        {
            const CORE::CString structName = GUCEF_NULL != tag ? tag->structName : type.GetParameterName( 0 );
            const bool classicLayout = !m_versions->IsAtLeast( UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME );
            const bool native = classicLayout ? CUnrealNativeStructs::IsNative( structName ) : nativeHint;
            return ReadStruct( reader, structName, native, GUCEF_NULL != tag ? tag->size : -1, node, depth );
        }
        case K_ARRAY: case K_SET: case K_MAP: case K_OPTIONAL:
            return ReadContainer( reader, tag, type, kind, nativeHint, node, depth );
        default:
            return reader.SetError( "Property kind " + type.GetName() + " is not supported" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealPropertyReader::ReadContainer( CUnrealArchiveReader& reader, const CUnrealPropertyTag* tag, const CUnrealPropertyTypeName& type, EKind kind, bool nativeHint, CORE::CDataNode& node, UInt32 depth )
{GUCEF_TRACE;

    if ( K_OPTIONAL == kind )
    {
        const CUnrealPropertyTypeName inner = type.GetParameter( 0 );
        const EKind innerKind = GetKind( inner.GetName() );
        const UInt64 start = reader.GetPosition();
        const UInt64 end = reader.GetEnd();
        for ( UInt32 attempt=0; attempt<2; ++attempt )
        {
            // the width of the "is set" flag is not stated by the tag: try a byte, then a 32 bit boolean
            reader.Seek( start );
            const bool isSet = 0 == attempt ? 0 != reader.ReadUInt8() : reader.ReadBool32();
            bool ok = !reader.HasError();
            CORE::CDataNode value( "value" );
            if ( ok && isSet )
            {
                if ( K_UNKNOWN == innerKind )
                    ok = false;
                else
                    ok = ReadValue( reader, GUCEF_NULL, inner, innerKind, false, nativeHint, value, depth + 1 );
            }
            if ( ok && reader.GetPosition() == end )
            {
                node.SetAttribute( CORE::CString( "set" ), CORE::CVariant( isSet ) );
                if ( isSet )
                    node.AddChild( value );
                return true;
            }
            reader.ClearError();
        }
        return reader.SetError( "Optional property value could not be read" );
    }

    const bool isMap = K_MAP == kind;
    const CUnrealPropertyTypeName keyType = type.GetParameter( 0 );
    const CUnrealPropertyTypeName valueType = isMap ? type.GetParameter( 1 ) : CUnrealPropertyTypeName();
    const EKind keyKind = GetKind( keyType.GetName() );
    const EKind valueKind = isMap ? GetKind( valueType.GetName() ) : K_UNKNOWN;
    if ( K_UNKNOWN == keyKind || ( isMap && K_UNKNOWN == valueKind ) )
        return reader.SetError( "Container of a property type that is not known: " + type.ToString() );

    node.SetNodeType( GUCEF_DATATYPE_ARRAY );
    const bool tagIsClassic = GUCEF_NULL != tag && tag->isClassic;

    // elements that were removed from the default come first for sets and maps; they are read and dropped
    if ( K_SET == kind || isMap )
    {
        const Int32 removed = reader.ReadInt32();
        if ( reader.HasError() || removed < 0 || !reader.CheckCount( (UInt64) removed, MinElementBytes( keyKind ), "removed elements" ) || !CountElements( reader, (UInt64) removed ) )
            return reader.SetError( "Container with a removed element count that is not valid" );
        for ( Int32 i=0; i<removed; ++i )
        {
            CORE::CDataNode scratch( "removed" );
            if ( !ReadValue( reader, GUCEF_NULL, keyType, keyKind, false, nativeHint, scratch, depth + 1 ) )
                return false;
        }
    }
    const Int32 count = reader.ReadInt32();
    if ( reader.HasError() )
        return false;
    const UInt32 minBytes = MinElementBytes( keyKind ) + ( isMap ? MinElementBytes( valueKind ) : 0 );
    if ( count < 0 || !reader.CheckCount( (UInt64) count, minBytes, "container elements" ) || !CountElements( reader, (UInt64) count ) )
        return reader.SetError( "Container with an element count that is not valid" );

    // the classic layout of an array of structs puts one tag in front of the elements that names the struct
    CUnrealPropertyTypeName elementType = keyType;
    bool bytesAreNames = false;
    if ( tagIsClassic && K_ARRAY == kind && K_STRUCT == keyKind )
    {
        CUnrealPropertyTag innerTag;
        bool isNone = false;
        if ( !innerTag.Read( reader, *m_names, *m_versions, m_limits, isNone ) || isNone )
            return reader.SetError( "The tag of the struct elements of an array is not valid" );
        elementType = CUnrealPropertyTypeName();
        elementType.AddNode( "StructProperty", 1 );
        elementType.AddNode( innerTag.structName, 0 );
    }
    if ( GUCEF_NULL != tag && K_BYTE == keyKind && count > 0 && !isMap )
    {
        const Int64 payload = (Int64) tag->size - ( K_SET == kind ? 8 : 4 );
        bytesAreNames = payload == (Int64) count * 8;
    }

    // long arrays of plain numbers are kept as one block of bytes, as they are in the file, instead of a node for every element
    if ( K_ARRAY == kind && count > (Int32) PackedArrayThreshold )
    {
        UInt32 elementBytes = 0;
        switch ( keyKind )
        {
            case K_BOOL: case K_INT8: elementBytes = 1; break;
            case K_BYTE: elementBytes = bytesAreNames ? 0 : 1; break;
            case K_INT16: case K_UINT16: elementBytes = 2; break;
            case K_INT32: case K_UINT32: case K_FLOAT: elementBytes = 4; break;
            case K_INT64: case K_UINT64: case K_DOUBLE: elementBytes = 8; break;
            case K_STRUCT:
            {
                // binary structs of a fixed size: the vectors, colours and such
                const CORE::CString structName = elementType.GetParameterName( 0 );
                const bool classicLayout = !m_versions->IsAtLeast( UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME );
                if ( classicLayout ? CUnrealNativeStructs::IsNative( structName ) : nativeHint )
                    elementBytes = CUnrealNativeStructs::GetFixedSize( structName, *m_versions );
                break;
            }
            default: break;
        }
        if ( elementBytes > 0 )
        {
            const UInt64 total = (UInt64) count * elementBytes;
            if ( !reader.CheckCount( total, 1, "packed array bytes" ) || total > 0x7FFFFFFF )
                return reader.SetError( "Packed array of a size that is not valid" );
            GUCEF::vector< UInt8 > block( (size_t) total );
            if ( !reader.Read( &block[ 0 ], total ) )
                return false;
            node.SetValue( CORE::CVariant( &block[ 0 ], (UInt32) total ) );
            node.SetAttribute( CORE::CString( "packed" ), CORE::CVariant( true ) );
            node.SetAttribute( CORE::CString( "count" ), CORE::CVariant( count ) );
            node.SetAttribute( CORE::CString( "elementType" ), CORE::CVariant( keyType.GetName() ) );
            if ( K_STRUCT == keyKind )
                node.SetAttribute( CORE::CString( "struct" ), CORE::CVariant( elementType.GetParameterName( 0 ) ) );
            node.SetAttribute( CORE::CString( "elementBytes" ), CORE::CVariant( elementBytes ) );
            return true;
        }
    }

    for ( Int32 i=0; i<count; ++i )
    {
        CORE::CDataNode* element = node.AddChild( CORE::Int32ToString( i ) );
        if ( isMap )
        {
            if ( !ReadValue( reader, GUCEF_NULL, keyType, keyKind, false, nativeHint, *element->AddChild( "key" ), depth + 1 ) )
                return false;
            if ( !ReadValue( reader, GUCEF_NULL, valueType, valueKind, false, nativeHint, *element->AddChild( "value" ), depth + 1 ) )
                return false;
        }
        else
        if ( !ReadValue( reader, GUCEF_NULL, elementType, keyKind, bytesAreNames, nativeHint, *element, depth + 1 ) )
            return false;
    }
    return !reader.HasError();
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

/*
 *  assetpluginUNREAL_TestApp: Test side writer of Unreal packages: byte writer, tagged property writer and package writer
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

#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#include "TestUnrealWriter.h"
#include "assetpluginUNREAL_CUnrealPngWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void CTestByteWriter::U8( UInt8 value ) { bytes.push_back( value ); }
void CTestByteWriter::U16( UInt16 value ) { U8( (UInt8) ( value & 0xFF ) ); U8( (UInt8) ( value >> 8 ) ); }
void CTestByteWriter::U32( UInt32 value ) { U16( (UInt16) ( value & 0xFFFF ) ); U16( (UInt16) ( value >> 16 ) ); }
void CTestByteWriter::I32( Int32 value ) { U32( (UInt32) value ); }
void CTestByteWriter::U64( UInt64 value ) { U32( (UInt32) ( value & 0xFFFFFFFF ) ); U32( (UInt32) ( value >> 32 ) ); }
void CTestByteWriter::I64( Int64 value ) { U64( (UInt64) value ); }
void CTestByteWriter::U32BE( UInt32 value ) { U8( (UInt8) ( value >> 24 ) ); U8( (UInt8) ( value >> 16 ) ); U8( (UInt8) ( value >> 8 ) ); U8( (UInt8) value ); }
void CTestByteWriter::U64BE( UInt64 value ) { U32BE( (UInt32) ( value >> 32 ) ); U32BE( (UInt32) ( value & 0xFFFFFFFF ) ); }
void CTestByteWriter::F32( Float32 value ) { UInt32 bits; memcpy( &bits, &value, 4 ); U32( bits ); }
void CTestByteWriter::F64( Float64 value ) { UInt64 bits; memcpy( &bits, &value, 8 ); U64( bits ); }
void CTestByteWriter::Bool32( bool value ) { U32( value ? 1 : 0 ); }
void CTestByteWriter::Guid( const CUnrealGuid& guid ) { U32( guid.a ); U32( guid.b ); U32( guid.c ); U32( guid.d ); }
void CTestByteWriter::Zero( size_t count ) { bytes.insert( bytes.end(), count, 0 ); }

void
CTestByteWriter::Raw( const void* data, size_t size )
{
    const UInt8* in = static_cast< const UInt8* >( data );
    bytes.insert( bytes.end(), in, in + size );
}

void CTestByteWriter::Append( const CTestByteWriter& other ) { bytes.insert( bytes.end(), other.bytes.begin(), other.bytes.end() ); }

void
CTestByteWriter::Str( const CORE::CString& text, bool wide )
{
    const char* chars = text.C_String();
    const size_t length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( 0 == length )
    {
        I32( 0 );
        return;
    }
    bool ascii = true;
    for ( size_t i=0; i<length; ++i )
        if ( (UInt8) chars[ i ] >= 0x80 )
            ascii = false;
    if ( ascii && !wide )
    {
        I32( (Int32) length + 1 );
        Raw( chars, length );
        U8( 0 );
        return;
    }
    // decode the UTF-8 into UTF-16
    GUCEF::vector< UInt16 > units;
    for ( size_t i=0; i<length; )
    {
        const UInt8 c = (UInt8) chars[ i ];
        UInt32 cp = c;
        size_t extra = 0;
        if ( c >= 0xF0 ) { cp = c & 0x07; extra = 3; }
        else if ( c >= 0xE0 ) { cp = c & 0x0F; extra = 2; }
        else if ( c >= 0xC0 ) { cp = c & 0x1F; extra = 1; }
        for ( size_t k=1; k<=extra && i+k<length; ++k )
            cp = ( cp << 6 ) | ( (UInt8) chars[ i+k ] & 0x3F );
        i += 1 + extra;
        if ( cp >= 0x10000 )
        {
            cp -= 0x10000;
            units.push_back( (UInt16) ( 0xD800 + ( cp >> 10 ) ) );
            units.push_back( (UInt16) ( 0xDC00 + ( cp & 0x3FF ) ) );
        }
        else
            units.push_back( (UInt16) cp );
    }
    I32( -( (Int32) units.size() + 1 ) );
    for ( size_t i=0; i<units.size(); ++i )
        U16( units[ i ] );
    U16( 0 );
}

void
CTestByteWriter::Patch32( size_t offset, Int32 value )
{
    for ( UInt32 i=0; i<4; ++i )
        bytes[ offset + i ] = (UInt8) ( ( (UInt32) value >> ( 8 * i ) ) & 0xFF );
}

void
CTestByteWriter::Patch64( size_t offset, Int64 value )
{
    for ( UInt32 i=0; i<8; ++i )
        bytes[ offset + i ] = (UInt8) ( ( (UInt64) value >> ( 8 * i ) ) & 0xFF );
}

/*-------------------------------------------------------------------------*/

UInt32
CTestNames::Index( const CORE::CString& name )
{
    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return (UInt32) i;
    names.push_back( name );
    return (UInt32) ( names.size() - 1 );
}

void
CTestNames::WriteName( CTestByteWriter& writer, const CORE::CString& text )
{
    // "Name_12" is the name "Name" with the number 13
    Int32 number = 0;
    CORE::CString base = text;
    const Int32 underscore = text.HasChar( '_', false );
    if ( underscore > 0 && (UInt32) underscore + 1 < text.Length() )
    {
        const CORE::CString digits = text.SubstrFromRange( (UInt32) underscore + 1, text.Length() );
        bool allDigits = true;
        for ( UInt32 i=0; i<digits.Length(); ++i )
            if ( digits.C_String()[ i ] < '0' || digits.C_String()[ i ] > '9' )
                allDigits = false;
        if ( allDigits && digits.Length() < 9 )
        {
            number = CORE::StringToInt32( digits ) + 1;
            base = text.SubstrFromRange( 0, (UInt32) underscore );
        }
    }
    writer.U32( Index( base ) );
    writer.I32( number );
}

/*-------------------------------------------------------------------------*/

CTestType
StructType( const CORE::CString& structName )
{
    return CTestType( "StructProperty", CTestType( structName, CTestType( "/Script/CoreUObject" ) ) );
}

/*-------------------------------------------------------------------------*/

static void
FlattenType( const CTestType& type, CTestNames& names, CTestByteWriter& out )
{
    names.WriteName( out, type.name );
    out.I32( (Int32) type.inner.size() );
    for ( size_t i=0; i<type.inner.size(); ++i )
        FlattenType( type.inner[ i ], names, out );
}

/*-------------------------------------------------------------------------*/

CTestPropertyWriter::CTestPropertyWriter( CTestByteWriter& out, CTestNames& names, Int32 ue4Version, Int32 ue5Version )
    : m_out( out )
    , m_names( names )
    , m_ue4( ue4Version )
    , m_ue5( ue5Version )
{
}

bool CTestPropertyWriter::IsNewLayout( void ) const { return m_ue5 >= UE5_PROPERTY_TAG_COMPLETE_TYPE_NAME; }

void
CTestPropertyWriter::ControlByte( void )
{
    if ( m_ue5 >= UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION )
        m_out.U8( 0 );
}

void
CTestPropertyWriter::None( void )
{
    m_names.WriteName( m_out, "None" );
}

CTestPropertyWriter::SOpen
CTestPropertyWriter::Begin( const CORE::CString& name, const CTestType& type, bool boolValue, Int32 arrayIndex, bool nativeFlag, bool withGuid, bool withExtension )
{
    SOpen open;
    m_names.WriteName( m_out, name );
    if ( IsNewLayout() )
    {
        FlattenType( type, m_names, m_out );
        open.sizeOffset = m_out.Size();
        m_out.I32( 0 );
        UInt8 flags = 0;
        if ( 0 != arrayIndex ) flags |= UE_TAGFLAG_HAS_ARRAY_INDEX;
        if ( withGuid ) flags |= UE_TAGFLAG_HAS_PROPERTY_GUID;
        if ( withExtension ) flags |= UE_TAGFLAG_HAS_PROPERTY_EXTENSIONS;
        if ( nativeFlag ) flags |= UE_TAGFLAG_HAS_BINARY_OR_NATIVE;
        if ( boolValue ) flags |= UE_TAGFLAG_BOOL_TRUE;
        m_out.U8( flags );
        if ( 0 != arrayIndex ) m_out.I32( arrayIndex );
        if ( withGuid ) m_out.Guid( CUnrealGuid( 1, 2, 3, 4 ) );
        if ( withExtension ) { m_out.U8( UE_TAGEXT_OVERRIDABLE_INFORMATION ); m_out.U8( 1 ); m_out.U8( 0 ); }
    }
    else
    {
        m_names.WriteName( m_out, type.name );
        open.sizeOffset = m_out.Size();
        m_out.I32( 0 );
        m_out.I32( arrayIndex );
        if ( "StructProperty" == type.name )
        {
            m_names.WriteName( m_out, type.inner.empty() ? CORE::CString( "None" ) : type.inner[ 0 ].name );
            if ( m_ue4 >= UE4_STRUCT_GUID_IN_PROPERTY_TAG )
                m_out.Guid( CUnrealGuid() );
        }
        else
        if ( "BoolProperty" == type.name )
            m_out.U8( boolValue ? 1 : 0 );
        else
        if ( "ByteProperty" == type.name || "EnumProperty" == type.name )
            m_names.WriteName( m_out, type.inner.empty() ? CORE::CString( "None" ) : type.inner[ 0 ].name );
        else
        if ( "ArrayProperty" == type.name || "SetProperty" == type.name )
            m_names.WriteName( m_out, type.inner[ 0 ].name );
        else
        if ( "MapProperty" == type.name )
        {
            m_names.WriteName( m_out, type.inner[ 0 ].name );
            m_names.WriteName( m_out, type.inner[ 1 ].name );
        }
        if ( m_ue4 >= UE4_PROPERTY_GUID_IN_PROPERTY_TAG )
        {
            m_out.U8( withGuid ? 1 : 0 );
            if ( withGuid ) m_out.Guid( CUnrealGuid( 1, 2, 3, 4 ) );
        }
        if ( m_ue5 >= UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION )
        {
            if ( withExtension ) { m_out.U8( UE_TAGEXT_OVERRIDABLE_INFORMATION ); m_out.U8( 1 ); m_out.U8( 0 ); }
            else m_out.U8( 0 );
        }
    }
    open.valueStart = m_out.Size();
    return open;
}

void
CTestPropertyWriter::End( const SOpen& open )
{
    m_out.Patch32( open.sizeOffset, (Int32) ( m_out.Size() - open.valueStart ) );
}

CTestPropertyWriter::SOpen
CTestPropertyWriter::BeginInnerStructTag( const CORE::CString& propertyName, const CORE::CString& structName )
{
    // the inner tag has the layout of a classic tag of a struct property
    SOpen open;
    m_names.WriteName( m_out, propertyName );
    m_names.WriteName( m_out, "StructProperty" );
    open.sizeOffset = m_out.Size();
    m_out.I32( 0 );
    m_out.I32( 0 );
    m_names.WriteName( m_out, structName );
    m_out.Guid( CUnrealGuid() );
    m_out.U8( 0 );
    if ( m_ue5 >= UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION )
        m_out.U8( 0 );
    open.valueStart = m_out.Size();
    return open;
}

/*-------------------------------------------------------------------------*/

CTestPackageWriter::CTestPackageWriter( Int32 legacyVersion, Int32 ue4Version, Int32 ue5Version )
    : legacy( legacyVersion )
    , ue4( ue4Version )
    , ue5( ue5Version )
    , licensee( 0 )
    , packageFlags( 0 )
    , packageName( "/Game/Test/Package" )
    , engineMajor( ue5Version > 0 ? 5 : 4 )
    , engineMinor( 3 )
    , enginePatch( 2 )
    , engineChangelist( 12345 )
    , engineBranch( "++UE5+Release-5.3" )
    , trailerVersion( 2 )
    , compressedChunks( 0 )
    , tag( UE_PACKAGE_FILE_TAG )
{
}

Int32
CTestPackageWriter::AddImport( const CORE::CString& classPackage, const CORE::CString& className, Int32 outer, const CORE::CString& objectName )
{
    SImport object;
    object.classPackage = classPackage;
    object.className = className;
    object.outer = outer;
    object.objectName = objectName;
    object.packageName = "None";
    names.Index( classPackage ); names.Index( className ); names.Index( "None" );
    imports.push_back( object );
    CTestByteWriter scratch;
    names.WriteName( scratch, objectName );
    return -(Int32) imports.size();
}

Int32
CTestPackageWriter::AddExport( Int32 classIndex, Int32 outer, const CORE::CString& objectName, const CTestByteWriter& body, Int64 scriptEnd )
{
    exports.push_back( SExport() );
    SExport& object = exports.back();
    object.classIndex = classIndex;
    object.outer = outer;
    object.objectName = objectName;
    object.body = body;
    object.scriptStart = 0;
    object.scriptEnd = scriptEnd < 0 ? (Int64) body.Size() : scriptEnd;
    object.forcedOffset = -1;
    object.forcedSize = -1;
    CTestByteWriter scratch;
    names.WriteName( scratch, objectName );
    return (Int32) exports.size();
}

UInt32
CTestPackageWriter::AddPayload( const GUCEF::vector< UInt8 >& raw, bool stored )
{
    SPayload payload;
    payload.raw = raw;
    payload.stored = stored;
    UInt64 hash = 1469598103934665603ULL;
    for ( size_t i=0; i<raw.size(); ++i )
        hash = ( hash ^ raw[ i ] ) * 1099511628211ULL;
    hash ^= payloads.size();
    for ( UInt32 i=0; i<20; ++i )
        payload.id[ i ] = (UInt8) ( ( hash >> ( 8 * ( i % 8 ) ) ) + i * 7 );
    payloads.push_back( payload );
    return (UInt32) ( payloads.size() - 1 );
}

void
CTestPackageWriter::BeginBody( CTestByteWriter& body ) const
{
    if ( ue5 >= UE5_PROPERTY_TAG_EXTENSION_AND_OVERRIDABLE_SERIALIZATION )
        body.U8( 0 );
}

void
CTestPackageWriter::WriteEditorBulkData( CTestByteWriter& out, UInt32 payloadIndex ) const
{
    out.U32( 0x200 );
    out.Guid( CUnrealGuid( 9, 8, 7, payloadIndex ) );
    out.Raw( payloads[ payloadIndex ].id, 20 );
    out.I64( (Int64) payloads[ payloadIndex ].raw.size() );
}

/*-------------------------------------------------------------------------*/

void
BuildStoredCompressedBuffer( const GUCEF::vector< UInt8 >& raw, GUCEF::vector< UInt8 >& out )
{
    // method 0: the 64 byte header, then the raw bytes in one block; no table of block sizes
    CTestByteWriter w;
    w.U32BE( 0xB7756362 );
    w.U32BE( 0 );
    w.U8( 0 ); w.U8( 0 ); w.U8( 0 ); w.U8( 0 );
    w.U32BE( raw.empty() ? 0 : 1 );
    w.U64BE( raw.size() );
    w.U64BE( 64 + raw.size() );
    w.Zero( 32 );
    // the checksum of the header covers the bytes after the checksum field
    {
        const UInt32 crc = CUnrealPngWriter::Crc32( &w.bytes[ 8 ], 56 );
        w.bytes[ 4 ] = (UInt8) ( crc >> 24 ); w.bytes[ 5 ] = (UInt8) ( crc >> 16 ); w.bytes[ 6 ] = (UInt8) ( crc >> 8 ); w.bytes[ 7 ] = (UInt8) crc;
    }
    w.Raw( raw.empty() ? GUCEF_NULL : &raw[ 0 ], raw.size() );
    out = w.bytes;
}

/*-------------------------------------------------------------------------*/

void
CTestPackageWriter::WriteSummary( CTestByteWriter& w, Int32 headerSize, Int32 nameOffset, Int32 softOffset, Int32 exportOffset, Int32 importOffset, Int64 trailerOffset )
{
    const bool editorOnlyFiltered = 0 != ( packageFlags & UE_PKG_FILTER_EDITOR_ONLY );
    w.U32( tag );
    w.I32( legacy );
    if ( -4 != legacy )
        w.I32( 864 );
    w.I32( ue4 );
    if ( legacy <= -8 )
        w.I32( ue5 );
    w.I32( licensee );
    if ( ue5 >= UE5_PACKAGE_SAVED_HASH )
    {
        for ( UInt32 i=0; i<20; ++i )
            w.U8( (UInt8) ( 0xA0 + i ) );
        w.I32( headerSize );
    }
    w.I32( (Int32) customVersions.size() );
    for ( size_t i=0; i<customVersions.size(); ++i )
    {
        w.Guid( customVersions[ i ].key );
        w.I32( customVersions[ i ].version );
    }
    if ( ue5 < UE5_PACKAGE_SAVED_HASH )
        w.I32( headerSize );
    w.Str( packageName );
    w.U32( packageFlags );
    w.I32( (Int32) names.names.size() );
    w.I32( nameOffset );
    if ( ue5 >= UE5_ADD_SOFTOBJECTPATH_LIST )
    {
        w.I32( (Int32) softPaths.size() );
        w.I32( softOffset );
    }
    if ( !editorOnlyFiltered && ue4 >= UE4_ADDED_PACKAGE_SUMMARY_LOCALIZATION_ID )
        w.Str( "locid" );
    if ( ue4 >= UE4_SERIALIZE_TEXT_IN_PACKAGES )
    {
        w.I32( 0 );
        w.I32( 0 );
    }
    w.I32( (Int32) exports.size() );
    w.I32( exportOffset );
    w.I32( (Int32) imports.size() );
    w.I32( importOffset );
    if ( ue5 >= UE5_VERSE_CELLS )
        w.Zero( 16 );
    if ( ue5 >= UE5_METADATA_SERIALIZATION_OFFSET )
        w.I32( 0 );
    w.I32( 0 ); // depends
    if ( ue4 >= UE4_ADD_STRING_ASSET_REFERENCES_MAP )
        w.Zero( 8 );
    if ( ue4 >= UE4_ADDED_SEARCHABLE_NAMES )
        w.I32( 0 );
    w.I32( 0 ); // thumbnails
    if ( ue5 >= UE5_IMPORT_TYPE_HIERARCHIES )
        w.Zero( 8 );
    if ( ue5 < UE5_PACKAGE_SAVED_HASH )
        w.Guid( CUnrealGuid( 0x11, 0x22, 0x33, 0x44 ) );
    if ( !editorOnlyFiltered )
    {
        if ( ue4 >= UE4_ADDED_PACKAGE_OWNER )
            w.Guid( CUnrealGuid( 0x55, 0x66, 0x77, 0x88 ) );
        if ( ue4 >= UE4_ADDED_PACKAGE_OWNER && ue4 < UE4_NON_OUTER_PACKAGE_IMPORT )
            w.Guid( CUnrealGuid( 0x99, 0xAA, 0xBB, 0xCC ) );
    }
    w.I32( 1 );
    w.I32( (Int32) exports.size() );
    w.I32( (Int32) names.names.size() );
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        if ( 1 == pass && ue4 < UE4_PACKAGE_SUMMARY_HAS_COMPATIBLE_ENGINE_VERSION )
            break;
        w.U16( engineMajor ); w.U16( engineMinor ); w.U16( enginePatch ); w.U32( engineChangelist ); w.Str( engineBranch );
    }
    w.U32( 0 );
    w.I32( compressedChunks );
    for ( Int32 i=0; i<compressedChunks; ++i )
        w.Zero( 20 );
    w.U32( 0xCAFE );
    w.I32( 0 );
    if ( ue4 >= UE4_ASSET_REGISTRY_TAGS )
        w.I32( 0 );
    if ( ue4 >= UE4_SUMMARY_HAS_BULKDATA_OFFSET )
        w.I64( 0 );
    w.I32( 0 ); // world tile info
    if ( ue4 >= UE4_CHANGED_CHUNKID_TO_BE_AN_ARRAY_OF_CHUNKIDS )
        w.I32( 0 );
    if ( ue4 >= UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS )
        w.Zero( 8 );
    if ( ue5 >= UE5_NAMES_REFERENCED_FROM_EXPORT_DATA )
        w.I32( (Int32) names.names.size() );
    if ( ue5 >= UE5_PAYLOAD_TOC )
        w.I64( trailerOffset );
    if ( ue5 >= UE5_DATA_RESOURCES )
        w.I32( 0 );
}

void
CTestPackageWriter::WriteSoftPath( CTestByteWriter& w, const CUnrealSoftObjectPath& path )
{
    if ( ue5 >= UE5_SOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES )
    {
        names.WriteName( w, path.packageName );
        names.WriteName( w, path.assetName );
    }
    else
        names.WriteName( w, path.packageName + "." + path.assetName );
    w.Str( path.subPath );
}

void
CTestPackageWriter::WriteImport( CTestByteWriter& w, const SImport& object )
{
    names.WriteName( w, object.classPackage );
    names.WriteName( w, object.className );
    w.I32( object.outer );
    names.WriteName( w, object.objectName );
    if ( ue4 >= UE4_NON_OUTER_PACKAGE_IMPORT && 0 == ( packageFlags & UE_PKG_FILTER_EDITOR_ONLY ) )
        names.WriteName( w, object.packageName );
    if ( ue5 >= UE5_OPTIONAL_RESOURCES )
        w.Bool32( false );
}

void
CTestPackageWriter::WriteExport( CTestByteWriter& w, const SExport& object, Int64 offset )
{
    w.I32( object.classIndex );
    w.I32( 0 );
    if ( ue4 >= UE4_TEMPLATE_INDEX_IN_COOKED_EXPORTS )
        w.I32( 0 );
    w.I32( object.outer );
    names.WriteName( w, object.objectName );
    w.U32( 0x00000008 );
    const Int64 size = object.forcedSize >= 0 ? object.forcedSize : (Int64) object.body.Size();
    const Int64 start = object.forcedOffset >= 0 ? object.forcedOffset : offset;
    if ( ue4 >= UE4_64BIT_EXPORTMAP_SERIALSIZES )
    {
        w.I64( size );
        w.I64( start );
    }
    else
    {
        w.I32( (Int32) size );
        w.I32( (Int32) start );
    }
    w.Bool32( false ); w.Bool32( false ); w.Bool32( false );
    if ( ue5 < UE5_REMOVE_OBJECT_EXPORT_PACKAGE_GUID )
        w.Guid( CUnrealGuid() );
    if ( ue5 >= UE5_TRACK_OBJECT_EXPORT_IS_INHERITED )
        w.Bool32( false );
    w.U32( 0 );
    w.Bool32( false );
    if ( ue4 >= UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT )
        w.Bool32( true );
    if ( ue5 >= UE5_OPTIONAL_RESOURCES )
        w.Bool32( false );
    if ( ue4 >= UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS )
    {
        w.I32( -1 );
        w.Zero( 16 );
    }
    if ( ue5 >= UE5_SCRIPT_SERIALIZATION_OFFSET && 0 == ( packageFlags & UE_PKG_UNVERSIONED_PROPERTIES ) )
    {
        w.I64( object.scriptStart );
        w.I64( object.scriptEnd );
    }
}

void
CTestPackageWriter::Build( GUCEF::vector< UInt8 >& out )
{
    // the sizes of the parts do not depend on the offsets: measure, then write with the real offsets
    CTestByteWriter summaryProbe;
    WriteSummary( summaryProbe, 0, 0, 0, 0, 0, -1 );
    CTestByteWriter softTable;
    CTestByteWriter importTable;
    CTestByteWriter exportProbe;
    for ( size_t i=0; i<softPaths.size(); ++i )
        WriteSoftPath( softTable, softPaths[ i ] );
    for ( size_t i=0; i<imports.size(); ++i )
        WriteImport( importTable, imports[ i ] );
    for ( size_t i=0; i<exports.size(); ++i )
        WriteExport( exportProbe, exports[ i ], 0 );

    // the tables above added the names they use; the name table is written last
    CTestByteWriter nameTable;
    for ( size_t i=0; i<names.names.size(); ++i )
    {
        nameTable.Str( names.names[ i ] );
        nameTable.U32( 0x12345678 );
    }
    const Int32 nameOffset = (Int32) summaryProbe.Size();
    const Int32 softOffset = nameOffset + (Int32) nameTable.Size();
    const Int32 importOffset = softOffset + (Int32) softTable.Size();
    const Int32 exportOffset = importOffset + (Int32) importTable.Size();
    const Int32 headerSize = exportOffset + (Int32) exportProbe.Size();

    CTestByteWriter exportTable;
    Int64 position = headerSize;
    for ( size_t i=0; i<exports.size(); ++i )
    {
        WriteExport( exportTable, exports[ i ], position );
        position += (Int64) exports[ i ].body.Size();
    }

    // the trailer
    CTestByteWriter trailer;
    Int64 trailerOffset = -1;
    if ( !payloads.empty() )
    {
        trailerOffset = position;
        UInt32 entryBytes = 44;
        if ( trailerVersion >= 1 ) entryBytes += 1;
        if ( trailerVersion >= 2 ) entryBytes += 4;
        const UInt32 headerLength = 28 + (UInt32) payloads.size() * entryBytes;
        CTestByteWriter data;
        CTestByteWriter table;
        for ( size_t i=0; i<payloads.size(); ++i )
        {
            GUCEF::vector< UInt8 > stored;
            if ( payloads[ i ].stored )
                BuildStoredCompressedBuffer( payloads[ i ].raw, stored );
            else
                stored = payloads[ i ].raw;
            table.Raw( payloads[ i ].id, 20 );
            table.I64( (Int64) data.Size() );
            table.U64( stored.size() );
            table.U64( payloads[ i ].raw.size() );
            if ( trailerVersion >= 2 ) table.U32( 0 );
            if ( trailerVersion >= 1 ) table.U8( 0 );
            data.Raw( stored.empty() ? GUCEF_NULL : &stored[ 0 ], stored.size() );
        }
        trailer.U64( 0xD1C43B2E80A5F697ULL );
        trailer.U32( (UInt32) trailerVersion );
        trailer.U32( headerLength );
        trailer.U64( data.Size() );
        trailer.U32( (UInt32) payloads.size() );
        trailer.Append( table );
        trailer.Append( data );
        trailer.U64( 0x29BFCA045138DE76ULL );
        trailer.U64( trailer.Size() + 12 );
        trailer.U32( UE_PACKAGE_FILE_TAG );
    }

    CTestByteWriter file;
    WriteSummary( file, headerSize, nameOffset, softOffset, exportOffset, importOffset, trailerOffset );
    file.Append( nameTable );
    file.Append( softTable );
    file.Append( importTable );
    file.Append( exportTable );
    for ( size_t i=0; i<exports.size(); ++i )
        file.Append( exports[ i ].body );
    file.Append( trailer );
    out = file.bytes;
}

/*-------------------------------------------------------------------------*/


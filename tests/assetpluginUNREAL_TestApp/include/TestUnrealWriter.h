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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_GUCEFCORE_H
#endif /* GUCEF_CORE_GUCEFCORE_H ? */

#include "assetpluginUNREAL_CUnrealCustomVersion.h"
#include "assetpluginUNREAL_CUnrealSoftObjectPath.h"

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#include "assetpluginUNREAL_CUnrealGuid.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Little endian byte writer */
class CTestByteWriter
{
    public:

    GUCEF::vector< GUCEF::UInt8 > bytes;

    size_t Size( void ) const { return bytes.size(); }

    void U8( GUCEF::UInt8 value );
    void U16( GUCEF::UInt16 value );
    void U32( GUCEF::UInt32 value );
    void I32( GUCEF::Int32 value );
    void U64( GUCEF::UInt64 value );
    void I64( GUCEF::Int64 value );
    void F32( GUCEF::Float32 value );
    void F64( GUCEF::Float64 value );
    void Bool32( bool value );
    void Guid( const GUCEF::ASSETPLUGIN::UNREAL::CUnrealGuid& guid );
    void Raw( const void* data, size_t size );
    void Append( const CTestByteWriter& other );
    void Zero( size_t count );
    void U32BE( GUCEF::UInt32 value );
    void U64BE( GUCEF::UInt64 value );

    /** A string as the engine stores it: ANSI when every character is 7 bit, UTF-16 otherwise (or when wide is set) */
    void Str( const GUCEF::CORE::CString& text, bool wide = false );

    void Patch32( size_t offset, GUCEF::Int32 value );
    void Patch64( size_t offset, GUCEF::Int64 value );
};

/*-------------------------------------------------------------------------*/

/** The name table under construction; Index() adds a name that is not in the table yet */
class CTestNames
{
    public:

    GUCEF::vector< GUCEF::CORE::CString > names;

    GUCEF::UInt32 Index( const GUCEF::CORE::CString& name );

    /** Writes a name: its index and its number; a "_N" suffix of the text becomes the number N+1 */
    void WriteName( CTestByteWriter& writer, const GUCEF::CORE::CString& text );
};

/*-------------------------------------------------------------------------*/

/** The type of a property as a tree: T( "ArrayProperty", T( "IntProperty" ) ) */
class CTestType
{
    public:

    GUCEF::CORE::CString name;
    GUCEF::vector< CTestType > inner;

    CTestType( void ) {}
    CTestType( const GUCEF::CORE::CString& typeName ) : name( typeName ) {}
    CTestType( const GUCEF::CORE::CString& typeName, const CTestType& a ) : name( typeName ) { inner.push_back( a ); }
    CTestType( const GUCEF::CORE::CString& typeName, const CTestType& a, const CTestType& b ) : name( typeName ) { inner.push_back( a ); inner.push_back( b ); }
};

/** A struct type with the package it comes from, as the complete type name layout writes it */
CTestType StructType( const GUCEF::CORE::CString& structName );

/*-------------------------------------------------------------------------*/

/** Writes tags in the layout the versions call for; the value is written by the caller between Begin() and End() */
class CTestPropertyWriter
{
    public:

    struct SOpen
    {
        size_t sizeOffset;
        size_t valueStart;
    };

    CTestPropertyWriter( CTestByteWriter& out, CTestNames& names, GUCEF::Int32 ue4Version, GUCEF::Int32 ue5Version );

    bool IsNewLayout( void ) const;

    /**
     *  Writes the tag. For a boolean property the value is the argument boolValue (it is part of the tag). Options: a static
     *  array index, the binary/native flag, a property GUID, the overridable extension.
     */
    SOpen Begin( const GUCEF::CORE::CString& name, const CTestType& type, bool boolValue = false, GUCEF::Int32 arrayIndex = 0, bool nativeFlag = false, bool withGuid = false, bool withExtension = false );

    void End( const SOpen& open );

    /** The tag in front of the elements of a classic array of structs; closed with End() after the elements */
    SOpen BeginInnerStructTag( const GUCEF::CORE::CString& propertyName, const GUCEF::CORE::CString& structName );

    void None( void );

    /** The byte that starts the data of an export from UE5 version 1011 */
    void ControlByte( void );

    CTestByteWriter& Out( void ) { return m_out; }

    CTestNames& Names( void ) { return m_names; }

    private:

    CTestByteWriter& m_out;
    CTestNames& m_names;
    GUCEF::Int32 m_ue4;
    GUCEF::Int32 m_ue5;
};

/*-------------------------------------------------------------------------*/

/** Builds a complete package with the summary of the versions asked for */
class CTestPackageWriter
{
    public:

    struct SImport
    {
        GUCEF::CORE::CString classPackage;
        GUCEF::CORE::CString className;
        GUCEF::Int32 outer;
        GUCEF::CORE::CString objectName;
        GUCEF::CORE::CString packageName;
    };

    struct SExport
    {
        GUCEF::Int32 classIndex;
        GUCEF::Int32 outer;
        GUCEF::CORE::CString objectName;
        CTestByteWriter body;
        GUCEF::Int64 scriptStart;
        GUCEF::Int64 scriptEnd;
        GUCEF::Int64 forcedOffset;      /**< when not negative: the serial offset written, for damaged packages */
        GUCEF::Int64 forcedSize;        /**< when not negative: the serial size written */
    };

    struct SPayload
    {
        GUCEF::vector< GUCEF::UInt8 > raw;
        GUCEF::UInt8 id[ 20 ];
        bool stored;
    };

    GUCEF::Int32 legacy;
    GUCEF::Int32 ue4;
    GUCEF::Int32 ue5;
    GUCEF::Int32 licensee;
    GUCEF::UInt32 packageFlags;
    GUCEF::CORE::CString packageName;
    GUCEF::UInt16 engineMajor;
    GUCEF::UInt16 engineMinor;
    GUCEF::UInt16 enginePatch;
    GUCEF::UInt32 engineChangelist;
    GUCEF::CORE::CString engineBranch;
    GUCEF::vector< GUCEF::ASSETPLUGIN::UNREAL::CUnrealCustomVersion > customVersions;
    GUCEF::vector< GUCEF::ASSETPLUGIN::UNREAL::CUnrealSoftObjectPath > softPaths;
    GUCEF::Int32 trailerVersion;
    GUCEF::Int32 compressedChunks;
    GUCEF::UInt32 tag;
    CTestNames names;
    GUCEF::vector< SImport > imports;
    GUCEF::vector< SExport > exports;
    GUCEF::vector< SPayload > payloads;

    CTestPackageWriter( GUCEF::Int32 legacyVersion, GUCEF::Int32 ue4Version, GUCEF::Int32 ue5Version );

    /** Returns the (negative) index of the import */
    GUCEF::Int32 AddImport( const GUCEF::CORE::CString& classPackage, const GUCEF::CORE::CString& className, GUCEF::Int32 outer, const GUCEF::CORE::CString& objectName );

    /** Returns the (positive) index of the export; the body is the data of the export, with scriptEnd -1 meaning all of it */
    GUCEF::Int32 AddExport( GUCEF::Int32 classIndex, GUCEF::Int32 outer, const GUCEF::CORE::CString& objectName, const CTestByteWriter& body, GUCEF::Int64 scriptEnd = -1 );

    /** Adds a payload to the trailer; returns its position. The id is made from the bytes. */
    GUCEF::UInt32 AddPayload( const GUCEF::vector< GUCEF::UInt8 >& raw, bool stored = true );

    /** Starts a body: the control byte of the versions that have one */
    void BeginBody( CTestByteWriter& body ) const;

    void Build( GUCEF::vector< GUCEF::UInt8 >& out );

    /** The 48 byte record of editor bulk data that refers to a payload of the trailer */
    void WriteEditorBulkData( CTestByteWriter& out, GUCEF::UInt32 payloadIndex ) const;

    private:

    void WriteSummary( CTestByteWriter& w, GUCEF::Int32 headerSize, GUCEF::Int32 nameOffset, GUCEF::Int32 softOffset, GUCEF::Int32 exportOffset, GUCEF::Int32 importOffset, GUCEF::Int64 trailerOffset );
    void WriteImport( CTestByteWriter& w, const SImport& object );
    void WriteExport( CTestByteWriter& w, const SExport& object, GUCEF::Int64 offset );
    void WriteSoftPath( CTestByteWriter& w, const GUCEF::ASSETPLUGIN::UNREAL::CUnrealSoftObjectPath& path );
};

/*-------------------------------------------------------------------------*/

/** Builds a compressed buffer (method 0, stored) of the raw bytes, as the payloads of a trailer hold them */
void BuildStoredCompressedBuffer( const GUCEF::vector< GUCEF::UInt8 >& raw, GUCEF::vector< GUCEF::UInt8 >& out );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALWRITER_H ? */

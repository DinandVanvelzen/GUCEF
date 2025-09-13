/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#define GUCEF_CORE_DVCPPSTRINGUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <typeinfo> 

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"           /* generic structs */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"          /* string class implementation */
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#undef DeleteFile

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns the given relative path into an absolute path
 *  You can use this function to implement support for variables in path names
 *  This function will also unify the dir seperators to the O/S standard.
 *
 *  Note that if 'resolveVars' is false only dir segments are processed
 *  this function uses ResolveVars(), see it's documentation on supported variables
 *
 *  Additional supported variables are:
 *      - ..           -> If dir segments are available before this one the path is shortened
 */
GUCEF_CORE_PUBLIC_CPP CString
RelativePath( const CString& relpath  ,
              bool resolveVars = true );


/*-------------------------------------------------------------------------*/

/**
 *  You can use this function to implement support for certain supported variables in strings
 *  Supported variables are:
 *      - $CURWORKDIR$ -> Is resolved to the current working directory
 *      - $MODULEDIR$  -> Is resolved to the directory this module is located in
 *      - $ENVVAR:FOO$ -> Resolves to the environment variable FOO's content
 *      - $HOSTNAME$   -> Is resolved to the hostname of the system
 *      - $PID$        -> Is resolved to the process ID of the current process
 */
GUCEF_CORE_PUBLIC_CPP CString
ResolveVars( const CString& strWithVars );

/*-------------------------------------------------------------------------*/

/**
 *  Get an OS defined temp directory
 */
GUCEF_CORE_PUBLIC_CPP CString
TempDir( void );

/*-------------------------------------------------------------------------*/

/**
 *  Get the directory the module is located in
 */
GUCEF_CORE_PUBLIC_CPP CString
ModuleDir( void );

/*-------------------------------------------------------------------------*/

/**
 *  Get the current application working directory
 */
GUCEF_CORE_PUBLIC_CPP CString
CurrentWorkingDir( void );

/*-------------------------------------------------------------------------*/

/**
 *  Determines the most compact relative path between the two paths given.
 *  the given paths can have variables in them, these will be resolved via
 *  RelativePath(). If 'resolveVars' is false only dir segments are processed
 *  not variables.
 */
GUCEF_CORE_PUBLIC_CPP CString
GetRelativePathToOtherPathRoot( const CString& fromPath ,
                                const CString& toPath   ,
                                bool resolveVars = true );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
AppendToPath( CString& path           ,
              const CString& addition );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
CombinePath( const CString& path     ,
             const CString& addition );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
StripFilename( const CString& pathPlusFilename );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
StripLastSubDir( const CString& completePath );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
ExtractFilename( const CString& pathPlusFilename );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP const TVersion
StringToVersion( const CString& version );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
VersionToString( const TVersion& version    ,
                 bool includeMinor = true   ,
                 bool includePatch = true   ,
                 bool includeRelease = true );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
BoolToString( const bool value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
StringToBool( const CString& str, bool defaultIfNeeded = false );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP std::size_t
StringToSizeT( const CString& str, std::size_t defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
SizeTToString( const std::size_t value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt64
StringToUInt64( const CString& str, UInt64 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
UInt64ToString( const UInt64 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP Int64
StringToInt64( const CString& str, Int64 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Int64ToString( const Int64 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP Int32
StringToInt32( const CString& str, Int32 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Int32ToString( const Int32 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt32
StringToUInt32( const CString& str, UInt32 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
UInt32ToString( const UInt32 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt16
StringToUInt16( const CString& str, UInt16 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
UInt16ToString( const UInt16 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP Int16
StringToInt16( const CString& str, Int16 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Int16ToString( const Int16 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP Int8
StringToInt8( const CString& str, Int8 defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
UInt8ToString( const UInt8 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP UInt8
StringToUInt8( const CString& str, UInt8 defaultValue = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Int8ToString( const Int8 value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString::StringVector
StringToStringVector( const CString& str                                                        , 
                      const CString::StringVector& defaultIfNeeded = CString::EmptyStringVector ,
                      char seperator = ','                                                      );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
StringVectorToString( const CString::StringVector& strVector          , 
                      const CString& defaultIfNeeded = CString::Empty ,
                      char seperator = ','                            );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString::StringSet
StringToStringSet( const CString& str                                                  , 
                   const CString::StringSet& defaultIfNeeded = CString::EmptyStringSet ,
                   char seperator = ','                                                );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
StringSetToString( const CString::StringSet& strSet                , 
                   const CString& defaultIfNeeded = CString::Empty ,
                   char seperator = ','                            );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
StringSetToStringVector( const CString::StringSet& strSet ,
                         CString::StringVector& strVector );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
PointerToString( const void* value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void*
StringToPointer( const CString& value, void* defaultIfNeeded = GUCEF_NULL );

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_NO_MD5_SUPPORT

GUCEF_CORE_PUBLIC_CPP CString
StringToMD5String( const CString& str );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
MD5ToString( const UInt8 md5Digest[ 16 ] );

#endif /* GUCEF_CORE_NO_MD5_SUPPORT ? */

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
ConvertHexCharsToByte( const char hexCharA  ,
                       const char hexCharB  ,
                       char& byte           );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP void
ConvertByteToHexChars( const char byte ,
                       char& hexCharA  ,
                       char& hexCharB  );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
ConvertBytesToHexString( const void* byteBuffer ,
                         UInt32 bufferSize      ,
                         bool addSpaces         ,
                         bool addHexPrefix      );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Base16Encode( const void* byteBuffer, UInt32 bufferSize );

/*-------------------------------------------------------------------------*/

/**
 *  Decodes a base16 encoded string into binary data
 */
GUCEF_CORE_PUBLIC_CPP bool
Base16Decode( const CString& base16Str, bool hasHexPrefix, void* byteBuffer, UInt32 bufferSize, UInt32& bytesUsed );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Base64Encode( const void* byteBuffer, UInt32 bufferSize );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
Base64Encode( const CString& toBeEncodedStr );

/*-------------------------------------------------------------------------*/

/**
 *  Decodes a base64 encoded string into binary data
 *
 *  Note that if you make bufferSize base64Str.ByteSize() you will always have a buffer 
 *  large enough since base 64 encoding never reduces the size of binary data
 */
GUCEF_CORE_PUBLIC_CPP bool
Base64Decode( const CString& base64Str, void* byteBuffer, UInt32 bufferSize, UInt32& bytesUsed );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
FloatToString( const float value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP float
StringToFloat( const CString& str, float defaultIfNeeded = 0.0f );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
DoubleToString( const double value );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP double
StringToDouble( const CString& str, double defaultIfNeeded = 0 );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
LastSubDir( const CString& path, UInt32 dirSepCp = GUCEF_DIRSEPCHAR );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
IsPathValid( const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
ExtractFileExtention( const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
StripFileExtention( const CString& path );

/*-------------------------------------------------------------------------*/

/**
 *  Checks whether the file path points to a file in the exact given dir path
 */
GUCEF_CORE_PUBLIC_CPP bool
IsFileInDir( const CString& dirPath  ,
             const CString& filePath );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to load the text file into a string
 *
 *  This function can unify any end-of-line characters in the given file
 *  to the given eolString. If no eolString is given then the platform
 *  native eol format is used.
 */
GUCEF_CORE_PUBLIC_CPP bool
LoadTextFileAsString( const CString& filePath      ,
                      CString& fileContent         ,
                      const bool unifyEol = false  ,
                      const char* eolString = NULL );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to write the string as a text file
 *
 *  This function can unify any end-of-line characters in the given content
 *  string to the given eolString. If no eolString is given then the platform
 *  native eol format is used.
 */
GUCEF_CORE_PUBLIC_CPP bool
WriteStringAsTextFile( const CString& filePath                      ,
                       const CString& fileContent                   ,
                       const bool unifyEol = false                  ,
                       const char* eolString = GUCEF_NULL           ,
                       const bool onlyWriteIfContentDiffers = false );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts recursively create all the dirs in the path if they don't exist
 */
GUCEF_CORE_PUBLIC_CPP bool
CreateDirs( const CString& path );

/*-------------------------------------------------------------------------*/

/**
 *  Attempts to determine if the given path is an absolute path
 */
GUCEF_CORE_PUBLIC_CPP bool
IsAbsolutePath( const CString& path );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
GetHostname( void );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
IsANumber( const CString& strToTest );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
Utf16toUtf8( const std::wstring& wstr ,
             std::string& str         );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool
Utf8toUtf16( const std::string& str ,
             std::wstring& wstr     );

/*-------------------------------------------------------------------------*/

inline CAsciiString ToAsciiString( const CUtf8String& str ) { return CAsciiString( str ); }
inline CAsciiString ToAsciiString( const CAsciiString& str ) { return str; }
inline CUtf8String ToUtf8String( const CUtf8String& str ) { return str; }
inline CUtf8String ToUtf8String( const CAsciiString& str ) { return CUtf8String( str ); }
inline CUtf8String ToUtf8String( const std::wstring& str ) { std::string out; Utf16toUtf8( str, out ); return out; }
inline std::wstring ToWString( const CUtf8String& str ) { std::wstring out; Utf8toUtf16( str, out ); return out; }
inline std::wstring ToWString( const CAsciiString& str ) { std::wstring out; Utf8toUtf16( str, out ); return out; }
inline std::wstring ToWString( const wchar_t* str ) { return std::wstring( str ); }
inline std::wstring ToWString( const char* str ) { std::wstring out; Utf8toUtf16( str, out ); return out; }

/*-------------------------------------------------------------------------*/

#if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
inline CString ToString( const CUtf8String& str ) { return CAsciiString( str ); }
inline CString ToString( const CAsciiString& str ) { return str; }
inline CString ToString( const CUtf8String::StringVector& el ) { CUtf8String out; return CAsciiString( out.Combine( el, ',' ) ); }
inline CString ToString( const CAsciiString::StringVector& el ) { CAsciiString out; return out.Combine( el, ',' ); }
inline CString ToString( const CUtf8String::StringSet& el ) { CUtf8String out; return CAsciiString( out.Combine( el, ',' ) ); }
inline CString ToString( const CAsciiString::StringSet& el ) { CAsciiString out; return out.Combine( el, ',' ); }
#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
inline CString ToString( const CUtf8String& str ) { return str; }
inline CString ToString( const CAsciiString& str ) { return CUtf8String( str ); }
inline CString ToString( const CUtf8String::StringVector& el, Int32 sepChar = ',' ) { CUtf8String out; return out.Combine( el, sepChar ); }
inline CString ToString( const CAsciiString::StringVector& el, char sepChar = ',' ) { CAsciiString out; return CUtf8String( out.Combine( el, sepChar ) ); }
inline CString ToString( const CUtf8String::StringSet& el, Int32 sepChar = ',' ) { CUtf8String out; return out.Combine( el, sepChar ); }
inline CString ToString( const CAsciiString::StringSet& el, char sepChar = ',' ) { CAsciiString out; return CUtf8String( out.Combine( el, sepChar ) ); }
inline CString ToString( const CUtf8String::StringMap& el ) { CUtf8String out; return out.Combine( el, '=', ',' ); }
#endif

inline CString ToString( UInt8 value ) { return UInt8ToString( value ); }
inline CString ToString( UInt16 value ) { return UInt16ToString( value ); }
inline CString ToString( UInt32 value ) { return UInt32ToString( value ); }
inline CString ToString( UInt64 value ) { return UInt64ToString( value ); }
inline CString ToString( Int8 value ) { return Int8ToString( value ); }
inline CString ToString( Int16 value ) { return Int16ToString( value ); }
inline CString ToString( Int32 value ) { return Int32ToString( value ); }
inline CString ToString( Int64 value ) { return Int64ToString( value ); }
inline CString ToString( Float32 value ) { return FloatToString( value ); }
inline CString ToString( Float64 value ) { return DoubleToString( value ); }
inline CString ToString( bool value ) { return BoolToString( value ); }
inline CString ToString( const char* value ) { return CString( value ); }
inline CString ToString( const wchar_t* value ) { return CString( value ); }
inline CString ToString( const void* value ) { return PointerToString( value ); }
inline CString ToString( const TVersion& value ) { return VersionToString( value ); }
inline CString ToString( const std::string& value ) { return CString( value ); }
inline CString ToString( const std::wstring& value ) { std::string out; Utf16toUtf8( value, out ); return out; }

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
#ifdef GUCEF_32BIT
inline CString ToString( size_t value ) { return UInt32ToString( (UInt32) value ); }
#else
inline CString ToString( size_t value ) { return UInt64ToString( (UInt64) value ); }
#endif
#endif

/*-------------------------------------------------------------------------*/

template < typename typeNameParam >
inline CString ToString( void ) { return typeid( typeNameParam ).name(); }

/*-------------------------------------------------------------------------*/

/**
 *  Helper for templates to convert a string to a build-in type
 */
template < typename numberType >
inline
numberType
StringToNumber( const CString& str, numberType defaultIfNeeded = 0 )
{GUCEF_TRACE;
    return defaultIfNeeded;
}

template <> inline Int8 StringToNumber< Int8 >( const CString& str, Int8 defaultIfNeeded ) {GUCEF_TRACE; return StringToInt8( str, defaultIfNeeded ); }
template <> inline UInt8 StringToNumber< UInt8 >( const CString& str, UInt8 defaultIfNeeded ) {GUCEF_TRACE; return StringToUInt8( str, defaultIfNeeded ); }
template <> inline Int16 StringToNumber< Int16 >( const CString& str, Int16 defaultIfNeeded ) {GUCEF_TRACE; return StringToInt16( str, defaultIfNeeded ); }
template <> inline UInt16 StringToNumber< UInt16 >( const CString& str, UInt16 defaultIfNeeded ) {GUCEF_TRACE; return StringToUInt16( str, defaultIfNeeded ); }
template <> inline Int32 StringToNumber< Int32 >( const CString& str, Int32 defaultIfNeeded ) {GUCEF_TRACE; return StringToInt32( str, defaultIfNeeded ); }
template <> inline UInt32 StringToNumber< UInt32 >( const CString& str, UInt32 defaultIfNeeded ) {GUCEF_TRACE; return StringToUInt32( str, defaultIfNeeded ); }
template <> inline Int64 StringToNumber< Int64 >( const CString& str, Int64 defaultIfNeeded ) {GUCEF_TRACE; return StringToInt64( str, defaultIfNeeded ); }
template <> inline UInt64 StringToNumber< UInt64 >( const CString& str, UInt64 defaultIfNeeded ) {GUCEF_TRACE; return StringToUInt64( str, defaultIfNeeded ); }
template <> inline Float32 StringToNumber< Float32 >( const CString& str, Float32 defaultIfNeeded ) {GUCEF_TRACE; return StringToFloat( str, defaultIfNeeded ); }
template <> inline Float64 StringToNumber< Float64 >( const CString& str, Float64 defaultIfNeeded ) {GUCEF_TRACE; return StringToDouble( str, defaultIfNeeded ); }

/*-------------------------------------------------------------------------*/

inline CString::StringSet ToStringSet( const CUtf8String& str ) { return StringToStringSet( str ); }
inline CString::StringSet ToStringSet( const CAsciiString& str ) { return StringToStringSet( str ); }
inline CString::StringSet ToStringSet( const CUtf8String::StringVector& vec ) { CString::StringSet set; CUtf8String::StringVector::const_iterator i=vec.begin(); while ( i != vec.end() ) { set.insert( (*i) ); ++i; } return set; }
inline CString::StringSet ToStringSet( const CAsciiString::StringVector& vec ) { CString::StringSet set; CAsciiString::StringVector::const_iterator i=vec.begin(); while ( i != vec.end() ) { set.insert( (*i) ); ++i; } return set; }
#if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
inline CString::StringSet ToStringSet( const CUtf8String::StringSet& orgSet ) { CString::StringSet set; CUtf8String::StringSet::const_iterator i=orgSet.begin(); while ( i != orgSet.end() ) { set.insert( (*i) ); ++i; } return set; }
inline CString::StringSet ToStringSet( const CAsciiString::StringSet& orgSet ) { return orgSet; }
#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
inline CString::StringSet ToStringSet( const CUtf8String::StringSet& orgSet ) { return orgSet; }
inline CString::StringSet ToStringSet( const CAsciiString::StringSet& orgSet ) { CString::StringSet set; CAsciiString::StringSet::const_iterator i=orgSet.begin(); while ( i != orgSet.end() ) { set.insert( (*i) ); ++i; } return set; }
#endif

/*-------------------------------------------------------------------------*/

inline CString::StringVector ToStringVector( const CUtf8String& str ) { return StringToStringVector( str ); }
inline CString::StringVector ToStringVector( const CAsciiString& str ) { return StringToStringVector( str ); }
inline CString::StringVector ToStringVector( const CUtf8String::StringSet& set ) { CString::StringVector vec; CUtf8String::StringSet::const_iterator i=set.begin(); while ( i != set.end() ) { vec.push_back( (*i) ); ++i; } return vec; }
inline CString::StringVector ToStringVector( const CAsciiString::StringSet& set ) { CString::StringVector vec; CAsciiString::StringSet::const_iterator i=set.begin(); while ( i != set.end() ) { vec.push_back( (*i) ); ++i; } return vec; }
#if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
inline CString::StringVector ToStringVector( const CUtf8String::StringVector& orgVec ) { CString::StringVector vec; CUtf8String::StringVector::const_iterator i=orgVec.begin(); while ( i != orgVec.end() ) { vec.push_back( (*i) ); ++i; } return vec; }
inline CString::StringVector ToStringVector( const CAsciiString::StringVector& orgVec ) { return orgVec; }
#elif ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
inline CString::StringVector ToStringVector( const CUtf8String::StringVector& orgVec ) { return orgVec; }
inline CString::StringVector ToStringVector( const CAsciiString::StringVector& orgVec ) { CString::StringVector vec; CAsciiString::StringVector::const_iterator i=orgVec.begin(); while ( i != orgVec.end() ) { vec.push_back( (*i) ); ++i; } return vec; }
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

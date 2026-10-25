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

#include <string.h>
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

class CDynamicBuffer;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns the given relative path into an absolute path
 *  You can use this function to implement support for variables in path names
 *  This function will also unify the dir separators to the O/S standard.
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
 *      - $CACHEDIR$   -> Is resolved to the per user directory for data that can be made again (SPECIALDIR_CACHE_DIR),
 *                        the temp dir when the platform has none
 *      - $APPDATADIR$ -> Is resolved to the per user directory for data of the user on this machine that should stay
 *                        (SPECIALDIR_LOCAL_APPDATA_DIR), the temp dir when the platform has none
 *      - $APPCONFIGDIR$ -> Is resolved to the per user directory for the settings of applications (SPECIALDIR_APP_CONFIG_DIR),
 *                        else as $APPDATADIR$
 */
GUCEF_CORE_PUBLIC_CPP CString
ResolveVars( const CString& strWithVars );

/*-------------------------------------------------------------------------*/

/**
 *  The other way round: a path in the form with the variable of the directory it is in, for example
 *  "$APPCONFIGDIR$/MyApp" for the settings directory of the user, so that it stays right on another machine or for
 *  another user. The variable whose directory is the longest start of the path is used, of $APPCONFIGDIR$,
 *  $APPDATADIR$, $CACHEDIR$, $MODULEDIR$ and $TEMPDIR$; a path in none of them is given back as it is
 */
GUCEF_CORE_PUBLIC_CPP CString
UnresolveVars( const CString& path );

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

/**
 *  Encodes binary data as base64url (RFC 4648 section 5): the alphabet with '-' and '_' in place of '+' and '/'.
 *  Without padding ('=') unless 'pad' is set, as JWT and JWS use it. One allocation for the result.
 *  @return false, with an empty 'result', when 'data' is NULL for a length above 0 or the text cannot be allocated
 */
GUCEF_CORE_PUBLIC_CPP bool
Base64UrlEncode( const void* data, UInt32 length, CString& result, bool pad = false );

/*-------------------------------------------------------------------------*/

/**
 *  Decodes base64url text (RFC 4648 section 5) into 'result', which is replaced. No allocation once 'result' has
 *  grown to the size needed. The decoder is strict:
 *  - padding is optional, but when it is there the text must be a whole number of groups of 4 with 1 or 2 '='
 *  - only the characters A-Z a-z 0-9 '-' '_' are accepted: '+', '/', whitespace and anything else is refused
 *  - a text length that leaves 1 character in the last group is refused
 *  - the unused bits of the last character must be 0, so every byte sequence has one text only
 *  Empty text gives empty data. On failure 'result' is empty.
 */
GUCEF_CORE_PUBLIC_CPP bool
Base64UrlDecode( const CString& text, CDynamicBuffer& result );

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

/**
 *  The login name of the user the process runs as, as UTF-8 (the account name, for example "dinand"), empty when it cannot be
 *  determined or the platform has no users (Android, WebAssembly). Thread safe.
 *  Linux and macOS: the password database entry of the effective user id, then the environment variables USER and LOGNAME.
 *  Windows: GetUserNameW(), then USERNAME.
 */
GUCEF_CORE_PUBLIC_CPP CString
GetCurrentUserName( void );

/*-------------------------------------------------------------------------*/

/**
 *  The full name of the user the process runs as, as UTF-8 (for example "Dinand van Velzen"), empty when the system has none
 *  recorded or the platform has no users. Thread safe.
 *  Linux and macOS: the first comma separated field of the GECOS field of the password database entry; empty when that
 *  is empty, callers fall back to GetCurrentUserName().
 *  Windows: GetUserNameExW( NameDisplay ), which only accounts that have a display name give; the login name when it does not.
 */
GUCEF_CORE_PUBLIC_CPP CString
GetCurrentUserDisplayName( void );

/*-------------------------------------------------------------------------*/

/** The user name from the environment ("USER", then "LOGNAME"; "USERNAME" on Windows) as GetEnv() sees it, so overrides apply; empty when there is none */
GUCEF_CORE_PUBLIC_CPP CString
GetUserNameFromEnvironment( void );

/*-------------------------------------------------------------------------*/

/** The full name in a GECOS field of the password database: the text before the first comma, trimmed; empty for an empty field */
GUCEF_CORE_PUBLIC_CPP CString
GetDisplayNameFromGecos( const CString& gecos );

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
inline CString ToString( const TVersion& value ) { return VersionToString( value ); }
inline CString ToString( const std::string& value ) { return CString( value ); }
inline CString ToString( const std::wstring& value ) { std::string out; Utf16toUtf8( value, out ); return out; }

/*-------------------------------------------------------------------------*/

// Catch-all for any other pointer type (including void*)
template <typename T>
inline CString ToString( T* pointer )
{GUCEF_TRACE;
    return PointerToString( static_cast<const void*>( pointer ) );
}

/*-------------------------------------------------------------------------*/

// Portable size_t routing:
// Enabled only when size_t is not the same type as UInt32/UInt64 or unsigned long
template < typename T >
inline CString ToString( const T value,
                         typename ::GUCEF::EnableIf<
                             ::GUCEF::TypesAreExactlySame< T, size_t >::value &&
                            !::GUCEF::TypesAreExactlySame< size_t, ::GUCEF::UInt32 >::value &&
                            !::GUCEF::TypesAreExactlySame< size_t, ::GUCEF::UInt64 >::value &&
                            !::GUCEF::TypesAreExactlySame< size_t, unsigned long >::value,
                             int
                         >::type* /*sfinae*/ = 0 )
{GUCEF_TRACE;
    return SizeTToString( static_cast<size_t>( value ) );
}

/*-------------------------------------------------------------------------*/

// Portable unsigned long routing:
// Enabled only when unsigned long is not the same type as UInt32/UInt64. Note that unsigned long can be the same type as size_t, for example on wasm32,
// in which case the size_t routing above is disabled and this one is the one that handles size_t
template < typename T >
inline CString ToString( const T value,
                         typename ::GUCEF::EnableIf<
                             ::GUCEF::TypesAreExactlySame< T, unsigned long >::value &&
                            !::GUCEF::TypesAreExactlySame< unsigned long, ::GUCEF::UInt32 >::value &&
                            !::GUCEF::TypesAreExactlySame< unsigned long, ::GUCEF::UInt64 >::value,
                             long
                         >::type* /*sfinae*/ = 0 )
{GUCEF_TRACE;
    return sizeof(unsigned long) == sizeof(::GUCEF::UInt64)
        ? UInt64ToString( static_cast< ::GUCEF::UInt64 >( value ) )
        : UInt32ToString( static_cast< ::GUCEF::UInt32 >( value ) );
}

/*-------------------------------------------------------------------------*/

// Portable signed long routing:
// Enabled only when long is not the same type as Int32/Int64
template < typename T >
inline CString ToString( const T value,
                         typename ::GUCEF::EnableIf<
                             ::GUCEF::TypesAreExactlySame< T, long >::value &&
                            !::GUCEF::TypesAreExactlySame< long, ::GUCEF::Int32 >::value &&
                            !::GUCEF::TypesAreExactlySame< long, ::GUCEF::Int64 >::value,
                             short
                         >::type* /*sfinae*/ = 0 )
{GUCEF_TRACE;
    return sizeof(long) == sizeof(::GUCEF::Int64)
        ? Int64ToString( static_cast< ::GUCEF::Int64 >( value ) )
        : Int32ToString( static_cast< ::GUCEF::Int32 >( value ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Cuts the name of the type out of the text of a compiler of the signature of the function template ToString< T >()
 *  ( __PRETTY_FUNCTION__ or __FUNCSIG__ ). Used when RTTI is not available to get a type name from.
 */
inline CString
TypeNameFromToStringSignature( const char* signature )
{
    CString text( signature );
    const char* start = strstr( signature, "typeNameParam = " );    // GCC and Clang: "... [with typeNameParam = Type]"
    const char* end = GUCEF_NULL;
    if ( GUCEF_NULL != start )
    {
        start += 16;
        end = strrchr( start, ']' );
        const char* more = strchr( start, ';' );                    // GCC adds the other typedefs it resolved: "; CString = ..."
        if ( GUCEF_NULL != more && ( GUCEF_NULL == end || more < end ) )
            end = more;
    }
    else
    {
        start = strstr( signature, "ToString<" );                   // Visual C++: "... ToString<Type>(void)"
        if ( GUCEF_NULL != start )
        {
            start += 9;
            end = strrchr( start, '>' );
        }
    }
    if ( GUCEF_NULL == start || GUCEF_NULL == end || end < start )
        return text;                                                // the whole signature is unique per type as well
    return text.SubstrFromRange( (UInt32) ( start - signature ), (UInt32) ( end - signature ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  A name of the type that is the same for the same type and different for another, used for example as the
 *  key of a factory. Uses typeid when RTTI is available, else the signature of this function template, so
 *  this works with RTTI disabled as well. The text can differ between the two.
 */
template < typename typeNameParam >
inline CString ToString( void )
{
    #ifdef GUCEF_RTTI_IS_SUPPORTED
    return typeid( typeNameParam ).name();
    #elif defined( _MSC_VER )
    return TypeNameFromToStringSignature( __FUNCSIG__ );
    #elif defined( __GNUC__ ) || defined( __clang__ )
    return TypeNameFromToStringSignature( __PRETTY_FUNCTION__ );
    #else
    #error "No way to get a type name without RTTI on this compiler"
    #endif
}

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

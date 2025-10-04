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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"         /* C string utils */
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "dvcppstringutils.h"   /* function prototypes of the functions implemented here */

#ifndef GUCEF_CORE_NO_MD5_SUPPORT
  #ifndef GUCEF_CORE_DVMD5UTILS_H
  #include "dvmd5utils.h"
  #define GUCEF_CORE_DVMD5UTILS_H
  #endif /* GUCEF_CORE_DVMD5UTILS_H ? */
#endif /* GUCEF_CORE_NO_MD5_SUPPORT ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>		/* WIN32 API */
  #undef min
  #undef max
  #define MAX_DIR_LENGTH MAX_PATH
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <unistd.h>             /* POSIX utilities */
  #include <limits.h>             /* Linux OS limits */
  #define MAX_DIR_LENGTH PATH_MAX
  #include <linux/kernel.h>
  #include <linux/string.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
  #include <unistd.h>             /* POSIX utilities */
  #include <limits.h>             /* Linux OS limits */
  #define MAX_DIR_LENGTH PATH_MAX
#else
  #error Unsupported OS
#endif

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"        /* all kinds of file utils */
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"        /* all kinds of file utils */
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

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

bool
IsANumber( const CString& strToTest )
{GUCEF_TRACE;

    if ( strToTest.IsNULLOrEmpty() )
        return false;

    UInt32 codePoint = 0;
    
    // Check for a leading sign
    UInt32 startPos = 0;
    codePoint = strToTest[ 0 ];
    if ( codePoint == '-' || codePoint == '+' )
        ++startPos;

    // Check for a decimal point
    bool decimalPointFound = false;
    for ( UInt32 i=startPos; i<strToTest.Length(); ++i )
    {
        UInt32 codePoint = strToTest[ i ];
        if ( codePoint == '.' )
        {
            if ( decimalPointFound )
                return false;
            else
                decimalPointFound = true;
        }
        else
        {
            if ( codePoint < 48 || codePoint > 57 )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
TempDir( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING )
    char buffer[ MAX_DIR_LENGTH+1 ];
    if ( 0 != ::GetTempPathA( MAX_DIR_LENGTH+1, buffer ) )
        return CString( buffer, MAX_DIR_LENGTH, true );
    #else
    wchar_t buffer[ MAX_DIR_LENGTH+1 ];
    if ( 0 != ::GetTempPathW( MAX_DIR_LENGTH+1, buffer ) )
        return CString( buffer, MAX_DIR_LENGTH, true );
    #endif
    #endif
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
ModuleDir( void )
{GUCEF_TRACE;

    char buffer[ MAX_DIR_LENGTH ];
    Module_Path( buffer, MAX_DIR_LENGTH );
    Strip_Filename( buffer );
    return CString( buffer, MAX_DIR_LENGTH, true );
}

/*-------------------------------------------------------------------------*/

CString
CurrentWorkingDir( void )
{GUCEF_TRACE;

    char buffer[ MAX_DIR_LENGTH ];
    Get_Current_Dir( buffer, MAX_DIR_LENGTH );
    return CString( buffer, MAX_DIR_LENGTH, true );
}

/*-------------------------------------------------------------------------*/

static inline void
DivisionRemainder( Int32 dividend   ,
                   Int32 divisor    ,
                   Int32& remainder ,
                   Int32& quotient  )
{GUCEF_TRACE;

    //remainder = dividend % divisor;
    //quotient = (dividend / divisor);

    quotient = dividend / divisor;
    remainder = dividend - divisor * quotient;
}

/*-------------------------------------------------------------------------*/

static const char* g_hexDigits = "0123456789ABCDEF";

/*-------------------------------------------------------------------------*/

void
ConvertHexCharsToByte( const char hexCharA  ,
                       const char hexCharB  ,
                       char& byte           )
{GUCEF_TRACE;

    char hexStr[ 3 ] = { hexCharA, hexCharB, 0 };
    int result = sscanf( hexStr, "%2hhx", &byte );
    if ( 1 != result )
        byte = 0;
}

/*-------------------------------------------------------------------------*/

void
ConvertByteToHexChars( const char byte ,
                       char& hexCharA  ,
                       char& hexCharB  )
{GUCEF_TRACE;

    Int32 digitIndex1 = 0;
    Int32 digitIndex2 = 0;

    DivisionRemainder( byte                ,
                       16                  ,
                       digitIndex1         ,
                       digitIndex2         );

    if ( digitIndex1 < 0 )
        digitIndex1 *= -1;
    if ( digitIndex2 < 0 )
        digitIndex2 *= -1;

    hexCharA = g_hexDigits[ digitIndex2 ];
    hexCharB = g_hexDigits[ digitIndex1 ];
}

/*-------------------------------------------------------------------------*/

CString
ConvertBytesToHexString( const void* byteBuffer ,
                         UInt32 bufferSize      ,
                         bool addSpaces         ,
                         bool addHexPrefix      )
{GUCEF_TRACE;

    const char* charByteBuffer = (const char*) byteBuffer;

    // Each byte is expressed as a 2 char hex value
    UInt32 hexStrSize = bufferSize * 2 + 1;
    if ( addSpaces ) hexStrSize += bufferSize;
    if ( addHexPrefix ) hexStrSize += 2*bufferSize;

    CAsciiString hexString;
    char* digits = hexString.Reserve( hexStrSize );

    Int32 digitIndex1 = 0;
    Int32 digitIndex2 = 0;
    UInt32 digitOffset = 0;
    UInt32 lastDigit = 0;

    for ( UInt32 i=0; i<bufferSize; ++i )
    {
        DivisionRemainder( charByteBuffer[ i ] ,
                           16                  ,
                           digitIndex1         ,
                           digitIndex2         );

        if ( digitIndex1 < 0 )
            digitIndex1 *= -1;
        if ( digitIndex2 < 0 )
            digitIndex2 *= -1;

        if ( addSpaces )
        {
            if ( addHexPrefix )
            {
                digitOffset = 5 * i;
                digits[ digitOffset ] = '0';
                digits[ digitOffset + 1 ] = 'x';
                digits[ digitOffset + 2 ] = g_hexDigits[ digitIndex2 ];
                digits[ digitOffset + 3 ] = g_hexDigits[ digitIndex1 ];
                digits[ digitOffset + 4 ] = ' ';
                lastDigit = digitOffset + 4;
            }
            else
            {
                digitOffset = 3 * i;
                digits[ digitOffset ] = g_hexDigits[ digitIndex2 ];
                digits[ digitOffset + 1 ] = g_hexDigits[ digitIndex1 ];
                digits[ digitOffset + 2 ] = ' ';
                lastDigit = digitOffset + 2;
            }
        }
        else
        {
            if ( addHexPrefix )
            {
                digitOffset = 4 * i;
                digits[ digitOffset ] = '0';
                digits[ digitOffset + 1 ] = 'x';
                digits[ digitOffset + 2 ] = g_hexDigits[ digitIndex2 ];
                digits[ digitOffset + 3 ] = g_hexDigits[ digitIndex1 ];
                lastDigit = digitOffset + 3;
            }
            else
            {
                digitOffset = 2 * i;
                digits[ digitOffset ] = g_hexDigits[ digitIndex2 ];
                digits[ digitOffset + 1 ] = g_hexDigits[ digitIndex1 ];
                lastDigit = digitOffset + 1;
            }
        }
    }
    if ( 0 < lastDigit )
    {
        ++lastDigit;
        digits[ lastDigit ] = '\0';
        hexString.SetLength( lastDigit );
        return ToString( hexString );
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
Base16Encode( const void* byteBuffer, UInt32 bufferSize )
{GUCEF_TRACE;

    return ConvertBytesToHexString( byteBuffer, bufferSize, false, false );
}

/*-------------------------------------------------------------------------*/

bool
Base16Decode( const CString& base16Str, bool hasHexPrefix, void* byteBuffer, UInt32 bufferSize, UInt32& bytesUsed )
{GUCEF_TRACE;

    char* charByteBuffer = static_cast< char* >( byteBuffer );
    CAsciiString asciiBase16Str = CORE::ToAsciiString( base16Str );
    if ( hasHexPrefix )
        asciiBase16Str = asciiBase16Str.ReplaceSubstr( "0x", "" );  // @TODO: case sensitive
    if ( asciiBase16Str.IsNULLOrEmpty() )
    {
        bytesUsed = 0;
        return true;
    }

    UInt32 byteCount = asciiBase16Str.Length() / 2;
    bool prefixZero = false;
    if ( byteCount * 2 < asciiBase16Str.Length() )
    {
        prefixZero = true;
        ++byteCount;
    }

    if ( bufferSize < byteCount )
        return false; // not enough space

    UInt32 base16Index = 0;
    UInt32 byteIndex = 0;
    if ( prefixZero )
    {
        char hexCharA = '0';
        char hexCharB = asciiBase16Str[ base16Index ];
        char byte = 0;
        ConvertHexCharsToByte( hexCharA, hexCharB, byte );
        charByteBuffer[ byteIndex ] = byte;
        ++base16Index; ++byteIndex;
    }

    for ( base16Index; base16Index < asciiBase16Str.Length(); ++base16Index )
    {
        char byte = 0;
        ConvertHexCharsToByte( asciiBase16Str[ base16Index ], asciiBase16Str[ base16Index+1 ], byte );
        charByteBuffer[ byteIndex ] = byte;
        ++base16Index; ++byteIndex;
    }

    bytesUsed = byteIndex;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
Base64Encode( const CString& toBeEncodedStr )
{GUCEF_TRACE;

    return Base64Encode( toBeEncodedStr.C_String(), toBeEncodedStr.ByteSize()-1 );
}

/*-------------------------------------------------------------------------*/

static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int base64_table[ 256 ] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

/*-------------------------------------------------------------------------*/

CString
Base64Encode( const void* byteBuffer, UInt32 bufferSize )
{GUCEF_TRACE;

    CString result;
    UInt32 base64StrLength = ( ( bufferSize + 2 ) / 3 ) * 4;
    char* base64StringBuffer = result.Reserve( base64StrLength+1, (Int32) base64StrLength );
    char* str = base64StringBuffer;

    // Base64 encoding can really add up wrt size requirements
    // We need to sanity check that we were able to allocate enough memory
    if ( GUCEF_NULL == str || base64StrLength+1 != result.ByteSize() )
        return CString::Empty;

    memset( base64StringBuffer, '=', base64StrLength );
    const char* charByteBuffer = (const char*) byteBuffer;

    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for ( size_t pos = 0; pos < bufferSize; ++pos )
    {
        char_array_3[ i++ ] = charByteBuffer[ pos ];
        if ( i == 3 )
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for ( i=0; i<4; ++i )
            {
                *str = base64_chars[ char_array_4[ i ] ];
                ++str;
            }
            i=0;
        }
    }

    if ( i > 0 )
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
        {
            *str = base64_chars[ char_array_4[ j ] ];
            ++str;
        }

        while( (i++ < 3) )
        {
            *str = '=';
            ++str;
        }

    }

    base64StringBuffer[ base64StrLength ] = '\0';
    result.DetermineLength();

    return result;
}

/*-------------------------------------------------------------------------*/

bool
Base64Decode( const CString& base64Str, void* byteBuffer, UInt32 bufferSize, UInt32& bytesUsed )
{GUCEF_TRACE;

    bytesUsed = 0;
    if ( bufferSize == 0 || base64Str.IsNULLOrEmpty() )
        return false;

    UInt32 base64StrLen = base64Str.ByteSize()-1;
    const unsigned char* p = (const unsigned char*) base64Str.C_String();

    UInt32 j = 0,
        pad1 = base64StrLen % 4 || p[ base64StrLen-1 ] == '=',
        pad2 = pad1 && ( base64StrLen % 4 > 2 || p[ base64StrLen-2 ] != '=' );
    const UInt32 last = ( base64StrLen - pad1 ) / 4 << 2;

    UInt32 bufferSizeNeeded = ( last / 4 ) * 3 + pad1 + pad2;
    if ( bufferSize < bufferSizeNeeded )
        return false;

    unsigned char* currentByte = (unsigned char*) byteBuffer;

    for ( UInt32 i=0; i<last; i+=4 )
    {
        int n = base64_table[ p[ i ] ] << 18 | base64_table[ p[ i+1 ] ] << 12 | base64_table[ p[ i+2 ] ] << 6 | base64_table[ p[ i+3 ] ];
        currentByte[ j++ ] = n >> 16;
        currentByte[ j++ ] = n >> 8 & 0xFF;
        currentByte[ j++ ] = n & 0xFF;
    }

    if ( pad1 > 0 )
    {
        int n = base64_table[ p[ last ] ] << 18 | base64_table[ p[ last+1 ] ] << 12;
        currentByte[ j++ ] = n >> 16;
        if ( pad2 > 0 )
        {
            n |= base64_table[ p[ last+2 ] ] << 6;
            currentByte[ j++ ] = n >> 8 & 0xFF;
        }
    }

    bytesUsed = bufferSizeNeeded;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
ResolveVars( const CString& strWithVars )
{GUCEF_TRACE;

    // First resolve any environment variables which in turn could contain other variables
    CString resultStr = strWithVars;
    Int32 idx = 0;
    do
    {
        idx = resultStr.HasSubstr( "$ENVVAR:", idx, true );
        if ( idx > -1 )
        {
            Int32 endIndex = resultStr.HasChar( '$', idx+8, true );

            // Break out if there is no closing char, this is invalid
            if ( endIndex < 0 ) break;

            CString varName = resultStr.SubstrFromRange( idx+8, endIndex );
            CString varValue = ::GUCEF::CORE::GetEnv( varName );
            resultStr = resultStr.ReplaceSubStr( idx, endIndex-idx, varValue );
        }
    }
    while ( idx > -1 );

    resultStr = resultStr.ReplaceSubstr( "$HOSTNAME$", GetHostname() );
    resultStr = resultStr.ReplaceSubstr( "$PID$", UInt32ToString( MT::GetProcessID() ) );
    resultStr = resultStr.ReplaceSubstr( "$TEMPDIR$", TempDir() );
    resultStr = resultStr.ReplaceSubstr( "$MODULEDIR$", ModuleDir() );
    resultStr = resultStr.ReplaceSubstr( "$CURWORKDIR$", CurrentWorkingDir() );

    return resultStr;
}

/*-------------------------------------------------------------------------*/

CString
RelativePath( const CString& relpath ,
              bool resolveVars       )
{GUCEF_TRACE;

    CString resultStr;

    if ( resolveVars )
    {
        // First resolve any environment variables which in turn could contain other variables
        resultStr = ResolveVars( relpath );
    }
    else
    {
        resultStr = relpath;
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // For Ms Windows we will also support resolving volume paths to their
    // 'regular' path which will work with the rest of the filesystem related functions
    if ( resultStr.HasSubstr( "\\\\?\\Volume{" ) == 0 )
    {
        // This is a volume path, resolve it to a regular path
        Int32 closingCurly = resultStr.HasChar( '}', 11, true );
        if ( closingCurly > 0 )
        {
            CString volumeGuid = resultStr.SubstrToIndex( (UInt32) closingCurly + 2, true );

            CString::StringSet volumePaths;
            if ( GetAllFileSystemPathNamesForVolume( volumeGuid, volumePaths ) )
            {
                if ( !volumePaths.empty() )
                {
                    CString pathOnVolume = *volumePaths.begin();
                    CString remnant = resultStr.SubstrToIndex( (UInt32) closingCurly+2, false );
                    resultStr = CombinePath( pathOnVolume, remnant );
                }
            }
            else
            {
                if ( GetAllFileSystemMountPointsForVolume( volumeGuid, volumePaths ) )
                {
                    if ( !volumePaths.empty() )
                    {
                        CString pathOnVolume = *volumePaths.begin();
                        CString remnant = resultStr.SubstrToIndex( (UInt32) closingCurly+2, false );
                        resultStr = CombinePath( pathOnVolume, remnant );
                    }
                }
            }
        }
    }

    #endif

    // Now the up dir segments,...
    // First make sure we use the same seperator char in both string across
    // the entire strings uniformly. This simplyfies the algoritm
    resultStr = resultStr.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR );

    // Now make sure there are no multiple repeating seperators
    // Not that unlikely if the string was human generated
    resultStr = resultStr.CompactRepeatingChar( GUCEF_DIRSEPCHAR );

    // Now keep chopping away up dir segments until we run out of segments
    // that meet our criterea.
    CString upDirSeg = CString( ".." ) + GUCEF_DIRSEPCHAR;
    Int32 upDirIdx = resultStr.HasSubstr( upDirSeg, false );
    if ( upDirIdx > 0 )
    {
        // Split the relative path into segments
        CString::StringVector relSegments( resultStr.ParseElements( GUCEF_DIRSEPCHAR, true ) );
        CString::StringVector resolvedSegments;

        // Process the relative path segments
        for ( CString::StringVector::iterator it = relSegments.begin(); it != relSegments.end(); ++it )
        {
            if ( *it == ".." )
            {
                if ( !resolvedSegments.empty() )
                {
                    if ( resolvedSegments.back() != ".." )
                        resolvedSegments.pop_back();
                    else
                        resolvedSegments.push_back( *it );
                }
                else
                {
                    resolvedSegments.push_back( *it );
                }
            }
            else
            {
                resolvedSegments.push_back( *it );
            }
        }

        // Join the segments back into a single path
        CString result;
        for ( CString::StringVector::iterator it = resolvedSegments.begin(); it != resolvedSegments.end(); ++it )
        {
            if ( !result.IsNULLOrEmpty() )
            {
                result += GUCEF_DIRSEPCHAR;
            }
            result += *it;
        }

        resultStr = result;
    }

    return resultStr;
}

/*---------------------------------------------------------------------------*/

CString
GetRelativePathToOtherPathRoot( const CString& fromPath ,
                                const CString& toPath   ,
                                bool resolveVars        )
{GUCEF_TRACE;

    typedef CString::StringVector  TStringVector;

    // First resolve any variables in the paths,.. normalize
    // This includes getting rid of any redundant foo/../bar segments
    CString absFromPath = RelativePath( fromPath, resolveVars );
    CString absToPath = RelativePath( toPath, resolveVars );

    CString fromPathRemainder = absFromPath.ReplaceChar( '\\', '/' );
    CString toPathRemainder = absToPath.ReplaceChar( '\\', '/' );

    #if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

    // For MS Win we need to take into account that paths are
    // the same even if the case doesn't match. Paths are
    // case-insensitive. We will make sure this doesnt affect
    // us by checking the equality in a case-insensitive manner.
    // This principle also applies to drive letters.

    // Now just check for the overlap in paths and based on the overlap
    // we will make a relative path
    UInt32 pathEquality = fromPathRemainder.FindMaxSegmentEquality( toPathRemainder, '/', true, false );

    #else

    // Now just check for the overlap in paths and based on the overlap
    // we will make a relative path
    UInt32 pathEquality = fromPathRemainder.FindMaxSegmentEquality( toPathRemainder, '/', true, true );

    #endif

    // If there is overlap we will need to determine how to get to the other path
    // If there is no overlap then there is nothing to do
    if ( pathEquality > 0 )
    {
        // fromPathRemainder
        //     foo/bar/example
        //   -->
        // toPathRemainder
        //     foo/lol
        //   =
        //     ../../lol

        CString relativePath;

        // We use the 'from' path to determine how many up dir segments we need
        fromPathRemainder = fromPathRemainder.CutChars( pathEquality, true, 0 );
        TStringVector upDirElements = fromPathRemainder.ParseElements( '/', false );        
        for ( UInt32 i=0; i<upDirElements.size(); ++i )
        {
            relativePath += "../";
        }

        toPathRemainder = toPathRemainder.CutChars( pathEquality, true, 0 );
        relativePath = CombinePath( relativePath, toPathRemainder );
        if ( !relativePath.IsNULLOrEmpty() && '/' == relativePath[ 0 ] )
            relativePath = relativePath.CutChars( 1, true, 0 );
        return RelativePath( relativePath, resolveVars );
    }

    // No overlap so we return the entire absolute path to the 'to' location
    return absToPath;
}

/*-------------------------------------------------------------------------*/

void
AppendToPath( CString& path           ,
              const CString& addition )
{GUCEF_TRACE;

    if ( addition.Length() == 0 ) return;

    if ( path.Length() == 0 )
    {
        path = addition;
        return;
    }

    Int32 pathLastChar = path[ path.Length()-1 ];
    if ( pathLastChar == '/' || pathLastChar == '\\' )
    {
        if ( addition[ 0 ] == '/' || addition[ 0 ] == '\\' )
        {
            path += (addition.C_String()+1);
            return;
        }
        path += addition;
        return;
    }
    else
    {
        if ( addition[ 0 ] == '/' || addition[ 0 ] == '\\' )
        {
            path += addition;
            return;
        }
        path += GUCEF_DIRSEPCHAR + addition;
    }
}

/*-------------------------------------------------------------------------*/

CString
CombinePath( const CString& path     ,
             const CString& addition )
{
    CString newPath( path );
    AppendToPath( newPath, addition );
    return newPath;
}

/*-------------------------------------------------------------------------*/

const TVersion
StringToVersion( const CString& versionStr )
{GUCEF_TRACE;

    TVersion version = { 0, 0, 0, 0 };
    CString::StringVector elements = versionStr.ParseElements( '.' );
    if ( elements.size() > 0 )
    {
        version.major = StringToUInt16( elements[ 0 ] );
        if ( elements.size() > 1 )
        {
            version.minor = StringToUInt16( elements[ 1 ] );
            if ( elements.size() > 2 )
            {
                version.patch = StringToUInt16( elements[ 2 ] );
                if ( elements.size() > 3 )
                {
                    version.release = StringToUInt16( elements[ 3 ] );
                }
            }
        }
    }
    return version;
}

/*-------------------------------------------------------------------------*/

CString
VersionToString( const TVersion& version ,
                 bool includeMinor       ,
                 bool includePatch       ,
                 bool includeRelease     )
{GUCEF_TRACE;

    char verstr[ 25 ];

    if ( includeRelease )
        snprintf( verstr, 25, "%d.%d.%d.%d", version.major, version.minor, version.patch, version.release );
    else
    if ( includePatch )
        snprintf( verstr, 25, "%d.%d.%d", version.major, version.minor, version.patch );
    else
    if ( includeMinor )
        snprintf( verstr, 25, "%d.%d", version.major, version.minor );
    else
        snprintf( verstr, 25, "%d", version.major );

    return verstr;
}

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
BoolToString( const bool value )
{GUCEF_TRACE;

    return Boolint_To_String( value ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
StringToBool( const CString& str, bool defaultIfNeeded )
{GUCEF_TRACE;

    UInt32 boolint = String_To_Boolint_WithDefault( str.C_String(), defaultIfNeeded ? 1 : 0 );
    return boolint > 0;
}

/*-------------------------------------------------------------------------*/

Int64
StringToInt64( const CString& str, Int64 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        Int64 value = defaultIfNeeded;
        #ifdef GUCEF_MSWIN_BUILD
        if ( 1 == sscanf( str.C_String(), "%I64d", &value ) )
        #else
        if ( 1 == sscanf( str.C_String(), "%lld", &value ) )
        #endif

            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
Int64ToString( const Int64 value )
{GUCEF_TRACE;

    char intBuffer[ 21 ];
    #ifdef GUCEF_MSWIN_BUILD
    snprintf( intBuffer, 21, "%I64d", value );
    #else
    snprintf( intBuffer, 21, "%lld", value );
    #endif

    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

std::size_t
StringToSizeT( const CString& str, std::size_t defaultIfNeeded )
{GUCEF_TRACE;

    return (std::size_t) StringToUInt64( str, defaultIfNeeded );
}

/*-------------------------------------------------------------------------*/

CString
SizeTToString( const std::size_t value )
{GUCEF_TRACE;

    return UInt64ToString( value );
}

/*-------------------------------------------------------------------------*/

UInt64
StringToUInt64( const CString& str, UInt64 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        UInt64 value = defaultIfNeeded;
        #ifdef GUCEF_MSWIN_BUILD
        if ( 1 == sscanf( str.C_String(), "%I64u", &value ) )
        #else
        if ( 1 == sscanf( str.C_String(), "%llu", &value ) )
        #endif

            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
UInt64ToString( const UInt64 value )
{GUCEF_TRACE;

    char intBuffer[ 21 ];
    #ifdef GUCEF_MSWIN_BUILD
    snprintf( intBuffer, 21, "%I64u", value );
    #else
    snprintf( intBuffer, 21, "%llu", value );
    #endif

    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

Int32
StringToInt32( const CString& str, Int32 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        Int32 value = defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%d", &value ) )
            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
Int32ToString( const Int32 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%d", value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

UInt32
StringToUInt32( const CString& str, UInt32 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        UInt32 value = defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%u", &value ) )
            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
UInt32ToString( const UInt32 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%u", value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

UInt16
StringToUInt16( const CString& str, UInt16 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        UInt32 value = (UInt32) defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%u", &value ) )
            return (UInt16) value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
UInt16ToString( const UInt16 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%u", (UInt32) value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

Int16
StringToInt16( const CString& str, Int16 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        Int32 value = (Int16) defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%d", &value ) )
            return (Int16) value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
Int16ToString( const Int16 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%d", (Int32) value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

Int8
StringToInt8( const CString& str, Int8 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        Int32 value = (Int8) defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%d", &value ) )
            return (Int8) value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
Int8ToString( const Int8 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%d", (Int32) value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

UInt8
StringToUInt8( const CString& str, UInt8 defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        UInt32 value = (UInt8) defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%u", &value ) )
            return (UInt8) value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
UInt8ToString( const UInt8 value )
{GUCEF_TRACE;

    char intBuffer[ 12 ];
    snprintf( intBuffer, 12, "%u", (UInt32) value );
    return CString( intBuffer );
}

/*-------------------------------------------------------------------------*/

CString
PointerToString( const void* value )
{GUCEF_TRACE;

    char addrBuffer[ 64 ];
    snprintf( addrBuffer, 64, "%p", value );
    return CString( addrBuffer );
}

/*-------------------------------------------------------------------------*/

void*
StringToPointer( const CString& str, void* defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        void* ptr = defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%p", &ptr ) )
            return ptr;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_NO_MD5_SUPPORT

CString
StringToMD5String( const CString& str )
{GUCEF_TRACE;

    UInt8 md5digest[ 16 ];

    md5fromstring( str.C_String() ,
                   md5digest      );

    return MD5ToString( md5digest );
}

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString
MD5ToString( const UInt8 md5Digest[ 16 ] )
{GUCEF_TRACE;

    char md5string[ 48 ];

    md5tostring( md5Digest ,
                 md5string );

    CString md5str;
    md5str.Set( md5string, 48 );

    return md5str;
}

#endif /* GUCEF_CORE_NO_MD5_SUPPORT ? */

/*-------------------------------------------------------------------------*/

CString
FloatToString( const float value )
{GUCEF_TRACE;

    char floatChars[ 16 ];
    snprintf( floatChars, 16, "%f", value );

    return floatChars;
}

/*-------------------------------------------------------------------------*/

float
StringToFloat( const CString& str, float defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        float value = defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%f", &value ) )
            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
DoubleToString( const double value )
{GUCEF_TRACE;

    char doubleChars[ 128 ];
    snprintf( doubleChars, 128, "%lf", value );

    return doubleChars;
}

/*-------------------------------------------------------------------------*/

double
StringToDouble( const CString& str, double defaultIfNeeded )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        double value = defaultIfNeeded;
        if ( 1 == sscanf( str.C_String(), "%lf", &value ) )
            return value;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
StringToStringVector( const CString& str                           ,
                      const CString::StringVector& defaultIfNeeded ,
                      char seperator                               )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        return str.ParseElements( seperator, true );
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
StringVectorToString( const CString::StringVector& strVector ,
                      const CString& defaultIfNeeded         ,
                      char seperator                         )
{GUCEF_TRACE;

    if ( !strVector.empty() )
    {
        CString result;
        CString::StringVector::const_iterator i = strVector.begin();
        while ( i != strVector.end() )
        {
            if ( result.IsNULLOrEmpty() )
                result = (*i);
            else
                result += seperator + (*i);
            ++i;
        }
        return result;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString::StringSet
StringToStringSet( const CString& str                        ,
                   const CString::StringSet& defaultIfNeeded ,
                   char seperator                            )
{GUCEF_TRACE;

    if ( !str.IsNULLOrEmpty() )
    {
        return str.ParseUniqueElements( seperator, true );
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

CString
StringSetToString( const CString::StringSet& strSet ,
                   const CString& defaultIfNeeded   ,
                   char seperator                   )
{GUCEF_TRACE;

    if ( !strSet.empty() )
    {
        CString result;
        CString::StringSet::const_iterator i = strSet.begin();
        while ( i != strSet.end() )
        {
            if ( result.IsNULLOrEmpty() )
                result = (*i);
            else
                result += seperator + (*i);
            ++i;
        }
        return result;
    }
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

void
StringSetToStringVector( const CString::StringSet& strSet ,
                         CString::StringVector& strVector )
{GUCEF_TRACE;

    strVector.reserve( strSet.size() );
    CString::StringSet::const_iterator i = strSet.begin();
    while ( i != strSet.end() )
    {
        strVector.push_back( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CString
LastSubDir( const CString& path, UInt32 dirSepCp )
{GUCEF_TRACE;

    if ( !path.IsNULLOrEmpty() )
    {
        Int32 lastSepCpIndex = path.HasChar( dirSepCp, false );
        if ( lastSepCpIndex < 0 )
            return path; // assume the entire thing is the last sub-dir

        if ( lastSepCpIndex+1 == path.Length() && lastSepCpIndex > 0 )
        {
            // We have a trailing separator
            // for a directory that means we want the section until the next separator minus the trailing separator
            Int32 nextSepCpIndex = path.HasChar( dirSepCp, (UInt32) lastSepCpIndex-1, false );
            if ( nextSepCpIndex < 0 )
                return path.SubstrToIndex( lastSepCpIndex+1, false );// assume the entire thing minus seperator is the last sub-dir
            else
                return path.SubstrFromRange( nextSepCpIndex+1, lastSepCpIndex ); // extract the sub-section we need
        }
        else
        {
            // We dont have a trailing separator
            // for a directory that means we want the section until the next separator
            return path.SubstrToIndex( lastSepCpIndex+1, false );
        }
    }
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
StripLastSubDir( const CString& completePath )
{GUCEF_TRACE;

    if ( 0 != completePath.Length() )
    {
        CString path = completePath.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR );
        if ( path[ path.Length()-1 ] == GUCEF_DIRSEPCHAR )
        {
            path = path.CutChars( 1, false );
        }

        CString remnamt = path.SubstrToChar( GUCEF_DIRSEPCHAR, false );
        if ( remnamt.Length() > 0 )
        {
            return path.CutChars( remnamt.Length()+1, false );
        }
        return completePath;
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

CString
StripFilename( const CString& pathPlusFilename )
{GUCEF_TRACE;

    Int32 sepCharIndex1 = pathPlusFilename.HasChar( GUCEF_DIRSEPCHAR, false );
    Int32 sepCharIndex2 = pathPlusFilename.HasChar( GUCEF_DIRSEPCHAROPPOSITE, false );
    Int32 sepCharIndex = sepCharIndex1 > sepCharIndex2 ? sepCharIndex1 : sepCharIndex2;

    if ( sepCharIndex > 0 )
    {
        return pathPlusFilename.SubstrToIndex( sepCharIndex, true );
    }
    return pathPlusFilename;
}

/*-------------------------------------------------------------------------*/

CString
StripFileExtention( const CString& path )
{GUCEF_TRACE;

    Int32 dotIndex = path.HasChar( '.', false );
    if ( dotIndex >= 0 )
    {
        return path.SubstrToIndex( dotIndex, true );
    }
    return path;
}

/*-------------------------------------------------------------------------*/

CString
ExtractFilename( const CString& pathPlusFilename )
{GUCEF_TRACE;

    Int32 sepCharIndex1 = pathPlusFilename.HasChar( GUCEF_DIRSEPCHAR, false );
    Int32 sepCharIndex2 = pathPlusFilename.HasChar( GUCEF_DIRSEPCHAROPPOSITE, false );
    Int32 sepCharIndex = sepCharIndex1 > sepCharIndex2 ? sepCharIndex1 : sepCharIndex2;

    if ( sepCharIndex > 0 )
    {
            // @TODO: this is inconsistent !!!
            //   frontToBack is exclusive of index while the reverse is not
            // hence for now +1 to sepCharIndex

        return pathPlusFilename.SubstrToIndex( sepCharIndex+1, false );
    }
    return pathPlusFilename;
}

/*-------------------------------------------------------------------------*/

bool
IsPathValid( const CString& path  )
{GUCEF_TRACE;

    return 0 != Is_Path_Valid( path.C_String() );
}

/*-------------------------------------------------------------------------*/

CString
ExtractFileExtention( const CString& path )
{GUCEF_TRACE;

    return Extract_File_Ext( path.C_String() );
}

/*-------------------------------------------------------------------------*/

bool
IsFileInDir( const CString& dirPath  ,
             const CString& filePath )
{GUCEF_TRACE;

    // must contain the dir path as a starting string
    if ( 0 == filePath.HasSubstr( dirPath, true ) )
    {
        // now we check for deeper nesting
        CString pathRemainder = filePath.CutChars( dirPath.Length(), true );
        CString filename = ExtractFilename( pathRemainder );
        UInt32 remnantSize = pathRemainder.Length() - filename.Length();
        if ( remnantSize == 1 )
        {
            // if the remainder is just a prefixed dir seperator then we don't count
            // the file as being in the dir, otherwise it isnt
            char testChar = *(pathRemainder.C_String());
            return testChar == '\\' || testChar == '/';
        }
        else
        if ( remnantSize == 0 )
        {
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PRIVATE_CPP void
UnifyStringEol( const char* eolString ,
                CString& textStr      )
{GUCEF_TRACE;

    // If no desired eol format is given then use the platform
    // native format
    const char* eolStrToUse = eolString;
    if ( NULL == eolStrToUse || *eolStrToUse == '\0' )
    {
        eolStrToUse = GUCEF_EOL;
    }

    // Turn everything into "\n" in case of a mixed EOL char string
    textStr = textStr.ReplaceSubstr( "\r\n", "\n" ).ReplaceSubstr( "\r", "\n" );
    // Now we unified the EOL segments into \n.

    if ( 0 != strcmp( eolStrToUse, "\n" ) )
    {
        // Convert into whatever is given as the desired format
        textStr = textStr.ReplaceSubstr( "\n", eolStrToUse );
    }
}

/*-------------------------------------------------------------------------*/

bool
WriteStringAsTextFile( const CString& filePath              ,
                       const CString& fileContent           ,
                       const bool unifyEol                  ,
                       const char* eolString                ,
                       const bool onlyWriteIfContentDiffers )
{GUCEF_TRACE;

    if ( filePath.IsNULLOrEmpty() ) 
        return false;

    CString unifiedContent;
    const CString* contentToWrite = &fileContent;
    if ( unifyEol )
    {
        unifiedContent = fileContent;
        UnifyStringEol( eolString, unifiedContent );
        contentToWrite = &unifiedContent;
    }

    if ( onlyWriteIfContentDiffers )
    {
        CString originalFileContent;
        if ( LoadTextFileAsString( filePath, originalFileContent, false, GUCEF_NULL ) )
        {
            if ( (*contentToWrite) == originalFileContent )
            {
                // Skip actually writing to disk since the file content is identical and the flag
                // onlyWriteIfContentDiffers = true
                return true;
            }
        }
    }

    FILE* fptr = fopen( filePath.C_String(), "wb" );
    if ( GUCEF_NULL != fptr )
    {
        if ( !contentToWrite->IsNULLOrEmpty() )
            fwrite( contentToWrite->C_String(), contentToWrite->ByteSize()-1, 1, fptr );
        fclose( fptr );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
LoadTextFileAsString( const CString& filePath ,
                      CString& fileContent    ,
                      const bool unifyEol     ,
                      const char* eolString   )
{GUCEF_TRACE;

    fileContent.Clear();

    FILE* fptr = fopen( filePath.C_String(), "rb" );
    if ( NULL != fptr )
    {
        char charBuffer[ 1024 ];

        size_t bytesRead = 0;
        do
        {
            // read a block
            bytesRead = fread( charBuffer, 1, 1024, fptr );

            // Look for a null terminator
            size_t delimter = bytesRead;
            for ( size_t i=0; i<bytesRead; ++i )
            {
                if ( 0 == charBuffer[ i ] )
                {
                    delimter = (int) i;
                }
            }

            if ( delimter+1 < bytesRead )
            {
                // This is not a text file
                // The only valid location for a zero terminator (if any)
                // is at the end of the file
                fclose( fptr );
                return false;
            }

            // Append what we can
            fileContent.Append( charBuffer, (UInt32)delimter );
        }
        while ( bytesRead == 1024 );

        fclose( fptr );

        if ( unifyEol )
        {
            UnifyStringEol( eolString, fileContent );
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
IsAbsolutePath( const CString& path )
{GUCEF_TRACE;

    // Check for MS Windows drive letter segment or MS Windows UNC path
    if ( 1 == path.HasSubstr( ":\\", true ) || 0 == path.HasSubstr( "\\", true ) )
        return true;

    // Check for Linux Root designator
    if ( !path.IsNULLOrEmpty() && path[ 0 ] == '/' )
        return true;

    // Check for
    if ( 1 == path.HasSubstr( ":\\", true ) )
        return true;

    return false;
}

/*-------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )

// function to calculate UTF8 buffer size with handling for invalid UTF16 sequences
size_t
CalculateUtf8BufferSizeForUtf16( const uint16_t* utf16, size_t utf16_length )
{GUCEF_TRACE;

    size_t utf8_buffer_size = 0;
    for (size_t i = 0; i < utf16_length; ++i) {
        if (utf16[i] >= 0xD800 && utf16[i] <= 0xDBFF) {
            // Check for a valid low surrogate following the high surrogate
            if (i + 1 < utf16_length && utf16[i + 1] >= 0xDC00 && utf16[i + 1] <= 0xDFFF) {
                utf8_buffer_size += 4; // Valid surrogate pair
                i++; // Skip the next unit as it's part of the surrogate pair
            } else {
                // Handle invalid surrogate pair (e.g., missing low surrogate)
                utf8_buffer_size += 3; // Allocate space as if it were a BMP character
                // Note: Actual handling might vary based on requirements (e.g., error reporting)
            }
        } else if (utf16[i] >= 0xDC00 && utf16[i] <= 0xDFFF) {
            // Handle a low surrogate without a preceding high surrogate
            utf8_buffer_size += 3; // Allocate space as if it were a BMP character
            // Note: Actual handling might vary based on requirements
        } else {
            utf8_buffer_size += 3; // BMP character
        }
    }
    return utf8_buffer_size;
}

#endif

/*-------------------------------------------------------------------------*/

bool
Utf16toUtf8( const std::wstring& wstr ,
             std::string& str         )
{GUCEF_TRACE;

    if ( wstr.empty() )
    {
        str = std::string();
        return true;
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    size_t charsNeeded = ::WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), 0, 0, 0, 0 );
    if ( charsNeeded == 0 )
        return false; // Failed converting UTF-16 string to UTF-8

    str.resize( charsNeeded, '\0' );
    int charsConverted = ::WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), (LPSTR)str.c_str(), (int)charsNeeded, 0, 0 );
    if ( charsConverted == 0 )
        return false; // Failed converting UTF-16 string to UTF-8

    return true;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // @TODO
//    size_t neededBufferSize = CalculateUtf8BufferSizeForUtf16( (const uint16_t*) wstr.c_str(), wstr.size() ) + 1;
//    str.resize( neededBufferSize, '\0' );
//    ::utf16_to_utf8( wstr.c_str(), str.c_str(), neededBufferSize );
//    return true;

    return false;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Emscripten uses UTF-8 internally for std::string and std::wstring is UTF-32.
    // A proper conversion would be needed here if std::wstring is used with non-ASCII.
    // For now, we assume ASCII or that the caller handles UTF-8 in std::string directly.
    str = std::string( wstr.begin(), wstr.end() );
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

bool
Utf8toUtf16( const std::string& str ,
             std::wstring& wstr     )
{GUCEF_TRACE;

    if ( str.empty() )
    {
        wstr = std::wstring();
        return true;
    }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    size_t charsNeeded = ::MultiByteToWideChar( CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0 );
    if ( charsNeeded == 0 )
        return false; // Failed converting UTF-8 string to UTF-16

    wstr.resize( charsNeeded );
    int charsConverted = ::MultiByteToWideChar( CP_UTF8, 0, str.data(), (int)str.size(), (LPWSTR)wstr.c_str(), (int)charsNeeded );
    if ( charsConverted == 0 )
        return false; // Failed converting UTF-8 string to UTF-16

    return true;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Emscripten uses UTF-8 internally for std::string and std::wstring is UTF-32.
    // A proper conversion would be needed here if std::wstring is used with non-ASCII.
    // For now, we assume ASCII or that the caller handles UTF-8 in std::string directly.
    wstr = std::wstring( str.begin(), str.end() );
    return true;

    #else

    GUCEF::vector<wchar_t> utf16_str;
    size_t i = 0;
    while ( i < str.size() )
    {
        uint32_t codepoint = 0;
        unsigned char ch = str[ i ];
        if ( ch <= 0x7F )
        {
            codepoint = ch;
            i += 1;
        }
        else if ( ch <= 0xBF )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Utf8toUtf16: Invalid UTF-8 sequence" );
            return false;
        }
        else if ( ch <= 0xDF )
        {
            if ( i + 1 >= str.size() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Utf8toUtf16: Invalid UTF-8 sequence" );
                return false;
            }

            codepoint = ((ch & 0x1F) << 6) | (str[i + 1] & 0x3F);
            i += 2;
        }
        else if ( ch <= 0xEF )
        {
            if ( i + 2 >= str.size() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Utf8toUtf16: Invalid UTF-8 sequence" );
                return false;
            }

            codepoint = ((ch & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
            i += 3;
        }
        else if ( ch <= 0xF7 )
        {
            if ( i + 3 >= str.size() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Utf8toUtf16: Invalid UTF-8 sequence" );
                return false;
            }

            codepoint = ((ch & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
            i += 4;
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Utf8toUtf16: Invalid UTF-8 sequence" );
            return false;
        }

        if ( codepoint <= 0xFFFF )
        {
            utf16_str.push_back( static_cast< UInt16 >( codepoint ) );
        } else
        {
            codepoint -= 0x10000;
            utf16_str.push_back( static_cast< UInt16 >( (codepoint >> 10) + 0xD800) );
            utf16_str.push_back( static_cast< UInt16 >( (codepoint & 0x3FF) + 0xDC00) );
        }
    }

    wstr = std::wstring( utf16_str.begin(), utf16_str.end() );
    return true;

    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

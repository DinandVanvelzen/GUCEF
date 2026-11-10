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

#ifndef GUCEF_CORE_CUTF32STRING_H
#define GUCEF_CORE_CUTF32STRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"    /* macros that are GUCEF specific and generic macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "ETypes.h"              /* simple types used */
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CUtf8String;
class CAsciiString;
class CIOAccess;

/**
 *  UTF-32 capable string class
 *
 *  Each code point is stored as a 32-bit unsigned integer (UInt32), giving
 *  fixed-width storage.  This means all index and length values are in code
 *  point units, not bytes, and random-access indexing is O(1).
 *
 *  The raw buffer is null-terminated with a UInt32(0) sentinel.
 *  ByteSize() returns (Length() + 1) * sizeof(UInt32).
 *
 *  Constructors that accept const char* or std::string decode from UTF-8.
 *  STL_String() and operator std::string() encode the content to UTF-8.
 *  C_String() returns a const UInt32* to the raw buffer.
 *
 *  Lowercase() / Uppercase() handle the Basic Latin block (U+0041-U+007A).
 *  Full Unicode case-folding would require an external mapping table.
 *
 *  IsFormattingValid() checks that all stored code points are valid Unicode
 *  scalar values (0x0000-0xD7FF and 0xE000-0x10FFFF, excluding surrogates).
 */
class GUCEF_CORE_PUBLIC_CPP CUtf32String
{
    public:

    typedef GUCEF::vector< CUtf32String, gucef_allocator< CUtf32String > >                                                              StringVector;
    typedef GUCEF::set< CUtf32String, std::less< CUtf32String >, gucef_allocator< CUtf32String > >                                      StringSet;
    typedef std::pair< const CUtf32String, CUtf32String >                                                                             StringPair;
    typedef GUCEF::map< CUtf32String, CUtf32String, std::less< CUtf32String >, gucef_allocator< StringPair > >                          StringMap;
    typedef std::unordered_map< CUtf32String, CUtf32String, std::less< CUtf32String >, gucef_allocator< StringPair > >               StringUMap;
    typedef std::pair< const CUtf32String, StringSet >                                                                               StringStringSetPair;
    typedef GUCEF::map< CUtf32String, StringSet, std::less< CUtf32String >, gucef_allocator< StringStringSetPair > >                    StringMapSet;
    typedef std::unordered_map< CUtf32String, StringSet, std::less< CUtf32String >, gucef_allocator< StringStringSetPair > >          StringUMapSet;
    typedef std::pair< const CUtf32String, StringMap >                                                                               StringStringMapPair;
    typedef std::pair< const CUtf32String, StringUMap >                                                                              StringStringUMapPair;
    typedef GUCEF::map< CUtf32String, StringMap, std::less< CUtf32String >, gucef_allocator< StringStringMapPair > >                    StringMapMap;
    typedef std::unordered_map< CUtf32String, StringUMap, std::less< CUtf32String >, gucef_allocator< StringStringUMapPair > >        StringUMapUMap;
    typedef std::pair< const CUtf32String, StringMapSet >                                                                            StringStringMapSetPair;
    typedef std::pair< const CUtf32String, StringUMapSet >                                                                           StringStringUMapSetPair;
    typedef GUCEF::map< CUtf32String, StringMapSet, std::less< CUtf32String >, gucef_allocator< StringStringMapSetPair > >              StringMapMapSet;
    typedef std::unordered_map< CUtf32String, StringUMapSet, std::less< CUtf32String >, gucef_allocator< StringStringUMapSetPair > > StringUMapUMapSet;

    static const CUtf32String   Empty;
    static const StringVector   EmptyStringVector;
    static const StringSet      EmptyStringSet;

    CUtf32String( void );

    CUtf32String( const CUtf32String& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CUtf32String( CUtf32String&& src ) GUCEF_NOEXCEPT;
    #endif

    /** Decodes the UTF-8 encoded CUtf8String into UTF-32 code points */
    CUtf32String( const CUtf8String& src );

    /** Each ASCII byte (0x00-0x7F) maps directly to its code point value */
    CUtf32String( const CAsciiString& src );

    /** Decodes the UTF-8 encoded std::string into UTF-32 code points */
    CUtf32String( const std::string& src );

    /** Decodes the null-terminated UTF-8 C-string into UTF-32 code points */
    CUtf32String( const char* src ) GUCEF_NOEXCEPT;

    /**
     *  Decodes byteSize bytes of UTF-8 from src into UTF-32 code points.
     *  @param reexamineByteSize if true, byteSize is treated as a maximum and
     *         actual extent is determined by the null terminator.
     */
    CUtf32String( const char* src              ,
                  UInt32 byteSize              ,
                  bool reexamineByteSize = false ) GUCEF_NOEXCEPT;

    /** Copies lengthInCodePoints code points from the null-terminated UTF-32 buffer */
    CUtf32String( const UInt32* src ) GUCEF_NOEXCEPT;

    /**
     *  Copies up to lengthInCodePoints code points from src.
     *  @param reexamineLength if true, length is treated as a maximum and
     *         actual extent is determined by the null code-point terminator.
     */
    CUtf32String( const UInt32* src                  ,
                  UInt32 lengthInCodePoints           ,
                  bool reexamineLength = false ) GUCEF_NOEXCEPT;

    /** Constructs a single-code-point string */
    explicit CUtf32String( const UInt32 utf32CodePoint );

    explicit CUtf32String( const int NULLvalue );

    ~CUtf32String();

    CUtf32String& operator=( const CUtf32String& src );

    /** Decodes the UTF-8 C-string and stores it as UTF-32 */
    CUtf32String& operator=( const char* src );

    CUtf32String& operator+=( const CUtf32String& other );

    /** Decodes the UTF-8 C-string and appends as UTF-32 */
    CUtf32String& operator+=( const char* other );

    /** Appends a single UTF-32 code point */
    CUtf32String& operator+=( UInt32 codePoint );

    CUtf32String operator+( const CUtf32String& addition ) const;

    CUtf32String operator+( const char* addition ) const;

    CUtf32String operator+( UInt32 codePoint ) const;

    bool operator==( const CUtf32String& other ) const;

    bool operator==( const char* other ) const;

    bool operator==( const int NULLvalue ) const;

    bool operator==( UInt32 codePoint ) const;

    bool operator!=( const CUtf32String& other ) const;

    bool operator!=( const char* other ) const;

    bool operator!=( const int NULLvalue ) const;

    bool operator!=( UInt32 codePoint ) const;

    bool operator<( const CUtf32String& other ) const;

    bool operator<=( const CUtf32String& other ) const;

    /** Returns the UTF-32 code point at code-point index @a index */
    UInt32 operator[]( const UInt32 index ) const;

    UInt32& operator[]( const UInt32 index );

    bool IsNULLOrEmpty( void ) const;

    /** Converts the string to a UTF-8 encoded std::string */
    operator std::string() const;

    /** Returns a pointer to the raw null-terminated UTF-32 (UInt32) buffer */
    const UInt32* C_String( void ) const;

    UInt32* C_String( void );

    /**
     *  Reserves a buffer of bufferSizeInCodePoints UInt32 slots (plus null terminator).
     *  The caller may write directly into the returned pointer and then call
     *  SetLength() or DetermineLength() to update the cached length.
     */
    UInt32* Reserve( const UInt32 bufferSizeInCodePoints, Int32 newLength = -1 );

    /** Returns a UTF-8 encoded std::string */
    std::string STL_String( void ) const;

    /** Returns the number of UTF-32 code points (not bytes) */
    UInt32 Length( void ) const;

    /** Returns the byte size of the buffer including the null terminator:
     *  (Length() + 1) * sizeof(UInt32) */
    UInt32 ByteSize( void ) const;

    /** Adjusts the logical length of the string (zero-fills or truncates) */
    void SetLength( UInt32 newLength );

    /**
     *  Determines the length by scanning for the UInt32 null terminator and
     *  updates the cached length value.  Useful after writing via Reserve().
     */
    UInt32 DetermineLength( void );

    /** Sets the string from a UTF-32 buffer with explicit code-point count */
    void Set( const UInt32* str,
              UInt32 lengthInCodePoints );

    /** Sets the string from a null-terminated UTF-32 buffer */
    void Set( const UInt32* str );

    /** Decodes the null-terminated UTF-8 C-string and stores it as UTF-32 */
    void Set( const char* src );

    /**
     *  Scans up to maxCodePoints code points for a null terminator and sets
     *  the string to that extent (or maxCodePoints if no null found).
     */
    void Scan( const UInt32* str,
               UInt32 maxCodePoints );

    void Append( const UInt32* appendStr,
                 UInt32 lengthInCodePoints );

    void Append( const UInt32* appendStr );

    /** Decodes the null-terminated UTF-8 C-string and appends as UTF-32 */
    void Append( const char* appendStr );

    CUtf32String Lowercase( void ) const;

    CUtf32String Uppercase( void ) const;

    CUtf32String ReplaceChar( UInt32 oldCodePoint,
                              UInt32 newCodePoint ) const;

    CUtf32String ReplaceChars( const UInt32* oldCodePoints    ,
                               UInt32 nrOfOldCodePoints       ,
                               UInt32 newCodePoint            ) const;

    CUtf32String ReplaceSubstr( const CUtf32String& substr      ,
                                const CUtf32String& replacement ) const;

    /**
     *  Replaces an enveloping substring with another.
     *  Example: "$MyEnvelope(" ")" -> "%{" "}"
     */
    CUtf32String ReplaceEnvelopingSubstr( const CUtf32String& envelopPrefix     ,
                                          const CUtf32String& envelopPostfix    ,
                                          const CUtf32String& newEnvelopPrefix  ,
                                          const CUtf32String& newEnvelopPostfix ) const;

    CUtf32String CutEnvelopedSubstr( const CUtf32String& envelopPrefix  ,
                                     const CUtf32String& envelopPostfix ,
                                     UInt32 envelopedStrIndex           ) const;

    CUtf32String ReplaceSubStr( UInt32 startIndex                 ,
                                UInt32 length                     ,
                                const CUtf32String& newSubstr     ) const;

    CUtf32String SubstrToChar( UInt32 searchCodePoint                   ,
                               bool frontToBack = true                  ,
                               bool returnEmptyIfCharNotFound = false   ) const;

    CUtf32String SubstrToChar( UInt32 searchCodePoint                   ,
                               UInt32 startIndex                        ,
                               bool frontToBack = true                  ,
                               bool returnEmptyIfCharNotFound = false   ) const;

    CUtf32String SubstrToSubstr( const CUtf32String& searchstr ,
                                 bool frontToBack = true        ) const;

    CUtf32String SubstrToSubstr( const CUtf32String& searchstr ,
                                 UInt32 startIndex              ,
                                 bool frontToBack = true        ) const;

    CUtf32String SubstrToIndex( UInt32 index            ,
                                bool frontToBack = true ) const;

    /**
     *  As per common convention:
     *  startIndex is inclusive and endIndex is exclusive.
     *  The code point at endIndex will NOT be included.
     */
    CUtf32String SubstrFromRange( UInt32 startIndex ,
                                  UInt32 endIndex   ) const;

    CUtf32String Trim( bool frontToBack ) const;

    Int32 HasChar( UInt32 searchCodePoint  ,
                   bool frontToBack = true ) const;

    Int32 HasChar( UInt32 searchCodePoint  ,
                   UInt32 startIndex       ,
                   bool frontToBack        ) const;

    Int32 HasSubstr( const CUtf32String& substr ,
                     Int32 startIndex           ,
                     bool frontToBack = true    ) const;

    Int32 HasSubstr( const CUtf32String& substr ,
                     bool frontToBack = true    ) const;

    UInt32 GetCharacterCount( UInt32 searchCodePoint ) const;

    UInt32 GetCharactersCount( const UInt32* searchCodePoints ,
                               UInt32 nrOfSearchCodePoints    ) const;

    UInt32 GetNonMatchCharactersCount( const UInt32* searchCodePoints ,
                                       UInt32 nrOfSearchCodePoints    ) const;

    UInt32 GetCharacterRepeatCount( UInt32 searchCodePoint ) const;

    UInt32 FindMaxSubstrEquality( const CUtf32String& searchStr ,
                                  UInt32 startOffset             ,
                                  bool frontToBack               ,
                                  bool isCaseSensitive           ) const;

    CUtf32String CutChars( UInt32 charcount        ,
                           bool frontToBack = true ,
                           UInt32 startOffset = 0  ) const;

    CUtf32String RemoveChar( UInt32 codePointToRemove ) const;

    bool HasRepeatingChar( UInt32 codePointToCheck ) const;

    CUtf32String CompactRepeatingChar( UInt32 codePointToCompact ) const;

    StringVector ParseElements( UInt32 separator               ,
                                bool addEmptyElements = true   ) const;

    StringSet ParseUniqueElements( UInt32 separator               ,
                                   bool addEmptyElements = true   ) const;

    static StringVector ParseElements( const UInt32* bufferPtr  ,
                                       UInt32 bufferSize        ,
                                       UInt32 separator         ,
                                       bool addEmptyElements    );

    static StringSet ParseUniqueElements( const UInt32* bufferPtr ,
                                          UInt32 bufferSize       ,
                                          UInt32 separator        ,
                                          bool addEmptyElements   );

    bool WildcardEquals( const CUtf32String& strWithWildcards         ,
                         UInt32 wildCardToken = static_cast<UInt32>('*') ,
                         bool caseSensitive = true                    ,
                         bool biDirectional = false                   ) const;

    bool WildcardEquals( const StringSet& strsWithWildcards               ,
                         UInt32 wildCardToken = static_cast<UInt32>('*')  ,
                         bool caseSensitive = true                        ,
                         bool biDirectional = false                       ) const;

    bool Equals( const CUtf32String& otherStr    ,
                 bool caseSensitive = true        ) const;

    bool NotEquals( const CUtf32String& otherStr ,
                    bool caseSensitive = true     ) const;

    CUtf32String Combine( const StringVector& elements, UInt32 separator ) const;
    CUtf32String Combine( const StringSet& elements,    UInt32 separator ) const;

    void Clear( void );

    /**
     *  Validates that all stored code points are valid Unicode scalar values
     *  (0x0000-0xD7FF and 0xE000-0x10FFFF, i.e. no surrogate halves and no
     *  values above U+10FFFF).
     */
    bool IsFormattingValid( void ) const;

    /**
     *  Reads a line (terminated by \\n, \\r, \\0 or EOF) from a CIOAccess
     *  source that is assumed to contain native-endian UTF-32 encoded data.
     *  Each code point occupies exactly 4 bytes in the stream.
     */
    static CUtf32String ReadLine( CIOAccess* io );

    /**
     *  Reads a whitespace-delimited token from a CIOAccess source that is
     *  assumed to contain native-endian UTF-32 encoded data.
     */
    static CUtf32String ReadString( CIOAccess* io );

    /**
     *  Links this string to an externally managed buffer (zero-copy, non-owning).
     *  LinkTo() is the ONLY way to enter linked/non-owning mode.
     */
    CUtf32String& LinkTo( const UInt32* externalBuffer, UInt32 lengthInCodePoints );
    CUtf32String& LinkTo( const CUtf32String& src );

    /**
     *  Returns true if this string is in linked (non-owning) mode.
     */
    bool IsLinked( void ) const;

    private:

    void PromoteToOwned( void );

    private:
    UInt32* m_string;   /**< null-terminated UTF-32 buffer (each element is one code point) */
    UInt32  m_length;   /**< number of code points, excluding the null terminator */
    bool    m_linked;   /**< true = borrows external buffer; do not delete on destruction */
};

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool operator!=( const char* lhs, const CUtf32String& rhs );
GUCEF_CORE_PUBLIC_CPP bool operator==( const char* lhs, const CUtf32String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf32String operator+( const char* lhs, const CUtf32String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf32String operator+( UInt32 lhs, const CUtf32String& rhs );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF32STRING_H ? */

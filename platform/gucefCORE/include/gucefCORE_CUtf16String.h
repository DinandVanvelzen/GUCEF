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

#ifndef GUCEF_CORE_CUTF16STRING_H
#define GUCEF_CORE_CUTF16STRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <wchar.h>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "ETypes.h"
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
 *  UTF-16 capable string class, modelled on CUtf8String.
 *
 *  Internally stores code points as an array of 16-bit code units (UInt16),
 *  null-terminated with UInt16(0).  BMP code points (U+0000–U+D7FF and
 *  U+E000–U+FFFF) occupy one code unit; supplementary code points
 *  (U+10000–U+10FFFF) occupy two code units (a surrogate pair).
 *
 *  Member variables:
 *   - m_string   : null-terminated UInt16 buffer
 *   - m_length   : number of Unicode code points (NOT code units)
 *   - m_byteSize : total bytes allocated, including the null code-unit;
 *                  equals (codeUnits + 1) * sizeof(UInt16)
 *
 *  All index and offset arguments are in code-point units (like CUtf8String).
 *  Individual code points are conveyed as UInt32 (UTF-32 scalar values).
 *
 *  C_String() returns const UInt16* to the raw buffer.
 *  STL_String() and operator std::string() encode to UTF-8.
 *  Constructors from const char* and std::string decode from UTF-8.
 *  Constructors from const wchar_t* convert from the platform wchar_t
 *  encoding (UTF-16 on Windows, UTF-32 on Linux/Mac).
 *
 *  Lowercase() / Uppercase() handle Basic Latin (U+0041–U+007A).
 *  IsFormattingValid() verifies the buffer contains only valid UTF-16
 *  (well-formed surrogate pairs, no lone surrogates).
 */
class GUCEF_CORE_PUBLIC_CPP CUtf16String
{
    public:

    typedef GUCEF::vector< CUtf16String, gucef_allocator< CUtf16String > >                                                              StringVector;
    typedef GUCEF::set< CUtf16String, std::less< CUtf16String >, gucef_allocator< CUtf16String > >                                      StringSet;
    typedef std::pair< const CUtf16String, CUtf16String >                                                                             StringPair;
    typedef GUCEF::map< CUtf16String, CUtf16String, std::less< CUtf16String >, gucef_allocator< StringPair > >                          StringMap;
    typedef std::unordered_map< CUtf16String, CUtf16String, std::less< CUtf16String >, gucef_allocator< StringPair > >               StringUMap;
    typedef std::pair< const CUtf16String, StringSet >                                                                               StringStringSetPair;
    typedef GUCEF::map< CUtf16String, StringSet, std::less< CUtf16String >, gucef_allocator< StringStringSetPair > >                    StringMapSet;
    typedef std::unordered_map< CUtf16String, StringSet, std::less< CUtf16String >, gucef_allocator< StringStringSetPair > >          StringUMapSet;
    typedef std::pair< const CUtf16String, StringMap >                                                                               StringStringMapPair;
    typedef std::pair< const CUtf16String, StringUMap >                                                                              StringStringUMapPair;
    typedef GUCEF::map< CUtf16String, StringMap, std::less< CUtf16String >, gucef_allocator< StringStringMapPair > >                    StringMapMap;
    typedef std::unordered_map< CUtf16String, StringUMap, std::less< CUtf16String >, gucef_allocator< StringStringUMapPair > >        StringUMapUMap;
    typedef std::pair< const CUtf16String, StringMapSet >                                                                            StringStringMapSetPair;
    typedef std::pair< const CUtf16String, StringUMapSet >                                                                           StringStringUMapSetPair;
    typedef GUCEF::map< CUtf16String, StringMapSet, std::less< CUtf16String >, gucef_allocator< StringStringMapSetPair > >              StringMapMapSet;
    typedef std::unordered_map< CUtf16String, StringUMapSet, std::less< CUtf16String >, gucef_allocator< StringStringUMapSetPair > > StringUMapUMapSet;

    static const CUtf16String   Empty;
    static const StringVector   EmptyStringVector;
    static const StringSet      EmptyStringSet;

    CUtf16String( void );

    CUtf16String( const CUtf16String& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CUtf16String( CUtf16String&& src ) GUCEF_NOEXCEPT;
    #endif

    /** Converts the UTF-8 encoded string to UTF-16 */
    CUtf16String( const CUtf8String& src );

    /** Converts the ASCII string to UTF-16 (each byte maps to its code point) */
    CUtf16String( const CAsciiString& src );

    /** Decodes the UTF-8 encoded std::string to UTF-16 */
    CUtf16String( const std::string& src );

    /** Decodes the null-terminated UTF-8 C-string to UTF-16 */
    CUtf16String( const char* src );

    /**
     *  Decodes byteSize bytes of UTF-8 to UTF-16.
     *  @param reexamineByteSize  if true, byteSize is treated as a maximum
     *         and actual extent is determined by the null terminator.
     */
    CUtf16String( const char* src              ,
                  UInt32 byteSize              ,
                  bool reexamineByteSize = false );

    /**
     *  Decodes byteSize bytes of UTF-8 with a pre-counted code-point length.
     */
    CUtf16String( const char* src              ,
                  UInt32 byteSize              ,
                  UInt32 lengthInCodePoints    ,
                  bool reexamineByteSize = false );

    /** Copies the null-terminated UTF-16 buffer */
    CUtf16String( const UInt16* src );

    /**
     *  Copies codeUnits code units from src.
     *  @param reexamine  if true, codeUnits is a maximum; scan for UInt16(0).
     */
    CUtf16String( const UInt16* src              ,
                  UInt32 codeUnits               ,
                  bool reexamine = false );

    /**
     *  Copies codeUnits code units from src with pre-counted code-point length.
     */
    CUtf16String( const UInt16* src              ,
                  UInt32 codeUnits               ,
                  UInt32 lengthInCodePoints       ,
                  bool reexamine = false );

    /** Converts from platform wchar_t encoding (UTF-16 on Windows, UTF-32 on Linux) */
    CUtf16String( const wchar_t* src );

    CUtf16String( const wchar_t* src             ,
                  UInt32 lengthInWCodePoints      ,
                  bool reexamine = false );

    /** Constructs a single-code-point string from a UTF-32 code point */
    explicit CUtf16String( const UInt32 utf32CodePoint );

    /** Constructs empty string (or asserts if NULLvalue != 0) */
    explicit CUtf16String( const Int32 NULLvalue );

    ~CUtf16String();

    CUtf16String& operator=( const CUtf16String& src );

    CUtf16String& operator=( const std::string& src );

    /** Decodes the UTF-8 C-string and stores as UTF-16 */
    CUtf16String& operator=( const char* src );

    CUtf16String& operator+=( const CUtf16String& other );

    /** Decodes the UTF-8 C-string and appends as UTF-16 */
    CUtf16String& operator+=( const char* other );

    /** Appends a single UTF-32 code point encoded as UTF-16 */
    CUtf16String& operator+=( UInt32 utf32CodePoint );

    CUtf16String& operator+=( char singleByte );

    CUtf16String operator+( const CUtf16String& addition ) const;

    CUtf16String operator+( const char* addition ) const;

    CUtf16String operator+( UInt32 utf32CodePoint ) const;

    CUtf16String operator+( char singleByte ) const;

    bool operator==( const CUtf16String& other ) const;

    bool operator==( const char* other ) const;

    bool operator==( const Int32 NULLvalue ) const;

    bool operator==( const UInt32 utf32CodePoint ) const;

    bool operator==( const char character ) const;

    bool operator!=( const CUtf16String& other ) const;

    bool operator!=( const char* other ) const;

    bool operator!=( const Int32 NULLvalue ) const;

    bool operator!=( const UInt32 utf32CodePoint ) const;

    bool operator!=( const char character ) const;

    bool operator<( const CUtf16String& other ) const;

    bool operator<=( const CUtf16String& other ) const;

    /**
     *  Returns the UTF-32 code point at code-point index @a index.
     *  O(N) – requires seeking from the start of the buffer.
     *  Returns 0 if index is out of range.
     */
    UInt32 operator[]( const UInt32 index ) const;

    bool IsNULLOrEmpty( void ) const;

    /** Converts to a UTF-8 encoded std::string */
    operator std::string() const;

    /** Returns a pointer to the raw null-terminated UTF-16 (UInt16) buffer */
    const UInt16* C_String( void ) const;

    UInt16* C_String( void );

    /**
     *  Returns the UTF-32 code point at code-point index @a index.
     *  O(N) – same as operator[].
     */
    UInt32 CodepointAtIndex( const UInt32 index ) const;

    /**
     *  Returns a pointer to the start of the code point at code-point index
     *  @a index, and sets codeUnitsFromStart to the offset in code units.
     *  Returns GUCEF_NULL if index is out of range.  O(N).
     */
    const UInt16* CodepointPtrAtIndex( const UInt32 index,
                                        UInt32& codeUnitsFromStart ) const;

    static const UInt16* CodepointPtrAtIndex( const UInt16* str          ,
                                               const UInt32 totalCodeUnits ,
                                               const UInt32 length         ,
                                               const UInt32 index          ,
                                               UInt32& codeUnitsFromStart  );

    /**
     *  Advances currentCpPos to the next code point boundary.
     *  Returns GUCEF_NULL if past the end of the buffer.
     */
    const UInt16* NextCodepointPtr( const UInt16* currentCpPos ) const;

    const UInt16* NextCodepointPtr( const UInt16* currentCpPos,
                                     UInt32& currentUtf32Cp    ) const;

    /**
     *  Reserves a buffer of codeUnits UInt16 slots (plus null terminator).
     *  Returns UInt16* for direct writing.  Call DetermineLength() afterward
     *  to update the cached length.
     */
    UInt16* Reserve( const UInt32 codeUnits, Int32 newLengthInCodePoints = -1 );

    /** Returns a UTF-8 encoded std::string */
    std::string STL_String( void ) const;

    /**
     *  Returns the number of Unicode code points.
     *  May differ from the code-unit count for strings with supplementary
     *  characters (surrogate pairs).
     */
    UInt32 Length( void ) const;

    /**
     *  Returns the total byte size of the underlying buffer including the
     *  null code-unit terminator.  equals (codeUnits + 1) * sizeof(UInt16).
     */
    UInt32 ByteSize( void ) const;

    /**
     *  Adjusts the logical length by truncating or zero-filling.
     *  @param maxCodeUnitSize  maximum code units per code point (2 for UTF-16)
     */
    void SetLength( UInt32 newLength, UInt32 maxCodeUnitSize = 2 );

    /**
     *  Scans the buffer for the null code unit and updates the cached length.
     *  Useful after writing via Reserve().
     */
    UInt32 DetermineLength( void );

    /* --- Set overloads ------------------------------------------------- */

    void Set( const UInt16* new_str              ,
              UInt32 codeUnits                    ,
              UInt32 lengthInCodePoints           ,
              bool reexamine = false              );

    void Set( const UInt16* new_str              ,
              UInt32 codeUnits                    ,
              bool reexamine = false              );

    /** Sets from a null-terminated UTF-16 buffer */
    void Set( const UInt16* new_str );

    void Set( const char* new_str              ,
              UInt32 byteSize                  ,
              UInt32 lengthInCodePoints        ,
              bool reexamineByteSize = false   );

    void Set( const char* new_str              ,
              UInt32 byteSize                  ,
              bool reexamineByteSize = false   );

    /** Decodes the null-terminated UTF-8 C-string and stores as UTF-16 */
    void Set( const char* new_str );

    void Set( const wchar_t* new_str           ,
              UInt32 lengthInWCodePoints        ,
              bool reexamine = false            );

    /** Converts the null-terminated wchar_t string and stores as UTF-16 */
    void Set( const wchar_t* new_str );

    void Set( const CUtf16String& new_str );

    /* --- Scan ----------------------------------------------------------- */

    /**
     *  Scans up to maxCodeUnits code units for a null code-unit and sets the
     *  string (or uses the full maxCodeUnits if no null found).
     */
    void Scan( const UInt16* newStr, UInt32 maxCodeUnits );

    /* --- Append --------------------------------------------------------- */

    void Append( const UInt16* appendStr               ,
                 UInt32 codeUnits                       ,
                 Int32 lengthInCodePoints = -1          );

    void Append( const UInt16* appendStr );

    /** Decodes the null-terminated UTF-8 C-string and appends as UTF-16 */
    void Append( const char* appendStr );

    /** Encodes the UTF-32 code point as UTF-16 and appends */
    void Append( UInt32 utf32CodePoint );

    void Append( Int32 utf32CodePoint );

    void Append( const CUtf16String& appendStr );

    /* --- Case ----------------------------------------------------------- */

    CUtf16String Lowercase( void ) const;

    CUtf16String Uppercase( void ) const;

    /* --- Replace -------------------------------------------------------- */

    CUtf16String ReplaceChar( UInt32 oldCodePoint,
                              UInt32 newCodePoint ) const;

    CUtf16String ReplaceChar( Int32 oldCodePoint,
                              Int32 newCodePoint ) const;

    CUtf16String ReplaceChars( const Int32* oldCodePoints    ,
                               UInt32 nrOfOldCodePoints       ,
                               Int32 newCodePoint             ) const;

    CUtf16String ReplaceSubstr( const CUtf16String& substr      ,
                                const CUtf16String& replacement ) const;

    CUtf16String ReplaceEnvelopingSubstr( const CUtf16String& envelopPrefix     ,
                                          const CUtf16String& envelopPostfix    ,
                                          const CUtf16String& newEnvelopPrefix  ,
                                          const CUtf16String& newEnvelopPostfix ) const;

    CUtf16String CutEnvelopedSubstr( const CUtf16String& envelopPrefix  ,
                                     const CUtf16String& envelopPostfix ,
                                     UInt32 envelopedStrIndex            ) const;

    CUtf16String ReplaceSubStr( UInt32 startIndex                ,
                                UInt32 length                    ,
                                const CUtf16String& newSubstr    ) const;

    /* --- Substring extraction ------------------------------------------- */

    CUtf16String SubstrToChar( UInt32 searchCodePoint               ,
                               bool frontToBack = true              ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CUtf16String SubstrToChar( char searchChar                       ,
                               bool frontToBack = true               ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CUtf16String SubstrToChar( UInt32 searchCodePoint               ,
                               UInt32 startIndex                    ,
                               bool frontToBack = true              ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CUtf16String SubstrToChar( char searchChar                       ,
                               UInt32 startIndex                     ,
                               bool frontToBack = true               ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CUtf16String SubstrToSubstr( const CUtf16String& searchstr ,
                                 bool frontToBack = true        ) const;

    CUtf16String SubstrToSubstr( const CUtf16String& searchstr ,
                                 UInt32 startIndex              ,
                                 bool frontToBack = true        ) const;

    CUtf16String SubstrToIndex( UInt32 index,
                                bool frontToBack = true ) const;

    /**
     *  As per common convention:
     *  startIndex is inclusive and endIndex is exclusive (both in code points).
     */
    CUtf16String SubstrFromRange( UInt32 startIndex ,
                                  UInt32 endIndex   ) const;

    CUtf16String Trim( bool frontToBack ) const;

    /* --- Search --------------------------------------------------------- */

    Int32 HasChar( UInt32 searchCodePoint  ,
                   bool frontToBack = true ) const;

    Int32 HasChar( char searchChar         ,
                   bool frontToBack = true ) const;

    Int32 HasChar( UInt32 searchCodePoint  ,
                   UInt32 startIndex       ,
                   bool frontToBack        ) const;

    Int32 HasChar( char searchChar         ,
                   UInt32 startIndex       ,
                   bool frontToBack        ) const;

    /**
     *  Searches for substr starting at code-point index startIndex.
     *  @param shiftSearch  if false, only checks position startIndex exactly.
     *  @return code-point index of first match, or -1 if not found.
     */
    Int32 HasSubstr( const CUtf16String& substr ,
                     Int32 startIndex           ,
                     bool frontToBack = true    ,
                     bool shiftSearch = true    ) const;

    Int32 HasSubstr( const CUtf16String& substr ,
                     bool frontToBack = true    ,
                     bool shiftSearch = true    ) const;

    /** Returns true iff this string begins with substr */
    bool StartsWith( const CUtf16String& substr ) const;

    /** Returns true iff this string ends with substr */
    bool EndsWith( const CUtf16String& substr ) const;

    /* --- Character counts ----------------------------------------------- */

    UInt32 GetCharacterCount( UInt32 searchCodePoint ) const;

    UInt32 GetCharacterCount( Int32 searchCodePoint ) const;

    UInt32 GetCharactersCount( const Int32* searchCodePoints,
                               UInt32 nrOfSearchCodePoints   ) const;

    UInt32 GetCharactersCount( const char* searchChars,
                               UInt32 nrOfSearchChars  ) const;

    UInt32 GetNonMatchCharactersCount( const CUtf16String& searchChars ) const;

    UInt32 GetNonMatchCharactersCount( const Int32* searchCodePoints,
                                       UInt32 nrOfSearchCodePoints   ) const;

    UInt32 GetNonMatchCharactersCount( const char* searchChars,
                                       UInt32 nrOfSearchChars  ) const;

    UInt32 GetCharacterRepeatCount( UInt32 searchCodePoint ) const;

    UInt32 GetCharacterRepeatCount( Int32 searchCodePoint ) const;

    /* --- Equality helpers ----------------------------------------------- */

    UInt32 FindMaxSubstrEquality( const CUtf16String& searchStr ,
                                  UInt32 startOffset             ,
                                  bool frontToBack               ,
                                  bool isCaseSensitive           ) const;

    UInt32 FindMaxSegmentEquality( const CUtf16String& otherStr    ,
                                   UInt32 segmentDividerCodePoint  ,
                                   bool startFront                 ,
                                   bool isCaseSensitive            ) const;

    /* --- Modification --------------------------------------------------- */

    CUtf16String CutChars( UInt32 charcount        ,
                           bool frontToBack = true ,
                           UInt32 startOffset = 0  ) const;

    CUtf16String RemoveChar( UInt32 codePointToRemove ) const;

    CUtf16String RemoveChar( Int32 codePointToRemove ) const;

    CUtf16String CompactRepeatingChar( UInt32 codePointToCompact ) const;

    CUtf16String CompactRepeatingChar( Int32 codePointToCompact ) const;

    /* --- Parsing -------------------------------------------------------- */

    StringVector ParseElements( UInt32 separator,
                                bool addEmptyElements = true ) const;

    StringSet ParseUniqueElements( UInt32 separator,
                                   bool addEmptyElements = true ) const;

    static StringVector ParseElements( const UInt16* bufferPtr  ,
                                       UInt32 bufferCodeUnits   ,
                                       UInt32 separator         ,
                                       bool addEmptyElements    );

    static StringSet ParseUniqueElements( const UInt16* bufferPtr ,
                                          UInt32 bufferCodeUnits  ,
                                          UInt32 separator        ,
                                          bool addEmptyElements   );

    /* --- Wildcard / equality -------------------------------------------- */

    bool WildcardEquals( const CUtf16String& strWithWildcards         ,
                         UInt32 wildCardToken = static_cast<UInt32>('*') ,
                         bool caseSensitive = true                    ,
                         bool biDirectional = false                   ) const;

    bool WildcardEquals( const StringSet& strsWithWildcards               ,
                         UInt32 wildCardToken = static_cast<UInt32>('*')  ,
                         bool caseSensitive = true                        ,
                         bool biDirectional = false                       ) const;

    bool Equals( const CUtf16String& otherStr ,
                 bool caseSensitive = true     ) const;

    bool NotEquals( const CUtf16String& otherStr ,
                    bool caseSensitive = true     ) const;

    CUtf16String Combine( const StringVector& elements, UInt32 separator ) const;
    CUtf16String Combine( const StringSet& elements,    UInt32 separator ) const;
    CUtf16String Combine( const StringMap& elements,    UInt32 valueSeperator, UInt32 kvSeperator ) const;

    void Clear( void );

    /**
     *  Validates that the buffer contains only well-formed UTF-16: every high
     *  surrogate (0xD800–0xDBFF) is followed by a low surrogate (0xDC00–0xDFFF)
     *  and no isolated surrogates exist.
     */
    bool IsFormattingValid( void ) const;

    /**
     *  Lossy conversion to ASCII: non-ASCII code points are replaced by
     *  @a asciiReplacement.
     */
    CAsciiString ForceToAscii( char asciiReplacement = '*' ) const;

    /* --- Static encode/decode helpers ----------------------------------- */

    /**
     *  Encodes a single UTF-32 code point into up to 2 UInt16 code units.
     *  @return number of code units written (1 or 2), or -1 on error.
     */
    static Int32 EncodeUtf32CodePointToUtf16( UInt32 utf32CodePoint   ,
                                               UInt16* outBuf          ,
                                               UInt32 outBufCodeUnits  );

    /**
     *  Decodes one UTF-16 code point from buf (up to bufCodeUnits long).
     *  @return number of code units consumed (1 or 2), or -1 on error.
     */
    static Int32 EncodeUtf16CodePointToUtf32( const UInt16* buf       ,
                                               UInt32 bufCodeUnits     ,
                                               UInt32& outUtf32        );

    static CUtf16String ReadLine( CIOAccess* io );

    static CUtf16String ReadString( CIOAccess* io );

    private:

    Int32 CodepointIndexAtPtr( const UInt16* subStrPtr, UInt32& codePoint ) const;

    private:
    UInt16* m_string;   /**< null-terminated UTF-16 buffer (UInt16 code units) */
    UInt32  m_length;   /**< number of Unicode code points */
    UInt32  m_byteSize; /**< storage in bytes including null code-unit: (codeUnits+1)*2 */
};

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool operator!=( const char* lhs, const CUtf16String& rhs );
GUCEF_CORE_PUBLIC_CPP bool operator==( const char* lhs, const CUtf16String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf16String operator+( const char* lhs, const CUtf16String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf16String operator+( const char lhs, const CUtf16String& rhs );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF16STRING_H ? */

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

#ifndef GUCEF_CORE_CUTF8STRING_H
#define GUCEF_CORE_CUTF8STRING_H

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

class CAsciiString;
class CIOAccess;

/**
 *  UTF8 capable string class
 *
 *  Given that UTF8 is a variable length encoding all operations require more
 *  checking and seeking than would be required for a pure ASCII class
 *  As such the AsciiString class is also provided for performance limited environments
 *
 *  Note that any reference to a "character" is historical and within the this context
 *  it is intended to be tought of as a synonym for a UTF8 code point of variable length.
 *  Similarly any index or offset is based on code point counts and not on byte offsets
 *  due to the variable length nature of UTF8 code points.
 *  For this reason you are also encouraged to use the member functions provided and not
 *  iterate the contents based on the index as this is an expensive operation involving 
 *  seeking the string.
 *
 *  Due to the variable length of each code point and the desire to not use raw pointers
 *  to reference indidual code points we represent each code point when conveyed in isolation
 *  as an Int32 which is a UTF32 code point. This removes length ambiguity in a 
 *  per-code-point-context
 *
 *  Please note that for performance not every operation is checked wrt the
 *  validity of UTF8 formatting. As such use IsFormattingValid() as needed.
 */
class GUCEF_CORE_PUBLIC_CPP CUtf8String
{
    public:

    // Because strings are so common and because we want to make our lives easier wrt their common usage and allocation in containers
    // we have a list of typedefs for the most common containers we use, providing shorter names for them
    typedef std::vector< CUtf8String, gucef_allocator< CUtf8String > >                                                          StringVector;
    typedef std::set< CUtf8String, std::less< CUtf8String >, gucef_allocator< CUtf8String > >                                   StringSet;
    typedef std::pair< const CUtf8String, CUtf8String >                                                                         StringPair;
    typedef std::unordered_map< CUtf8String, CUtf8String, std::less< CUtf8String >, gucef_allocator< StringPair > >             StringUMap;
    typedef std::map< CUtf8String, CUtf8String, std::less< CUtf8String >, gucef_allocator< StringPair > >                       StringMap;
    typedef std::pair< const CUtf8String, StringSet >                                                                           StringStringSetPair;
    typedef std::map< CUtf8String, StringSet, std::less< CUtf8String >, gucef_allocator< StringStringSetPair > >                StringMapSet;
    typedef std::unordered_map< CUtf8String, StringSet, std::less< CUtf8String >, gucef_allocator< StringStringSetPair > >      StringUMapSet;
    typedef std::pair< const CUtf8String, StringMap >                                                                           StringStringMapPair;
    typedef std::pair< const CUtf8String, StringUMap >                                                                          StringStringUMapPair;
    typedef std::map< CUtf8String, StringMap, std::less< CUtf8String >, gucef_allocator< StringStringMapPair > >                StringMapMap;
    typedef std::unordered_map< CUtf8String, StringUMap, std::less< CUtf8String >, gucef_allocator< StringStringUMapPair > >    StringUMapUMap;
    typedef std::pair< const CUtf8String, StringMapSet >                                                                        StringStringMapSetPair;
    typedef std::pair< const CUtf8String, StringUMapSet >                                                                       StringStringUMapSetPair;
    typedef std::map< CUtf8String, StringMapSet, std::less< CUtf8String >, gucef_allocator< StringStringMapSetPair > >          StringMapMapSet;
    typedef std::unordered_map< CUtf8String, StringUMapSet, std::less< CUtf8String >, gucef_allocator< StringStringUMapSetPair > >      StringUMapUMapSet;
    typedef std::pair< const CUtf8String, StringMapMap >                                                                                StringStringMapMapPair;
    typedef std::pair< const CUtf8String, StringUMapUMap >                                                                              StringStringUMapUMapPair;
    typedef std::map< CUtf8String, StringMapMap, std::less< CUtf8String >, gucef_allocator< StringStringMapMapPair > >                  StringMapMapMap;
    typedef std::unordered_map< CUtf8String, StringUMapUMap, std::less< CUtf8String >, gucef_allocator< StringStringUMapUMapPair > >    StringUMapUMapUMap;
    typedef std::pair< const CUtf8String, StringMapMapMap >                                                                             StringStringMapMapMapPair;
    typedef std::pair< const CUtf8String, StringUMapUMapUMap >                                                                          StringStringUMapUMapUMapPair;
    typedef std::map< CUtf8String, StringMapMapMap, std::less< CUtf8String >, gucef_allocator< StringStringMapMapMapPair > >            StringMapMapMapMap;
    typedef std::unordered_map< CUtf8String, StringUMapUMapUMap, std::less< CUtf8String >, gucef_allocator< StringStringUMapUMapUMapPair > >       StringUMapUMapUMapUMap;
    typedef std::pair< const CUtf8String, StringMapMapSet >                                                                                        StringStringMapMapSetPair;
    typedef std::pair< const CUtf8String, StringUMapUMapSet >                                                                                      StringStringUMapUMapSetPair;
    typedef std::map< CUtf8String, StringMapMapSet, std::less< CUtf8String >, gucef_allocator< StringStringMapMapSetPair > >                       StringMapMapMapSet;
    typedef std::unordered_map< CUtf8String, StringUMapUMapSet, std::less< CUtf8String >, gucef_allocator< StringStringUMapUMapSetPair > >         StringUMapUMapUMapSet;

    static const CUtf8String    Empty;
    static const StringVector   EmptyStringVector;
    static const StringSet      EmptyStringSet;

    CUtf8String( void );

    CUtf8String( const CUtf8String& src );

    CUtf8String( const CAsciiString& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CUtf8String( CUtf8String&& src ) GUCEF_NOEXCEPT;
    #endif

    CUtf8String( const std::string& src );

    CUtf8String( const char *src );

    CUtf8String( const char* src                ,
                 UInt32 byteSize                ,
                 bool reexamineByteSize = false );

    CUtf8String( const char* src                ,
                 UInt32 byteSize                ,
                 UInt32 lengthInCodePoints      ,
                 bool reexamineByteSize = false );

    CUtf8String( const wchar_t* src );

    CUtf8String( const wchar_t* src             ,
                 UInt32 lengthInWCodePoints     ,
                 bool reexamineByteSize = false );

    explicit CUtf8String( const char src );

    explicit CUtf8String( const Int32 NULLvalueOrUtf32 );

    explicit CUtf8String( const UInt32 utf32 );

    ~CUtf8String();

    CUtf8String& operator=( const CUtf8String &src );

    CUtf8String& operator=( const std::string& src );
    
    /**
     *      Copys the source string.
     *      If the src string is NULL then the current string will be
     *      deallocated and also set to NULL.
     */
    CUtf8String& operator=( const char *src );

    CUtf8String& operator+=( const CUtf8String &other );

    CUtf8String& operator+=( const char *other );

    CUtf8String& operator+=( char lastchar );

    CUtf8String operator+( const CUtf8String& addition ) const;

    CUtf8String operator+( const char* addition ) const;

    CUtf8String operator+( const char addition ) const;

    bool operator==( const CUtf8String &other ) const;

    bool operator==( const char *other ) const;

    bool operator==( const Int32 NULLvalue ) const;

    bool operator==( const UInt32 NULLvalueOrUtf32 ) const;

    bool operator==( const char character ) const;

    bool operator!=( const CUtf8String &other ) const;

    bool operator!=( const char *other ) const;

    bool operator!=( const Int32 NULLvalue ) const;

    bool operator!=( const UInt32 NULLvalueOrUtf32 ) const;

    bool operator!=( const char character ) const;

    bool operator<( const CUtf8String& other ) const;

    bool operator<=( const CUtf8String& other ) const;

    /** 
     *  Uses the concept of code point indeces to index into the string
     *  Uses CodepointAtIndex()
     *  Note that this operation requires seeking and is expensive
     *
     *  @return UTF32 encoded singular code point at the code point index requested or 0 if index is invalid
     */
    UInt32 operator[]( const UInt32 index ) const;

    bool IsNULLOrEmpty( void ) const;

    operator std::string() const;

    /* operator const char*() const; */

    const char* C_String( void ) const;

    char* C_String( void );

    /** 
     *  UTF8 class specific:
     *  Uses the concept of code point indeces to index into the string
     *  Note that this operation requires seeking and is expensive
     *
     *  @return UTF32 encoded singular code point at the code point index requested or 0 if index is invalid
     */
    UInt32 CodepointAtIndex( const UInt32 index ) const;

    /**
     *  UTF8 class specific:
     *  Scans the UTF8 variable length string to the memory offset of the desired codepoint index
     *  Uses the concept of code point indeces to index into the string
     *  Note that this operation requires seeking and is expensive
     */ 
    const char* CodepointPtrAtIndex( const UInt32 index, UInt32& bytesFromStart ) const;

    /**
     *  UTF8 class specific:
     *  Scans the UTF8 variable length string to the memory offset of the desired codepoint index
     *  Uses the concept of code point indeces to index into the string
     *  Note that this operation requires seeking and is expensive
     */ 
    static const char* CodepointPtrAtIndex( const char* str        ,
                                            const UInt32 byteSize  ,
                                            const UInt32 length    ,
                                            const UInt32 index     , 
                                            UInt32& bytesFromStart );

    /**
     *  UTF8 class specific:
     *  Advances the pointer given to the next code point position while also performing range checking
     *  for this string.
     *  @return pointer to the next utf8 code point, if none exists or in case of error GUCEF_NULL is returned
     */ 
    const char* NextCodepointPtr( const char* currentCpPos ) const;

    /**
     *  UTF8 class specific:
     *  Advances the pointer given to the next code point position while also performing range checking
     *  for this string.
     *  @return pointer to the next utf8 code point, if none exists or in case of error GUCEF_NULL is returned
     */ 
    const char* NextCodepointPtr( const char* currentCpPos, UInt32& currentUtf32Cp ) const;

    /**
     *  If you wish to manipulate characters in a char buffer
     *  while still having containment as a string object this function
     *  is for you. It will set the string to have an unititialized char buffer
     *  of byteSize bytes.
     */
    char* Reserve( const UInt32 byteSize, Int32 newLength = -1 );

    std::string STL_String( void ) const;

    /**
     *  Returns the length of the string counted in code points
     *  It may differ from the storage size due to UTF8s variable length encoding. 
     *  For size of the underlying storage use ByteSize() instead
     */
    UInt32 Length( void ) const;

    /**
     *  Returns the size of the underlying storage used to store the string's code points
     *  It may exceed the number of code points due to UTF8s variable length encoding of 1-4 bytes per code point
     *  For a count of the number of code points used Length() instead
     *  Note that the returned value includes the null terminator
     */
    UInt32 ByteSize( void ) const;

    /**
     *  Allows you to set the length of the string.
     *  Reserve() and DetermineLength() should be used in most cases instead.
     *  Mostly provided for compatibility with ASCII string handling
     */
    void SetLength( UInt32 newLength, UInt32 maxCodePointSize = 4 );

    /**
     *  This function will give you the length of the string just like the regular Length()
     *  However it will determine the length based on the null terminator and overwrite the cached
     *  length value. This is useful in combination with the Reserve() function.
     */
    UInt32 DetermineLength( void );

    /**
     *  This member functions allows you to set the string using a potentially
     *  non-null-terminated char array as the source. The source will
     *  be copied and a null terminator will be added if needed
     *  It also allows specifying the UTF8 code point length
     *
     *  @param reexamineByteSize if true the byteSize passed will be treated as a maxima up to which a new bytesize is to be determined 
     */
    void Set( const char* new_str            ,
              UInt32 byteSize                ,
              UInt32 lengthInCodePoints      ,
              bool reexamineByteSize = false );

    /**
     *  This member functions allows you to set the string using a potentially
     *  non-null-terminated char array as the source. The source will
     *  be copied and a null terminator will be added if needed
     *
     *  @param reexamineByteSize if true the byteSize passed will be treated as a maxima up to which a new bytesize is to be determined 
     */
    void Set( const char* new_str            ,
              UInt32 byteSize                ,
              bool reexamineByteSize = false );

    /**
     *  This member functions allows you to set the string using a 
     *  null-terminated UTF8 compliant source. 
     */
    void Set( const char* new_str );

    /**
     *  This member functions allows you to set the string using a potentially
     *  non-null-terminated char array as the source. The source will
     *  be copied and a null terminator will be added if needed
     *  The wchar_t encoding will be converted to UTF8
     */
    void Set( const wchar_t* new_str         ,
              UInt32 lengthInWCodePoints     ,
              bool reexamineByteSize = false );

    /**
     *  This member functions allows you to set the string using a 
     *  null-terminated wchar_t compliant source.
     *  The wchar_t encoding will be converted to UTF8
     */
    void Set( const wchar_t* new_str );

    /**
     *  This member functions allows you to set the string using another UTF8 string object 
     */
    void Set( const CUtf8String& new_str );

    /**
     *  This member functions allows you to set the string using a
     *  null-terminated char array as the source while specifying a
     *  maximum buffer scan length. If no null-terminator is found
     *  the maximum number of bytes indicated will be used as the string
     *  and a null-terminator will be added.
     */
    void Scan( const char* newStr     ,
               const UInt32 maxLength );

    /**
     *  This member functions allows you to add to the string using a
     *  potentially non-null-terminated char array as the source. The source will
     *  be copyed and a null terminator will be added.
     */
    void Append( const char* appendstr         ,
                 UInt32 byteSize               ,
                 Int32 lengthInCodePoints = -1 );

    void Append( const char* appendstr );

    void Append( const UInt32 utf32CodePoint );

    void Append( const Int32 utf32CodePoint );

    void Append( const CUtf8String& appendstr );

    CUtf8String Lowercase( void ) const;

    CUtf8String Uppercase( void ) const;

    CUtf8String ReplaceChar( Int32 oldchar ,
                             Int32 newchar ) const;

    CUtf8String ReplaceChars( const Int32* oldchars ,
                              UInt32 nrOfOldChars   ,
                              Int32 newchar         ) const;

    CUtf8String ReplaceSubstr( const CUtf8String& substr      ,
                               const CUtf8String& replacement ) const;

    /**
     *  Replaces an enveloping substring with another
     *  Example: "$MyEnvelope(" ")" -> "%{" "}"
     */
    CUtf8String ReplaceEnvelopingSubstr( const CUtf8String& envelopPrefix     ,
                                         const CUtf8String& envelopPostfix    ,
                                         const CUtf8String& newEnvelopPrefix  ,
                                         const CUtf8String& newEnvelopPostfix ) const;

    CUtf8String CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                    const CUtf8String& envelopPostfix ,
                                    UInt32 envelopedStrIndex          ) const;

    CUtf8String ReplaceSubStr( UInt32 startIndex        ,
                               UInt32 length            ,
                               const CUtf8String& newSubstr ) const;

    CUtf8String SubstrToChar( UInt32 searchchar                      ,
                              bool frontToBack = true                ,
                              bool returnEmptyIfCharNotFound = false ) const;

    CUtf8String SubstrToChar( char searchchar                        ,
                              bool frontToBack = true                ,
                              bool returnEmptyIfCharNotFound = false ) const;

    CUtf8String SubstrToChar( UInt32 searchchar                      ,
                              UInt32 startIndex                      ,
                              bool frontToBack = true                ,
                              bool returnEmptyIfCharNotFound = false ) const;

    CUtf8String SubstrToChar( char searchchar                        ,
                              UInt32 startIndex                      ,
                              bool frontToBack = true                ,
                              bool returnEmptyIfCharNotFound = false ) const;

    CUtf8String SubstrToSubstr( const CUtf8String& searchstr ,
                                bool frontToBack = true  ) const;

    CUtf8String SubstrToSubstr( const CUtf8String& searchstr ,
                                UInt32 startIndex        ,
                                bool frontToBack = true  ) const;

    CUtf8String SubstrToIndex( UInt32 index            ,
                               bool frontToBack = true ) const;

    /**
     *  As per common convention:
     *  startIndex is inclusive and endIndex is exclusive
     *  Put another way: the char pointed to by endIndex will NOT be included
     */
    CUtf8String SubstrFromRange( UInt32 startIndex ,
                                 UInt32 endIndex   ) const;

    CUtf8String Trim( bool frontToBack ) const;

    Int32 HasChar( UInt32 searchchar       ,
                   bool frontToBack = true ) const;

    Int32 HasChar( char searchchar         ,
                   bool frontToBack = true ) const;

    Int32 HasChar( UInt32 searchchar       ,
                   const UInt32 startIndex ,
                   bool frontToBack        ) const;

    Int32 HasChar( char searchchar         ,
                   const UInt32 startIndex ,
                   bool frontToBack        ) const;

    Int32 HasSubstr( const CUtf8String& substr ,
                     Int32 startIndex          ,
                     bool frontToBack = true   ,
                     bool shiftSearch = true   ) const;

    Int32 HasSubstr( const CUtf8String& substr ,
                     bool frontToBack = true   ,
                     bool shiftSearch = true   ) const;

    /**
     *  Convenience function to check if the string starts with the given substring
     *  Uses HasSubstr() with frontToBack = true, shiftSearch = false and a startIndex of 0
     */
    bool StartsWith( const CUtf8String& substr ) const;

    /**
     *  Convenience function to check if the string ends with the given substring
     *  Uses HasSubstr() with frontToBack = false, shiftSearch = false and a startIndex of the last element
     */
    bool EndsWith( const CUtf8String& substr ) const;

    /**
     *  Counts how many times the given UTF32 denoted search character
     *  occurs within the UTF8 encoded string
     */
    UInt32 GetCharacterCount( const Int32 searchChar ) const;

    /**
     *  Counts how many times any of the given UTF32 denoted search characters
     *  occurs within the UTF8 encoded string
     */
    UInt32 GetCharactersCount( const Int32* searchChars ,
                               UInt32 nrOfSearchChars   ) const;

    /**
     *  Counts how many times any of the given ASCII denoted search characters
     *  occurs within the UTF8 encoded string
     */
    UInt32 GetCharactersCount( const char* searchChars ,
                               UInt32 nrOfSearchChars  ) const;

    /**
     *  Counts how many times characters differing from the given UTF32 denoted search characters
     *  occur within the UTF8 encoded string
     */
    UInt32 GetNonMatchCharactersCount( const Int32* searchChars ,
                                       UInt32 nrOfSearchChars   ) const;

    /**
     *  Counts how many times characters differing from the given ASCII denoted search characters
     *  occur within the UTF8 encoded string
     */
    UInt32 GetNonMatchCharactersCount( const char* searchChars ,
                                       UInt32 nrOfSearchChars  ) const;

    /**
     *  Counts how many times the given UTF32 denoted search character
     *  occurs followed by how many repeat instances within the UTF8 encoded string
     */
    UInt32 GetCharacterRepeatCount( const Int32 searchChar ) const;

    /**
     *  Find the max number of matching code points
     *  returns the code point counts that matched
     */
    UInt32 FindMaxSubstrEquality( const CUtf8String& searchStr ,
                                  const UInt32 startOffset     ,
                                  bool frontToBack             ,
                                  bool isCaseSentive           ) const;

    /**
     *  Find the max number of matching code points segment
     *  Each segment is deliniated by 'segmentDividerCodePoint' 
     *  returns the code point counts that matched
     */
    UInt32 FindMaxSegmentEquality( const CUtf8String& otherStr   ,
                                   Int32 segmentDividerCodePoint ,
                                   bool startFront               ,
                                   bool isCaseSentive            ) const;

    CUtf8String CutChars( UInt32 charcount        ,
                          bool frontToBack = true ,
                          UInt32 startOffset = 0  ) const;

    CUtf8String RemoveChar( const Int32 charToRemove ) const;

    CUtf8String CompactRepeatingChar( const Int32 charToCompact ) const;

    StringVector ParseElements( UInt32 seperator             ,
                                bool addEmptyElements = true ) const;

    StringSet ParseUniqueElements( UInt32 seperator             ,
                                   bool addEmptyElements = true ) const;

    static StringVector ParseElements( const UInt8* bufferPtr ,
                                       UInt32 bufferSize      ,
                                       UInt32 seperator       ,
                                       bool addEmptyElements  );

    static StringSet ParseUniqueElements( const UInt8* bufferPtr ,
                                          UInt32 bufferSize      ,
                                          UInt32 seperator       ,
                                          bool addEmptyElements  );

    bool WildcardEquals( const CUtf8String& strWithWildcards  ,
                         const UInt32 wildCardToken = '*'     ,
                         const bool caseSensitive = true      ,
                         const bool biDirectional = false     ) const;

    /**
     *  It is common for wildcard comparisons to be done against a list
     *  as such we provide this variant as a convenience
     */
    bool WildcardEquals( const StringSet& strsWithWildcards ,
                         const UInt32 wildCardToken = '*'   ,
                         const bool caseSensitive = true    ,
                         const bool biDirectional = false   ) const;

    bool Equals( const CUtf8String& otherStr     ,
                 const bool caseSensitive = true ) const;

    bool NotEquals( const CUtf8String& otherStr     ,
                    const bool caseSensitive = true ) const;

    CUtf8String Combine( const StringVector& elements, Int32 seperator ) const;
    CUtf8String Combine( const StringSet& elements, Int32 seperator ) const;
    CUtf8String Combine( const StringMap& elements, Int32 valueSeperator, Int32 kvSeperator ) const;
    
    void Clear( void );

    /**
     *  Explicitly validates whether the bytes contained within conform with the formatting rules
     *  In this case whether the bytes confirm to the rules of a UTF8 string
     *  To prevent exploits you should utilize this member function when accepting data from external sources
     */
    bool IsFormattingValid( void ) const;

    /**
     *  UTF8 class specific:
     *  Explicitly reduce the content down to ASCII.
     *  This is a lossy operation. Any non-ASCII UTF8 code point is replaced
     *  by the 'asciiReplacement' character.
     */
    CAsciiString ForceToAscii( char asciiReplacement = '*' ) const;

    /**
     *  UTF8 class specific:
     *  Provides external code the ability to leverage UTF translation
     *  @return the number of bytes the UTF32 code point takes up when encoded as UTF8 inside the given buffer, -1 on error
     */
    static Int32 EncodeUtf32CodePointToUtf8( const UInt32 utf32CodePoint ,
                                             char* outUtf8Buffer         ,
                                             UInt32 outUtf8BufferSize    );
    
    /**
     *  UTF8 class specific:
     *  Provides external code the ability to leverage UTF translation
     *  @return the number of bytes the UTF8 code point takes up and thus how many bytes were read from 'utf8Buffer', -1 on error
     */
    static Int32 EncodeUtf8CodePointToUtf32( const char* utf8Buffer        ,
                                             const UInt32 utf8BufferSize   ,
                                             UInt32& outUtf32CodePoint     );

    static bool ReadUtf32CodePoint( CIOAccess* io, UInt32* utf32CodePoint );

    static CUtf8String ReadLine( CIOAccess* io );

    static CUtf8String ReadString( CIOAccess* io );

    private:

    //  Scans the UTF8 variable length string to the ptr address given to determine which code point index that is 
    // and the associated code point. Returns -1 if not a valid sub string pointer
    Int32 CodepointIndexAtPtr( const char* subStrPtr, UInt32& codePoint ) const;
    
    private:
    char* m_string;    /**< our actual null-terminated UTF8 encoded string */
    UInt32 m_length;   /**< length of the string counted in code points */
    UInt32 m_byteSize; /**< storage byte size of the string */
};

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool operator!=( const char* lhs, const CUtf8String& rhs );
GUCEF_CORE_PUBLIC_CPP bool operator==( const char* lhs, const CUtf8String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf8String operator+( const char* lhs, const CUtf8String& rhs );
GUCEF_CORE_PUBLIC_CPP CUtf8String operator+( const char lhs, const CUtf8String& rhs );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF8STRING_H ? */

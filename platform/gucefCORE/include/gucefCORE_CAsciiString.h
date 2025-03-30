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

#ifndef GUCEF_CORE_CASCIISTRING_H
#define GUCEF_CORE_CASCIISTRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>
#include <set>
#include <string>
#include <map>

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
class CIOAccess;

class GUCEF_CORE_PUBLIC_CPP CAsciiString
{
    public:

    typedef std::vector< CAsciiString, gucef_allocator< CAsciiString > >                                       StringVector;
    typedef std::set< CAsciiString, std::less< CAsciiString >, gucef_allocator< CAsciiString > >               StringSet;
    typedef std::pair< const CAsciiString, CAsciiString >                                                      StringPair;
    typedef std::map< CAsciiString, CAsciiString, std::less< CAsciiString >, gucef_allocator< StringPair > >   StringMap;
    typedef std::pair< const CAsciiString, StringSet >                                                                            StringStringSetPair;
    typedef std::map< CAsciiString, StringSet, std::less< CAsciiString >, gucef_allocator< StringStringSetPair > >                StringMapSet;
    typedef std::pair< const CAsciiString, StringMap >                                                                            StringStringMapPair;
    typedef std::map< CAsciiString, StringMap, std::less< CAsciiString >, gucef_allocator< StringStringMapPair > >                StringMapMap;
    typedef std::pair< const CAsciiString, StringMapSet >                                                                         StringStringMapSetPair;
    typedef std::map< CAsciiString, StringMapSet, std::less< CAsciiString >, gucef_allocator< StringStringMapSetPair > >          StringMapMapSet;
    typedef std::pair< const CAsciiString, StringMapMap >                                                                         StringStringMapMapPair;
    typedef std::map< CAsciiString, StringMapMap, std::less< CAsciiString >, gucef_allocator< StringStringMapMapPair > >          StringMapMapMap;
    typedef std::pair< const CAsciiString, StringMapMapMap >                                                                      StringStringMapMapMapPair;
    typedef std::map< CAsciiString, StringMapMap, std::less< CAsciiString >, gucef_allocator< StringStringMapMapMapPair > >       StringMapMapMapMap;
    typedef std::pair< const CAsciiString, StringMapMapSet >                                                                      StringStringMapMapSetPair;
    typedef std::map< CAsciiString, StringMapMapSet, std::less< CAsciiString >, gucef_allocator< StringStringMapMapSetPair > >    StringMapMapMapSet;

    static const CAsciiString   Empty;
    static const StringVector   EmptyStringVector;
    static const StringSet      EmptyStringSet;

    CAsciiString( void );

    CAsciiString( const CAsciiString& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CAsciiString( CAsciiString&& src ) GUCEF_NOEXCEPT;
    #endif

    CAsciiString( const CUtf8String& src ) GUCEF_NOEXCEPT;

    CAsciiString( const std::string& src ) GUCEF_NOEXCEPT;

    CAsciiString( const char *src ) GUCEF_NOEXCEPT;

    /**
     *
     *  @param reexamineLength if true the length passed will be treated as a maxima up to which a new length is to be determined
     */
    CAsciiString( const char* src              ,
                  UInt32 length                ,
                  bool reexamineLength = false ) GUCEF_NOEXCEPT;

    explicit CAsciiString( const char src );

    explicit CAsciiString( const int NULLvalue );

    ~CAsciiString();

    CAsciiString& operator=( const CAsciiString &src );

    CAsciiString& operator=( const std::string& src );

    /**
     *      Copys the source string.
     *      If the src string is NULL then the current string will be
     *      deallocated and also set to NULL.
     */
    CAsciiString& operator=( const char *src );

    CAsciiString& operator+=( const CAsciiString &other );

    CAsciiString& operator+=( const char *other );

    CAsciiString& operator+=( char lastchar );

    CAsciiString operator+( const CAsciiString& addition ) const;

    CAsciiString operator+( const char* addition ) const;

    CAsciiString operator+( const char addition ) const;

    bool operator==( const CAsciiString &other ) const;

    bool operator==( const char *other ) const;

    bool operator==( const int NULLvalue ) const;

    bool operator==( const char character ) const;

    bool operator!=( const CAsciiString &other ) const;

    bool operator!=( const char *other ) const;

    bool operator!=( const int NULLvalue ) const;

    bool operator!=( const char character ) const;

    bool operator<( const CAsciiString& other ) const;

    bool operator<=( const CAsciiString& other ) const;

    char operator[]( const UInt32 index ) const;

    char& operator[]( const UInt32 index );

    bool IsNULLOrEmpty( void ) const;

    operator std::string() const;

    /* operator const char*() const; */

    const char* C_String( void ) const;

    char* C_String( void );

    /**
     *  If you wish to manipulate characters in a char buffer
     *  while still having containment as a string object this function
     *  is for you. It will set the string to have an unititialized char buffer
     *  of bufferSize bytes.
     */
    char* Reserve( const UInt32 bufferSize, Int32 newLength = -1 );

    std::string STL_String( void ) const;

    UInt32 Length( void ) const;

    UInt32 ByteSize( void ) const;

    /**
     *  Allows you to set the length of the string.
     */
    void SetLength( UInt32 newLength );

    /**
     *  This function will give you the length of the string just like the regular Length()
     *  However it will determine the length based on the null terminator and overwrite the cached
     *  length value. This is useful in combination with the Reserve() function.
     */
    UInt32 DetermineLength( void );

    /**
     *      This member functions allows you to set the string using a
     *      non-null-terminated char array as the source. The source will
     *      be copied and a null terminator will be added.
     */
    void Set( const char *new_str ,
              UInt32 len          );

    void Set( const char* new_str );

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
     *      This member functions allows you to add to the string using a
     *      non-null-terminated char array as the source. The source will
     *      be copyed and a null terminator will be added.
     */
    void Append( const char* appendstr ,
                 UInt32 len            );

    void Append( const char* appendstr );

    CAsciiString Lowercase( void ) const;

    CAsciiString Uppercase( void ) const;

    CAsciiString ReplaceChar( char oldchar ,
                              char newchar ) const;

    CAsciiString ReplaceChars( const char* oldchars ,
                               UInt32 nrOfOldChars  ,
                               char newchar         ) const;

    CAsciiString ReplaceSubstr( const CAsciiString& substr      ,
                                const CAsciiString& replacement ) const;

    /**
     *  Replaces an enveloping substring with another
     *  Example: "$MyEnvelope(" ")" -> "%{" "}"
     */
    CAsciiString ReplaceEnvelopingSubstr( const CAsciiString& envelopPrefix     ,
                                          const CAsciiString& envelopPostfix    ,
                                          const CAsciiString& newEnvelopPrefix  ,
                                          const CAsciiString& newEnvelopPostfix ) const;

    CAsciiString CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                     const CUtf8String& envelopPostfix ,
                                     UInt32 envelopedStrIndex          ) const;

    CAsciiString ReplaceSubStr( UInt32 startIndex        ,
                                UInt32 length            ,
                                const CAsciiString& newSubstr ) const;

    CAsciiString SubstrToChar( char searchchar                        ,
                               bool frontToBack = true                ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CAsciiString SubstrToChar( char searchchar                        ,
                               UInt32 startIndex                      ,
                               bool frontToBack = true                ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CAsciiString SubstrToSubstr( const CAsciiString& searchstr ,
                                 bool frontToBack = true  ) const;

    CAsciiString SubstrToSubstr( const CAsciiString& searchstr ,
                                 UInt32 startIndex        ,
                                 bool frontToBack = true  ) const;

    CAsciiString SubstrToIndex( UInt32 index            ,
                                bool frontToBack = true ) const;

    /**
     *  As per common convention:
     *  startIndex is inclusive and endIndex is exclusive
     *  Put another way: the char pointed to by endIndex will NOT be included
     */
    CAsciiString SubstrFromRange( UInt32 startIndex ,
                                 UInt32 endIndex   ) const;

    CAsciiString Trim( bool frontToBack ) const;

    Int32 HasChar( char searchchar         ,
                   bool frontToBack = true ) const;

    Int32 HasChar( char searchchar         ,
                   const UInt32 startIndex ,
                   bool frontToBack        ) const;

    Int32 HasSubstr( const CAsciiString& substr   ,
                     Int32 startIndex        ,
                     bool frontToBack = true ) const;

    Int32 HasSubstr( const CAsciiString& substr   ,
                     bool frontToBack = true ) const;

    UInt32 GetCharacterCount( const char searchChar ) const;

    /**
     *  Counts how many times any of the given ASCII denoted search characters
     *  occurs within the ASCII encoded string
     */
    UInt32 GetCharactersCount( const char* searchChars ,
                               UInt32 nrOfSearchChars  ) const;

    /**
     *  Counts how many times characters differing from the given ASCII denoted search characters
     *  occur within the ASCII encoded string
     */
    UInt32 GetNonMatchCharactersCount( const char* searchChars ,
                                       UInt32 nrOfSearchChars  ) const;

    UInt32 GetCharacterRepeatCount( const char searchChar ) const;

    UInt32 FindMaxSubstrEquality( const CAsciiString& searchStr ,
                                  const UInt32 startOffset      ,
                                  bool frontToBack              ,
                                  bool isCaseSentive            ) const;

    CAsciiString CutChars( UInt32 charcount        ,
                           bool frontToBack = true ,
                           UInt32 startOffset = 0  ) const;

    CAsciiString RemoveChar( const char charToRemove ) const;

    bool HasRepeatingChar( const char charToCheck ) const;

    CAsciiString CompactRepeatingChar( const char charToCompact ) const;

    StringVector ParseElements( char seperator               ,
                                bool addEmptyElements = true ) const;

    StringSet ParseUniqueElements( char seperator               ,
                                   bool addEmptyElements = true ) const;

    static StringVector ParseElements( const char* bufferPtr  ,
                                       UInt32 bufferSize      ,
                                       char seperator         ,
                                       bool addEmptyElements  );

    static StringSet ParseUniqueElements( const char* bufferPtr ,
                                          UInt32 bufferSize     ,
                                          char seperator        ,
                                          bool addEmptyElements );

    bool WildcardEquals( const CAsciiString& strWithWildcards  ,
                         const char wildCardToken = '*'        ,
                         const bool caseSensitive = true       ,
                         const bool biDirectional = false      ) const;

    /**
     *  It is common for wildcard comparisons to be done against a list
     *  as such we provide this variant as a convenience
     */
    bool WildcardEquals( const StringSet& strsWithWildcards ,
                         const Int32 wildCardToken = '*'    ,
                         const bool caseSensitive = true    ,
                         const bool biDirectional = false   ) const;

    bool Equals( const CAsciiString& otherStr    ,
                 const bool caseSensitive = true ) const;

    bool NotEquals( const CAsciiString& otherStr    ,
                    const bool caseSensitive = true ) const;

    CAsciiString Combine( const StringVector& elements, char seperator ) const;
    CAsciiString Combine( const StringSet& elements, char seperator ) const;

    void Clear( void );

    /**
     *  Explicitly validates whether the bytes contained within conform with the formatting rules
     *  In this case whether the bytes confirm to the rules of a ASCII string (0-127 only, single byte per code point)
     *  To prevent exploits you should utilize this member function when accepting data from external sources
     */
    bool IsFormattingValid( void ) const;

    static CAsciiString ReadLine( CIOAccess* io );

    static CAsciiString ReadString( CIOAccess* io );

    private:
    char* m_string;    /**< our actual null-terminated string */
    UInt32 m_length;   /**< length of the string */
};

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP bool operator!=( const char* lhs, const CAsciiString& rhs );
GUCEF_CORE_PUBLIC_CPP bool operator==( const char* lhs, const CAsciiString& rhs );
GUCEF_CORE_PUBLIC_CPP CAsciiString operator+( const char* lhs, const CAsciiString& rhs );
GUCEF_CORE_PUBLIC_CPP CAsciiString operator+( const char lhs, const CAsciiString& rhs );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASCIISTRING_H ? */

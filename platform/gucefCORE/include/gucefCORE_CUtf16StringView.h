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

#ifndef GUCEF_CORE_CUTF16STRING_VIEW_H
#define GUCEF_CORE_CUTF16STRING_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CUTF16STRING_H
#include "gucefCORE_CUtf16String.h"
#define GUCEF_CORE_CUTF16STRING_H
#endif /* GUCEF_CORE_CUTF16STRING_H ? */

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

/**
 *  Non-owning view of a UTF-16 string buffer.
 *
 *  CUtf16StringView wraps a CUtf16String that is permanently kept in linked
 *  (non-owning) mode, enforcing the non-owning contract at the type level.
 *  It is impossible to have a CUtf16StringView that owns memory.
 *
 *  Copying a view copies the link (rebinds to the same external buffer).
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CUtf16String& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that the referenced buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CUtf16StringView
{
    public:

    CUtf16StringView( void );

    /**
     *  @param codeUnits  number of UInt16 elements excluding the null terminator
     */
    CUtf16StringView( const UInt16* buffer, UInt32 codeUnits, UInt32 lengthInCodePoints );

    explicit CUtf16StringView( const CUtf16String& src );

    CUtf16StringView( const CUtf16StringView& src );

    CUtf16StringView& operator=( const CUtf16StringView& src );

    ~CUtf16StringView();

    /* --- Const read operations — all delegate to the inner string --- */

    UInt32           Length( void ) const;
    UInt32           ByteSize( void ) const;
    bool             IsNULLOrEmpty( void ) const;
    bool             IsLinked( void ) const;
    bool             IsFormattingValid( void ) const;
    const UInt16*    C_String( void ) const;
    std::string      STL_String( void ) const;

    operator std::string() const;

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

    UInt32 operator[]( const UInt32 index ) const;

    bool Equals( const CUtf16String& otherStr  ,
                 bool caseSensitive = true      ) const;

    bool NotEquals( const CUtf16String& otherStr ,
                    bool caseSensitive = true     ) const;

    bool WildcardEquals( const CUtf16String& strWithWildcards                    ,
                         UInt32 wildCardToken = static_cast<UInt32>( '*' )       ,
                         bool caseSensitive = true                               ,
                         bool biDirectional = false                              ) const;

    bool WildcardEquals( const CUtf16String::StringSet& strsWithWildcards           ,
                         UInt32 wildCardToken = static_cast<UInt32>( '*' )          ,
                         bool caseSensitive = true                                  ,
                         bool biDirectional = false                                 ) const;

    Int32 HasChar( UInt32 searchchar       ,
                   bool frontToBack = true ) const;

    Int32 HasChar( char searchChar         ,
                   bool frontToBack = true ) const;

    Int32 HasChar( UInt32 searchchar  ,
                   UInt32 startIndex  ,
                   bool frontToBack   ) const;

    Int32 HasChar( char searchChar    ,
                   UInt32 startIndex  ,
                   bool frontToBack   ) const;

    Int32 HasSubstr( const CUtf16String& substr ,
                     Int32 startIndex           ,
                     bool frontToBack = true    ,
                     bool shiftSearch = true    ) const;

    Int32 HasSubstr( const CUtf16String& substr ,
                     bool frontToBack = true    ,
                     bool shiftSearch = true    ) const;

    bool StartsWith( const CUtf16String& substr ) const;
    bool EndsWith( const CUtf16String& substr ) const;

    CUtf16String Lowercase( void ) const;
    CUtf16String Uppercase( void ) const;
    CUtf16String Trim( bool frontToBack ) const;

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

    CUtf16String SubstrFromRange( UInt32 startIndex ,
                                  UInt32 endIndex   ) const;

    UInt32 CodepointAtIndex( const UInt32 index ) const;

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

    UInt32 FindMaxSubstrEquality( const CUtf16String& searchStr ,
                                  UInt32 startOffset             ,
                                  bool frontToBack               ,
                                  bool isCaseSensitive           ) const;

    UInt32 FindMaxSegmentEquality( const CUtf16String& otherStr    ,
                                   UInt32 segmentDividerCodePoint  ,
                                   bool startFront                 ,
                                   bool isCaseSensitive            ) const;

    CUtf16String CutChars( UInt32 charcount        ,
                           bool frontToBack = true ,
                           UInt32 startOffset = 0  ) const;

    CUtf16String RemoveChar( UInt32 codePointToRemove ) const;

    CUtf16String RemoveChar( Int32 codePointToRemove ) const;

    CUtf16String CompactRepeatingChar( UInt32 codePointToCompact ) const;

    CUtf16String CompactRepeatingChar( Int32 codePointToCompact ) const;

    CUtf16String::StringVector ParseElements( UInt32 separator,
                                             bool addEmptyElements = true ) const;

    CUtf16String::StringSet ParseUniqueElements( UInt32 separator,
                                                bool addEmptyElements = true ) const;

    CUtf16String Combine( const CUtf16String::StringVector& elements, UInt32 separator ) const;
    CUtf16String Combine( const CUtf16String::StringSet& elements,    UInt32 separator ) const;

    CAsciiString ForceToAscii( char asciiReplacement = '*' ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CUtf16String& is accepted without copying.
     */
    operator const CUtf16String&( void ) const;

    /**
     *  Explicit accessor to the inner linked string.
     */
    const CUtf16String& AsString( void ) const;

    private:

    CUtf16String m_str;  /**< invariant: m_str.IsLinked() == true at all times */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF16STRING_VIEW_H ? */

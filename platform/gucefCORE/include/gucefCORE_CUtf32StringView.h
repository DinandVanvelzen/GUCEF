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

#ifndef GUCEF_CORE_CUTF32STRING_VIEW_H
#define GUCEF_CORE_CUTF32STRING_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CUTF32STRING_H
#include "gucefCORE_CUtf32String.h"
#define GUCEF_CORE_CUTF32STRING_H
#endif /* GUCEF_CORE_CUTF32STRING_H ? */

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
 *  Non-owning view of a UTF-32 string buffer.
 *
 *  CUtf32StringView wraps a CUtf32String that is permanently kept in linked
 *  (non-owning) mode, enforcing the non-owning contract at the type level.
 *  It is impossible to have a CUtf32StringView that owns memory.
 *
 *  Copying a view copies the link (rebinds to the same external buffer).
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CUtf32String& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that the referenced buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CUtf32StringView
{
    public:

    CUtf32StringView( void );

    /**
     *  @param lengthInCodePoints  number of UInt32 elements excluding the null terminator
     */
    CUtf32StringView( const UInt32* buffer, UInt32 lengthInCodePoints );

    explicit CUtf32StringView( const CUtf32String& src );

    CUtf32StringView( const CUtf32StringView& src );

    CUtf32StringView& operator=( const CUtf32StringView& src );

    ~CUtf32StringView();

    /* --- Const read operations — all delegate to the inner string --- */

    UInt32           Length( void ) const;
    UInt32           ByteSize( void ) const;
    bool             IsNULLOrEmpty( void ) const;
    bool             IsLinked( void ) const;
    bool             IsFormattingValid( void ) const;
    const UInt32*    C_String( void ) const;
    std::string      STL_String( void ) const;

    operator std::string() const;

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

    UInt32 operator[]( const UInt32 index ) const;

    bool Equals( const CUtf32String& otherStr  ,
                 bool caseSensitive = true      ) const;

    bool NotEquals( const CUtf32String& otherStr ,
                    bool caseSensitive = true     ) const;

    bool WildcardEquals( const CUtf32String& strWithWildcards                   ,
                         UInt32 wildCardToken = static_cast<UInt32>( '*' )      ,
                         bool caseSensitive = true                               ,
                         bool biDirectional = false                              ) const;

    bool WildcardEquals( const CUtf32String::StringSet& strsWithWildcards           ,
                         UInt32 wildCardToken = static_cast<UInt32>( '*' )          ,
                         bool caseSensitive = true                                  ,
                         bool biDirectional = false                                 ) const;

    Int32 HasChar( UInt32 searchchar       ,
                   bool frontToBack = true ) const;

    Int32 HasChar( UInt32 searchchar  ,
                   UInt32 startIndex  ,
                   bool frontToBack   ) const;

    Int32 HasSubstr( const CUtf32String& substr ,
                     Int32 startIndex            ,
                     bool frontToBack = true     ) const;

    Int32 HasSubstr( const CUtf32String& substr ,
                     bool frontToBack = true     ) const;

    CUtf32String Lowercase( void ) const;
    CUtf32String Uppercase( void ) const;
    CUtf32String Trim( bool frontToBack ) const;

    CUtf32String ReplaceChar( UInt32 oldCodePoint,
                              UInt32 newCodePoint ) const;

    CUtf32String ReplaceChars( const UInt32* oldCodePoints    ,
                               UInt32 nrOfOldCodePoints       ,
                               UInt32 newCodePoint            ) const;

    CUtf32String ReplaceSubstr( const CUtf32String& substr      ,
                                const CUtf32String& replacement ) const;

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

    CUtf32String SubstrFromRange( UInt32 startIndex ,
                                  UInt32 endIndex   ) const;

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

    CUtf32String::StringVector ParseElements( UInt32 separator               ,
                                             bool addEmptyElements = true   ) const;

    CUtf32String::StringSet ParseUniqueElements( UInt32 separator               ,
                                                bool addEmptyElements = true   ) const;

    CUtf32String Combine( const CUtf32String::StringVector& elements, UInt32 separator ) const;
    CUtf32String Combine( const CUtf32String::StringSet& elements,    UInt32 separator ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CUtf32String& is accepted without copying.
     */
    operator const CUtf32String&( void ) const;

    /**
     *  Explicit accessor to the inner linked string.
     */
    const CUtf32String& AsString( void ) const;

    private:

    CUtf32String m_str;  /**< invariant: m_str.IsLinked() == true at all times */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF32STRING_VIEW_H ? */

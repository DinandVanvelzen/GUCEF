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

#ifndef GUCEF_CORE_CASCIISTRING_VIEW_H
#define GUCEF_CORE_CASCIISTRING_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CASCIISTRING_H
#include "gucefCORE_CAsciiString.h"
#define GUCEF_CORE_CASCIISTRING_H
#endif /* GUCEF_CORE_CASCIISTRING_H ? */

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
 *  Non-owning view of an ASCII string buffer.
 *
 *  CAsciiStringView wraps a CAsciiString that is permanently kept in linked
 *  (non-owning) mode, enforcing the non-owning contract at the type level.
 *  It is impossible to have a CAsciiStringView that owns memory.
 *
 *  Copying a view copies the link (rebinds to the same external buffer).
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CAsciiString& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that the referenced buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CAsciiStringView
{
    public:

    CAsciiStringView( void );

    CAsciiStringView( const char* buffer, UInt32 length );

    explicit CAsciiStringView( const CAsciiString& src );

    CAsciiStringView( const CAsciiStringView& src );

    CAsciiStringView& operator=( const CAsciiStringView& src );

    ~CAsciiStringView();

    /* --- Const read operations — all delegate to the inner string --- */

    UInt32          Length( void ) const;
    UInt32          ByteSize( void ) const;
    bool            IsNULLOrEmpty( void ) const;
    bool            IsLinked( void ) const;
    bool            IsFormattingValid( void ) const;
    const char*     C_String( void ) const;
    std::string     STL_String( void ) const;

    operator std::string() const;

    bool operator==( const CAsciiString& other ) const;
    bool operator==( const char* other ) const;
    bool operator==( const int NULLvalue ) const;
    bool operator==( const char character ) const;

    bool operator!=( const CAsciiString& other ) const;
    bool operator!=( const char* other ) const;
    bool operator!=( const int NULLvalue ) const;
    bool operator!=( const char character ) const;

    bool operator<( const CAsciiString& other ) const;
    bool operator<=( const CAsciiString& other ) const;

    char operator[]( const UInt32 index ) const;

    bool Equals( const CAsciiString& otherStr    ,
                 bool caseSensitive = true        ) const;

    bool NotEquals( const CAsciiString& otherStr    ,
                    bool caseSensitive = true        ) const;

    bool WildcardEquals( const CAsciiString& strWithWildcards ,
                         char wildCardToken = '*'             ,
                         bool caseSensitive = true            ,
                         bool biDirectional = false           ) const;

    bool WildcardEquals( const CAsciiString::StringSet& strsWithWildcards ,
                         const Int32 wildCardToken = '*'                   ,
                         const bool caseSensitive = true                   ,
                         const bool biDirectional = false                  ) const;

    Int32 HasChar( char searchchar         ,
                   bool frontToBack = true ) const;

    Int32 HasChar( char searchchar         ,
                   const UInt32 startIndex ,
                   bool frontToBack        ) const;

    Int32 HasSubstr( const CAsciiString& substr ,
                     Int32 startIndex           ,
                     bool frontToBack = true    ) const;

    Int32 HasSubstr( const CAsciiString& substr ,
                     bool frontToBack = true    ) const;

    CAsciiString Lowercase( void ) const;
    CAsciiString Uppercase( void ) const;
    CAsciiString Trim( bool frontToBack ) const;

    CAsciiString ReplaceChar( char oldchar ,
                              char newchar ) const;

    CAsciiString ReplaceChars( const char* oldchars ,
                               UInt32 nrOfOldChars  ,
                               char newchar         ) const;

    CAsciiString ReplaceSubstr( const CAsciiString& substr      ,
                                const CAsciiString& replacement ) const;

    CAsciiString ReplaceEnvelopingSubstr( const CAsciiString& envelopPrefix     ,
                                          const CAsciiString& envelopPostfix    ,
                                          const CAsciiString& newEnvelopPrefix  ,
                                          const CAsciiString& newEnvelopPostfix ) const;

    CAsciiString CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                     const CUtf8String& envelopPostfix ,
                                     UInt32 envelopedStrIndex          ) const;

    CAsciiString ReplaceSubStr( UInt32 startIndex             ,
                                UInt32 length                 ,
                                const CAsciiString& newSubstr ) const;

    CAsciiString SubstrToChar( char searchchar                        ,
                               bool frontToBack = true                ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CAsciiString SubstrToChar( char searchchar                        ,
                               UInt32 startIndex                      ,
                               bool frontToBack = true                ,
                               bool returnEmptyIfCharNotFound = false ) const;

    CAsciiString SubstrToSubstr( const CAsciiString& searchstr ,
                                 bool frontToBack = true        ) const;

    CAsciiString SubstrToSubstr( const CAsciiString& searchstr ,
                                 UInt32 startIndex             ,
                                 bool frontToBack = true       ) const;

    CAsciiString SubstrToIndex( UInt32 index            ,
                                bool frontToBack = true ) const;

    CAsciiString SubstrFromRange( UInt32 startIndex ,
                                  UInt32 endIndex   ) const;

    UInt32 GetCharacterCount( const char searchChar ) const;

    UInt32 GetCharactersCount( const char* searchChars ,
                               UInt32 nrOfSearchChars  ) const;

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

    CAsciiString::StringVector ParseElements( char seperator               ,
                                             bool addEmptyElements = true ) const;

    CAsciiString::StringSet ParseUniqueElements( char seperator               ,
                                                bool addEmptyElements = true ) const;

    CAsciiString Combine( const CAsciiString::StringVector& elements, char seperator ) const;
    CAsciiString Combine( const CAsciiString::StringSet& elements,    char seperator ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CAsciiString& is accepted without copying.
     */
    operator const CAsciiString&( void ) const;

    /**
     *  Explicit accessor to the inner linked string.
     */
    const CAsciiString& AsString( void ) const;

    private:

    CAsciiString m_str;  /**< invariant: m_str.IsLinked() == true at all times */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASCIISTRING_VIEW_H ? */

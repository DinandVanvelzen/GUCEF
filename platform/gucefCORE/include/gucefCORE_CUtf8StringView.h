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

#ifndef GUCEF_CORE_CUTF8STRING_VIEW_H
#define GUCEF_CORE_CUTF8STRING_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CUTF8STRING_H
#include "gucefCORE_CUtf8String.h"
#define GUCEF_CORE_CUTF8STRING_H
#endif /* GUCEF_CORE_CUTF8STRING_H ? */

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
 *  Non-owning view of a UTF-8 string buffer.
 *
 *  CUtf8StringView wraps a CUtf8String that is permanently kept in linked
 *  (non-owning) mode, enforcing the non-owning contract at the type level.
 *  It is impossible to have a CUtf8StringView that owns memory.
 *
 *  Copying a view copies the link (rebinds to the same external buffer).
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CUtf8String& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that the referenced buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CUtf8StringView
{
    public:

    CUtf8StringView( void );

    CUtf8StringView( const char* buffer, UInt32 byteSize, UInt32 lengthInCodePoints );

    /**
     *  Construct from a null-terminated UTF-8 buffer.
     *  byteSize and length are computed via utf8size / utf8len.
     */
    explicit CUtf8StringView( const char* nullTerminatedBuffer );

    explicit CUtf8StringView( const CUtf8String& src );

    CUtf8StringView( const CUtf8StringView& src );

    CUtf8StringView& operator=( const CUtf8StringView& src );

    ~CUtf8StringView();

    /* --- Const read operations — all delegate to the inner string --- */

    UInt32          Length( void ) const;
    UInt32          ByteSize( void ) const;
    bool            IsNULLOrEmpty( void ) const;
    bool            IsLinked( void ) const;
    bool            IsFormattingValid( void ) const;
    const char*     C_String( void ) const;
    std::string     STL_String( void ) const;

    operator std::string() const;

    bool operator==( const CUtf8String& other ) const;
    bool operator==( const char* other ) const;
    bool operator==( const Int32 NULLvalue ) const;
    bool operator==( const UInt32 NULLvalueOrUtf32 ) const;
    bool operator==( const char character ) const;

    bool operator!=( const CUtf8String& other ) const;
    bool operator!=( const char* other ) const;
    bool operator!=( const Int32 NULLvalue ) const;
    bool operator!=( const UInt32 NULLvalueOrUtf32 ) const;
    bool operator!=( const char character ) const;

    bool operator<( const CUtf8String& other ) const;
    bool operator<=( const CUtf8String& other ) const;

    UInt32 operator[]( const UInt32 index ) const;

    bool Equals( const CUtf8String& otherStr    ,
                 bool caseSensitive = true        ) const;

    bool NotEquals( const CUtf8String& otherStr    ,
                    bool caseSensitive = true        ) const;

    bool WildcardEquals( const CUtf8String& strWithWildcards  ,
                         const UInt32 wildCardToken = '*'      ,
                         const bool caseSensitive = true       ,
                         const bool biDirectional = false      ) const;

    bool WildcardEquals( const CUtf8String::StringSet& strsWithWildcards ,
                         const UInt32 wildCardToken = '*'                 ,
                         const bool caseSensitive = true                  ,
                         const bool biDirectional = false                 ) const;

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

    bool StartsWith( const CUtf8String& substr ) const;
    bool EndsWith( const CUtf8String& substr ) const;

    CUtf8String Lowercase( void ) const;
    CUtf8String Uppercase( void ) const;
    CUtf8String Trim( bool frontToBack ) const;

    CUtf8String ReplaceChar( Int32 oldchar ,
                             Int32 newchar ) const;

    CUtf8String ReplaceChars( const Int32* oldchars ,
                              UInt32 nrOfOldChars   ,
                              Int32 newchar         ) const;

    CUtf8String ReplaceSubstr( const CUtf8String& substr      ,
                               const CUtf8String& replacement ) const;

    CUtf8String ReplaceEnvelopingSubstr( const CUtf8String& envelopPrefix     ,
                                         const CUtf8String& envelopPostfix    ,
                                         const CUtf8String& newEnvelopPrefix  ,
                                         const CUtf8String& newEnvelopPostfix ) const;

    CUtf8String CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                    const CUtf8String& envelopPostfix ,
                                    UInt32 envelopedStrIndex          ) const;

    CUtf8String ReplaceSubStr( UInt32 startIndex             ,
                               UInt32 length                 ,
                               const CUtf8String& newSubstr  ) const;

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

    CUtf8String SubstrFromRange( UInt32 startIndex ,
                                 UInt32 endIndex   ) const;

    UInt32 CodepointAtIndex( const UInt32 index ) const;

    UInt32 GetCharacterCount( const Int32 searchChar ) const;

    UInt32 GetCharactersCount( const Int32* searchChars ,
                               UInt32 nrOfSearchChars   ) const;

    UInt32 GetCharactersCount( const char* searchChars ,
                               UInt32 nrOfSearchChars  ) const;

    UInt32 GetNonMatchCharactersCount( const CUtf8String& searchChars ) const;

    UInt32 GetNonMatchCharactersCount( const Int32* searchChars ,
                                       UInt32 nrOfSearchChars   ) const;

    UInt32 GetNonMatchCharactersCount( const char* searchChars ,
                                       UInt32 nrOfSearchChars  ) const;

    UInt32 GetCharacterRepeatCount( const Int32 searchChar ) const;

    UInt32 FindMaxSubstrEquality( const CUtf8String& searchStr ,
                                  const UInt32 startOffset     ,
                                  bool frontToBack             ,
                                  bool isCaseSentive           ) const;

    UInt32 FindMaxSegmentEquality( const CUtf8String& otherStr   ,
                                   Int32 segmentDividerCodePoint ,
                                   bool startFront               ,
                                   bool isCaseSentive            ) const;

    CUtf8String CutChars( UInt32 charcount        ,
                          bool frontToBack = true ,
                          UInt32 startOffset = 0  ) const;

    CUtf8String RemoveChar( const Int32 charToRemove ) const;

    CUtf8String CompactRepeatingChar( const Int32 charToCompact ) const;

    CUtf8String::StringVector ParseElements( UInt32 seperator             ,
                                             bool addEmptyElements = true ) const;

    CUtf8String::StringSet ParseUniqueElements( UInt32 seperator             ,
                                                bool addEmptyElements = true ) const;

    CUtf8String Combine( const CUtf8String::StringVector& elements, Int32 seperator ) const;
    CUtf8String Combine( const CUtf8String::StringSet& elements,    Int32 seperator ) const;

    CAsciiString ForceToAscii( char asciiReplacement = '*' ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CUtf8String& is accepted without copying.
     */
    operator const CUtf8String&( void ) const;

    /**
     *  Explicit accessor to the inner linked string.
     */
    const CUtf8String& AsString( void ) const;

    private:

    CUtf8String m_str;  /**< invariant: m_str.IsLinked() == true at all times */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUTF8STRING_VIEW_H ? */

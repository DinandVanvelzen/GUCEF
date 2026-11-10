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

#ifndef GUCEF_CORE_CUTF16STRING_VIEW_H
#include "gucefCORE_CUtf16StringView.h"
#define GUCEF_CORE_CUTF16STRING_VIEW_H
#endif /* GUCEF_CORE_CUTF16STRING_VIEW_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUtf16StringView::CUtf16StringView( void )
    : m_str()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::CUtf16StringView( const UInt16* buffer, UInt32 codeUnits, UInt32 lengthInCodePoints )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( buffer, codeUnits, lengthInCodePoints );
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::CUtf16StringView( const CUtf16String& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::CUtf16StringView( const CUtf16StringView& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src.m_str );
}

/*-------------------------------------------------------------------------*/

CUtf16StringView&
CUtf16StringView::operator=( const CUtf16StringView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_str.LinkTo( src.m_str );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::~CUtf16StringView()
{GUCEF_TRACE;

    /* m_str destructor will skip delete because it is linked */
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::Length( void ) const
{GUCEF_TRACE;

    return m_str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::ByteSize( void ) const
{GUCEF_TRACE;

    return m_str.ByteSize();
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_str.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const UInt16*
CUtf16StringView::C_String( void ) const
{GUCEF_TRACE;

    return m_str.C_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::Equals( const CUtf16String& otherStr ,
                           bool caseSensitive           ) const
{GUCEF_TRACE;

    return m_str.Equals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::WildcardEquals( const CUtf16String& strWithWildcards ,
                                   UInt32 wildCardToken                  ,
                                   bool caseSensitive                    ,
                                   bool biDirectional                    ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasChar( UInt32 searchchar ,
                            bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::Lowercase( void ) const
{GUCEF_TRACE;

    return m_str.Lowercase();
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::Uppercase( void ) const
{GUCEF_TRACE;

    return m_str.Uppercase();
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.Trim( frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrFromRange( UInt32 startIndex ,
                                    UInt32 endIndex   ) const
{GUCEF_TRACE;

    return m_str.SubstrFromRange( startIndex, endIndex );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::IsLinked( void ) const
{GUCEF_TRACE;

    return m_str.IsLinked();
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::IsFormattingValid( void ) const
{GUCEF_TRACE;

    return m_str.IsFormattingValid();
}

/*-------------------------------------------------------------------------*/

std::string
CUtf16StringView::STL_String( void ) const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::operator std::string() const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator==( const CUtf16String& other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator==( const char* other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator==( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return m_str == NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator==( const UInt32 utf32CodePoint ) const
{GUCEF_TRACE;

    return m_str == utf32CodePoint;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator==( const char character ) const
{GUCEF_TRACE;

    return m_str == character;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator!=( const CUtf16String& other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator!=( const char* other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator!=( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return m_str != NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator!=( const UInt32 utf32CodePoint ) const
{GUCEF_TRACE;

    return m_str != utf32CodePoint;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator!=( const char character ) const
{GUCEF_TRACE;

    return m_str != character;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator<( const CUtf16String& other ) const
{GUCEF_TRACE;

    return m_str < other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::operator<=( const CUtf16String& other ) const
{GUCEF_TRACE;

    return m_str <= other;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::NotEquals( const CUtf16String& otherStr ,
                              bool caseSensitive           ) const
{GUCEF_TRACE;

    return m_str.NotEquals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::WildcardEquals( const CUtf16String::StringSet& strsWithWildcards ,
                                   UInt32 wildCardToken                              ,
                                   bool caseSensitive                                ,
                                   bool biDirectional                                ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strsWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasChar( char searchChar ,
                            bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchChar, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasChar( UInt32 searchchar ,
                            UInt32 startIndex ,
                            bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasChar( char searchChar  ,
                            UInt32 startIndex ,
                            bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchChar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasSubstr( const CUtf16String& substr ,
                              Int32 startIndex           ,
                              bool frontToBack           ,
                              bool shiftSearch           ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, startIndex, frontToBack, shiftSearch );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf16StringView::HasSubstr( const CUtf16String& substr ,
                              bool frontToBack           ,
                              bool shiftSearch           ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, frontToBack, shiftSearch );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::StartsWith( const CUtf16String& substr ) const
{GUCEF_TRACE;

    return m_str.StartsWith( substr );
}

/*-------------------------------------------------------------------------*/

bool
CUtf16StringView::EndsWith( const CUtf16String& substr ) const
{GUCEF_TRACE;

    return m_str.EndsWith( substr );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceChar( UInt32 oldCodePoint,
                                UInt32 newCodePoint ) const
{GUCEF_TRACE;

    return m_str.ReplaceChar( oldCodePoint, newCodePoint );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceChar( Int32 oldCodePoint,
                                Int32 newCodePoint ) const
{GUCEF_TRACE;

    return m_str.ReplaceChar( oldCodePoint, newCodePoint );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceChars( const Int32* oldCodePoints    ,
                                 UInt32 nrOfOldCodePoints       ,
                                 Int32 newCodePoint             ) const
{GUCEF_TRACE;

    return m_str.ReplaceChars( oldCodePoints, nrOfOldCodePoints, newCodePoint );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceSubstr( const CUtf16String& substr      ,
                                  const CUtf16String& replacement ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubstr( substr, replacement );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceEnvelopingSubstr( const CUtf16String& envelopPrefix     ,
                                            const CUtf16String& envelopPostfix    ,
                                            const CUtf16String& newEnvelopPrefix  ,
                                            const CUtf16String& newEnvelopPostfix ) const
{GUCEF_TRACE;

    return m_str.ReplaceEnvelopingSubstr( envelopPrefix, envelopPostfix, newEnvelopPrefix, newEnvelopPostfix );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::CutEnvelopedSubstr( const CUtf16String& envelopPrefix  ,
                                       const CUtf16String& envelopPostfix ,
                                       UInt32 envelopedStrIndex            ) const
{GUCEF_TRACE;

    return m_str.CutEnvelopedSubstr( envelopPrefix, envelopPostfix, envelopedStrIndex );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::ReplaceSubStr( UInt32 startIndex                ,
                                  UInt32 length                    ,
                                  const CUtf16String& newSubstr    ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubStr( startIndex, length, newSubstr );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToChar( UInt32 searchCodePoint               ,
                                 bool frontToBack                     ,
                                 bool returnEmptyIfCharNotFound        ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchCodePoint, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToChar( char searchChar                ,
                                 bool frontToBack               ,
                                 bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchChar, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToChar( UInt32 searchCodePoint               ,
                                 UInt32 startIndex                    ,
                                 bool frontToBack                     ,
                                 bool returnEmptyIfCharNotFound        ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchCodePoint, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToChar( char searchChar                ,
                                 UInt32 startIndex              ,
                                 bool frontToBack               ,
                                 bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchChar, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToSubstr( const CUtf16String& searchstr ,
                                   bool frontToBack              ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToSubstr( const CUtf16String& searchstr ,
                                   UInt32 startIndex              ,
                                   bool frontToBack               ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::SubstrToIndex( UInt32 index, bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.SubstrToIndex( index, frontToBack );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::CodepointAtIndex( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str.CodepointAtIndex( index );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharacterCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharacterCount( Int32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharactersCount( const Int32* searchCodePoints,
                                       UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchCodePoints, nrOfSearchCodePoints );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharactersCount( const char* searchChars,
                                       UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetNonMatchCharactersCount( const CUtf16String& searchChars ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetNonMatchCharactersCount( const Int32* searchCodePoints,
                                               UInt32 nrOfSearchCodePoints   ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchCodePoints, nrOfSearchCodePoints );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetNonMatchCharactersCount( const char* searchChars,
                                               UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharacterRepeatCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterRepeatCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::GetCharacterRepeatCount( Int32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterRepeatCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::FindMaxSubstrEquality( const CUtf16String& searchStr ,
                                          UInt32 startOffset             ,
                                          bool frontToBack               ,
                                          bool isCaseSensitive           ) const
{GUCEF_TRACE;

    return m_str.FindMaxSubstrEquality( searchStr, startOffset, frontToBack, isCaseSensitive );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf16StringView::FindMaxSegmentEquality( const CUtf16String& otherStr    ,
                                           UInt32 segmentDividerCodePoint  ,
                                           bool startFront                 ,
                                           bool isCaseSensitive            ) const
{GUCEF_TRACE;

    return m_str.FindMaxSegmentEquality( otherStr, segmentDividerCodePoint, startFront, isCaseSensitive );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::CutChars( UInt32 charcount        ,
                             bool frontToBack        ,
                             UInt32 startOffset      ) const
{GUCEF_TRACE;

    return m_str.CutChars( charcount, frontToBack, startOffset );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::RemoveChar( UInt32 codePointToRemove ) const
{GUCEF_TRACE;

    return m_str.RemoveChar( codePointToRemove );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::RemoveChar( Int32 codePointToRemove ) const
{GUCEF_TRACE;

    return m_str.RemoveChar( codePointToRemove );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::CompactRepeatingChar( UInt32 codePointToCompact ) const
{GUCEF_TRACE;

    return m_str.CompactRepeatingChar( codePointToCompact );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::CompactRepeatingChar( Int32 codePointToCompact ) const
{GUCEF_TRACE;

    return m_str.CompactRepeatingChar( codePointToCompact );
}

/*-------------------------------------------------------------------------*/

CUtf16String::StringVector
CUtf16StringView::ParseElements( UInt32 separator, bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseElements( separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf16String::StringSet
CUtf16StringView::ParseUniqueElements( UInt32 separator, bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseUniqueElements( separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::Combine( const CUtf16String::StringVector& elements, UInt32 separator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, separator );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CUtf16StringView::Combine( const CUtf16String::StringSet& elements, UInt32 separator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, separator );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CUtf16StringView::ForceToAscii( char asciiReplacement ) const
{GUCEF_TRACE;

    return m_str.ForceToAscii( asciiReplacement );
}

/*-------------------------------------------------------------------------*/

CUtf16StringView::operator const CUtf16String&( void ) const
{GUCEF_TRACE;

    return m_str;
}

/*-------------------------------------------------------------------------*/

const CUtf16String&
CUtf16StringView::AsString( void ) const
{GUCEF_TRACE;

    return m_str;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

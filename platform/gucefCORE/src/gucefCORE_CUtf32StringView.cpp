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

#ifndef GUCEF_CORE_CUTF32STRING_VIEW_H
#include "gucefCORE_CUtf32StringView.h"
#define GUCEF_CORE_CUTF32STRING_VIEW_H
#endif /* GUCEF_CORE_CUTF32STRING_VIEW_H ? */

#include <string>

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

CUtf32StringView::CUtf32StringView( void )
    : m_str()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::CUtf32StringView( const UInt32* buffer, UInt32 lengthInCodePoints )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( buffer, lengthInCodePoints );
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::CUtf32StringView( const CUtf32String& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::CUtf32StringView( const CUtf32StringView& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src.m_str );
}

/*-------------------------------------------------------------------------*/

CUtf32StringView&
CUtf32StringView::operator=( const CUtf32StringView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_str.LinkTo( src.m_str );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::~CUtf32StringView()
{GUCEF_TRACE;

    /* m_str destructor will skip delete because it is linked */
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::Length( void ) const
{GUCEF_TRACE;

    return m_str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::ByteSize( void ) const
{GUCEF_TRACE;

    return m_str.ByteSize();
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_str.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const UInt32*
CUtf32StringView::C_String( void ) const
{GUCEF_TRACE;

    return m_str.C_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::Equals( const CUtf32String& otherStr ,
                           bool caseSensitive           ) const
{GUCEF_TRACE;

    return m_str.Equals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::WildcardEquals( const CUtf32String& strWithWildcards ,
                                   UInt32 wildCardToken                  ,
                                   bool caseSensitive                    ,
                                   bool biDirectional                    ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32StringView::HasChar( UInt32 searchchar ,
                            bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32StringView::HasSubstr( const CUtf32String& substr ,
                              Int32 startIndex            ,
                              bool frontToBack            ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32StringView::HasSubstr( const CUtf32String& substr ,
                              bool frontToBack            ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::Lowercase( void ) const
{GUCEF_TRACE;

    return m_str.Lowercase();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::Uppercase( void ) const
{GUCEF_TRACE;

    return m_str.Uppercase();
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.Trim( frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrFromRange( UInt32 startIndex ,
                                    UInt32 endIndex   ) const
{GUCEF_TRACE;

    return m_str.SubstrFromRange( startIndex, endIndex );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::IsLinked( void ) const
{GUCEF_TRACE;

    return m_str.IsLinked();
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::IsFormattingValid( void ) const
{GUCEF_TRACE;

    return m_str.IsFormattingValid();
}

/*-------------------------------------------------------------------------*/

std::string
CUtf32StringView::STL_String( void ) const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::operator std::string() const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator==( const CUtf32String& other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator==( const char* other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator==( const int NULLvalue ) const
{GUCEF_TRACE;

    return m_str == NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator==( UInt32 codePoint ) const
{GUCEF_TRACE;

    return m_str == codePoint;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator!=( const CUtf32String& other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator!=( const char* other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator!=( const int NULLvalue ) const
{GUCEF_TRACE;

    return m_str != NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator!=( UInt32 codePoint ) const
{GUCEF_TRACE;

    return m_str != codePoint;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator<( const CUtf32String& other ) const
{GUCEF_TRACE;

    return m_str < other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::operator<=( const CUtf32String& other ) const
{GUCEF_TRACE;

    return m_str <= other;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::NotEquals( const CUtf32String& otherStr ,
                              bool caseSensitive           ) const
{GUCEF_TRACE;

    return m_str.NotEquals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::WildcardEquals( const CUtf32String::StringSet& strsWithWildcards ,
                                   UInt32 wildCardToken                              ,
                                   bool caseSensitive                                ,
                                   bool biDirectional                                ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strsWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf32StringView::HasChar( UInt32 searchchar ,
                            UInt32 startIndex ,
                            bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::ReplaceChar( UInt32 oldCodePoint,
                                UInt32 newCodePoint ) const
{GUCEF_TRACE;

    return m_str.ReplaceChar( oldCodePoint, newCodePoint );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::ReplaceChars( const UInt32* oldCodePoints    ,
                                 UInt32 nrOfOldCodePoints       ,
                                 UInt32 newCodePoint            ) const
{GUCEF_TRACE;

    return m_str.ReplaceChars( oldCodePoints, nrOfOldCodePoints, newCodePoint );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::ReplaceSubstr( const CUtf32String& substr      ,
                                  const CUtf32String& replacement ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubstr( substr, replacement );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::ReplaceEnvelopingSubstr( const CUtf32String& envelopPrefix     ,
                                            const CUtf32String& envelopPostfix    ,
                                            const CUtf32String& newEnvelopPrefix  ,
                                            const CUtf32String& newEnvelopPostfix ) const
{GUCEF_TRACE;

    return m_str.ReplaceEnvelopingSubstr( envelopPrefix, envelopPostfix, newEnvelopPrefix, newEnvelopPostfix );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::CutEnvelopedSubstr( const CUtf32String& envelopPrefix  ,
                                       const CUtf32String& envelopPostfix ,
                                       UInt32 envelopedStrIndex           ) const
{GUCEF_TRACE;

    return m_str.CutEnvelopedSubstr( envelopPrefix, envelopPostfix, envelopedStrIndex );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::ReplaceSubStr( UInt32 startIndex                 ,
                                  UInt32 length                     ,
                                  const CUtf32String& newSubstr     ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubStr( startIndex, length, newSubstr );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrToChar( UInt32 searchCodePoint             ,
                                 bool frontToBack                   ,
                                 bool returnEmptyIfCharNotFound     ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchCodePoint, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrToChar( UInt32 searchCodePoint             ,
                                 UInt32 startIndex                  ,
                                 bool frontToBack                   ,
                                 bool returnEmptyIfCharNotFound     ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchCodePoint, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrToSubstr( const CUtf32String& searchstr ,
                                   bool frontToBack              ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrToSubstr( const CUtf32String& searchstr ,
                                   UInt32 startIndex              ,
                                   bool frontToBack               ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::SubstrToIndex( UInt32 index            ,
                                  bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.SubstrToIndex( index, frontToBack );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::GetCharacterCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::GetCharactersCount( const UInt32* searchCodePoints ,
                                       UInt32 nrOfSearchCodePoints    ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchCodePoints, nrOfSearchCodePoints );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::GetNonMatchCharactersCount( const UInt32* searchCodePoints ,
                                               UInt32 nrOfSearchCodePoints    ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchCodePoints, nrOfSearchCodePoints );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::GetCharacterRepeatCount( UInt32 searchCodePoint ) const
{GUCEF_TRACE;

    return m_str.GetCharacterRepeatCount( searchCodePoint );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf32StringView::FindMaxSubstrEquality( const CUtf32String& searchStr ,
                                          UInt32 startOffset             ,
                                          bool frontToBack               ,
                                          bool isCaseSensitive           ) const
{GUCEF_TRACE;

    return m_str.FindMaxSubstrEquality( searchStr, startOffset, frontToBack, isCaseSensitive );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::CutChars( UInt32 charcount        ,
                             bool frontToBack        ,
                             UInt32 startOffset      ) const
{GUCEF_TRACE;

    return m_str.CutChars( charcount, frontToBack, startOffset );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::RemoveChar( UInt32 codePointToRemove ) const
{GUCEF_TRACE;

    return m_str.RemoveChar( codePointToRemove );
}

/*-------------------------------------------------------------------------*/

bool
CUtf32StringView::HasRepeatingChar( UInt32 codePointToCheck ) const
{GUCEF_TRACE;

    return m_str.HasRepeatingChar( codePointToCheck );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::CompactRepeatingChar( UInt32 codePointToCompact ) const
{GUCEF_TRACE;

    return m_str.CompactRepeatingChar( codePointToCompact );
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringVector
CUtf32StringView::ParseElements( UInt32 separator, bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseElements( separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf32String::StringSet
CUtf32StringView::ParseUniqueElements( UInt32 separator, bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseUniqueElements( separator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::Combine( const CUtf32String::StringVector& elements, UInt32 separator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, separator );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CUtf32StringView::Combine( const CUtf32String::StringSet& elements, UInt32 separator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, separator );
}

/*-------------------------------------------------------------------------*/

CUtf32StringView::operator const CUtf32String&( void ) const
{GUCEF_TRACE;

    return m_str;
}

/*-------------------------------------------------------------------------*/

const CUtf32String&
CUtf32StringView::AsString( void ) const
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

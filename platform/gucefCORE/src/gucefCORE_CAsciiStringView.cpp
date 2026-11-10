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

#ifndef GUCEF_CORE_CASCIISTRING_VIEW_H
#include "gucefCORE_CAsciiStringView.h"
#define GUCEF_CORE_CASCIISTRING_VIEW_H
#endif /* GUCEF_CORE_CASCIISTRING_VIEW_H ? */

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

CAsciiStringView::CAsciiStringView( void )
    : m_str()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::CAsciiStringView( const char* buffer, UInt32 length )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( buffer, length );
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::CAsciiStringView( const CAsciiString& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::CAsciiStringView( const CAsciiStringView& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src.m_str );
}

/*-------------------------------------------------------------------------*/

CAsciiStringView&
CAsciiStringView::operator=( const CAsciiStringView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_str.LinkTo( src.m_str );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::~CAsciiStringView()
{GUCEF_TRACE;

    /* m_str destructor will skip delete because it is linked */
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::Length( void ) const
{GUCEF_TRACE;

    return m_str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::ByteSize( void ) const
{GUCEF_TRACE;

    return m_str.ByteSize();
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_str.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const char*
CAsciiStringView::C_String( void ) const
{GUCEF_TRACE;

    return m_str.C_String();
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::Equals( const CAsciiString& otherStr ,
                           bool caseSensitive          ) const
{GUCEF_TRACE;

    return m_str.Equals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::WildcardEquals( const CAsciiString& strWithWildcards ,
                                   char wildCardToken                   ,
                                   bool caseSensitive                   ,
                                   bool biDirectional                   ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiStringView::HasChar( char searchchar ,
                            bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiStringView::HasSubstr( const CAsciiString& substr ,
                              Int32 startIndex           ,
                              bool frontToBack           ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiStringView::HasSubstr( const CAsciiString& substr ,
                              bool frontToBack           ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::Lowercase( void ) const
{GUCEF_TRACE;

    return m_str.Lowercase();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::Uppercase( void ) const
{GUCEF_TRACE;

    return m_str.Uppercase();
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.Trim( frontToBack );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrFromRange( UInt32 startIndex ,
                                    UInt32 endIndex   ) const
{GUCEF_TRACE;

    return m_str.SubstrFromRange( startIndex, endIndex );
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::IsLinked( void ) const
{GUCEF_TRACE;

    return m_str.IsLinked();
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::IsFormattingValid( void ) const
{GUCEF_TRACE;

    return m_str.IsFormattingValid();
}

/*-------------------------------------------------------------------------*/

std::string
CAsciiStringView::STL_String( void ) const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::operator std::string() const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator==( const CAsciiString& other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator==( const char* other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator==( const int NULLvalue ) const
{GUCEF_TRACE;

    return m_str == NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator==( const char character ) const
{GUCEF_TRACE;

    return m_str == character;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator!=( const CAsciiString& other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator!=( const char* other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator!=( const int NULLvalue ) const
{GUCEF_TRACE;

    return m_str != NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator!=( const char character ) const
{GUCEF_TRACE;

    return m_str != character;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator<( const CAsciiString& other ) const
{GUCEF_TRACE;

    return m_str < other;
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::operator<=( const CAsciiString& other ) const
{GUCEF_TRACE;

    return m_str <= other;
}

/*-------------------------------------------------------------------------*/

char
CAsciiStringView::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::NotEquals( const CAsciiString& otherStr ,
                              bool caseSensitive          ) const
{GUCEF_TRACE;

    return m_str.NotEquals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::WildcardEquals( const CAsciiString::StringSet& strsWithWildcards ,
                                   const Int32 wildCardToken                        ,
                                   const bool caseSensitive                         ,
                                   const bool biDirectional                         ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strsWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CAsciiStringView::HasChar( char searchchar         ,
                            const UInt32 startIndex ,
                            bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::ReplaceChar( char oldchar ,
                                char newchar ) const
{GUCEF_TRACE;

    return m_str.ReplaceChar( oldchar, newchar );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::ReplaceChars( const char* oldchars ,
                                 UInt32 nrOfOldChars  ,
                                 char newchar         ) const
{GUCEF_TRACE;

    return m_str.ReplaceChars( oldchars, nrOfOldChars, newchar );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::ReplaceSubstr( const CAsciiString& substr      ,
                                  const CAsciiString& replacement ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubstr( substr, replacement );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::ReplaceEnvelopingSubstr( const CAsciiString& envelopPrefix     ,
                                            const CAsciiString& envelopPostfix    ,
                                            const CAsciiString& newEnvelopPrefix  ,
                                            const CAsciiString& newEnvelopPostfix ) const
{GUCEF_TRACE;

    return m_str.ReplaceEnvelopingSubstr( envelopPrefix, envelopPostfix, newEnvelopPrefix, newEnvelopPostfix );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                       const CUtf8String& envelopPostfix ,
                                       UInt32 envelopedStrIndex          ) const
{GUCEF_TRACE;

    return m_str.CutEnvelopedSubstr( envelopPrefix, envelopPostfix, envelopedStrIndex );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::ReplaceSubStr( UInt32 startIndex             ,
                                  UInt32 length                 ,
                                  const CAsciiString& newSubstr ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubStr( startIndex, length, newSubstr );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrToChar( char searchchar                ,
                                 bool frontToBack               ,
                                 bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrToChar( char searchchar                ,
                                 UInt32 startIndex              ,
                                 bool frontToBack               ,
                                 bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrToSubstr( const CAsciiString& searchstr ,
                                   bool frontToBack              ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrToSubstr( const CAsciiString& searchstr ,
                                   UInt32 startIndex             ,
                                   bool frontToBack              ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::SubstrToIndex( UInt32 index            ,
                                  bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.SubstrToIndex( index, frontToBack );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::GetCharacterCount( const char searchChar ) const
{GUCEF_TRACE;

    return m_str.GetCharacterCount( searchChar );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::GetCharactersCount( const char* searchChars ,
                                       UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::GetNonMatchCharactersCount( const char* searchChars ,
                                               UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::GetCharacterRepeatCount( const char searchChar ) const
{GUCEF_TRACE;

    return m_str.GetCharacterRepeatCount( searchChar );
}

/*-------------------------------------------------------------------------*/

UInt32
CAsciiStringView::FindMaxSubstrEquality( const CAsciiString& searchStr ,
                                          const UInt32 startOffset      ,
                                          bool frontToBack              ,
                                          bool isCaseSentive            ) const
{GUCEF_TRACE;

    return m_str.FindMaxSubstrEquality( searchStr, startOffset, frontToBack, isCaseSentive );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::CutChars( UInt32 charcount        ,
                             bool frontToBack        ,
                             UInt32 startOffset      ) const
{GUCEF_TRACE;

    return m_str.CutChars( charcount, frontToBack, startOffset );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::RemoveChar( const char charToRemove ) const
{GUCEF_TRACE;

    return m_str.RemoveChar( charToRemove );
}

/*-------------------------------------------------------------------------*/

bool
CAsciiStringView::HasRepeatingChar( const char charToCheck ) const
{GUCEF_TRACE;

    return m_str.HasRepeatingChar( charToCheck );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::CompactRepeatingChar( const char charToCompact ) const
{GUCEF_TRACE;

    return m_str.CompactRepeatingChar( charToCompact );
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringVector
CAsciiStringView::ParseElements( char seperator       ,
                                  bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseElements( seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CAsciiString::StringSet
CAsciiStringView::ParseUniqueElements( char seperator        ,
                                        bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseUniqueElements( seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::Combine( const CAsciiString::StringVector& elements, char seperator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, seperator );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CAsciiStringView::Combine( const CAsciiString::StringSet& elements, char seperator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, seperator );
}

/*-------------------------------------------------------------------------*/

CAsciiStringView::operator const CAsciiString&( void ) const
{GUCEF_TRACE;

    return m_str;
}

/*-------------------------------------------------------------------------*/

const CAsciiString&
CAsciiStringView::AsString( void ) const
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

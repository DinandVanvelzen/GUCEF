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

#ifndef GUCEF_CORE_CUTF8STRING_VIEW_H
#include "gucefCORE_CUtf8StringView.h"
#define GUCEF_CORE_CUTF8STRING_VIEW_H
#endif /* GUCEF_CORE_CUTF8STRING_VIEW_H ? */

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

CUtf8StringView::CUtf8StringView( void )
    : m_str()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::CUtf8StringView( const char* buffer, UInt32 byteSize, UInt32 lengthInCodePoints )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( buffer, byteSize, lengthInCodePoints );
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::CUtf8StringView( const char* nullTerminatedBuffer )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( nullTerminatedBuffer );
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::CUtf8StringView( const CUtf8String& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::CUtf8StringView( const CUtf8StringView& src )
    : m_str()
{GUCEF_TRACE;

    m_str.LinkTo( src.m_str );
}

/*-------------------------------------------------------------------------*/

CUtf8StringView&
CUtf8StringView::operator=( const CUtf8StringView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_str.LinkTo( src.m_str );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::~CUtf8StringView()
{GUCEF_TRACE;

    /* m_str destructor will skip free() because it is linked */
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::Length( void ) const
{GUCEF_TRACE;

    return m_str.Length();
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::ByteSize( void ) const
{GUCEF_TRACE;

    return m_str.ByteSize();
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_str.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const char*
CUtf8StringView::C_String( void ) const
{GUCEF_TRACE;

    return m_str.C_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::Equals( const CUtf8String& otherStr ,
                          bool caseSensitive          ) const
{GUCEF_TRACE;

    return m_str.Equals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasChar( char searchchar ,
                           bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::Lowercase( void ) const
{GUCEF_TRACE;

    return m_str.Lowercase();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::Uppercase( void ) const
{GUCEF_TRACE;

    return m_str.Uppercase();
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::Trim( bool frontToBack ) const
{GUCEF_TRACE;

    return m_str.Trim( frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrFromRange( UInt32 startIndex ,
                                   UInt32 endIndex   ) const
{GUCEF_TRACE;

    return m_str.SubstrFromRange( startIndex, endIndex );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::IsLinked( void ) const
{GUCEF_TRACE;

    return m_str.IsLinked();
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::IsFormattingValid( void ) const
{GUCEF_TRACE;

    return m_str.IsFormattingValid();
}

/*-------------------------------------------------------------------------*/

std::string
CUtf8StringView::STL_String( void ) const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::operator std::string() const
{GUCEF_TRACE;

    return m_str.STL_String();
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator==( const CUtf8String& other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator==( const char* other ) const
{GUCEF_TRACE;

    return m_str == other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator==( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return m_str == NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator==( const UInt32 NULLvalueOrUtf32 ) const
{GUCEF_TRACE;

    return m_str == NULLvalueOrUtf32;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator==( const char character ) const
{GUCEF_TRACE;

    return m_str == character;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator!=( const CUtf8String& other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator!=( const char* other ) const
{GUCEF_TRACE;

    return m_str != other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator!=( const Int32 NULLvalue ) const
{GUCEF_TRACE;

    return m_str != NULLvalue;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator!=( const UInt32 NULLvalueOrUtf32 ) const
{GUCEF_TRACE;

    return m_str != NULLvalueOrUtf32;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator!=( const char character ) const
{GUCEF_TRACE;

    return m_str != character;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator<( const CUtf8String& other ) const
{GUCEF_TRACE;

    return m_str < other;
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::operator<=( const CUtf8String& other ) const
{GUCEF_TRACE;

    return m_str <= other;
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::operator[]( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str[ index ];
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::NotEquals( const CUtf8String& otherStr ,
                             bool caseSensitive          ) const
{GUCEF_TRACE;

    return m_str.NotEquals( otherStr, caseSensitive );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::WildcardEquals( const CUtf8String& strWithWildcards ,
                                  const UInt32 wildCardToken          ,
                                  const bool caseSensitive            ,
                                  const bool biDirectional            ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::WildcardEquals( const CUtf8String::StringSet& strsWithWildcards ,
                                  const UInt32 wildCardToken                       ,
                                  const bool caseSensitive                         ,
                                  const bool biDirectional                         ) const
{GUCEF_TRACE;

    return m_str.WildcardEquals( strsWithWildcards, wildCardToken, caseSensitive, biDirectional );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasChar( UInt32 searchchar ,
                           bool frontToBack  ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasChar( UInt32 searchchar       ,
                           const UInt32 startIndex ,
                           bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasChar( char searchchar         ,
                           const UInt32 startIndex ,
                           bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.HasChar( searchchar, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasSubstr( const CUtf8String& substr ,
                             Int32 startIndex          ,
                             bool frontToBack          ,
                             bool shiftSearch          ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, startIndex, frontToBack, shiftSearch );
}

/*-------------------------------------------------------------------------*/

Int32
CUtf8StringView::HasSubstr( const CUtf8String& substr ,
                             bool frontToBack          ,
                             bool shiftSearch          ) const
{GUCEF_TRACE;

    return m_str.HasSubstr( substr, frontToBack, shiftSearch );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::StartsWith( const CUtf8String& substr ) const
{GUCEF_TRACE;

    return m_str.StartsWith( substr );
}

/*-------------------------------------------------------------------------*/

bool
CUtf8StringView::EndsWith( const CUtf8String& substr ) const
{GUCEF_TRACE;

    return m_str.EndsWith( substr );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::ReplaceChar( Int32 oldchar ,
                               Int32 newchar ) const
{GUCEF_TRACE;

    return m_str.ReplaceChar( oldchar, newchar );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::ReplaceChars( const Int32* oldchars ,
                                UInt32 nrOfOldChars   ,
                                Int32 newchar         ) const
{GUCEF_TRACE;

    return m_str.ReplaceChars( oldchars, nrOfOldChars, newchar );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::ReplaceSubstr( const CUtf8String& substr      ,
                                 const CUtf8String& replacement ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubstr( substr, replacement );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::ReplaceEnvelopingSubstr( const CUtf8String& envelopPrefix     ,
                                           const CUtf8String& envelopPostfix    ,
                                           const CUtf8String& newEnvelopPrefix  ,
                                           const CUtf8String& newEnvelopPostfix ) const
{GUCEF_TRACE;

    return m_str.ReplaceEnvelopingSubstr( envelopPrefix, envelopPostfix, newEnvelopPrefix, newEnvelopPostfix );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::CutEnvelopedSubstr( const CUtf8String& envelopPrefix  ,
                                      const CUtf8String& envelopPostfix ,
                                      UInt32 envelopedStrIndex          ) const
{GUCEF_TRACE;

    return m_str.CutEnvelopedSubstr( envelopPrefix, envelopPostfix, envelopedStrIndex );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::ReplaceSubStr( UInt32 startIndex            ,
                                 UInt32 length                ,
                                 const CUtf8String& newSubstr ) const
{GUCEF_TRACE;

    return m_str.ReplaceSubStr( startIndex, length, newSubstr );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToChar( UInt32 searchchar              ,
                                bool frontToBack               ,
                                bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToChar( char searchchar                ,
                                bool frontToBack               ,
                                bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToChar( UInt32 searchchar              ,
                                UInt32 startIndex              ,
                                bool frontToBack               ,
                                bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToChar( char searchchar                ,
                                UInt32 startIndex              ,
                                bool frontToBack               ,
                                bool returnEmptyIfCharNotFound ) const
{GUCEF_TRACE;

    return m_str.SubstrToChar( searchchar, startIndex, frontToBack, returnEmptyIfCharNotFound );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToSubstr( const CUtf8String& searchstr ,
                                  bool frontToBack             ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToSubstr( const CUtf8String& searchstr ,
                                  UInt32 startIndex            ,
                                  bool frontToBack             ) const
{GUCEF_TRACE;

    return m_str.SubstrToSubstr( searchstr, startIndex, frontToBack );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::SubstrToIndex( UInt32 index            ,
                                 bool frontToBack        ) const
{GUCEF_TRACE;

    return m_str.SubstrToIndex( index, frontToBack );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::CodepointAtIndex( const UInt32 index ) const
{GUCEF_TRACE;

    return m_str.CodepointAtIndex( index );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetCharacterCount( const Int32 searchChar ) const
{GUCEF_TRACE;

    return m_str.GetCharacterCount( searchChar );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetCharactersCount( const Int32* searchChars ,
                                      UInt32 nrOfSearchChars   ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetCharactersCount( const char* searchChars ,
                                      UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetNonMatchCharactersCount( const CUtf8String& searchChars ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetNonMatchCharactersCount( const Int32* searchChars ,
                                              UInt32 nrOfSearchChars   ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetNonMatchCharactersCount( const char* searchChars ,
                                              UInt32 nrOfSearchChars  ) const
{GUCEF_TRACE;

    return m_str.GetNonMatchCharactersCount( searchChars, nrOfSearchChars );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::GetCharacterRepeatCount( const Int32 searchChar ) const
{GUCEF_TRACE;

    return m_str.GetCharacterRepeatCount( searchChar );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::FindMaxSubstrEquality( const CUtf8String& searchStr ,
                                         const UInt32 startOffset     ,
                                         bool frontToBack             ,
                                         bool isCaseSentive           ) const
{GUCEF_TRACE;

    return m_str.FindMaxSubstrEquality( searchStr, startOffset, frontToBack, isCaseSentive );
}

/*-------------------------------------------------------------------------*/

UInt32
CUtf8StringView::FindMaxSegmentEquality( const CUtf8String& otherStr   ,
                                          Int32 segmentDividerCodePoint ,
                                          bool startFront               ,
                                          bool isCaseSentive            ) const
{GUCEF_TRACE;

    return m_str.FindMaxSegmentEquality( otherStr, segmentDividerCodePoint, startFront, isCaseSentive );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::CutChars( UInt32 charcount        ,
                            bool frontToBack        ,
                            UInt32 startOffset      ) const
{GUCEF_TRACE;

    return m_str.CutChars( charcount, frontToBack, startOffset );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::RemoveChar( const Int32 charToRemove ) const
{GUCEF_TRACE;

    return m_str.RemoveChar( charToRemove );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::CompactRepeatingChar( const Int32 charToCompact ) const
{GUCEF_TRACE;

    return m_str.CompactRepeatingChar( charToCompact );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringVector
CUtf8StringView::ParseElements( UInt32 seperator      ,
                                 bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseElements( seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String::StringSet
CUtf8StringView::ParseUniqueElements( UInt32 seperator      ,
                                       bool addEmptyElements ) const
{GUCEF_TRACE;

    return m_str.ParseUniqueElements( seperator, addEmptyElements );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::Combine( const CUtf8String::StringVector& elements, Int32 seperator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, seperator );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CUtf8StringView::Combine( const CUtf8String::StringSet& elements, Int32 seperator ) const
{GUCEF_TRACE;

    return m_str.Combine( elements, seperator );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CUtf8StringView::ForceToAscii( char asciiReplacement ) const
{GUCEF_TRACE;

    return m_str.ForceToAscii( asciiReplacement );
}

/*-------------------------------------------------------------------------*/

CUtf8StringView::operator const CUtf8String&( void ) const
{GUCEF_TRACE;

    return m_str;
}

/*-------------------------------------------------------------------------*/

const CUtf8String&
CUtf8StringView::AsString( void ) const
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

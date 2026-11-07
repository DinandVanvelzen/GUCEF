/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_COM_CIMAPVALUEPARSER_H
#define GUCEF_COM_CIMAPVALUEPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPVALUE_H
#include "gucefCOM_CImapValue.h"
#define GUCEF_COM_CIMAPVALUE_H
#endif /* GUCEF_COM_CIMAPVALUE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads values from the bytes of an IMAP response, literals included (a literal is the text {n}, a CR LF and
 *  n bytes). It checks everything before it allocates anything: the size of a literal against what is left, the
 *  nesting of lists, the number of values. Nothing that comes from a server can make it read outside of the
 *  data or take memory beyond the size of the data.
 */
class GUCEF_COM_PUBLIC_CPP CImapValueParser
{
    public:

    static const UInt32 DefaultMaxDepth = 32;
    static const UInt32 DefaultMaxValues = 1000000;

    CImapValueParser( const void* data, UInt32 size, UInt32 maxDepth = DefaultMaxDepth, UInt32 maxValues = DefaultMaxValues );

    /** Skips spaces, then reads one value. False when the data is not a value or a limit is exceeded; 'value' is then cleared */
    bool ParseValue( CImapValue& value );

    /** Reads values up to the end of the data (or up to a closing parenthesis when 'untilParenthesis') into a list */
    bool ParseValuesUntilEnd( CImapValue& list );

    void SkipSpaces( void );

    bool AtEnd( void ) const;

    UInt32 GetPosition( void ) const;

    /** The next byte, 0 at the end */
    char Peek( void ) const;

    /** Moves over the byte */
    void Advance( void );

    /** Reads the characters up to a space, a parenthesis or the end: for the words in front of the values */
    CORE::CString ReadWord( void );

    /** The rest of the data, from the position */
    CORE::CString ReadRest( void );

    const CORE::CString& GetError( void ) const;

    private:

    bool ParseValueAtDepth( CImapValue& value, UInt32 depth );
    bool Fail( const char* reason );

    private:

    const char* m_data;
    UInt32 m_size;
    UInt32 m_position;
    UInt32 m_maxDepth;
    UInt32 m_maxValues;
    UInt32 m_valueCount;
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPVALUEPARSER_H ? */

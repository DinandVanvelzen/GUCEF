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

#ifndef GUCEF_COM_CIMAPVALUE_H
#define GUCEF_COM_CIMAPVALUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

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
 *  A value of an IMAP response (RFC 3501 section 9): NIL, an atom, a number, a string (quoted or literal) or a
 *  parenthesized list of values. The tree that the tokenizer makes of the data of a response.
 *
 *  A string keeps its bytes as they came: a literal can be binary (the body of a message). GetString() reads the
 *  bytes as text (made valid UTF-8). An atom with a section in brackets, like BODY[HEADER.FIELDS (FROM TO)]<0>,
 *  is one atom.
 */
class GUCEF_COM_PUBLIC_CPP CImapValue
{
    public:

    enum EType
    {
        TYPE_NIL = 0,
        TYPE_ATOM,
        TYPE_NUMBER,
        TYPE_STRING,
        TYPE_LIST
    };

    CImapValue( void );

    CImapValue( const CImapValue& src );

    ~CImapValue();

    CImapValue& operator=( const CImapValue& src );

    EType GetType( void ) const;

    bool IsNil( void ) const;
    bool IsAtom( void ) const;
    bool IsNumber( void ) const;
    bool IsString( void ) const;
    bool IsList( void ) const;

    /** An atom or a string: text, or an atom that says NIL is not text */
    bool IsText( void ) const;

    /** The text of an atom or a number, the bytes of a string as text (valid UTF-8), empty for the others */
    CORE::CString GetString( void ) const;

    /** The bytes of a string; for the other types the bytes of their text */
    const CORE::CDynamicBuffer& GetBytes( void ) const;

    UInt64 GetNumber( void ) const;

    bool AtomEquals( const char* text ) const;

    UInt32 GetListSize( void ) const;

    /** The element of a list, a NIL value for an index that is not there */
    const CImapValue& GetElement( UInt32 index ) const;

    void SetNil( void );
    void SetAtom( const CORE::CString& text );
    void SetNumber( UInt64 number );
    void SetString( const void* data, UInt32 size );
    void SetList( void );

    /** Adds to a list and returns the new element */
    CImapValue* AddElement( void );

    void Clear( void );

    private:

    void CopyFrom( const CImapValue& src );

    private:

    EType m_type;
    CORE::CString m_text;
    CORE::CDynamicBuffer m_bytes;
    UInt64 m_number;
    GUCEF::vector< CImapValue* > m_elements;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPVALUE_H ? */

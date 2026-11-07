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

#ifndef GUCEF_COM_CMAILHEADERS_H
#define GUCEF_COM_CMAILHEADERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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
 *  The header fields of a message or of one part of it, in the order they have. The values are the wire values:
 *  unfolded (no line breaks) but not decoded, so a value can still hold RFC 2047 encoded words.
 *  GetDecoded() gives the text.
 *
 *  Nothing that could break out of a header gets in: a field name has to be a valid name, a value cannot hold CR,
 *  LF or NUL. AddText() takes ordinary text and stores it in a form that is safe on the wire (RFC 2047).
 */
class GUCEF_COM_PUBLIC_CPP CMailHeaders
{
    public:

    struct SField
    {
        CORE::CString name;     /**< as written */
        CORE::CString value;    /**< raw wire value, unfolded */
    };
    typedef struct SField TField;
    typedef GUCEF::vector< TField > TFieldVector;

    CMailHeaders( void );

    CMailHeaders( const CMailHeaders& src );

    ~CMailHeaders();

    CMailHeaders& operator=( const CMailHeaders& src );

    /** Adds a field with a value that is already in wire form. @return false (nothing added) for an invalid name or a value with CR, LF or NUL */
    bool Add( const CORE::CString& name, const CORE::CString& wireValue );

    /** Adds a field with ordinary UTF-8 text, encoded as RFC 2047 words when it has to be. @return false for an invalid name */
    bool AddText( const CORE::CString& name, const CORE::CString& text );

    /** Replaces all fields of the name by one. @return false when Add() would refuse it, the existing fields are then left alone */
    bool Set( const CORE::CString& name, const CORE::CString& wireValue );

    /** Removes all fields of the name. @return the number removed */
    UInt32 Remove( const CORE::CString& name );

    bool Has( const CORE::CString& name ) const;

    /** The raw value of the first field of the name, empty when there is none */
    const CORE::CString& Get( const CORE::CString& name ) const;

    /** The text of the first field of the name, encoded words decoded */
    CORE::CString GetDecoded( const CORE::CString& name ) const;

    /** All values of the fields of the name, in order */
    void GetAll( const CORE::CString& name, CORE::CString::StringVector& values ) const;

    UInt32 GetCount( void ) const;

    const TField& GetField( UInt32 index ) const;

    const TFieldVector& GetFields( void ) const;

    void Clear( void );

    /** Appends all fields, folded, each ended with CR LF. @return false when a field cannot be written (nothing is appended then) */
    bool AppendTo( CORE::CDynamicBuffer& out ) const;

    private:

    TFieldVector m_fields;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILHEADERS_H ? */

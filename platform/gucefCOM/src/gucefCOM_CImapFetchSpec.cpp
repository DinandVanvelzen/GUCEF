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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/


#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCOM_CImapFetchSpec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapFetchSpec::CImapFetchSpec( void )
    : uid( true )
    , flags( false )
    , internalDate( false )
    , size( false )
    , envelope( false )
    , bodyStructure( false )
    , sections()
    , markSeen( false )
    , partialOffset( 0 )
    , partialLength( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapFetchSpec::Build( CORE::CString& items ) const
{GUCEF_TRACE;

    CORE::CString text;
    #define GUCEF_COM_ADD_ITEM( condition, name ) if ( condition ) { if ( !text.IsNULLOrEmpty() ) text += " "; text += name; }
    GUCEF_COM_ADD_ITEM( uid, "UID" )
    GUCEF_COM_ADD_ITEM( flags, "FLAGS" )
    GUCEF_COM_ADD_ITEM( internalDate, "INTERNALDATE" )
    GUCEF_COM_ADD_ITEM( size, "RFC822.SIZE" )
    GUCEF_COM_ADD_ITEM( envelope, "ENVELOPE" )
    GUCEF_COM_ADD_ITEM( bodyStructure, "BODYSTRUCTURE" )
    #undef GUCEF_COM_ADD_ITEM

    for ( size_t i=0; i<sections.size(); ++i )
    {
        // a section is letters, digits, dots, spaces, parentheses and header names: no brackets, quotes or line breaks
        const char* s = sections[ i ].C_String();
        for ( UInt32 k=0; k<ByteLength( sections[ i ] ); ++k )
        {
            const char c = s[ k ];
            const bool ok = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '.' == c || ' ' == c || '(' == c || ')' == c || '-' == c || '_' == c;
            if ( !ok )
                return false;
        }
        if ( !text.IsNULLOrEmpty() )
            text += " ";
        text += markSeen ? "BODY[" : "BODY.PEEK[";
        text += sections[ i ] + "]";
        if ( 0 == i && partialLength > 0 )
            text += "<" + CORE::UInt32ToString( partialOffset ) + "." + CORE::UInt32ToString( partialLength ) + ">";
    }
    if ( text.IsNULLOrEmpty() )
        return false;
    items = text;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

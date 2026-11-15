/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBPAGESCRIPTHELPERS_H
#include "gucefASSETSTORE_CWebPageScriptHelpers.h"
#define GUCEF_ASSETSTORE_CWEBPAGESCRIPTHELPERS_H
#endif /* GUCEF_ASSETSTORE_CWEBPAGESCRIPTHELPERS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Call( const char* function, const CORE::CString& argument )
{
    return CORE::CString( "(" ) + function + ")(" + CWebPageScriptHelpers::JsStringLiteral( argument ) + ")";
}

const char* HexDigits = "0123456789abcdef";

}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::JsStringLiteral( const CORE::CString& text )
{GUCEF_TRACE;

    // built as bytes: the text is UTF-8 and only ASCII is replaced, the other bytes are copied as they are
    GUCEF::vector< char > out;
    const UInt32 byteCount = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    out.reserve( byteCount + 16 );
    out.push_back( '"' );
    const unsigned char* c = (const unsigned char*) text.C_String();
    for ( UInt32 i=0; i<byteCount; ++i )
    {
        const unsigned char b = c[ i ];
        if ( '"' == b || '\\' == b )
        {
            out.push_back( '\\' );
            out.push_back( (char) b );
        }
        else
        if ( b < 0x20 || 0x7F == b )
        {
            const char escape[ 6 ] = { '\\', 'u', '0', '0', HexDigits[ ( b >> 4 ) & 0x0F ], HexDigits[ b & 0x0F ] };
            out.insert( out.end(), escape, escape + 6 );
        }
        else
        if ( 0xE2 == b && i + 2 < byteCount && 0x80 == c[ i + 1 ] && ( 0xA8 == c[ i + 2 ] || 0xA9 == c[ i + 2 ] ) )
        {
            // the line and paragraph separators end a line in older JavaScript, also inside a literal
            const char escape[ 6 ] = { '\\', 'u', '2', '0', '2', 0xA8 == c[ i + 2 ] ? '8' : '9' };
            out.insert( out.end(), escape, escape + 6 );
            i += 2;
        }
        else
            out.push_back( (char) b );
    }
    out.push_back( '"' );

    CORE::CString literal;
    literal.Set( &out[ 0 ], (UInt32) out.size() );
    return literal;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::ElementStateExpression( const CORE::CString& selector )
{GUCEF_TRACE;

    return Call( "function(s){try{var e=document.querySelector(s);if(e===null){return 0;}"
                 "var shown=e.getClientRects().length>0&&window.getComputedStyle(e).visibility!=='hidden';"
                 "return (shown&&!e.disabled)?2:1;}catch(x){return -1;}}", selector );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::FocusAndClearExpression( const CORE::CString& selector )
{GUCEF_TRACE;

    return Call( "function(s){try{var e=document.querySelector(s);if(e===null){return false;}"
                 "e.scrollIntoView({block:'center'});e.focus();"
                 "var d=Object.getOwnPropertyDescriptor(Object.getPrototypeOf(e),'value');"
                 "if(d&&d.set){d.set.call(e,'');}else{e.value='';}"
                 "e.dispatchEvent(new Event('input',{bubbles:true}));"
                 "return document.activeElement===e;}catch(x){return false;}}", selector );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::ValueLengthExpression( const CORE::CString& selector )
{GUCEF_TRACE;

    return Call( "function(s){try{var e=document.querySelector(s);if(e===null||typeof e.value!=='string'){return -1;}"
                 "return e.value.length;}catch(x){return -1;}}", selector );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::ClickExpression( const CORE::CString& selector )
{GUCEF_TRACE;

    return Call( "function(s){try{var e=document.querySelector(s);if(e===null){return false;}"
                 "e.scrollIntoView({block:'center'});e.click();return true;}catch(x){return false;}}", selector );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::SubmitExpression( const CORE::CString& selector )
{GUCEF_TRACE;

    return Call( "function(s){try{var e=document.querySelector(s);if(e===null){return false;}"
                 "var f=(e.tagName==='FORM')?e:e.form;if(!f){return false;}"
                 "if(typeof f.requestSubmit==='function'){f.requestSubmit();}else{f.submit();}"
                 "return true;}catch(x){return false;}}", selector );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::ContainsTextExpression( const CORE::CString& text )
{GUCEF_TRACE;

    return Call( "function(t){try{return document.body!==null&&document.body.innerText.indexOf(t)>=0;}catch(x){return false;}}", text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebPageScriptHelpers::LocationExpression( void )
{GUCEF_TRACE;

    return "(function(){try{return String(window.location.href);}catch(x){return '';}})()";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

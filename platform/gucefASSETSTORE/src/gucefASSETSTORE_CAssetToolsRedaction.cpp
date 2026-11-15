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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "gucefASSETSTORE_CAssetToolsRedaction.h"

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

CAssetToolsRedaction::CAssetToolsRedaction( void )
    : m_lock()
    , m_redactor( CORE::REDACT_URL_USERINFO | CORE::REDACT_URL_QUERY_LISTED | CORE::REDACT_URL_FRAGMENT |
                  CORE::REDACT_NAME_WORDS | CORE::REDACT_TEXT_URLS | CORE::REDACT_TEXT_AUTH_SCHEMES |
                  CORE::REDACT_TEXT_KEY_VALUES )
    , m_secretCount( 0 )
{GUCEF_TRACE;

    m_redactor.SetMaxLength( 0 );
    m_redactor.AddSensitiveName( "pwd" );
    m_redactor.AddSensitiveName( "password" );
    m_redactor.AddSensitiveName( "*token*" );
    m_redactor.AddSensitiveName( "*secret*" );
    m_redactor.AddSensitiveName( "*cookie*" );
    m_redactor.AddSensitiveName( "authorization" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsRedaction::AddSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    if ( secret.IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    if ( !m_redactor.AddSecret( secret ) )
        return false;
    ++m_secretCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsRedaction::RemoveSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_redactor.RemoveSecret( secret ) )
        return false;
    if ( m_secretCount > 0 )
        --m_secretCount;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetToolsRedaction::GetSecretCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_secretCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolsRedaction::Sanitize( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString clean;
    const UInt32 length = text.Length();
    for ( UInt32 i=0; i<length; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( ( c < 0x20 && '\n' != c && '\t' != c ) || 0x7F == c )
            continue;
        // U+200E, U+200F, U+202A to U+202E, U+2066 to U+2069: marks and overrides of the text direction
        if ( 0xE2 == c && i + 2 < length )
        {
            const unsigned char c1 = (unsigned char) text[ i+1 ];
            const unsigned char c2 = (unsigned char) text[ i+2 ];
            if ( ( 0x80 == c1 && ( 0x8E == c2 || 0x8F == c2 || ( c2 >= 0xAA && c2 <= 0xAE ) ) ) ||
                 ( 0x81 == c1 && c2 >= 0xA6 && c2 <= 0xA9 ) )
            {
                i += 2;
                continue;
            }
        }
        clean += (char) c;
    }
    return clean;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolsRedaction::ForResult( const CORE::CString& text ) const
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return text;
    MT::CScopeMutex lock( m_lock );
    return Sanitize( m_redactor.RedactText( text ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetToolsRedaction::RedactTreeLocked( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    const CORE::CVariant& value = node.GetValue();
    if ( value.IsString() )
    {
        const CORE::CString text = value.AsString();
        const CORE::CString redacted = Sanitize( m_redactor.RedactText( text ) );
        if ( redacted != text )
            node.SetValue( CORE::CVariant( redacted ) );
    }
    CORE::CString::StringVector names;
    CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin();
    while ( a != node.AttributeEnd() )
    {
        if ( (*a).second.IsString() )
            names.push_back( (*a).first );
        ++a;
    }
    for ( size_t n=0; n<names.size(); ++n )
    {
        const CORE::CString text = node.GetAttributeValue( names[ n ] ).AsString();
        const CORE::CString redacted = Sanitize( m_redactor.RedactText( text ) );
        if ( redacted != text )
            node.SetAttribute( names[ n ], redacted );
    }
    CORE::CDataNode::iterator c = node.Begin();
    while ( c != node.End() )
    {
        RedactTreeLocked( *(*c) );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetToolsRedaction::RedactTree( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RedactTreeLocked( node );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

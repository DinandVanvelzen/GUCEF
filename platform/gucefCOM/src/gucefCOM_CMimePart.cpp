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

#include "gucefCOM_CMimePart.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMimePart::CMimePart( void )
    : headers()
    , contentType( "text/plain" )
    , contentTypeParams()
    , transferEncoding()
    , disposition()
    , dispositionParams()
    , contentId()
    , body()
    , children()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMimePart::CMimePart( const CMimePart& src )
    : headers()
    , contentType()
    , contentTypeParams()
    , transferEncoding()
    , disposition()
    , dispositionParams()
    , contentId()
    , body()
    , children()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CMimePart::~CMimePart()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CMimePart&
CMimePart::operator=( const CMimePart& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Clear();
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMimePart::CopyFrom( const CMimePart& src )
{GUCEF_TRACE;

    headers = src.headers;
    contentType = src.contentType;
    contentTypeParams = src.contentTypeParams;
    transferEncoding = src.transferEncoding;
    disposition = src.disposition;
    dispositionParams = src.dispositionParams;
    contentId = src.contentId;
    body = src.body;
    for ( size_t i=0; i<src.children.size(); ++i )
        children.push_back( GUCEF_NEW CMimePart( *src.children[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CMimePart::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<children.size(); ++i )
        GUCEF_DELETE children[ i ];
    children.clear();
    body.Clear();
}

/*-------------------------------------------------------------------------*/

void
CMimePart::ApplyHeaders( void )
{GUCEF_TRACE;

    CORE::CString value;
    const CORE::CString& typeField = headers.Get( "Content-Type" );
    if ( typeField.IsNULLOrEmpty() )
    {
        contentType = "text/plain";
        contentTypeParams.clear();
    }
    else
    {
        CMimeCodec::ParseValueWithParameters( typeField, value, contentTypeParams );
        contentType = value.Lowercase();
        // a type must be type/subtype, anything else is treated as the default
        if ( contentType.HasChar( '/' ) <= 0 )
            contentType = "text/plain";
    }

    transferEncoding = headers.Get( "Content-Transfer-Encoding" ).Lowercase();
    Int32 semicolon = transferEncoding.HasChar( ';' );
    if ( semicolon >= 0 )
        transferEncoding = transferEncoding.SubstrToIndex( (UInt32) semicolon, true );
    transferEncoding = transferEncoding.Trim( true ).Trim( false );

    const CORE::CString& dispositionField = headers.Get( "Content-Disposition" );
    disposition.Clear();
    dispositionParams.clear();
    if ( !dispositionField.IsNULLOrEmpty() )
    {
        CMimeCodec::ParseValueWithParameters( dispositionField, value, dispositionParams );
        disposition = value.Lowercase();
    }

    contentId = headers.Get( "Content-ID" );
    contentId = contentId.Trim( true ).Trim( false );
    if ( contentId.Length() >= 2 && '<' == contentId.C_String()[ 0 ] )
    {
        Int32 close = contentId.HasChar( '>' );
        if ( close > 1 )
            contentId = contentId.SubstrFromRange( 1, (UInt32) close );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMimePart::IsMultipart( void ) const
{GUCEF_TRACE;

    return contentType.StartsWith( "multipart/" );
}

/*-------------------------------------------------------------------------*/

bool
CMimePart::IsText( void ) const
{GUCEF_TRACE;

    return contentType.StartsWith( "text/" );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMimePart::GetCharset( void ) const
{GUCEF_TRACE;

    return CMimeCodec::FindParameter( contentTypeParams, "charset" );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMimePart::GetBoundary( void ) const
{GUCEF_TRACE;

    return CMimeCodec::FindParameter( contentTypeParams, "boundary" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMimePart::GetFileName( void ) const
{GUCEF_TRACE;

    const CORE::CString& name = CMimeCodec::FindParameter( dispositionParams, "filename" );
    if ( !name.IsNULLOrEmpty() )
        return name;
    return CMimeCodec::FindParameter( contentTypeParams, "name" );
}

/*-------------------------------------------------------------------------*/

CMimePart*
CMimePart::AddChild( void )
{GUCEF_TRACE;

    CMimePart* child = GUCEF_NEW CMimePart();
    children.push_back( child );
    return child;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

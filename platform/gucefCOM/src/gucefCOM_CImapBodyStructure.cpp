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


#include "gucefCOM_CImapBodyStructure.h"

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

void
ReadParams( const CImapValue& list, CMimeCodec::TParamVector& params )
{
    params.clear();
    if ( !list.IsList() )
        return;
    for ( UInt32 i=0; i+1<list.GetListSize() && params.size() < 100; i+=2 )
    {
        CMimeCodec::TParam param;
        param.name = list.GetElement( i ).GetString().Lowercase();
        param.value = list.GetElement( i + 1 ).GetString();
        params.push_back( param );
    }
}
/*-------------------------------------------------------------------------*/

CORE::CString
LowerText( const CImapValue& value )
{
    return value.GetString().Lowercase();
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapBodyStructure::CImapBodyStructure( void )
    : isMultipart( false )
    , type()
    , subtype()
    , params()
    , contentId()
    , description()
    , encoding()
    , size( 0 )
    , lines( 0 )
    , disposition()
    , dispositionParams()
    , children()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapBodyStructure::CImapBodyStructure( const CImapBodyStructure& src )
    : isMultipart( false )
    , type()
    , subtype()
    , params()
    , contentId()
    , description()
    , encoding()
    , size( 0 )
    , lines( 0 )
    , disposition()
    , dispositionParams()
    , children()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CImapBodyStructure::~CImapBodyStructure()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CImapBodyStructure&
CImapBodyStructure::operator=( const CImapBodyStructure& src )
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
CImapBodyStructure::CopyFrom( const CImapBodyStructure& src )
{GUCEF_TRACE;

    isMultipart = src.isMultipart;
    type = src.type;
    subtype = src.subtype;
    params = src.params;
    contentId = src.contentId;
    description = src.description;
    encoding = src.encoding;
    size = src.size;
    lines = src.lines;
    disposition = src.disposition;
    dispositionParams = src.dispositionParams;
    for ( size_t i=0; i<src.children.size(); ++i )
        children.push_back( GUCEF_NEW CImapBodyStructure( *src.children[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CImapBodyStructure::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<children.size(); ++i )
        GUCEF_DELETE children[ i ];
    children.clear();
    isMultipart = false;
    type.Clear();
    subtype.Clear();
    params.clear();
    contentId.Clear();
    description.Clear();
    encoding.Clear();
    size = 0;
    lines = 0;
    disposition.Clear();
    dispositionParams.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapBodyStructure::GetContentType( void ) const
{GUCEF_TRACE;

    return type + "/" + subtype;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapBodyStructure::GetFileName( void ) const
{GUCEF_TRACE;

    const CORE::CString& name = CMimeCodec::FindParameter( dispositionParams, "filename" );
    if ( !name.IsNULLOrEmpty() )
        return CMimeCodec::DecodeHeaderText( name );
    return CMimeCodec::DecodeHeaderText( CMimeCodec::FindParameter( params, "name" ) );
}

/*-------------------------------------------------------------------------*/

bool
CImapBodyStructure::FromValue( const CImapValue& value, CImapBodyStructure& structure )
{GUCEF_TRACE;

    structure.Clear();
    if ( !value.IsList() || 0 == value.GetListSize() )
        return false;

    // a multipart starts with the lists of its parts, then the subtype and the extension data
    if ( value.GetElement( 0 ).IsList() )
    {
        structure.isMultipart = true;
        structure.type = "multipart";
        UInt32 i = 0;
        while ( i < value.GetListSize() && value.GetElement( i ).IsList() )
        {
            CImapBodyStructure* child = GUCEF_NEW CImapBodyStructure();
            structure.children.push_back( child );
            if ( !FromValue( value.GetElement( i ), *child ) )
            {
                structure.Clear();
                return false;
            }
            ++i;
        }
        structure.subtype = LowerText( value.GetElement( i ) );
        ReadParams( value.GetElement( i + 1 ), structure.params );
        const CImapValue& disposition = value.GetElement( i + 2 );
        if ( disposition.IsList() )
        {
            structure.disposition = LowerText( disposition.GetElement( 0 ) );
            ReadParams( disposition.GetElement( 1 ), structure.dispositionParams );
        }
        return true;
    }

    if ( value.GetListSize() < 7 )
        return false;
    structure.type = LowerText( value.GetElement( 0 ) );
    structure.subtype = LowerText( value.GetElement( 1 ) );
    ReadParams( value.GetElement( 2 ), structure.params );
    structure.contentId = value.GetElement( 3 ).GetString();
    structure.description = CMimeCodec::DecodeHeaderText( value.GetElement( 4 ).GetString() );
    structure.encoding = LowerText( value.GetElement( 5 ) );
    structure.size = (UInt32)( value.GetElement( 6 ).GetNumber() > 0xFFFFFFFFull ? 0xFFFFFFFFull : value.GetElement( 6 ).GetNumber() );

    UInt32 next = 7;
    if ( structure.type == "text" )
    {
        structure.lines = (UInt32) value.GetElement( 7 ).GetNumber();
        next = 8;
    }
    else
    if ( structure.type == "message" && structure.subtype == "rfc822" )
    {
        structure.lines = (UInt32) value.GetElement( 9 ).GetNumber();
        next = 10;
    }
    // md5, then the disposition
    const CImapValue& disposition = value.GetElement( next + 1 );
    if ( disposition.IsList() )
    {
        structure.disposition = LowerText( disposition.GetElement( 0 ) );
        ReadParams( disposition.GetElement( 1 ), structure.dispositionParams );
    }
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

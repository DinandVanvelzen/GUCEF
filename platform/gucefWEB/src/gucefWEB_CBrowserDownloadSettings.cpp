/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CBrowserDownloadSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CBrowserDownloadSettings::MinFileNameLength;

/*-------------------------------------------------------------------------*/

CBrowserDownloadSettings::CBrowserDownloadSettings( void )
    : CORE::CIConfigurable()
    , maxBytes( 256 * 1024 * 1024 )
    , minBytes( 1 )
    , allowedExtensions()
    , allowedMimeTypes()
    , allowHtml( false )
    , maxFileNameLength( 100 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBrowserDownloadSettings::CBrowserDownloadSettings( const CBrowserDownloadSettings& src )
    : CORE::CIConfigurable( src )
    , maxBytes( src.maxBytes )
    , minBytes( src.minBytes )
    , allowedExtensions( src.allowedExtensions )
    , allowedMimeTypes( src.allowedMimeTypes )
    , allowHtml( src.allowHtml )
    , maxFileNameLength( src.maxFileNameLength )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBrowserDownloadSettings::~CBrowserDownloadSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBrowserDownloadSettings&
CBrowserDownloadSettings::operator=( const CBrowserDownloadSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        maxBytes = src.maxBytes;
        minBytes = src.minBytes;
        allowedExtensions = src.allowedExtensions;
        allowedMimeTypes = src.allowedMimeTypes;
        allowHtml = src.allowHtml;
        maxFileNameLength = src.maxFileNameLength;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "maxBytes", maxBytes );
    cfg.SetAttribute( "minBytes", minBytes );
    cfg.SetValuesOfChildByName( "allowedExtensions", allowedExtensions );
    cfg.SetValuesOfChildByName( "allowedMimeTypes", allowedMimeTypes );
    cfg.SetAttribute( "allowHtml", allowHtml );
    cfg.SetAttribute( "maxFileNameLength", maxFileNameLength );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBrowserDownloadSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CBrowserDownloadSettings loaded( *this );
    loaded.maxBytes = cfg.GetAttributeValueOrChildValueByName( "maxBytes" ).AsUInt64( maxBytes, true );
    loaded.minBytes = cfg.GetAttributeValueOrChildValueByName( "minBytes" ).AsUInt64( minBytes, true );
    CORE::CString::StringVector list;
    if ( cfg.GetStringListByName( "allowedExtensions", list ) )
    {
        loaded.allowedExtensions.clear();
        for ( size_t i=0; i<list.size(); ++i )
        {
            CORE::CString extension = list[ i ].Trim( true ).Trim( false ).Lowercase();
            if ( !extension.IsNULLOrEmpty() && extension.C_String()[ 0 ] == '.' )
                extension = extension.CutChars( 1, true );
            if ( !extension.IsNULLOrEmpty() )
                loaded.allowedExtensions.push_back( extension );
        }
    }
    if ( cfg.GetStringListByName( "allowedMimeTypes", list ) )
        loaded.allowedMimeTypes = list;
    loaded.allowHtml = cfg.GetAttributeValueOrChildValueByName( "allowHtml" ).AsBool( allowHtml, true );
    loaded.maxFileNameLength = cfg.GetAttributeValueOrChildValueByName( "maxFileNameLength" ).AsUInt32( maxFileNameLength, true );

    if ( loaded.maxFileNameLength < MinFileNameLength )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadSettings: maxFileNameLength " << loaded.maxFileNameLength << " is below the minimum of " << MinFileNameLength );
        return false;
    }
    if ( 0 != loaded.maxBytes && loaded.minBytes > loaded.maxBytes )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BrowserDownloadSettings: minBytes " << loaded.minBytes << " is above maxBytes " << loaded.maxBytes );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBrowserDownloadSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CBrowserDownloadSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

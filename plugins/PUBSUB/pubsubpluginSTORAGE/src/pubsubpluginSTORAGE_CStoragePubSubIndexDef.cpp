/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality
 *                       providing a storage interface
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "pubsubpluginSTORAGE_CStoragePubSubIndexDef.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStoragePubSubIndexDef::CStoragePubSubIndexDef( void )
    : keySource( KEY_SOURCE_META_DATA )
    , keyName()
    , indexType( INDEX_TYPE_SORTED_UINT64 )
    , samplingInterval( 1 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubIndexDef::CStoragePubSubIndexDef( const CStoragePubSubIndexDef& src )
    : keySource( src.keySource )
    , keyName( src.keyName )
    , indexType( src.indexType )
    , samplingInterval( src.samplingInterval )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CStoragePubSubIndexDef&
CStoragePubSubIndexDef::operator=( const CStoragePubSubIndexDef& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        keySource        = src.keySource;
        keyName          = src.keyName;
        indexType        = src.indexType;
        samplingInterval = src.samplingInterval;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexDef::operator==( const CStoragePubSubIndexDef& other ) const
{GUCEF_TRACE;

    return keySource        == other.keySource        &&
           keyName          == other.keyName          &&
           indexType        == other.indexType        &&
           samplingInterval == other.samplingInterval;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubIndexDef::GetTypeCode( void ) const
{GUCEF_TRACE;

    switch ( indexType )
    {
        case INDEX_TYPE_SORTED_UINT64: return "su64";
        case INDEX_TYPE_SORTED_INT64:  return "si64";
        case INDEX_TYPE_SORTED_STRING: return "sstr";
        default:                       return "su64";
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt16
CStoragePubSubIndexDef::GetEntrySize( void ) const
{GUCEF_TRACE;

    switch ( indexType )
    {
        case INDEX_TYPE_SORTED_UINT64: return 16;  /* UInt64(8) + fileId UInt32(4) + msgIndex UInt32(4) */
        case INDEX_TYPE_SORTED_INT64:  return 16;
        case INDEX_TYPE_SORTED_STRING: return 0;   /* variable, not yet supported */
        default:                       return 16;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CStoragePubSubIndexDef::GetIndexFilename( void ) const
{GUCEF_TRACE;

    CORE::CString prefix = ( keySource == KEY_SOURCE_META_DATA ) ? "mk" : "k";
    return "idx." + prefix + '.' + keyName + '.' + GetTypeCode() + ".sidx";
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexDef::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString keySourceStr = config.GetAttributeValueOrChildValueByName( "keySource" ).AsString( "mk", true );
    if ( keySourceStr == "mk" )
        keySource = KEY_SOURCE_META_DATA;
    else if ( keySourceStr == "k" )
        keySource = KEY_SOURCE_KV_PAIR;
    else
        keySource = KEY_SOURCE_META_DATA;

    keyName = config.GetAttributeValueOrChildValueByName( "keyName" ).AsString( keyName, true );

    CORE::CString indexTypeStr = config.GetAttributeValueOrChildValueByName( "indexType" ).AsString( "su64", true );
    if ( indexTypeStr == "su64" )
        indexType = INDEX_TYPE_SORTED_UINT64;
    else if ( indexTypeStr == "si64" )
        indexType = INDEX_TYPE_SORTED_INT64;
    else if ( indexTypeStr == "sstr" )
        indexType = INDEX_TYPE_SORTED_STRING;
    else
        indexType = INDEX_TYPE_SORTED_UINT64;

    samplingInterval = config.GetAttributeValueOrChildValueByName( "samplingInterval" ).AsUInt32( samplingInterval, true );
    if ( samplingInterval < 1 )
        samplingInterval = 1;

    return !keyName.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CStoragePubSubIndexDef::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool success = true;

    CORE::CString keySourceStr = ( keySource == KEY_SOURCE_META_DATA ) ? "mk" : "k";
    success = config.SetAttribute( "keySource",        keySourceStr     ) && success;
    success = config.SetAttribute( "keyName",          keyName          ) && success;
    success = config.SetAttribute( "indexType",        GetTypeCode()    ) && success;
    success = config.SetAttribute( "samplingInterval", samplingInterval ) && success;

    return success;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

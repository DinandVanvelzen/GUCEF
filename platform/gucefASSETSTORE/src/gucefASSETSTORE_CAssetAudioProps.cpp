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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#include "gucefASSETSTORE_CAssetAudioProps.h"

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

CAssetAudioProps::CAssetAudioProps( void )
    : CORE::CIDataNodeSerializable()
    , isSet( false )
    , lengthSeconds( 0.0 )
    , channels( 0 )
    , sampleRate( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetAudioProps::~CAssetAudioProps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetAudioProps::Clear( void )
{GUCEF_TRACE;

    *this = CAssetAudioProps();
}

/*-------------------------------------------------------------------------*/

bool
CAssetAudioProps::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "isSet", isSet ) && success;
    success = domRootNode.SetAttribute( "lengthSeconds", CORE::CVariant( lengthSeconds ) ) && success;
    success = domRootNode.SetAttribute( "channels", channels ) && success;
    success = domRootNode.SetAttribute( "sampleRate", sampleRate ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetAudioProps::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetAudioProps defaults;
    isSet = domRootNode.GetAttributeValueOrChildValueByName( "isSet" ).AsBool( defaults.isSet, true );
    lengthSeconds = domRootNode.GetAttributeValueOrChildValueByName( "lengthSeconds" ).AsFloat64( defaults.lengthSeconds, true );
    channels = domRootNode.GetAttributeValueOrChildValueByName( "channels" ).AsInt64( defaults.channels, true );
    sampleRate = domRootNode.GetAttributeValueOrChildValueByName( "sampleRate" ).AsInt64( defaults.sampleRate, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetAudioProps::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetAudioProps( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetAudioProps::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetAudioProps";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

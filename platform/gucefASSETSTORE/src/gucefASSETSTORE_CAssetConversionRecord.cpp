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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetConversionRecord.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetConversionRecord::OutcomeConverted = "converted";
const CORE::CString CAssetConversionRecord::OutcomeRefused = "refused";
const CORE::CString CAssetConversionRecord::OutcomeFailed = "failed";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConversionRecord::CAssetConversionRecord( void )
    : CORE::CIDataNodeSerializable()
    , profile()
    , converters()
    , outcome()
    , reason()
    , reportPath()
    , convertedAtMs( 0 )
    , convertedCount( 0 )
    , approximatedCount( 0 )
    , cachedCount( 0 )
    , skippedCount( 0 )
    , failedCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionRecord::~CAssetConversionRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionRecord::IsSet( void ) const
{GUCEF_TRACE;

    return !outcome.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetConversionRecord::MakeConverterEntry( const CORE::CString& name, UInt32 version )
{GUCEF_TRACE;

    return name + "/" + CORE::ToString( version );
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionRecord::ParseConverterEntry( const CORE::CString& entry, CORE::CString& name, UInt32& version )
{GUCEF_TRACE;

    const Int32 slash = entry.HasChar( '/', false );
    if ( slash <= 0 || (UInt32) slash + 1 >= entry.Length() )
        return false;
    name = entry.SubstrToIndex( (UInt32) slash, true );
    version = CORE::StringToUInt32( entry.CutChars( (UInt32) slash + 1, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "profile", profile );
    success = domRootNode.SetValuesOfChildByName( "converters", converters ) && success;
    success = domRootNode.SetAttribute( "outcome", outcome ) && success;
    success = domRootNode.SetAttribute( "reason", reason ) && success;
    success = domRootNode.SetAttribute( "reportPath", reportPath ) && success;
    success = domRootNode.SetAttribute( "convertedAtMs", convertedAtMs ) && success;
    success = domRootNode.SetAttribute( "convertedCount", convertedCount ) && success;
    success = domRootNode.SetAttribute( "approximatedCount", approximatedCount ) && success;
    success = domRootNode.SetAttribute( "cachedCount", cachedCount ) && success;
    success = domRootNode.SetAttribute( "skippedCount", skippedCount ) && success;
    success = domRootNode.SetAttribute( "failedCount", failedCount ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConversionRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetConversionRecord();
    profile = domRootNode.GetAttributeValueOrChildValueByName( "profile" ).AsString();
    domRootNode.GetValuesOfChildByName( "converters", converters );
    outcome = domRootNode.GetAttributeValueOrChildValueByName( "outcome" ).AsString();
    reason = domRootNode.GetAttributeValueOrChildValueByName( "reason" ).AsString();
    reportPath = domRootNode.GetAttributeValueOrChildValueByName( "reportPath" ).AsString();
    convertedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "convertedAtMs" ).AsInt64( 0, true );
    convertedCount = domRootNode.GetAttributeValueOrChildValueByName( "convertedCount" ).AsUInt32( 0, true );
    approximatedCount = domRootNode.GetAttributeValueOrChildValueByName( "approximatedCount" ).AsUInt32( 0, true );
    cachedCount = domRootNode.GetAttributeValueOrChildValueByName( "cachedCount" ).AsUInt32( 0, true );
    skippedCount = domRootNode.GetAttributeValueOrChildValueByName( "skippedCount" ).AsUInt32( 0, true );
    failedCount = domRootNode.GetAttributeValueOrChildValueByName( "failedCount" ).AsUInt32( 0, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetConversionRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetConversionRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetConversionRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetConversionRecord";
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

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
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */

#include "gucefASSETSTORE_CAssetLicence.h"

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

const CORE::CString CAssetLicence::NoMachineLearning = "noMachineLearning";
const CORE::CString CAssetLicence::AttributionRequired = "attributionRequired";
const CORE::CString CAssetLicence::NoCommercialUse = "noCommercialUse";
const CORE::CString CAssetLicence::NoRedistribution = "noRedistribution";
const CORE::CString CAssetLicence::EditorialOnly = "editorialOnly";

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetLicence::CAssetLicence( void )
    : CORE::CIDataNodeSerializable()
    , licenceId()
    , licenceClass( "unknown" )
    , name()
    , url()
    , text()
    , restrictions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLicence::~CAssetLicence()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetLicence::HasRestriction( const CORE::CString& flag ) const
{GUCEF_TRACE;

    return restrictions.end() != restrictions.find( flag );
}

/*-------------------------------------------------------------------------*/

bool
CAssetLicence::AllowsMachineLearning( void ) const
{GUCEF_TRACE;

    return !HasRestriction( NoMachineLearning );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLicence::MakeLicenceId( void ) const
{GUCEF_TRACE;

    CAssetDigest digest( CAssetDigest::Sha256 );
    const CORE::CString key = name.Lowercase() + "\n" + url + "\n" + text;
    digest.Update( key.C_String(), ByteLengthOf( key ) );
    return "lic-" + digest.FinalHex().SubstrToIndex( 16, true );
}

/*-------------------------------------------------------------------------*/

void
CAssetLicence::Clear( void )
{GUCEF_TRACE;

    *this = CAssetLicence();
}

/*-------------------------------------------------------------------------*/

bool
CAssetLicence::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "licenceId", licenceId ) && success;
    success = domRootNode.SetAttribute( "licenceClass", licenceClass ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "url", url ) && success;
    success = domRootNode.SetAttribute( "text", text ) && success;
    success = domRootNode.SetValuesOfChildByName( "restrictions", restrictions ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLicence::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetLicence defaults;
    licenceId = domRootNode.GetAttributeValueOrChildValueByName( "licenceId" ).AsString( defaults.licenceId );
    licenceClass = domRootNode.GetAttributeValueOrChildValueByName( "licenceClass" ).AsString( defaults.licenceClass );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( defaults.name );
    url = domRootNode.GetAttributeValueOrChildValueByName( "url" ).AsString( defaults.url );
    text = domRootNode.GetAttributeValueOrChildValueByName( "text" ).AsString( defaults.text );
    restrictions.clear();
    domRootNode.GetValuesOfChildByName( "restrictions", restrictions );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLicence::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLicence( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLicence::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLicence::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLicence";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLicence::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLicence";
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

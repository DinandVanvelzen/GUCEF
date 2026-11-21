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

#include "gucefASSETSTORE_CAssetLibrarySyncPlan.h"

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

CAssetLibrarySyncPlan::CAssetLibrarySyncPlan( void )
    : CORE::CIDataNodeSerializable()
    , storeId()
    , mode()
    , listed( 0 )
    , pages( 0 )
    , listingComplete( false )
    , counts()
    , skipped()
    , queuedItems( 0 )
    , queuedDownloads( 0 )
    , estimatedBytes( 0 )
    , sizeUnknownItems( 0 )
    , storageFreeBytes( 0 )
    , storageTotalBytes( 0 )
    , storageReserveBytes( 0 )
    , storageVerdict( "unknown" )
    , items()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncPlan::~CAssetLibrarySyncPlan()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlan::Clear( void )
{GUCEF_TRACE;

    *this = CAssetLibrarySyncPlan();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncPlan::GetCount( const CORE::CString& name ) const
{GUCEF_TRACE;

    TCountMap::const_iterator i = counts.find( name );
    return i == counts.end() ? 0 : (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlan::AddCount( const CORE::CString& name, UInt32 amount )
{GUCEF_TRACE;

    counts[ name ] += amount;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncPlan::AddSkipped( const CORE::CString& reason )
{GUCEF_TRACE;

    skipped[ reason ] += 1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncPlan::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( TCountMap::const_iterator i = counts.begin(); i != counts.end(); ++i )
        if ( (*i).second > 0 )
            text += ( text.IsNULLOrEmpty() ? "" : ", " ) + CORE::ToString( (*i).second ) + " " + (*i).first;
    return text.IsNULLOrEmpty() ? CORE::CString( "nothing listed" ) : text;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncPlan::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "storeId", storeId );
    success = domRootNode.SetAttribute( "mode", mode ) && success;
    success = domRootNode.SetAttribute( "listed", listed ) && success;
    success = domRootNode.SetAttribute( "pages", pages ) && success;
    success = domRootNode.SetAttribute( "listingComplete", listingComplete ) && success;
    success = domRootNode.SetAttribute( "queuedItems", queuedItems ) && success;
    success = domRootNode.SetAttribute( "queuedDownloads", queuedDownloads ) && success;
    success = domRootNode.SetAttribute( "estimatedBytes", estimatedBytes ) && success;
    success = domRootNode.SetAttribute( "sizeUnknownItems", sizeUnknownItems ) && success;
    CORE::CDataNode* countsNode = domRootNode.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    for ( TCountMap::const_iterator i = counts.begin(); i != counts.end() && GUCEF_NULL != countsNode; ++i )
        success = countsNode->SetAttribute( (*i).first, (*i).second ) && success;
    CORE::CDataNode* skippedNode = domRootNode.AddChild( "skipped", GUCEF_DATATYPE_OBJECT );
    for ( TCountMap::const_iterator i = skipped.begin(); i != skipped.end() && GUCEF_NULL != skippedNode; ++i )
        success = skippedNode->SetAttribute( (*i).first, (*i).second ) && success;
    CORE::CDataNode* storageNode = domRootNode.AddChild( "storage", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != storageNode )
    {
        success = storageNode->SetAttribute( "freeBytes", storageFreeBytes ) && success;
        success = storageNode->SetAttribute( "totalBytes", storageTotalBytes ) && success;
        success = storageNode->SetAttribute( "reserveBytes", storageReserveBytes ) && success;
        success = storageNode->SetAttribute( "verdict", storageVerdict ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncPlan::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString();
    mode = domRootNode.GetAttributeValueOrChildValueByName( "mode" ).AsString();
    listed = domRootNode.GetAttributeValueOrChildValueByName( "listed" ).AsUInt32( 0, true );
    pages = domRootNode.GetAttributeValueOrChildValueByName( "pages" ).AsUInt32( 0, true );
    listingComplete = domRootNode.GetAttributeValueOrChildValueByName( "listingComplete" ).AsBool( false, true );
    queuedItems = domRootNode.GetAttributeValueOrChildValueByName( "queuedItems" ).AsUInt32( 0, true );
    queuedDownloads = domRootNode.GetAttributeValueOrChildValueByName( "queuedDownloads" ).AsUInt32( 0, true );
    estimatedBytes = domRootNode.GetAttributeValueOrChildValueByName( "estimatedBytes" ).AsUInt64( 0, true );
    sizeUnknownItems = domRootNode.GetAttributeValueOrChildValueByName( "sizeUnknownItems" ).AsUInt32( 0, true );
    const CORE::CDataNode* countsNode = domRootNode.FindChild( "counts" );
    if ( GUCEF_NULL != countsNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = countsNode->AttributeBegin();
        for ( ; a != countsNode->AttributeEnd(); ++a )
            counts[ (*a).first ] = (*a).second.AsUInt32( 0, true );
    }
    const CORE::CDataNode* skippedNode = domRootNode.FindChild( "skipped" );
    if ( GUCEF_NULL != skippedNode )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = skippedNode->AttributeBegin();
        for ( ; a != skippedNode->AttributeEnd(); ++a )
            skipped[ (*a).first ] = (*a).second.AsUInt32( 0, true );
    }
    const CORE::CDataNode* storageNode = domRootNode.FindChild( "storage" );
    if ( GUCEF_NULL != storageNode )
    {
        storageFreeBytes = storageNode->GetAttributeValueOrChildValueByName( "freeBytes" ).AsUInt64( 0, true );
        storageTotalBytes = storageNode->GetAttributeValueOrChildValueByName( "totalBytes" ).AsUInt64( 0, true );
        storageReserveBytes = storageNode->GetAttributeValueOrChildValueByName( "reserveBytes" ).AsUInt64( 0, true );
        storageVerdict = storageNode->GetAttributeValueOrChildValueByName( "verdict" ).AsString( storageVerdict, true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLibrarySyncPlan::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLibrarySyncPlan( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncPlan::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncPlan";
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

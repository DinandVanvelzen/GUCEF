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

#ifndef GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H
#define GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A page of a list of the asset store service (section 4.1 of the service design): the entries, the token of the next
 *  page (empty on the last page), the total when it is known cheaply (0 otherwise) and the catalogue revision the page
 *  was read at, so a client can tell whether what it cached is current.
 *
 *  TEntry is a CORE::CIDataNodeSerializable value type that can be copied. TTypeName is a class with a static function
 *  Name() that returns the unique cloneable type name of the page type, so CORE::CICloneable::CloneableAs() works for
 *  every page type without run time type information (see gucefASSETSTORE_AssetServicePages.h for the page types).
 *  Serialised as an object with "total", "revision", "nextPageToken" and an array "entries".
 */
template < class TEntry, class TTypeName >
class CTAssetServicePage : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< TEntry > TEntryVector;

    TEntryVector entries;
    CORE::CString nextPageToken;                 /**< empty on the last page */
    UInt64 total;                                /**< all entries of the list, 0 when it is not known */
    UInt64 revision;                             /**< the catalogue revision the page was read at, 0 when not known */

    CTAssetServicePage( void );

    virtual ~CTAssetServicePage() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
CTAssetServicePage< TEntry, TTypeName >::CTAssetServicePage( void )
    : CORE::CIDataNodeSerializable()
    , entries()
    , nextPageToken()
    , total( 0 )
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
CTAssetServicePage< TEntry, TTypeName >::~CTAssetServicePage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
void
CTAssetServicePage< TEntry, TTypeName >::Clear( void )
{GUCEF_TRACE;

    entries.clear();
    nextPageToken.Clear();
    total = 0;
    revision = 0;
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
bool
CTAssetServicePage< TEntry, TTypeName >::Serialize( CORE::CDataNode& domRootNode                        ,
                                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "total", total );
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    success = domRootNode.SetAttribute( "nextPageToken", nextPageToken ) && success;
    CORE::CDataNode* entriesNode = domRootNode.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == entriesNode )
        return false;
    for ( size_t i=0; i<entries.size(); ++i )
        success = entries[ i ].Serialize( *entriesNode->AddChild( "entry", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
bool
CTAssetServicePage< TEntry, TTypeName >::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    total = domRootNode.GetAttributeValueOrChildValueByName( "total" ).AsUInt64( 0, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( 0, true );
    nextPageToken = domRootNode.GetAttributeValueOrChildValueByName( "nextPageToken" ).AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* entriesNode = domRootNode.FindChild( "entries" );
    if ( GUCEF_NULL != entriesNode )
    {
        CORE::CDataNode::const_iterator i = entriesNode->ConstBegin();
        while ( i != entriesNode->ConstEnd() )
        {
            TEntry entry;
            if ( entry.Deserialize( *(*i), settings ) )
                entries.push_back( entry );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
CORE::CICloneable*
CTAssetServicePage< TEntry, TTypeName >::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CTAssetServicePage< TEntry, TTypeName >( *this );
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
const char*
CTAssetServicePage< TEntry, TTypeName >::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
const char*
CTAssetServicePage< TEntry, TTypeName >::CloneableTypeName( void )
{GUCEF_TRACE;

    return TTypeName::Name();
}

/*-------------------------------------------------------------------------*/

template < class TEntry, class TTypeName >
const CORE::CString&
CTAssetServicePage< TEntry, TTypeName >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = TTypeName::Name();
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

#endif /* GUCEF_ASSETSTORE_CTASSETSERVICEPAGE_H ? */

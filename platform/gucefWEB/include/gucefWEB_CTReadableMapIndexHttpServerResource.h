/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#define GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
class CTReadableMapIndexHttpServerResource : public CCodecBasedHTTPServerResource
{
    public:

    typedef GUCEF::map< CollectionKeyType, SomeValueClass > TCollectionMap;

    CTReadableMapIndexHttpServerResource( const CORE::CString& collectionName      ,
                                          const CORE::CString& collectionEntryName ,
                                          const CORE::CString& keyPropertyName     ,
                                          TCollectionMap* collection               ,
                                          MT::CILockable* collectionLock           );

    virtual ~CTReadableMapIndexHttpServerResource();

    virtual bool Serialize( const CString& resourcePath         ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Signals whether the resource is a collection of other resources
     *  Since this implements a collection it is hardcoded to 'true'
     */
    virtual bool IsCollection( void ) const;

    void SetResourceKeyPropertyName( const CString& keyPropertyName );

    const CString& GetResourceKeyPropertyName( void ) const;

    protected:

    TCollectionMap* m_collection;
    MT::CILockable* m_collectionLock;
    CORE::CString m_collectionName;
    CORE::CString m_collectionEntryName;
    CORE::CString m_keyPropertyName;

    private:

    CTReadableMapIndexHttpServerResource( void );
    CTReadableMapIndexHttpServerResource( const CTReadableMapIndexHttpServerResource& src );
    CTReadableMapIndexHttpServerResource& operator=( const CTReadableMapIndexHttpServerResource& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::CTReadableMapIndexHttpServerResource( const CORE::CString& collectionName      ,
                                                                                                                 const CORE::CString& collectionEntryName ,
                                                                                                                 const CORE::CString& keyPropertyName     ,
                                                                                                                 TCollectionMap* collection               ,
                                                                                                                 MT::CILockable* collectionLock           )
    : CCodecBasedHTTPServerResource()
    , m_collection( collection )
    , m_collectionLock( collectionLock )
    , m_collectionName( collectionName )
    , m_collectionEntryName( collectionEntryName )
    , m_keyPropertyName( keyPropertyName )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = false;
    m_allowCreate = false;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::~CTReadableMapIndexHttpServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
bool
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::IsCollection( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
void
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::SetResourceKeyPropertyName( const CString& keyPropertyName )
{GUCEF_TRACE;

    m_keyPropertyName = keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
const CString&
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::GetResourceKeyPropertyName( void ) const
{GUCEF_TRACE;

    return m_keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class SomeValueClass >
bool
CTReadableMapIndexHttpServerResource< CollectionKeyType, SomeValueClass >::Serialize( const CString& resourcePath         ,
                                                                                      CORE::CDataNode& output             ,
                                                                                      const CORE::CString& representation ,
                                                                                      const CORE::CString& params         )
{GUCEF_TRACE;

    output = CORE::CDataNode( m_collectionName, GUCEF_DATATYPE_ARRAY );

    MT::CObjectScopeReadOnlyLock lock( m_collectionLock );

    bool totalSuccess = true;
    typename TCollectionMap::iterator i = m_collection->begin();
    while ( i != m_collection->end() )
    {
        CORE::CDataNode* childNode = output.AddChild( m_collectionEntryName, GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != childNode )
            childNode->SetAttribute( m_keyPropertyName, (*i).first );
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H ? */

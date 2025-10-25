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

#ifndef GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H
#define GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_WEB_CCONFIGURABLEHTTPSERVERRESOURCE_H
#include "gucefWEB_CConfigurableHttpServerResource.h"
#define GUCEF_WEB_CCONFIGURABLEHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCONFIGURABLEHTTPSERVERRESOURCE_H ? */

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

template< typename CollectionKeyType, class ConfigurableDerivedClass >
class CTConfigurableMapHttpServerResource : public CCodecBasedHTTPServerResource
{
    public:

    typedef GUCEF::map< CollectionKeyType, ConfigurableDerivedClass > TConfigurableCollectionMap;

    CTConfigurableMapHttpServerResource( const CORE::CString& collectionName    ,
                                         const CORE::CString& keyPropertyName   ,
                                         TConfigurableCollectionMap* collection ,
                                         MT::CILockable* collectionLock         );

    virtual ~CTConfigurableMapHttpServerResource();

    virtual bool Serialize( const CString& resourcePath         ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CString& params               ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Create a new (contained) resource
     *
     *  @param transactionID The unique identifier of the create operation. If a resource was created earlier using the same transactionId, this operation should not create a new one but return the originally created resource instead.
     *  @param input The deserialized input with a DataNode based representation of the resource to create.
     *  @param representation The representation of the resource to create
     *  @param resourceOutput The created IResource. The representation of this resource shall match the representation of the resource to create.
     *  @param supportedRepresentationsOutput In case the representation is not supported for creation, this returns the list of supported representations.
     *  @return The status of the operation. See TCreateState.
     */
    virtual TCreateState CreateResource( const CString& resourcePath                   ,
                                         const CString& transactionID                  ,
                                         const CORE::CDataNode& input                  ,
                                         const CString& representation                 ,
                                         const CString& params                         ,
                                         THTTPServerResourcePtr& resourceOutput        ,
                                         TStringVector& supportedRepresentationsOutput ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Signals whether the resource is a collection of other resources
     *  Since this implements a collection it is hardcoded to 'true'
     */
    virtual bool IsCollection( const CORE::CString& resourcePath ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetResourceKeyPropertyName( const CString& keyPropertyName );

    const CString& GetResourceKeyPropertyName( void ) const;

    protected:

    TConfigurableCollectionMap* m_collection;
    MT::CILockable* m_collectionLock;
    CORE::CString m_collectionName;
    CORE::CString m_keyPropertyName;

    private:

    CTConfigurableMapHttpServerResource( void );
    CTConfigurableMapHttpServerResource( const CTConfigurableMapHttpServerResource& src );
    CTConfigurableMapHttpServerResource& operator=( const CTConfigurableMapHttpServerResource& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::CTConfigurableMapHttpServerResource( const CORE::CString& collectionName    ,
                                                                                                                         const CORE::CString& keyPropertyName   ,
                                                                                                                         TConfigurableCollectionMap* collection ,
                                                                                                                         MT::CILockable* collectionLock         )
    : CCodecBasedHTTPServerResource()
    , m_collection( collection )
    , m_collectionLock( collectionLock )
    , m_collectionName( collectionName )
    , m_keyPropertyName( keyPropertyName )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = false;
    m_allowCreate = false;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::~CTConfigurableMapHttpServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
bool
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::IsCollection( const CORE::CString& resourcePath ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
void
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::SetResourceKeyPropertyName( const CString& keyPropertyName )
{GUCEF_TRACE;

    m_keyPropertyName = keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
const CString&
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::GetResourceKeyPropertyName( void ) const
{GUCEF_TRACE;

    return m_keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
bool
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::Serialize( const CString& resourcePath         ,
                                                                                               CORE::CDataNode& output             ,
                                                                                               const CORE::CString& representation ,
                                                                                               const CString& params               )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( m_collectionLock );

    bool totalSuccess = true;
    output = CORE::CDataNode( m_collectionName, GUCEF_DATATYPE_ARRAY );
    typename TConfigurableCollectionMap::iterator i = m_collection->begin();
    while ( i != m_collection->end() )
    {
        CORE::CDataNode& childNode = output.AddChild( (*i).first );

        // @TODO: add param based views
        //totalSuccess = (*i).second->SaveConfig( childNode ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, class ConfigurableDerivedClass >
CCodecBasedHTTPServerResource::TCreateState
CTConfigurableMapHttpServerResource< CollectionKeyType, ConfigurableDerivedClass >::CreateResource( const CString& resourcePath                   ,
                                                                                                    const CString& transactionID                  ,
                                                                                                    const CORE::CDataNode& input                  ,
                                                                                                    const CString& representation                 ,
                                                                                                    const CString& params                         ,
                                                                                                    THTTPServerResourcePtr& resourceOutput        ,
                                                                                                    TStringVector& supportedRepresentationsOutput )
{GUCEF_TRACE;

    if ( m_keyPropertyName.IsNULLOrEmpty() )
    {
        return ECreateState::CREATESTATE_FAILED;
    }

    CORE::CString entryKey = input.GetAttributeValueOrChildValueByName( m_keyPropertyName );
    if ( entryKey.IsNULLOrEmpty() )
    {
        return ECreateState::CREATESTATE_UNSUPPORTEDREPRESENTATION;
    }

    MT::CObjectScopeLock lock( m_collectionLock );

    typename TConfigurableCollectionMap::iterator i = m_collection->find( entryKey );
    if ( i != m_collection->end() )
    {
        return ECreateState::CREATESTATE_CONFLICTING;
    }

    ConfigurableDerivedClass& newEntry = (*m_collection)[ entryKey ];
    if ( !newEntry.LoadConfig( input ) )
    {
        m_collection->erase( entryKey );
        return ECreateState::CREATESTATE_DESERIALIZATIONFAILED;
    }

    return ECreateState::CREATESTATE_CREATED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H ? */

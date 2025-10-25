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

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#define GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H

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

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H
#include "gucefCORE_CTDataNodeSerializableObj.h"
#define GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H
#endif /* GUCEF_CORE_CTDATANODESERIALIZABLEOBJ_H ? */

#ifndef GUCEF_CORE_DATANODE_SERIALIZABLES_H
#include "gucefCORE_datanode_serializables.h"
#define GUCEF_CORE_DATANODE_SERIALIZABLES_H
#endif /* GUCEF_CORE_DATANODE_SERIALIZABLES_H ? */

#ifndef GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H
#include "gucefCORE_CGenericValueToDataNodeSerializer.h"
#define GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H
#endif /* GUCEF_CORE_CGENERICVALUETODATANODESERIALIZER_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CSTDPARAMPARSER_H
#include "gucefWEB_CStdParamParser.h"
#define GUCEF_WEB_CSTDPARAMPARSER_H
#endif /* GUCEF_WEB_CSTDPARAMPARSER_H ? */

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

/**
 *  Template for easily mapping a logical server resource to an STL map (or similar) container
 */
template< typename CollectionKeyType, typename SerializableObj >
class CTDataNodeSerializableMapHttpServerResource : public CCodecBasedHTTPServerResource
{
    public:

    typedef GUCEF::map< CollectionKeyType, SerializableObj > TSerializableCollectionMap;

    /**
     *  Default constructor generates a placeholder resource which is not yet usable.
     *  You must use LinkTo() in order to map the logical resource to your actual data
     */
    CTDataNodeSerializableMapHttpServerResource( void );
    
    CTDataNodeSerializableMapHttpServerResource( const CORE::CString& collectionName                                                                      ,
                                                 const CORE::CString& keyPropertyName                                                                     ,
                                                 const CORE::CDataNodeSerializableSettings* serializerOptions = GUCEF_NULL                                ,
                                                 TSerializableCollectionMap* collection = GUCEF_NULL                                                      ,
                                                 MT::CILockable* collectionLock = GUCEF_NULL                                                              ,
                                                 bool sourceObjKeyFromObj = false                                                                         ,
                                                 bool treatCollectionAsArray = false                                                                      ,
                                                 CORE::CIValueToDataNodeSerializer* valueSerializer = CORE::CGenericValueToDataNodeSerializer::Instance() );

    virtual ~CTDataNodeSerializableMapHttpServerResource();

    void LinkTo( const CORE::CString& collectionName                                                                      ,
                 const CORE::CString& keyPropertyName                                                                     ,
                 const CORE::CDataNodeSerializableSettings* serializerOptions                                             ,
                 TSerializableCollectionMap* collection                                                                   ,
                 MT::CILockable* collectionLock                                                                           ,
                 bool sourceObjKeyFromObj = false                                                                         ,
                 bool treatCollectionAsArray = false                                                                      ,
                 CORE::CIValueToDataNodeSerializer* valueSerializer = CORE::CGenericValueToDataNodeSerializer::Instance() );

    virtual bool Serialize( const CString& resourcePath         ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& query          ) GUCEF_VIRTUAL_OVERRIDE;

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
                                         const CORE::CString& transactionID            ,
                                         const CORE::CDataNode& input                  ,
                                         const CORE::CString& representation           ,
                                         const CORE::CString& params                   ,
                                         THTTPServerResourcePtr& resourceOutput        ,
                                         TStringVector& supportedRepresentationsOutput ) GUCEF_VIRTUAL_OVERRIDE;

    void SetResourceKeyPropertyName( const CString& keyPropertyName );

    const CString& GetResourceKeyPropertyName( void ) const;

    protected:

    const CORE::CDataNodeSerializableSettings* m_serializerOptions;
    TSerializableCollectionMap* m_collection;
    MT::CILockable* m_collectionLock;
    CORE::CString m_collectionName;
    CORE::CString m_keyPropertyName;
    bool m_sourceObjKeyFromObj;
    bool m_treatCollectionAsArray;
    CORE::CIValueToDataNodeSerializer* m_valueSerializer;

    private:

    CTDataNodeSerializableMapHttpServerResource( const CTDataNodeSerializableMapHttpServerResource& src );
    CTDataNodeSerializableMapHttpServerResource& operator=( const CTDataNodeSerializableMapHttpServerResource& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::CTDataNodeSerializableMapHttpServerResource( void )
    : CCodecBasedHTTPServerResource()
    , m_serializerOptions( GUCEF_NULL )
    , m_collection( GUCEF_NULL )
    , m_collectionLock( GUCEF_NULL )
    , m_collectionName()
    , m_keyPropertyName()
    , m_sourceObjKeyFromObj( false )
    , m_treatCollectionAsArray( false )
    , m_valueSerializer( CORE::CGenericValueToDataNodeSerializer::Instance() )
{GUCEF_TRACE;

    m_allowSerialize = false;
    m_allowDeserialize = false;
    m_allowCreate = false;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::CTDataNodeSerializableMapHttpServerResource( const CORE::CString& collectionName                          ,
                                                                                                                                const CORE::CString& keyPropertyName                         ,
                                                                                                                                const CORE::CDataNodeSerializableSettings* serializerOptions ,
                                                                                                                                TSerializableCollectionMap* collection                       ,
                                                                                                                                MT::CILockable* collectionLock                               ,
                                                                                                                                bool sourceObjKeyFromObj                                     ,
                                                                                                                                bool treatCollectionAsArray                                  ,
                                                                                                                                CORE::CIValueToDataNodeSerializer* valueSerializer           )
    : CCodecBasedHTTPServerResource()
    , m_serializerOptions( serializerOptions )
    , m_collection( collection )
    , m_collectionLock( collectionLock )
    , m_collectionName( collectionName )
    , m_keyPropertyName( keyPropertyName )
    , m_sourceObjKeyFromObj( sourceObjKeyFromObj )
    , m_treatCollectionAsArray( treatCollectionAsArray )
    , m_valueSerializer( valueSerializer )
{GUCEF_TRACE;

    m_allowSerialize = true;
    m_allowDeserialize = false;
    m_allowCreate = true;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::~CTDataNodeSerializableMapHttpServerResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
void
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::LinkTo( const CORE::CString& collectionName                          ,
                                                                                           const CORE::CString& keyPropertyName                         ,
                                                                                           const CORE::CDataNodeSerializableSettings* serializerOptions ,
                                                                                           TSerializableCollectionMap* collection                       ,
                                                                                           MT::CILockable* collectionLock                               ,
                                                                                           bool sourceObjKeyFromObj                                     ,
                                                                                           bool treatCollectionAsArray                                  ,
                                                                                           CORE::CIValueToDataNodeSerializer* valueSerializer           )
{GUCEF_TRACE;

    m_collectionName = collectionName;
    m_keyPropertyName = keyPropertyName;
    m_serializerOptions = serializerOptions;
    m_collection = collection;
    m_collectionLock = collectionLock;
    m_sourceObjKeyFromObj = sourceObjKeyFromObj;
    m_treatCollectionAsArray = treatCollectionAsArray;
    m_valueSerializer = valueSerializer;

    m_allowSerialize = true;
    m_allowDeserialize = false;
    m_allowCreate = true;

    if ( GUCEF_NULL == valueSerializer )
        m_valueSerializer = CORE::CGenericValueToDataNodeSerializer::Instance();
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
void
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::SetResourceKeyPropertyName( const CString& keyPropertyName )
{GUCEF_TRACE;

    m_keyPropertyName = keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
const CString&
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::GetResourceKeyPropertyName( void ) const
{GUCEF_TRACE;

    return m_keyPropertyName;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
bool
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::Serialize( const CString& resourcePath         ,
                                                                                              CORE::CDataNode& output             ,
                                                                                              const CORE::CString& representation ,
                                                                                              const CString& params               )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_collection )
        return false;

    // Make sure we always have some degree of serializer options
    const CORE::CDataNodeSerializableSettings* serializerOptions = m_serializerOptions;
    if ( GUCEF_NULL == serializerOptions )
    {
        static const CORE::CDataNodeSerializableSettings defaultDummy;
        serializerOptions = &defaultDummy;
    }

    bool foundSupportedParams = false;
    CStdParams stdParams( *serializerOptions );
    CORE::CTSharedPtr< const CORE::CDataNodeSerializableSettings, MT::CNoLock > requestSpecificSerializerOptions;
    if ( CStdParamParser::ParseStdParams( params, stdParams, foundSupportedParams ) && foundSupportedParams && stdParams.levelOfDetailWasProvided )
    {
        // We place the clone in a shared ptr so that when we leave this scope the clone is properly disposed of
        requestSpecificSerializerOptions = CORE::CTSharedPtr< CORE::CDataNodeSerializableSettings, MT::CNoLock >( static_cast< CORE::CDataNodeSerializableSettings* >( serializerOptions->Clone() ) );
        const_cast< CORE::CDataNodeSerializableSettings* >( requestSpecificSerializerOptions.GetPointerAlways() )->levelOfDetail = stdParams.levelOfDetail;
        serializerOptions = requestSpecificSerializerOptions.GetPointerAlways();
    }

    bool totalSuccess = true;
    output = CORE::CDataNode( m_collectionName, GUCEF_DATATYPE_ARRAY );

    MT::CObjectScopeReadOnlyLock lock( m_collectionLock );

    if ( VIEW_TYPE_INDEX_ONLY == stdParams.viewType && !m_sourceObjKeyFromObj )
    {
        typename TSerializableCollectionMap::iterator i = m_collection->begin();
        while ( i != m_collection->end() )
        {
            CORE::CDataNode* childNode = output.AddValueAsChild( (*i).first );
            if ( GUCEF_NULL == childNode )
                totalSuccess = false;
            ++i;
        }
    }
    else
    {
        // When serializing details we need the name of each child and as such the parent cannot be an array by default 
        // which is the default value of m_treatCollectionAsArray
        // However depending on context of what the data is this can be overriden since the object itself may hold a usable id/key
        if ( !m_treatCollectionAsArray )
            output.SetNodeType( GUCEF_DATATYPE_OBJECT );

        typename TSerializableCollectionMap::iterator i = m_collection->begin();
        while ( i != m_collection->end() )
        {
            CORE::CDataNode* childNode = output.AddChild( CORE::ToString( (*i).first ) );
            if ( GUCEF_NULL != childNode )
            {
                if ( m_valueSerializer->SerializeType< SerializableObj >( (*i).second, *childNode, *serializerOptions ) )
                {

                }
                else
                {
                    totalSuccess = false;
                }
            }
            else
            {
                totalSuccess = false;
            }
            ++i;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

template< typename CollectionKeyType, typename SerializableObj >
CCodecBasedHTTPServerResource::TCreateState
CTDataNodeSerializableMapHttpServerResource< CollectionKeyType, SerializableObj >::CreateResource( const CString& resourcePath                   ,
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

    const CORE::CVariant& keyPropertyValue = input.GetAttributeValueOrChildValueByName( m_keyPropertyName );
    CollectionKeyType entryKey = keyPropertyValue.AsTValue< CollectionKeyType >();
    if ( keyPropertyValue.IsNULLOrEmpty() )
    {
        return ECreateState::CREATESTATE_UNSUPPORTEDREPRESENTATION;
    }

    // Obtain a write lock to the collection
    MT::CObjectScopeLock lock( m_collectionLock );

    if ( GUCEF_NULL == m_collection )
        return TCreateState::CREATESTATE_FAILED;

    typename TSerializableCollectionMap::iterator i = m_collection->find( entryKey );
    if ( i != m_collection->end() )
    {
        return ECreateState::CREATESTATE_CONFLICTING;
    }

    SerializableObj& newEntry = (*m_collection)[ entryKey ];
    if ( GUCEF_NULL != m_serializerOptions )
    {
        if ( !m_valueSerializer->DeserializeType< SerializableObj >( newEntry, input, *m_serializerOptions ) )
        {
            m_collection->erase( entryKey );
            return ECreateState::CREATESTATE_DESERIALIZATIONFAILED;
        }
    }
    else
    {
        CORE::CDataNodeSerializableSettings defaultSerializableSettings;
        if ( !m_valueSerializer->DeserializeType< SerializableObj >( newEntry, input, defaultSerializableSettings ) )
        {
            m_collection->erase( entryKey );
            return ECreateState::CREATESTATE_DESERIALIZATIONFAILED;
        }
    }

    return ECreateState::CREATESTATE_CREATED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace WEB */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H ? */

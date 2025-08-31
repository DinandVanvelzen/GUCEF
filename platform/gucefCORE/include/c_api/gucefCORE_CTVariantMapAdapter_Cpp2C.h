/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CTVARIANTVECTORAPI_H
#define GUCEF_CORE_CTVARIANTVECTORAPI_H
#ifdef __cplusplus

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_VARIANTMAP_H
#include "gucefCORE_c_variantmap.h"
#define GUCEF_CORE_C_VARIANTMAP_H
#endif /* GUCEF_CORE_C_VARIANTMAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_TYPEINFOTEMPLATEEXT_H
#include "gucefCORE_TypeInfoTemplateExt.h"
#define GUCEF_CORE_TYPEINFOTEMPLATEEXT_H
#endif /* GUCEF_CORE_TYPEINFOTEMPLATEEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASS TEMPLATES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  template which adds a C API adapter to the std::map class
 *  Since we cannot use templates in C, we need to create a C API adapter 
 *  which is build around the variant concept in order to have a generic interface
 *  to the std::map class.
 *  Note that this template is not intended to work with any and all types.
 *  The class is intended to work with maps holding types that are supported by the CVariant class.
 */
template < typename TK, typename TV, class TKC = std::less< TK >, class AllocType = gucef_allocator< std::pair< const TK, TV > > >
class CTVariantMapAdapterCpp2C
{
    public:
    
    typedef std::map< TK, TV, TKC, AllocType > TLinkedMapType;    
    typedef std::pair< const TK, TV >          TLinkedMapEntryPairType;
    typedef TK                                 TLinkedMapKeyType;
    typedef TV                                 TLinkedMapValueType;
    typedef TKC                                TLinkedMapKeyCompareType;
    typedef AllocType                          TLinkedMapAllocatorType;
    
    CTVariantMapAdapterCpp2C( void );
    ~CTVariantMapAdapterCpp2C();

    void SetMapAccessPtr( TLinkedMapType* map );
    void SetMapAccessPtr( const TLinkedMapType* map );

    /*
     *  The below are the C API functions which mimic the std::vector functions
     *  The following set works with const and non-const maps
     */

    static void at_key( void* privdata, TVariantData* searchKey, TVariantData* outValue );    /**< attempts to retrieve an existing key's value pair from the map. It does not create new entries if no such key exists */
    static void at_index( void* privdata, UInt32 index, TVariantData* outValue );             /**< attempts to retrieve an existing key's value pair from the map. It does not create new entries if no such key exists */
    static size_t size( void* privdata );                                                     /**< returns the number of key-value pairs in the map */

    /*
     *  The below are the C API functions which mimic the std::map functions
     *  The following set works with non-const maps only
     */

    static UInt8 insert( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible );    /**< attempts to insert a key-value pair into the map. It does not overwrite thus will fail if the key exists */
    static UInt8 set( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible );       /**< attempts to set an existing key's value pair into the map. It does not overwrite thus will fail if the key exists */
    static void clear( void* privdata );                                                                    /**< clears the map */
    static UInt8 erase( void* privdata, const TVariantData* searchKey );                                    /**< attempts to erase a key-value pair from the map, if the key doesnt exists the operation fails and is a no-op */

    /*
     *  The below are the C API functions which are helper since not all information can be carried across
     *  the C boundary when mimicing the std::map functions
     *  The following works with const and non-const maps
     */

    static Int8 is_const( void* privdata );
    static UInt8 type_id_of_key_elements( void* privdata );
    static UInt8 type_id_of_value_elements( void* privdata );
    static UInt32 byte_size_of_key_element_type( void* privdata );
    static UInt32 byte_size_of_value_element_type( void* privdata );

    TVariantMapApi* GetCStyleAccess( void );

    const TVariantMapApi* GetCStyleAccess( void ) const;
    
    private:
    
    TLinkedMapType* m_map;
    const TLinkedMapType* m_constMap;
    TVariantMapApi m_cApi;
};

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::CTVariantMapAdapterCpp2C( void )
    : m_map( GUCEF_NULL )
    , m_constMap( GUCEF_NULL )
{GUCEF_TRACE;

    memset( &m_cApi, 0, sizeof( m_cApi ) );
    
    m_cApi.at_key = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::at_key;
    m_cApi.at_index = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::at_index;
    m_cApi.size = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::size;
    
    m_cApi.insert = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::insert;
    m_cApi.set = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::set;
    m_cApi.clear = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::clear;    
    m_cApi.erase = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::erase;    
    
    m_cApi.is_const = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::is_const;
    m_cApi.type_id_of_key_elements = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::type_id_of_key_elements;
    m_cApi.byte_size_of_key_element_type = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::byte_size_of_key_element_type;
    m_cApi.type_id_of_value_elements = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::type_id_of_value_elements;
    m_cApi.byte_size_of_value_element_type = &CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::byte_size_of_value_element_type;
    
    m_cApi.privateData = static_cast<void*>( this );
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::~CTVariantMapAdapterCpp2C( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::SetMapAccessPtr( TLinkedMapType* map )
{GUCEF_TRACE;

    m_map = map;
    m_constMap = map;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::SetMapAccessPtr( const TLinkedMapType* map )
{GUCEF_TRACE;

    m_map = GUCEF_NULL;
    m_constMap = map;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
TVariantMapApi* 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::GetCStyleAccess( void )
{GUCEF_TRACE;

    return &m_cApi;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
const TVariantMapApi* 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::GetCStyleAccess( void ) const
{GUCEF_TRACE;

    return &m_cApi;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::insert( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata && GUCEF_NULL != key && GUCEF_NULL != value )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_map )
        {
            // This helper template is NOT intended to work with any and all types.
            // the type TK should be supported by the CVariant class.
            CVariant searchKeyVar;
            searchKeyVar.LinkTo( key );
            TK nativeSearchKey = searchKeyVar.AsTValue< TK >();

            // For an insert we need to check if the key already exists
            // We specifically do NOT want to overwrite existing keys
            typename TLinkedMapType::iterator i = api->m_map->find( nativeSearchKey );
            if ( i == api->m_map->end() )
            {
                CVariant valueToInsertVar;
                valueToInsertVar.LinkTo( value );

                if ( 0 != linkIfPossible )
                {
                    // @TODO: check if the value type supports linking
                    TK nativeValueToInsert = valueToInsertVar.AsTValue< TK >();
                    (*api->m_map)[ nativeSearchKey ] = nativeValueToInsert;
                }
                else
                {
                    TK nativeValueToInsert = valueToInsertVar.AsTValue< TK >();
                    (*api->m_map)[ nativeSearchKey ] = nativeValueToInsert;
                }
                return 1; // success
            }
        }
    }
    return 0; // failure to insert
}   

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::set( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata && GUCEF_NULL != key && GUCEF_NULL != value )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_map )
        {
            // This helper template is NOT intended to work with any and all types.
            // the type TK should be supported by the CVariant class.
            CVariant searchKeyVar;
            searchKeyVar.LinkTo( key );
            TK nativeSearchKey = searchKeyVar.AsTValue< TK >();

            // For a set we need to check if the key already exists
            // We specifically DO want to overwrite existing keys
            typename TLinkedMapType::iterator i = api->m_map->find( nativeSearchKey );
            if ( i != api->m_map->end() )
            {
                CVariant valueToInsertVar;
                valueToInsertVar.LinkTo( value );

                if ( 0 != linkIfPossible )
                {
                    // @TODO: check if the value type supports linking
                    (*i).second = valueToInsertVar.AsTValue< TK >();
                }
                else
                {
                    (*i).second = valueToInsertVar.AsTValue< TK >();
                }
                return 1; // success
            }
        }
    }
    return 0; // failure to insert
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::clear( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_map )
        {
            api->m_map->clear();
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
size_t 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::size( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_constMap )
        {
            return api->m_constMap->size();
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::at_key( void* privdata, TVariantData* searchKey, TVariantData* outValue )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata && GUCEF_NULL != searchKey && GUCEF_NULL != outValue )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_constMap )
        {
            // This helper template is NOT intended to work with any and all types.
            // the type TK should be supported by the CVariant class.
            CVariant searchKeyVar;
            searchKeyVar.LinkTo( searchKey );
            TK nativeSearchKey = searchKeyVar.AsTValue< TK >();

            typename TLinkedMapType::const_iterator i = api->m_constMap->find( nativeSearchKey );
            if ( i != api->m_constMap->end() )
            {
                const TV& foundValue = (*i).second;
                CVariant valueAdapter;
                if GUCEF_PREDICT_TRUE( valueAdapter.SetTValue< TV >( foundValue, true ) )
                {
                    *outValue = *valueAdapter.CStyleAccess();
                }
                else
                {
                    // failed to set the value
                    memset( outValue, 0, sizeof( TVariantData ) );
                    outValue->containedType = GUCEF_DATATYPE_UNKNOWN;

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTVariantMapAdapterCpp2C:at_key: Failed to set the value for the key \"" + searchKeyVar.AsString() + "\" on a variant" );
                }
            }
            else
            {
                // no such key
                memset( outValue, 0, sizeof( TVariantData ) );
                outValue->containedType = GUCEF_DATATYPE_UNKNOWN;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::at_index( void* privdata, UInt32 searchIndex, TVariantData* outValue )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata && GUCEF_NULL != outValue )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_constMap )
        {
            if ( searchIndex < static_cast< UInt32 >( api->m_constMap->size() ) )
            {
                typename TLinkedMapType::const_iterator i = api->m_constMap->begin();
                std::advance( i, searchIndex );

                const TV& foundValue = (*i).second;
                CVariant valueAdapter;
                if GUCEF_PREDICT_TRUE( valueAdapter.SetTValue< TV >( foundValue, true ) )
                {
                    *outValue = *valueAdapter.CStyleAccess();
                }
                else
                {
                    // failed to set the value
                    memset( outValue, 0, sizeof( TVariantData ) );
                    outValue->containedType = GUCEF_DATATYPE_UNKNOWN;

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTVariantMapAdapterCpp2C:at_key: Failed to set the value at index " + ToString( searchIndex ) + " on a variant" );
                }
            }
            else
            {
                // no such key - out of range
                memset( outValue, 0, sizeof( TVariantData ) );
                outValue->containedType = GUCEF_DATATYPE_UNKNOWN;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::erase( void* privdata, const TVariantData* searchKey )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata && GUCEF_NULL != searchKey )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        if GUCEF_PREDICT_TRUE( GUCEF_NULL != api->m_map )
        {
            // This helper template is NOT intended to work with any and all types.
            // the type TK should be supported by the CVariant class.
            CVariant searchKeyVar;
            searchKeyVar.LinkTo( searchKey );
            TK nativeSearchKey = searchKeyVar.AsTValue< TK >();

            api->m_map->erase( nativeSearchKey );
            return 1; // success
        }
    }
    return 0; // failure to erase - prerequisite not met
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
Int8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::is_const( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >* >( privdata );
        return GUCEF_NULL == api->m_map ? 1 : 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::type_id_of_key_elements( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        return TryToGetGucefTypeIdForTType< TK >();
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt32 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::byte_size_of_key_element_type( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        return static_cast< UInt32 >( sizeof( TK ) );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::type_id_of_value_elements( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        return TryToGetGucefTypeIdForTType< TV >();
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, typename TKC, class AllocType >
UInt32 
CTVariantMapAdapterCpp2C< TK, TV, TKC, AllocType >::byte_size_of_value_element_type( void* privdata )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != privdata )
    {
        return static_cast< UInt32 >( sizeof( TV ) );
    }
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* __cplusplus ? */
#endif /* GUCEF_CORE_CTVARIANTVECTORAPI_H ? */

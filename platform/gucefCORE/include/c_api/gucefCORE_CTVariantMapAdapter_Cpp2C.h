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
template < typename TK, typename TV, class AllocType >
class CTVariantMapAdapterCpp2C
{
    public:
    
    typedef std::map< TK, TV, AllocType >      TLinkedMapType;
    
    CTVariantMapAdapterCpp2C( void );
    ~CTVariantMapAdapterCpp2C();

    void SetMapAccessPtr( TLinkedMapType* map );
    void SetMapAccessPtr( const TLinkedMapType* map );

    /*
     *  The below are the C API functions which mimic the std::vector functions
     *  The following set works with const and non-const maps
     */

    static size_t size( void* privdata );                                               /**< returns the number of key-value pairs in the map */
    static void at( void* privdata, TVariantData* searchKey, TVariantData* outValue );  /**< attempts to retrieve an existing key's value pair from the map. It does not create new entries if no such key exists */

    /*
     *  The below are the C API functions which mimic the std::vector functions
     *  The following set works with non-const vectors only
     */

    static UInt8 insert( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible );    /**< attempts to insert a key-value pair into the map. It does not overwrite thus will fail if the key exists */
    static UInt8 set( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible );       /**< attempts to set an existing key's value pair into the map. It does not overwrite thus will fail if the key exists */
    static void clear( void* privdata );                                                                    /**< clears the map */
    static UInt8 erase( void* privdata, TVariantData* searchKey );                                          /**< attempts to erase a key-value pair from the map, if the key doesnt exists the operation fails and is a no-op */

    /*
     *  The below are the C API functions which are helper since not all information can be carried across
     *  the C boundary when mimicing the std::vector functions
     *  The following works with const and non-const vectors
     */

    static Int8 is_const( void* privdata );
    static UInt8 type_id_of_elements( void* privdata );
    static UInt32 byte_size_of_element_type( void* privdata );
    
    private:
    
    TLinkedMapType* m_map;
    const TLinkedMapType* m_constMap;
    TVariantMapApi m_cApi;
};

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::CTVariantMapAdapterCpp2C( void )
    : m_map( GUCEF_NULL )
    , m_constMap( GUCEF_NULL )
{GUCEF_TRACE;

    memset( &m_cApi, 0, sizeof( m_cApi ) );
    m_cApi.clear = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::clear;
    m_cApi.size = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::size;
    m_cApi.at = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::at;
    m_cApi.is_const = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::is_const;
    m_cApi.type_id_of_key_elements = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::type_id_of_key_elements;
    m_cApi.byte_size_of_key_element_type = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::byte_size_of_key_element_type;
    m_cApi.type_id_of_value_elements = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::type_id_of_value_elements;
    m_cApi.byte_size_of_value_element_type = &CTVariantMapAdapterCpp2C< TK, TV, AllocType >::byte_size_of_value_element_type;
    m_cApi.privateData = static_cast<void*>( this );
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::~CTVariantMapAdapterCpp2C( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::SetMapAccessPtr( TLinkedMapType* map )
{GUCEF_TRACE;

    m_map = map;
    m_constMap = map;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::SetMapAccessPtr( const TLinkedMapType* map )
{GUCEF_TRACE;

    m_map = GUCEF_NULL;
    m_constMap = map;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::push_back( void* privdata, TVariantData* data, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        CTVariantMapAdapterCpp2C< T, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< T, AllocType >* >( privdata );
        CVariant cppVariant;
        cppVariant.LinkTo( data );
        m_vector->push_back( cppVariant.AsTValue< T >() );
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::clear( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_map )
    {
        CTVariantMapAdapterCpp2C< TK, TV, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, AllocType >* >( privdata );
        api->m_map->clear();
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
size_t 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::size( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constMap )
    {
        CTVariantMapAdapterCpp2C< TK, TV, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, AllocType >* >( privdata );
        return api->m_constMap->size();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
void 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::at( void* privdata, TVariantData* searchKey, TVariantData* outValue )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constVector && GUCEF_NULL != entryData )
    {
        CTVariantMapAdapterCpp2C< TK, TV, AllocType >* api = static_cast< CTVariantMapAdapterCpp2C< TK, TV, AllocType >* >( privdata );
        if ( index < static_cast< UInt32 >( api->m_constVector->size() ) )
        {
            const T& element = api->m_constVector->at( index );

            // This helper template is NOT intended to work with any and all types.
            // If you get a compiler error here, the type T is not supported by the CVariant class.
            CVariant cppVariant( element ); // <- if you get a compiler error here the type T is not supported by the CVariant class

            *entryData = *cppVariant.CStyleAccess();    
        }
        else
        {
            // index out of bounds
            entryData->containedType = GUCEF_DATATYPE_UNKNOWN;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
Int8 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::is_const( void* privdata )
{GUCEF_TRACE;

    return GUCEF_NULL == m_map ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::type_id_of_key_elements( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constMap )
    {
        return TryToGetGucefTypeIdForTType< TK >();
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
UInt32 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::byte_size_of_key_element_type( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constMap )
    {
        return static_cast< UInt32 >( sizeof( TK ) );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
UInt8 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::type_id_of_value_elements( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constMap )
    {
        return TryToGetGucefTypeIdForTType< TV >();
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename TK, typename TV, class AllocType >
UInt32 
CTVariantMapAdapterCpp2C< TK, TV, AllocType >::byte_size_of_value_element_type( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constMap )
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

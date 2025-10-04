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

#include <vector>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_VARIANTVECTOR_H
#include "gucefCORE_c_variantvector.h"
#define GUCEF_CORE_C_VARIANTVECTOR_H
#endif /* GUCEF_CORE_C_VARIANTVECTOR_H ? */

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
 *  template which adds a C API adapter to the std::vector class
 *  Since we cannot use templates in C, we need to create a C API adapter 
 *  which is build around the variant concept in order to have a generic interface
 *  to the std::vector class.
 *  Note that this template is not intended to work with any and all types.
 *  The class is intended to work with vectors holding types that are supported by the CVariant class.
 */
template < typename T, class AllocType >
class CTVariantVectorApi
{
    public:
    
    typedef GUCEF::vector< T, AllocType >      TLinkedVectorType;
    
    CTVariantVectorApi( void );
    ~CTVariantVectorApi();

    void SetVectorAccessPtr( TLinkedVectorType* vector );
    void SetVectorAccessPtr( const TLinkedVectorType* vector );

    /*
     *  The below are the C API functions which mimic the std::vector functions
     *  The following set works with const and non-const vectors
     */

    static size_t size( void* privdata );
    static void back( void* privdata, TVariantData* backData, UInt8 linkIfPossible );
    static void front( void* privdata, TVariantData* frontData, UInt8 linkIfPossible );    
    static void at( void* privdata, UInt32 index, TVariantData* entryData );

    /*
     *  The below are the C API functions which mimic the std::vector functions
     *  The following set works with non-const vectors only
     */

    static void push_back( void* privdata, TVariantData* data, UInt8 linkIfPossible );
    static void pop_back( void* privdata );
    static void clear( void* privdata );

    /*
     *  The below are the C API functions which are helper since not all information can be carried across
     *  the C boundary when mimicing the std::vector functions
     *  The following works with const and non-const vectors
     */

    static Int8 is_const( void* privdata );
    static UInt8 type_id_of_elements( void* privdata );
    static UInt32 byte_size_of_element_type( void* privdata );
    
    private:
    
    TLinkedVectorType* m_vector;
    const TLinkedVectorType* m_constVector;
    TVariantVectorApi m_cApi;
};

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
CTVariantVectorApi< T, AllocType >::CTVariantVectorApi( void )
{GUCEF_TRACE;

    m_cApi.push_back = &CTVariantVectorApi< T >::push_back;
    m_cApi.clear = &CTVariantVectorApi< T >::clear;
    m_cApi.size = &CTVariantVectorApi< T >::size;
    m_cApi.back = &CTVariantVectorApi< T >::back;
    m_cApi.front = &CTVariantVectorApi< T >::front;
    m_cApi.pop_back = &CTVariantVectorApi< T >::pop_back;
    m_cApi.at = &CTVariantVectorApi< T >::at;
    m_cApi.is_const = &CTVariantVectorApi< T >::is_const;
    m_cApi.type_id_of_elements = &CTVariantVectorApi< T >::type_id_of_elements;
    m_cApi.byte_size_of_element_type = &CTVariantVectorApi< T >::byte_size_of_element_type;
    m_cApi.privateData = static_cast<void*>( this );
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
CTVariantVectorApi< T, AllocType >::~CTVariantVectorApi( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::SetVectorAccessPtr( TLinkedVectorType* vector )
{GUCEF_TRACE;

    m_vector = vector;
    m_constVector = vector;
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::SetVectorAccessPtr( const TLinkedVectorType* vector )
{GUCEF_TRACE;

    m_vector = GUCEF_NULL;
    m_constVector = vector;
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::push_back( void* privdata, TVariantData* data, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        CVariant cppVariant;
        cppVariant.LinkTo( data );
        m_vector->push_back( cppVariant.AsTValue< T >() );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::clear( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        api->m_vector->clear();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
size_t 
CTVariantVectorApi< T, AllocType >::size( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constVector )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        return api->m_constVector->size();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::back( void* privdata, TVariantData* backData, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constVector && GUCEF_NULL != backData )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        const T& backValue = api->m_constVector->back();
        
        // This helper template is NOT intended to work with any and all types.
        // If you get a compiler error here, the type T is not supported by the CVariant class.
        CVariant cppVariant( backValue ); // <- if you get a compiler error here the type T is not supported by the CVariant class

        *backData = *cppVariant.CStyleAccess();        
        return;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::front( void* privdata, TVariantData* frontData, UInt8 linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constVector && GUCEF_NULL != frontData )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        const T& frontValue = api->m_constVector->front();
        
        // This helper template is NOT intended to work with any and all types.
        // If you get a compiler error here, the type T is not supported by the CVariant class.
        CVariant cppVariant( frontValue ); // <- if you get a compiler error here the type T is not supported by the CVariant class

        *frontData = *cppVariant.CStyleAccess();        
        return;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::pop_back( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
        api->m_vector->pop_back();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
void 
CTVariantVectorApi< T, AllocType >::at( void* privdata, UInt32 index, TVariantData* entryData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_constVector && GUCEF_NULL != entryData )
    {
        CTVariantVectorApi< T, AllocType >* api = static_cast< CTVariantVectorApi< T, AllocType >* >( privdata );
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

template < typename T, class AllocType >
Int8 
CTVariantVectorApi< T, AllocType >::is_const( void* privdata )
{GUCEF_TRACE;

    return GUCEF_NULL == m_vector ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
UInt8 
CTVariantVectorApi< T, AllocType >::type_id_of_elements( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        return TryToGetGucefTypeIdForTType< T >();
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename T, class AllocType >
UInt32 
CTVariantVectorApi< T, AllocType >::byte_size_of_element_type( void* privdata )
{GUCEF_TRACE;

    if ( GUCEF_NULL != privdata && GUCEF_NULL != m_vector )
    {
        return static_cast< UInt32 >( sizeof( T ) );
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

/*
 * Copyright (C) Dinand Vanvelzen. 2002 - 2006.  All rights reserved.
 *
 * All source code herein is the property of Dinand Vanvelzen. You may not sell
 * or otherwise commercially exploit the source or things you created based on
 * the source.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL DINAND VANVELZEN BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF 
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT 
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

#ifndef GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
#define GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"   
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#include "ExceptionThrowMacros.h"
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { 
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Similar to the CTCloneableObj template only with a twist
 *  This template will allow you to link to an object without having 
 *  to copy it. You can then pass the wrapped linked object to an interface
 *  that uses cloning. If the need for a clone arises then a real copy will 
 *  be made, but not before. This allows you to optimize passing large objects
 *  as cloneables.
 */
template< typename T > 
class CTLinkedCloneableObj : public CICloneable
{
    public:

    CTLinkedCloneableObj( void );
    
    // preserves the link, does not copy the wrapped object
    explicit CTLinkedCloneableObj( const T* data );

    // creates a copy of the wrapped object
    CTLinkedCloneableObj( const CTLinkedCloneableObj& src );
        
    virtual ~CTLinkedCloneableObj();
    
    CTLinkedCloneableObj& operator=( const CTLinkedCloneableObj& src );
    
    virtual CICloneable* Clone( void ) const;    
    
    const T& GetData( void ) const;
    
    void LinkTo( const T* data );

    void Clear( void );

    inline operator const T&() const;

    inline operator const T*() const;

    GUCEF_DEFINE_INLINED_MSGEXCEPTION( ENULLPointer );

    private:
    const T* m_data;
    bool m_linked;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CTLinkedCloneableObj< T >::CTLinkedCloneableObj( void )
        : m_data( GUCEF_NULL )    ,
          m_linked( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T >
CTLinkedCloneableObj< T >::CTLinkedCloneableObj( const T* data )
        : m_data( data )   ,
          m_linked( true )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data )
    {
        GUCEF_EMSGTHROW( ENULLPointer, "CTLinkedCloneableObj( const T* data ): This operation is impossible without a valid object" );
    }
}

/*-------------------------------------------------------------------------*/

template< typename T >
CTLinkedCloneableObj< T >::CTLinkedCloneableObj( const CTLinkedCloneableObj< T >& src )
        : m_data( GUCEF_NULL != src.m_data ? GUCEF_NEW T( *src.m_data ) : GUCEF_NULL ) ,
          m_linked( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
template< typename T >
CTLinkedCloneableObj< T >::~CTLinkedCloneableObj()
{GUCEF_TRACE;
    
    Clear();
}

/*-------------------------------------------------------------------------*/
    
template< typename T >
void
CTLinkedCloneableObj< T >::Clear( void )
{GUCEF_TRACE;

    if ( !m_linked )
    {
        GUCEF_DELETE m_data;
    }
    m_data = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/
    
template< typename T >
CTLinkedCloneableObj< T >& 
CTLinkedCloneableObj< T >::operator=( const CTLinkedCloneableObj< T >& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        Clear();
        if ( GUCEF_NULL != src.m_data )
            m_data = GUCEF_NEW T( *src.m_data );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename T >
void
CTLinkedCloneableObj< T >::LinkTo( const T* data )
{GUCEF_TRACE;

    Clear();
    m_data = data;
    m_linked = true;
}

/*-------------------------------------------------------------------------*/
    
template< typename T >
CICloneable* 
CTLinkedCloneableObj< T >::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CTLinkedCloneableObj< T >( *this );
}

/*-------------------------------------------------------------------------*/

template< typename T >
const T& 
CTLinkedCloneableObj< T >::GetData( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_data )
    {
        return *m_data;
    }
    
    GUCEF_EMSGTHROW( ENULLPointer, "CTLinkedCloneableObj::GetData(): This operation is impossible without a valid object" );
}

/*-------------------------------------------------------------------------*/

template< typename T >
inline
CTLinkedCloneableObj< T >::operator const T&() const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_data )
    {
        return *m_data;
    }
    
    GUCEF_EMSGTHROW( ENULLPointer, "CTLinkedCloneableObj: operator const T&(): This operation is impossible without a valid object" );
}

/*-------------------------------------------------------------------------*/

template< typename T >
inline
CTLinkedCloneableObj< T >::operator const T*() const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H ? */

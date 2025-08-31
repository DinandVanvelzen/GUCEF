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

#ifndef GUCEF_CORE_CTNUMERICID_H
#define GUCEF_CORE_CTNUMERICID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CINUMERICIDGENERATORBASE_H
#include "CINumericIDGeneratorBase.h"
#define GUCEF_CORE_CINUMERICIDGENERATORBASE_H
#endif /* GUCEF_CORE_CINUMERICIDGENERATORBASE_H ? */

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"   
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#include "ExceptionThrowMacros.h"
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */

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

class CINumericID
{
    public:
};

/*-------------------------------------------------------------------------*/

template < typename intType >
class CTNumericID : public CINumericID
{
    public:

    typedef intType TIntegerTypeUsedForId;

    CTNumericID( void );

    /**
     *  Implementation of a copy constructor which takes a const
     *  but in reality the src will be altered to invalidate it
     *  Only provided as const to allow for certain assignment operations
     */
    CTNumericID( const CTNumericID& src );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CTNumericID( CTNumericID&& src ) GUCEF_NOEXCEPT;
    #endif

    CTNumericID( intType id                              ,
                 CINumericIDGeneratorBasePtr idGenerator );

    ~CTNumericID();

    /**
     *  Implementation of an assignment operator which takes a const
     *  but in reality the src will be altered to invalidate it
     *  Only provided as const to allow for certain assignment operations
     */
    CTNumericID& operator=( const CTNumericID& src );

    bool operator<( const CTNumericID& other ) const;

    bool operator>( const CTNumericID& other ) const;

    bool operator==( const CTNumericID& other ) const;

    bool operator!=( const CTNumericID& other ) const;

    bool IsInitialized( void ) const;

    /**
     *  Conversion operator that allows the numeric ID object to
     *  be used as if it where of type T.
     */
    operator intType() const;

    GUCEF_DEFINE_INLINED_MSGEXCEPTION( ENotInitialized );
    GUCEF_DEFINE_INLINED_MSGEXCEPTION( EIllegalOperation );

    private:

    intType m_id;
    bool m_initialized;
    CINumericIDGeneratorBasePtr m_idGenerator;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >::CTNumericID( void )
    : m_idGenerator() 
    , m_id( 0 )              
    , m_initialized( false )
{GUCEF_TRACE;

    // The ID will have to be initialized before use or an exception will be thrown
}

/*-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >::CTNumericID( const CTNumericID& src )
    : m_idGenerator()  
    , m_id( 0 )              
    , m_initialized( false )
{GUCEF_TRACE;

    CTNumericID& nonConstSrc = const_cast< CTNumericID< intType >& >( src );

    m_idGenerator = nonConstSrc.m_idGenerator;
    m_id = nonConstSrc.m_id;
    m_initialized = nonConstSrc.m_initialized;

    // Assume ownership of the ID and invalidate the source ID
    nonConstSrc.m_initialized = false;
    nonConstSrc.m_idGenerator.Unlink();
    nonConstSrc.m_id = 0;
}

/*-------------------------------------------------------------------------*/
#ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED

template < typename intType >
CTNumericID< intType >::CTNumericID( CTNumericID&& src ) GUCEF_NOEXCEPT
    : m_idGenerator()  
    , m_id( 0 )              
    , m_initialized( false )
{GUCEF_TRACE;

    m_idGenerator = src.m_idGenerator;
    m_id = src.m_id;
    m_initialized = src.m_initialized;

    // Assume ownership of the ID and invalidate the source ID
    src.m_initialized = false;
    src.m_idGenerator.Unlink();
    src.m_id = 0;
}

#endif /* GUCEF_RVALUE_REFERENCES_SUPPORTED ? */
/*-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >::CTNumericID( const intType id                        ,
                                     CINumericIDGeneratorBasePtr idGenerator )
    : m_idGenerator( idGenerator ) ,
      m_id( id )                   ,
      m_initialized( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >::~CTNumericID()
{GUCEF_TRACE;

    if ( m_initialized && !m_idGenerator.IsNULL() )
    {
        m_idGenerator->ReleaseID( this );
    }
}

/*-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >&
CTNumericID< intType >::operator=( const CTNumericID& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        if ( !m_initialized )
        {
            // Copy the attributes
            m_initialized = src.m_initialized;
            m_idGenerator = src.m_idGenerator;
            m_id = src.m_id;

            // Assume ownership of the ID and invalidate the source ID
            CTNumericID& nonConstSrc = const_cast< CTNumericID& >( src );
            nonConstSrc.m_initialized = false;
            nonConstSrc.m_idGenerator.Unlink();
            nonConstSrc.m_id = 0;
        }
        else
        {
            GUCEF_EMSGTHROW( EIllegalOperation, "CTNumericID<>::operator=(): the ID is already initialized" );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template < typename intType >
bool
CTNumericID< intType >::operator<( const CTNumericID& other ) const
{GUCEF_TRACE;

    if ( m_initialized )
    {
        return m_id < other.m_id;
    }
    GUCEF_EMSGTHROW( ENotInitialized, "CTNumericID<>::operator<(): the ID is not initialized" );
}

/*-------------------------------------------------------------------------*/

template < typename intType >
bool
CTNumericID< intType >::operator>( const CTNumericID& other ) const
{GUCEF_TRACE;

    if ( m_initialized )
    {
        return m_id > other.m_id;
    }
    GUCEF_EMSGTHROW( ENotInitialized, "CTNumericID<>::operator<(): the ID is not initialized" );
}

/*-------------------------------------------------------------------------*/

template < typename intType >
bool
CTNumericID< intType >::operator==( const CTNumericID& other ) const
{GUCEF_TRACE;

    if ( m_initialized )
    {
        return m_id == other.m_id;
    }
    GUCEF_EMSGTHROW( ENotInitialized, "CTNumericID<>::operator==(): the ID is not initialized" );
}

/*-------------------------------------------------------------------------*/

template < typename intType >
bool
CTNumericID< intType >::operator!=( const CTNumericID& other ) const
{GUCEF_TRACE;

    if ( m_initialized )
    {
        return m_id != other.m_id;
    }
    GUCEF_EMSGTHROW( ENotInitialized, "CTNumericID<>::operator!=(): the ID is not initialized" );
}

/*-------------------------------------------------------------------------*/

template < typename intType >
bool
CTNumericID< intType >::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

template < typename intType >
CTNumericID< intType >::operator intType() const
{GUCEF_TRACE;

    if ( m_initialized )
    {
        return m_id;
    }
    GUCEF_EMSGTHROW( ENotInitialized, "CTNumericID<>::operator intType(): the ID is not initialized" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTNUMERICID_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-03-2007 :
        - Dinand: re-added this header

-----------------------------------------------------------------------------*/

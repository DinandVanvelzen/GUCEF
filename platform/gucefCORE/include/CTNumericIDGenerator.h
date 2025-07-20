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

#ifndef GUCEF_CORE_CTNUMERICIDGENERATOR_H
#define GUCEF_CORE_CTNUMERICIDGENERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <limits>
#include <set>
#include <deque>

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"   
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#include "ExceptionThrowMacros.h"
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTNUMERICID_H
#include "CTNumericID.h"
#define GUCEF_CORE_CTNUMERICID_H
#endif /* GUCEF_CORE_CTNUMERICID_H ? */

#ifndef GUCEF_CORE_CINUMERICIDGENERATORBASE_H
#include "CINumericIDGeneratorBase.h"
#define GUCEF_CORE_CINUMERICIDGENERATORBASE_H
#endif /* GUCEF_CORE_CINUMERICIDGENERATORBASE_H ? */

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
 *  class template for a numeric ID generator
 *  The IDs generated per generator are guaranteed to be unique
 *
 *  Note that the generator template is meant to be used with
 *  integer arguments, nothing else.
 */
template < typename intType, class LockType >
class CTNumericIDGenerator : public CINumericIDGeneratorBase ,
                             public CTSharedObjCreator< CTNumericIDGenerator< intType, LockType >, LockType >
{
    public:

    typedef CTNumericID< intType >                                                                              TNumericID;
    typedef typename CTSharedObjCreator< CTNumericIDGenerator< intType, LockType >, LockType >::TSharedPtrType  TNumericIDGeneratorTypedPtr;
    typedef intType                                                                                             TIntegerTypeUsedForId;

    CTNumericIDGenerator( void );

    virtual ~CTNumericIDGenerator();

    TNumericID GenerateID( const bool releaseIDOnDestruction = true );

    virtual void ReleaseID( CINumericID* idObj ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetRecycleCheckThreshold( UInt32 recycleThreshold ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetRecycleCheckThreshold( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    GUCEF_DEFINE_INLINED_MSGEXCEPTION( EMaximumReached );

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTNumericIDGenerator( const CTNumericIDGenerator& src );            /**< not implemented, makes no sense */
    CTNumericIDGenerator& operator=( const CTNumericIDGenerator& src ); /**< not implemented, makes no sense */

    private:

    intType m_lastID;
    intType m_maxValue;
    std::deque< intType > m_availableIDs;
    UInt32 m_recycleThreshold;
    LockType m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename intType, class LockType >
CTNumericIDGenerator< intType, LockType >::CTNumericIDGenerator( void )
    : CINumericIDGeneratorBase()
    , CTSharedObjCreator< CTNumericIDGenerator< intType, LockType >, LockType >( this )
    , m_lastID( 0 )     
    , m_maxValue( std::numeric_limits< intType >::max() )
    , m_recycleThreshold( 512 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
CTNumericIDGenerator< intType, LockType >::~CTNumericIDGenerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
typename CTNumericIDGenerator< intType, LockType >::TNumericID
CTNumericIDGenerator< intType, LockType >::GenerateID( const bool releaseIDOnDestruction /* = true */ )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    // Check against the recycle threshold
    if ( m_availableIDs.size() >= m_recycleThreshold )
    {
        // use a recycled Id instead
        typename CTNumericIDGenerator< intType, LockType >::TNumericID id( m_availableIDs.back(), releaseIDOnDestruction ? CreateSharedPtr() : CINumericIDGeneratorBasePtr() );
        m_availableIDs.pop_back();
        return id;        
    }

    // Check against the max value
    // Note that if T is a signed type max will most likely be a negative value
    // so we cannot use > or < operators
    if ( m_lastID+1 != m_maxValue )
    {
        ++m_lastID;
        typename CTNumericIDGenerator< intType, LockType >::TNumericID id( m_lastID, releaseIDOnDestruction ? CreateSharedPtr() : CINumericIDGeneratorBasePtr() );
        return id;
    }

    // We ran out of numbers we can dish out within the range constraints provided
    // further drain the pool of available ids if possible
    if ( !m_availableIDs.empty() )
    {
        // use a recycled Id instead
        typename CTNumericIDGenerator< intType, LockType >::TNumericID id( m_availableIDs.back(), releaseIDOnDestruction ? CreateSharedPtr() : CINumericIDGeneratorBasePtr() );
        m_availableIDs.pop_back();
        return id;
    }

    // If we get here then we ran out of IDs and we also ran out of options
    GUCEF_EMSGTHROW( EMaximumReached, "GUCEF::CORE::CTNumericIDGenerator: ID maximum reached, no more IDs are available" );
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
void
CTNumericIDGenerator< intType, LockType >::ReleaseID( CINumericID* idObj )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    TNumericID* id = static_cast< TNumericID* >( idObj );
    m_availableIDs.push_front( *id );
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
void
CTNumericIDGenerator< intType, LockType >::SetRecycleCheckThreshold( UInt32 recycleThreshold )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_recycleThreshold = recycleThreshold;
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
UInt32
CTNumericIDGenerator< intType, LockType >::GetRecycleCheckThreshold( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this );
    UInt32 threshold = m_recycleThreshold;
    lock.EarlyReaderUnlock();
    return threshold;
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
const MT::CILockable*
CTNumericIDGenerator< intType, LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
MT::TLockStatus
CTNumericIDGenerator< intType, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock();
}

/*-------------------------------------------------------------------------*/

template < typename intType, class LockType >
MT::TLockStatus
CTNumericIDGenerator< intType, LockType >::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTNUMERICIDGENERATOR_H ? */

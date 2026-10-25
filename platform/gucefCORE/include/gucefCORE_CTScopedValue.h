/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTSCOPEDVALUE_H
#define GUCEF_CORE_CTSCOPEDVALUE_H

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
 *  Gives a variable a value for the time of a scope and puts back what it had when the scope ends, the way CScopeMutex
 *  holds a lock for one. For the flags that say "busy" around a block of code (a guard against reentrancy for example):
 *
 *      if ( m_raising )
 *          return;
 *      {
 *          CTScopedValue< bool > raising( m_raising, true );
 *          <do stuff that can come back here, return early or throw>
 *      }
 *
 *  The value is restored at the end of the block, on a return from it and when an exception unwinds it. What is put back
 *  is what the variable had when the scope began, so scopes of the same variable nest.
 *  The variable must outlive the scope. Not for variables other threads read or write: the scope does not lock.
 *  It is not for a state that outlasts the block, such as a mode that starts at one event and ends at another.
 */
template< typename T >
class CTScopedValue
{
    public:

    /**
     *  @param variable     the variable that has the value for the time of the scope
     *  @param scopedValue  the value it has until the scope ends
     */
    CTScopedValue( T& variable, const T& scopedValue )
        : m_variable( variable )
        , m_previousValue( variable )
    {
        m_variable = scopedValue;
    }

    /** Puts back the value the variable had */
    ~CTScopedValue()
    {
        m_variable = m_previousValue;
    }

    /** The value the variable had before the scope */
    const T& GetPreviousValue( void ) const
    {
        return m_previousValue;
    }

    private:

    CTScopedValue( const CTScopedValue& src );              /* Copying doesnt make sense */
    CTScopedValue& operator=( const CTScopedValue& src );   /* Copying doesnt make sense */

    T& m_variable;
    T m_previousValue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTSCOPEDVALUE_H ? */

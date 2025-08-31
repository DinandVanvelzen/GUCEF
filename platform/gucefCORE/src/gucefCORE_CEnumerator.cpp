/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CENUMERABLE_H
#include "gucefCORE_CIEnumerable.h"
#define GUCEF_CORE_CENUMERABLE_H
#endif /* GUCEF_CORE_CENUMERABLE_H ? */

#include "gucefCORE_CEnumerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConstEnumerator::CConstEnumerator( void )
    : m_enumerable( GUCEF_NULL )
    , m_enumeratorData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConstEnumerator::CConstEnumerator( const CConstEnumerator& src )
    : m_enumerable( src.m_enumerable )
    , m_enumeratorData( src.m_enumeratorData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConstEnumerator::CConstEnumerator( const CIEnumerable* enumerable ,
                                    const CVariant& enumeratorData )
    : m_enumerable( const_cast< CIEnumerable* >( enumerable ) )   // <- constness it maintained via the type of operations this class exposes
    , m_enumeratorData( enumeratorData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConstEnumerator::~CConstEnumerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConstEnumerator&
CConstEnumerator::operator=( const CConstEnumerator& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_enumerable = src.m_enumerable;
        m_enumeratorData = src.m_enumeratorData;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CConstEnumerator& 
CConstEnumerator::operator++( void )
{GUCEF_TRACE;

    if ( !MoveNext() )
    {
        
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CConstEnumerator& 
CConstEnumerator::operator--( void )
{GUCEF_TRACE;

    if ( !MovePrev() )
    {
        
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator<( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 > Compare( other );
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator<=( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 >= Compare( other );
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator>( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 < Compare( other );
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator>=( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 <= Compare( other );
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator==( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 == Compare( other );
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::operator!=( const CConstEnumerator& other )
{GUCEF_TRACE;

    return 0 != Compare( other );
}

/*-------------------------------------------------------------------------*/

UInt8
CConstEnumerator::GetTypeOfCurrent( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetTypeOfCurrent( m_enumeratorData );
    return GUCEF_DATATYPE_UNKNOWN;
}
    
/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::GetCurrent( CVariant& value, bool linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetCurrent( m_enumeratorData, value, linkIfPossible );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::GetCurrent( const CIEnumerable** enumerable )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetCurrent( m_enumeratorData, enumerable );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::GetIdOfCurrent( CVariant& value, bool linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetIdOfCurrent( m_enumeratorData, value, linkIfPossible );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::GetNameOfCurrent( CVariant& value, bool linkIfPossible )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetNameOfCurrent( m_enumeratorData, value, linkIfPossible );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::CanEnumerateForward( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->CanEnumerateForward( m_enumeratorData );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::CanEnumerateBackward( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->CanEnumerateBackward( m_enumeratorData );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CConstEnumerator::MoveNext( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->MoveNext( m_enumeratorData );
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CConstEnumerator::MovePrev( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->MovePrev( m_enumeratorData );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConstEnumerator::IsAtEnd( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->IsAtEnd( m_enumeratorData );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32 
CConstEnumerator::Compare( const CConstEnumerator& other ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
    {
        if ( m_enumerable == other.m_enumerable )
        {
            return m_enumerable->Compare( m_enumeratorData, other.m_enumeratorData );
        }
        else
        {
            return memcmp( (const void*) m_enumerable, (const void*) other.m_enumerable, sizeof( CIEnumerable* ) );
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/
//
//CConstEnumerator::CStyleAccess()
//{GUCEF_TRACE;
//
//}

/*-------------------------------------------------------------------------*/

CEnumerator::CEnumerator( void )
    : CConstEnumerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEnumerator::CEnumerator( CIEnumerable* enumerable       ,
                          const CVariant& enumeratorData )
    : CConstEnumerator( enumerable     ,
                        enumeratorData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEnumerator::CEnumerator( const CEnumerator& src )
    : CConstEnumerator( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEnumerator::~CEnumerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEnumerator&
CEnumerator::operator=( const CEnumerator& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CConstEnumerator::operator=( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CEnumerator& 
CEnumerator::operator++( void )
{GUCEF_TRACE;

    CConstEnumerator::operator++();
    return *this;
}

/*-------------------------------------------------------------------------*/

CEnumerator& 
CEnumerator::operator--( void )
{GUCEF_TRACE;

    CConstEnumerator::operator--();
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator<( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator<( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator<=( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator<=( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator>( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator>( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator>=( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator>=( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator==( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator==( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::operator!=( const CEnumerator& other )
{GUCEF_TRACE;

    return CConstEnumerator::operator!=( other );
}

/*-------------------------------------------------------------------------*/

bool
CEnumerator::GetCurrent( CIEnumerable** enumerable )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_enumerable )
        return m_enumerable->GetCurrent( m_enumeratorData, enumerable );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

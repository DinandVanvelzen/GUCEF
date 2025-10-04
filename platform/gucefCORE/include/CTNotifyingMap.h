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

#ifndef GUCEF_CORE_CTNOTIFYINGMAP_H
#define GUCEF_CORE_CTNOTIFYINGMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"                          /* notifier component of the notifier-observer design pattern */
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"                     /* template for easy definition of cloneable interfaces on objects */
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CNOTIFYINGMAPEVENTS_H
#include "CNotifyingMapEvents.h"                /* base class defining the events used by the template instantiations */
#define GUCEF_CORE_CNOTIFYINGMAPEVENTS_H
#endif /* GUCEF_CORE_CNOTIFYINGMAPEVENTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class wrapping a std::map and adding notification to it's functionality
 *
 *  Note that not all mutators available on an std::map can be used on this template
 *  because they would prevent notification to occur in a reliable manner
 */
template< class indexType, class mappedType >
class CTNotifyingMap : public CNotifier           ,
                       public CNotifyingMapEvents
{
    public:

    typedef indexType key_type;       /**< 	The map's key type, Key. */
    typedef mappedType data_type;     /**< The type of object associated with the keys. */

    typedef typename GUCEF::map< indexType, mappedType > TWrappedMap;
    typedef typename TWrappedMap::value_type value_type;
    typedef typename TWrappedMap::const_iterator const_iterator;
    typedef typename TWrappedMap::const_reverse_iterator const_reverse_iterator;
    typedef typename TWrappedMap::_Pairib _Pairib;
    typedef typename TWrappedMap::size_type size_type;

    typedef CTCloneableObj< const_iterator > TItemModifiedEventData;


    CTNotifyingMap( void );

    CTNotifyingMap( const CTNotifyingMap& src );

    ~CTNotifyingMap();

    const_iterator find( const key_type& indexValue ) const;

    const_iterator operator[]( const key_type& indexValue ) const;

    size_type count( const key_type& k );

    void erase( const key_type& indexValue );

    void erase( const const_iterator& itemPos );

    void insert( const key_type& indexValue ,
                 const data_type& dataValue );

    void insert( const const_iterator& itemPos ,
                 const key_type& indexValue    ,
                 const data_type& dataValue    );

    bool modify( const key_type& indexValue ,
                 const data_type& dataValue );

    const_iterator begin( void ) const;

    const_iterator end( void ) const;

    const_reverse_iterator rbegin( void ) const;

    const_reverse_iterator rend( void ) const;

    bool empty( void ) const;

    void clear( void );

    size_type max_size( void ) const;

    virtual CString GetType( void ) const;

    private:

    GUCEF::map< indexType, mappedType > m_map;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
CTNotifyingMap< indexType, mappedType >::CTNotifyingMap( void )
    : m_map()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
CTNotifyingMap< indexType, mappedType >::CTNotifyingMap( const CTNotifyingMap< indexType, mappedType >& src )
    : m_map( src.m_map )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
CTNotifyingMap< indexType, mappedType >::~CTNotifyingMap()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_iterator
CTNotifyingMap< indexType, mappedType >::find( const key_type& indexValue ) const
{GUCEF_TRACE;
    return m_map.find( indexValue );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_iterator
CTNotifyingMap< indexType, mappedType >::operator[]( const key_type& indexValue ) const
{GUCEF_TRACE;
    return m_map.find( indexValue );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::size_type
CTNotifyingMap< indexType, mappedType >::count( const key_type& k )
{GUCEF_TRACE;
    return m_map.count( k );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
void
CTNotifyingMap< indexType, mappedType >::erase( const key_type& indexValue )
{GUCEF_TRACE;

    m_map.erase( indexValue );
    NotifyObservers( ItemErasedEvent );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
void
CTNotifyingMap< indexType, mappedType >::erase( const const_iterator& itemPos )
{GUCEF_TRACE;

    m_map.erase( itemPos );
    NotifyObservers( ItemErasedEvent );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
void
CTNotifyingMap< indexType, mappedType >::insert( const const_iterator& itemPos ,
                                                 const key_type& indexValue    ,
                                                 const data_type& dataValue    )
{GUCEF_TRACE;

    m_map.insert( itemPos, _Pairib( indexValue, dataValue ) );
    NotifyObservers( ItemAddedEvent );
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
void
CTNotifyingMap< indexType, mappedType >::insert( const key_type& indexValue ,
                                                 const data_type& dataValue )
{GUCEF_TRACE;

    const_iterator i( m_map.find( indexValue ) );
    if ( i != m_map.end() )
    {
        ( *i ).second = dataValue;
        NotifyObservers( ItemModifiedEvent, TItemModifiedEventData( i ) );
    }
    else
    {
        m_map.insert( indexValue, dataValue );
        NotifyObservers( ItemAddedEvent );
    }
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
bool
CTNotifyingMap< indexType, mappedType >::modify( const key_type& indexValue ,
                                                 const data_type& dataValue )
{GUCEF_TRACE;

    const_iterator i( m_map.find( indexValue ) );
    if ( i != m_map.end() )
    {
        ( *i ).second = dataValue;
        NotifyObservers( ItemModifiedEvent, TItemModifiedEventData( i ) );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_iterator
CTNotifyingMap< indexType, mappedType >::begin( void ) const
{GUCEF_TRACE;
    return m_map.begin();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_iterator
CTNotifyingMap< indexType, mappedType >::end( void ) const
{GUCEF_TRACE;
    return m_map.end();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_reverse_iterator
CTNotifyingMap< indexType, mappedType >::rbegin( void ) const
{GUCEF_TRACE;
    return m_map.rbegin();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::const_reverse_iterator
CTNotifyingMap< indexType, mappedType >::rend( void ) const
{GUCEF_TRACE;
    return m_map.rend();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
bool
CTNotifyingMap< indexType, mappedType >::empty( void ) const
{GUCEF_TRACE;
    return m_map.empty();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
void
CTNotifyingMap< indexType, mappedType >::clear( void )
{GUCEF_TRACE;
    m_map.clear();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
typename CTNotifyingMap< indexType, mappedType >::size_type
CTNotifyingMap< indexType, mappedType >::max_size( void ) const
{GUCEF_TRACE;
    return m_map.max_size();
}

/*-------------------------------------------------------------------------*/

template< class indexType, class mappedType >
CString
CTNotifyingMap< indexType, mappedType >::GetType( void ) const
{
    return "GUCEF::CORE::CTNotifyingMap< indexType, mappedType >";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTNOTIFYINGMAP_H ? */

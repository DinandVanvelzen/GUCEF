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

#ifndef GUCEF_CORE_CDATETIMERANGE_H
#define GUCEF_CORE_CDATETIMERANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

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
 *  Class representing a range of time between two CDateTime instances
 */
class GUCEF_CORE_PUBLIC_CPP CDateTimeRange
{
    public:

    static const CDateTimeRange Empty;

    CDateTimeRange( void );

    CDateTimeRange( const CDateTime& start, const CDateTime& end );

    CDateTimeRange( const CDateTimeRange& src );

    ~CDateTimeRange();

    CDateTimeRange& operator=( const CDateTimeRange& src );

    bool operator==( const CDateTimeRange& other ) const;

    bool operator!=( const CDateTimeRange& other ) const;

    bool operator<( const CDateTimeRange& other ) const;

    bool operator<=( const CDateTimeRange& other ) const;

    bool operator>( const CDateTimeRange& other ) const;

    bool operator>=( const CDateTimeRange& other ) const;

    bool operator<( const CDateTime& dt ) const;

    bool operator<=( const CDateTime& dt ) const;

    bool operator>( const CDateTime& dt ) const;

    bool operator>=( const CDateTime& dt ) const;

    void SetStart( const CDateTime& start );

    const CDateTime& GetStart( void ) const;

    CDateTime& GetStart( void );

    void SetEnd( const CDateTime& end );

    const CDateTime& GetEnd( void ) const;

    CDateTime& GetEnd( void );

    bool HasRangeBegun( void ) const;

    bool HasRangeEnded( void ) const;

    UInt64 TimeTillStartInMilliSecs( void ) const;

    UInt64 TimeTillEndInMilliSecs( void ) const;

    UInt64 GetTimeRangeTimeInMilliSecs( void ) const;

    void Clear( void );

    private:

    CDateTime m_start;
    CDateTime m_end;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATETIMERANGE_H ? */

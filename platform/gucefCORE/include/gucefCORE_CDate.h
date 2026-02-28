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

#ifndef GUCEF_CORE_CDATE_H
#define GUCEF_CORE_CDATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIDATE_H
#include "gucefCORE_CIDate.h"
#define GUCEF_CORE_CIDATE_H
#endif /* GUCEF_CORE_CIDATE_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CDate : public CIDate
{
    public:

    enum EDateMonths
    {
        DATEMONTHS_UNKNOWN = 0,

        DATEMONTHS_JANUARY,
        DATEMONTHS_FEBRUARY,
        DATEMONTHS_MARCH,
        DATEMONTHS_APRIL,
        DATEMONTHS_MAY,
        DATEMONTHS_JUNE,
        DATEMONTHS_JULY,
        DATEMONTHS_AUGUST,
        DATEMONTHS_SEPTEMBER,
        DATEMONTHS_OCTOBER,
        DATEMONTHS_NOVEMBER,
        DATEMONTHS_DECEMBER
    };
    typedef enum EDateMonths TDateMonths;

    static CDate TodaysLocalDate( void );

    static CDate TodaysUTCDate( void );

    static CDate UnixEpochDate( void );

    bool SetYear( Int16 year );

    virtual Int16 GetYear( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetMonth( UInt8 month );
    
    virtual UInt8 GetMonth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetMonthName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool SetDay( UInt8 day );
    
    virtual UInt8 GetDay( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool IsLeapYear( void ) const;

    UInt8 GetDaysInMonth( void ) const;

    /**
     *  Returns the day of week for this date.
     *  0 = Sunday, 1 = Monday, ..., 6 = Saturday
     *  Uses the Tomohiko Sakamoto algorithm.
     */
    UInt8 GetDayOfWeek( void ) const;

    CDate& operator=( const CDate& src );

    bool operator==( const CDate& src ) const;

    bool operator!=( const CDate& src ) const;

    CDate( void );
    CDate( const time_t& src, bool isUtc );
    CDate( UInt16 year, UInt8 month, UInt8 day );
    CDate( const CDate& src );
    virtual ~CDate();

    virtual CString ToIso8601DateString( bool includeDelimeters ) const;
    
    protected:

    Int16 m_year;
    UInt8 m_month;
    UInt8 m_day;
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CDate& value ) { return value.ToIso8601DateString( true ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATE_H ? */

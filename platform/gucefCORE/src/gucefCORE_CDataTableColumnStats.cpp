/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CDataTableColumnStats.h"

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

CDataTableColumnStats::CDataTableColumnStats( void )
    : hasMinMax( false )
    , minValue()
    , maxValue()
    , minExact( false )
    , maxExact( false )
    , hasNullCount( false )
    , nullCount( 0 )
    , hasNanCount( false )
    , nanCount( 0 )
    , valueCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDataTableColumnStats::Clear( void )
{GUCEF_TRACE;

    hasMinMax = false;
    minValue.Clear();
    maxValue.Clear();
    minExact = false;
    maxExact = false;
    hasNullCount = false;
    nullCount = 0;
    hasNanCount = false;
    nanCount = 0;
    valueCount = 0;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnStats::IsAllNull( void ) const
{GUCEF_TRACE;

    return hasNullCount && nullCount == valueCount;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableColumnStats::operator==( const CDataTableColumnStats& other ) const
{GUCEF_TRACE;

    if ( hasMinMax != other.hasMinMax || hasNullCount != other.hasNullCount || valueCount != other.valueCount )
        return false;
    if ( hasNullCount && nullCount != other.nullCount )
        return false;
    if ( hasNanCount != other.hasNanCount || ( hasNanCount && nanCount != other.nanCount ) )
        return false;
    if ( hasMinMax )
    {
        if ( minExact != other.minExact || maxExact != other.maxExact )
            return false;
        if ( minValue.GetTypeId() != other.minValue.GetTypeId() || maxValue.GetTypeId() != other.maxValue.GetTypeId() )
            return false;
        if ( minValue != other.minValue || maxValue != other.maxValue )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

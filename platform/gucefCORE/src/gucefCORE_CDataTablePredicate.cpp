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

#include "gucefCORE_CDataTablePredicate.h"

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

CDataTablePredicate::CDataTablePredicate( void )
    : columnName()
    , op( DATATABLE_PREDICATE_EQUALS )
    , low()
    , high()
    , lowInclusive( true )
    , highInclusive( true )
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePredicate::CDataTablePredicate( const CString& column, TDataTablePredicateOp predicateOp, const CVariant& operand )
    : columnName( column )
    , op( predicateOp )
    , low( operand )
    , high()
    , lowInclusive( true )
    , highInclusive( true )
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePredicate::CDataTablePredicate( const CString& column, const CVariant& rangeLow, bool isLowInclusive, const CVariant& rangeHigh, bool isHighInclusive )
    : columnName( column )
    , op( DATATABLE_PREDICATE_RANGE )
    , low( rangeLow )
    , high( rangeHigh )
    , lowInclusive( isLowInclusive )
    , highInclusive( isHighInclusive )
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePredicate::CDataTablePredicate( const CString& column, const CVariant::VariantVector& setValues )
    : columnName( column )
    , op( DATATABLE_PREDICATE_IN )
    , low()
    , high()
    , lowInclusive( true )
    , highInclusive( true )
    , values( setValues )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataTablePredicate::IsWellFormed( CString& error ) const
{GUCEF_TRACE;

    if ( columnName.IsNULLOrEmpty() )
    {
        error = "A predicate names no column";
        return false;
    }
    switch ( op )
    {
        case DATATABLE_PREDICATE_EQUALS:
        case DATATABLE_PREDICATE_NOT_EQUALS:
        case DATATABLE_PREDICATE_LESS:
        case DATATABLE_PREDICATE_LESS_OR_EQUAL:
        case DATATABLE_PREDICATE_GREATER:
        case DATATABLE_PREDICATE_GREATER_OR_EQUAL:
        {
            if ( !low.IsInitialized() || low.IsNULL() )
            {
                error = "Predicate " + CString( DataTablePredicateOpToString( op ) ) + " on \"" + columnName + "\" has no operand";
                return false;
            }
            return true;
        }
        case DATATABLE_PREDICATE_RANGE:
        {
            if ( ( !low.IsInitialized() || low.IsNULL() ) && ( !high.IsInitialized() || high.IsNULL() ) )
            {
                error = "Range on \"" + columnName + "\" has neither bound";
                return false;
            }
            return true;
        }
        case DATATABLE_PREDICATE_IN:
        {
            if ( values.empty() )
            {
                error = "IN on \"" + columnName + "\" has no values";
                return false;
            }
            for ( UInt32 i=0; i<(UInt32) values.size(); ++i )
            {
                if ( !values[ i ].IsInitialized() || values[ i ].IsNULL() )
                {
                    error = "IN on \"" + columnName + "\" holds a null, which never matches: use IS_NULL";
                    return false;
                }
            }
            return true;
        }
        case DATATABLE_PREDICATE_IS_NULL:
        case DATATABLE_PREDICATE_IS_NOT_NULL:
        {
            return true;
        }
        default:
        {
            error = "Predicate on \"" + columnName + "\" has an unknown operator";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

CDataTableResolvedPredicate::CDataTableResolvedPredicate( void )
    : columnIndex( 0 )
    , op( DATATABLE_PREDICATE_EQUALS )
    , hasLow( false )
    , hasHigh( false )
    , low()
    , high()
    , lowInclusive( true )
    , highInclusive( true )
    , values()
    , matchesNothing( false )
    , matchesAllNonNull( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

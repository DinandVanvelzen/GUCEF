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

#ifndef GUCEF_CORE_CDATATABLEPREDICATE_H
#define GUCEF_CORE_CDATATABLEPREDICATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  One condition on one column. The predicates of a query are AND'ed; a disjunction on one column is IN.
 *
 *  Comparisons follow SQL three valued logic: a null cell never matches a comparison or set test, NOT_EQUALS
 *  included; IS_NULL and IS_NOT_NULL test nulls. Comparisons with NaN are false. Text and binary compare bytewise,
 *  unsigned, lexicographically.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTablePredicate
{
    public:

    CString columnName;
    TDataTablePredicateOp op;           /**< EQUALS, NOT_EQUALS, LESS, LESS_OR_EQUAL, GREATER, GREATER_OR_EQUAL, RANGE, IN, IS_NULL, IS_NOT_NULL */
    CVariant low;                       /**< RANGE: CVariant::Empty for unbounded; the single operand of the comparisons */
    CVariant high;                      /**< RANGE: CVariant::Empty for unbounded */
    bool lowInclusive;                  /**< RANGE */
    bool highInclusive;                 /**< RANGE */
    CVariant::VariantVector values;     /**< IN */

    CDataTablePredicate( void );

    /** A predicate with one operand: the comparisons, or IS_NULL / IS_NOT_NULL with an empty operand */
    CDataTablePredicate( const CString& column, TDataTablePredicateOp predicateOp, const CVariant& operand = CVariant::Empty );

    /** A RANGE predicate */
    CDataTablePredicate( const CString& column, const CVariant& rangeLow, bool isLowInclusive, const CVariant& rangeHigh, bool isHighInclusive );

    /** An IN predicate */
    CDataTablePredicate( const CString& column, const CVariant::VariantVector& setValues );

    /** Operands present for the operator: one for the comparisons, a bound for RANGE, a value for IN */
    bool IsWellFormed( CString& error ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  A predicate resolved against a schema by CDataTableQuery::Resolve: the column index and the literals in the native
 *  form of the column (section 5.2). Literals that cannot be represented exactly keep the predicate's meaning: lower
 *  bounds round up and upper bounds round down with the inclusive flags adjusted; an EQUALS with such a literal
 *  matches nothing and a NOT_EQUALS matches every non-null row.
 *
 *  Native forms: Int64 for signed integers, decimal mantissas at the column scale, date day counts and timestamp ticks
 *  in the column unit; UInt64 for unsigned integers; Float64 for floats; UTF8 string bytes for text; a binary blob for
 *  binary; BOOLEAN_INT32 for booleans.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableResolvedPredicate
{
    public:

    UInt32 columnIndex;                 /**< schema column */
    TDataTablePredicateOp op;
    bool hasLow;                        /**< RANGE, GREATER, GREATER_OR_EQUAL: a lower bound is set */
    bool hasHigh;                       /**< RANGE, LESS, LESS_OR_EQUAL: an upper bound is set */
    CVariant low;                       /**< lower bound, or the operand of EQUALS / NOT_EQUALS */
    CVariant high;                      /**< upper bound */
    bool lowInclusive;
    bool highInclusive;
    CVariant::VariantVector values;     /**< IN: sorted ascending and without duplicates */
    bool matchesNothing;                /**< no row can match, for example EQUALS with a literal the column cannot hold */
    bool matchesAllNonNull;             /**< every non-null row matches, for example NOT_EQUALS with such a literal */

    CDataTableResolvedPredicate( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEPREDICATE_H ? */

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

#include "gucefCORE_DataTableTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Looks the text up in a table of names indexed by enum value, exact match */
bool
LookupName( const CString& text, const char* const* names, UInt32 nameCount, UInt32& index )
{
    for ( UInt32 i=0; i<nameCount; ++i )
    {
        if ( text == names[ i ] )
        {
            index = i;
            return true;
        }
    }
    return false;
}

const char* const g_logicalTypeNames[] =
{
    "unsupported", "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64",
    "float32", "float64", "decimal", "date", "timestamp", "utf8", "binary"
};
const UInt32 g_logicalTypeCount = sizeof( g_logicalTypeNames ) / sizeof( g_logicalTypeNames[ 0 ] );

const char* const g_timeUnitNames[] = { "ms", "us", "ns" };
const char* const g_transformNames[] = { "identity", "year", "month", "day" };
const char* const g_encodingHintNames[] = { "auto", "plain", "dictionary" };
const char* const g_dictionaryModeNames[] = { "auto", "always", "never" };
const char* const g_writeModeNames[] = { "insert", "upsert", "replacePartitions" };
const char* const g_filterModeNames[] = { "exact", "pruneOnly" };
const char* const g_predicateOpNames[] = { "equals", "notEquals", "less", "lessOrEqual", "greater", "greaterOrEqual", "range", "in", "isNull", "isNotNull" };

}

/*-------------------------------------------------------------------------*/

const char*
DataTableStatusToString( TDataTableStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DATATABLE_STATUS_OK:               return "OK";
        case DATATABLE_STATUS_END:              return "END";
        case DATATABLE_STATUS_PENDING:          return "PENDING";
        case DATATABLE_STATUS_PRUNED:           return "PRUNED";
        case DATATABLE_STATUS_FAILED:           return "FAILED";
        case DATATABLE_STATUS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case DATATABLE_STATUS_NOT_FOUND:        return "NOT_FOUND";
        case DATATABLE_STATUS_ALREADY_EXISTS:   return "ALREADY_EXISTS";
        case DATATABLE_STATUS_SCHEMA_MISMATCH:  return "SCHEMA_MISMATCH";
        case DATATABLE_STATUS_TYPE_MISMATCH:    return "TYPE_MISMATCH";
        case DATATABLE_STATUS_CONFLICT:         return "CONFLICT";
        case DATATABLE_STATUS_CORRUPT_DATA:     return "CORRUPT_DATA";
        case DATATABLE_STATUS_UNSUPPORTED:      return "UNSUPPORTED";
        case DATATABLE_STATUS_IO_ERROR:         return "IO_ERROR";
        case DATATABLE_STATUS_READ_ONLY:        return "READ_ONLY";
        case DATATABLE_STATUS_CANCELLED:        return "CANCELLED";
        case DATATABLE_STATUS_OUT_OF_MEMORY:    return "OUT_OF_MEMORY";
        case DATATABLE_STATUS_LIMIT_EXCEEDED:   return "LIMIT_EXCEEDED";
        default:                                return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

const char*
DataTableLogicalTypeToString( TDataTableLogicalType type )
{GUCEF_TRACE;

    if ( (UInt32) type < g_logicalTypeCount )
        return g_logicalTypeNames[ type ];
    return g_logicalTypeNames[ DATATABLE_TYPE_UNSUPPORTED ];
}

/*-------------------------------------------------------------------------*/

bool
DataTableLogicalTypeFromString( const CString& text, TDataTableLogicalType& type )
{GUCEF_TRACE;

    UInt32 index = 0;
    if ( LookupName( text, g_logicalTypeNames, g_logicalTypeCount, index ) )
    {
        type = (TDataTableLogicalType) index;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
DataTableLogicalTypeByteWidth( TDataTableLogicalType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case DATATABLE_TYPE_INT8:
        case DATATABLE_TYPE_UINT8:      return 1;
        case DATATABLE_TYPE_INT16:
        case DATATABLE_TYPE_UINT16:     return 2;
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_FLOAT32:
        case DATATABLE_TYPE_DATE:       return 4;
        case DATATABLE_TYPE_INT64:
        case DATATABLE_TYPE_UINT64:
        case DATATABLE_TYPE_FLOAT64:
        case DATATABLE_TYPE_DECIMAL:
        case DATATABLE_TYPE_TIMESTAMP:  return 8;
        default:                        return 0;
    }
}

/*-------------------------------------------------------------------------*/

TDataTableStorage
DataTableLogicalTypeStorage( TDataTableLogicalType type )
{GUCEF_TRACE;

    switch ( type )
    {
        case DATATABLE_TYPE_BOOL:           return DATATABLE_STORAGE_BITS;
        case DATATABLE_TYPE_UTF8:
        case DATATABLE_TYPE_BINARY:         return DATATABLE_STORAGE_VARIABLE;
        case DATATABLE_TYPE_UNSUPPORTED:    return DATATABLE_STORAGE_NONE;
        default:                            return 0 != DataTableLogicalTypeByteWidth( type ) ? DATATABLE_STORAGE_FIXED : DATATABLE_STORAGE_NONE;
    }
}

/*-------------------------------------------------------------------------*/

bool
DataTableLogicalTypeIsInteger( TDataTableLogicalType type )
{GUCEF_TRACE;

    return type >= DATATABLE_TYPE_INT8 && type <= DATATABLE_TYPE_UINT64;
}

/*-------------------------------------------------------------------------*/

bool
DataTableLogicalTypeIsUnsigned( TDataTableLogicalType type )
{GUCEF_TRACE;

    return type >= DATATABLE_TYPE_UINT8 && type <= DATATABLE_TYPE_UINT64;
}

/*-------------------------------------------------------------------------*/

#define GUCEF_DATATABLE_ENUM_TEXT( TypeName, EnumType, names )                                                     \
    const char* DataTable##TypeName##ToString( EnumType value )                                                     \
    {GUCEF_TRACE;                                                                                                   \
        const UInt32 count = sizeof( names ) / sizeof( names[ 0 ] );                                               \
        return (UInt32) value < count ? names[ value ] : "";                                                        \
    }                                                                                                               \
    bool DataTable##TypeName##FromString( const CString& text, EnumType& value )                                    \
    {GUCEF_TRACE;                                                                                                   \
        UInt32 index = 0;                                                                                           \
        if ( !LookupName( text, names, sizeof( names ) / sizeof( names[ 0 ] ), index ) )                           \
            return false;                                                                                           \
        value = (EnumType) index;                                                                                   \
        return true;                                                                                                \
    }

GUCEF_DATATABLE_ENUM_TEXT( TimeUnit, TDataTableTimeUnit, g_timeUnitNames )
GUCEF_DATATABLE_ENUM_TEXT( PartitionTransform, TDataTablePartitionTransform, g_transformNames )
GUCEF_DATATABLE_ENUM_TEXT( EncodingHint, TDataTableEncodingHint, g_encodingHintNames )
GUCEF_DATATABLE_ENUM_TEXT( DictionaryMode, TDataTableDictionaryMode, g_dictionaryModeNames )
GUCEF_DATATABLE_ENUM_TEXT( WriteMode, TDataTableWriteMode, g_writeModeNames )
GUCEF_DATATABLE_ENUM_TEXT( FilterMode, TDataTableFilterMode, g_filterModeNames )
GUCEF_DATATABLE_ENUM_TEXT( PredicateOp, TDataTablePredicateOp, g_predicateOpNames )

#undef GUCEF_DATATABLE_ENUM_TEXT

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

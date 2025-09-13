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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#include "gucefCORE_c_variantdata.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* 
TypeNameForGucefTypeId( UInt8 typeId )
{
    switch ( typeId )
    {
        case GUCEF_DATATYPE_UINT8:                          return GUCEF_DATATYPE_NAME_UINT8;
        case GUCEF_DATATYPE_INT8:                           return GUCEF_DATATYPE_NAME_INT8;
        case GUCEF_DATATYPE_LE_UINT16:                      return GUCEF_DATATYPE_NAME_LE_UINT16;
        case GUCEF_DATATYPE_BE_UINT16:                      return GUCEF_DATATYPE_NAME_BE_UINT16;
        case GUCEF_DATATYPE_LE_INT16:                       return GUCEF_DATATYPE_NAME_LE_INT16;
        case GUCEF_DATATYPE_BE_INT16:                       return GUCEF_DATATYPE_NAME_BE_INT16;
        case GUCEF_DATATYPE_LE_UINT32:                      return GUCEF_DATATYPE_NAME_LE_UINT32;
        case GUCEF_DATATYPE_BE_UINT32:                      return GUCEF_DATATYPE_NAME_BE_UINT32;
        case GUCEF_DATATYPE_LE_INT32:                       return GUCEF_DATATYPE_NAME_LE_INT32;
        case GUCEF_DATATYPE_BE_INT32:                       return GUCEF_DATATYPE_NAME_BE_INT32;
        case GUCEF_DATATYPE_LE_UINT64:                      return GUCEF_DATATYPE_NAME_LE_UINT64;
        case GUCEF_DATATYPE_BE_UINT64:                      return GUCEF_DATATYPE_NAME_BE_UINT64;
        case GUCEF_DATATYPE_LE_INT64:                       return GUCEF_DATATYPE_NAME_LE_INT64;
        case GUCEF_DATATYPE_BE_INT64:                       return GUCEF_DATATYPE_NAME_BE_INT64;
        case GUCEF_DATATYPE_LE_FLOAT32:                     return GUCEF_DATATYPE_NAME_LE_FLOAT32;
        case GUCEF_DATATYPE_BE_FLOAT32:                     return GUCEF_DATATYPE_NAME_BE_FLOAT32;
        case GUCEF_DATATYPE_LE_FLOAT64:                     return GUCEF_DATATYPE_NAME_LE_FLOAT64;
        case GUCEF_DATATYPE_BE_FLOAT64:                     return GUCEF_DATATYPE_NAME_BE_FLOAT64;
        case GUCEF_DATATYPE_NUMERIC:                        return GUCEF_DATATYPE_NAME_NUMERIC;
        case GUCEF_DATATYPE_ASCII_STRING:                   return GUCEF_DATATYPE_NAME_ASCII_STRING;
        case GUCEF_DATATYPE_UTF8_STRING:                    return GUCEF_DATATYPE_NAME_UTF8_STRING;
        case GUCEF_DATATYPE_UTF16_LE_STRING:                return GUCEF_DATATYPE_NAME_UTF16_LE_STRING;
        case GUCEF_DATATYPE_UTF16_BE_STRING:                return GUCEF_DATATYPE_NAME_UTF16_BE_STRING;
        case GUCEF_DATATYPE_UTF32_LE_STRING:                return GUCEF_DATATYPE_NAME_UTF32_LE_STRING;
        case GUCEF_DATATYPE_UTF32_BE_STRING:                return GUCEF_DATATYPE_NAME_UTF32_BE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_INT32:                  return GUCEF_DATATYPE_NAME_BOOLEAN_INT32;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:           return GUCEF_DATATYPE_NAME_BOOLEAN_ASCII_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF8_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING:        return GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_LE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING:        return GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_BE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING:        return GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_LE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING:        return GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_BE_STRING;
        case GUCEF_DATATYPE_BINARY_BLOB:                    return GUCEF_DATATYPE_NAME_BINARY_BLOB;
        case GUCEF_DATATYPE_BINARY_BSOB:                    return GUCEF_DATATYPE_NAME_BINARY_BSOB;
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:      return GUCEF_DATATYPE_NAME_CONST_DATA_MEMORY_ADDRESS;
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:            return GUCEF_DATATYPE_NAME_DATA_MEMORY_ADDRESS;
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:        return GUCEF_DATATYPE_NAME_FUNCTION_MEMORY_ADDRESS;
        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:  return GUCEF_DATATYPE_NAME_DATETIME_ISO8601_ASCII_STRING;
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:   return GUCEF_DATATYPE_NAME_DATETIME_ISO8601_UTF8_STRING;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:  return GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:  return GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:    return GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:    return GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_INT32T2_LE_FRACTION:            return GUCEF_DATATYPE_NAME_INT32T2_LE_FRACTION;
        case GUCEF_DATATYPE_INT32T2_BE_FRACTION:            return GUCEF_DATATYPE_NAME_INT32T2_BE_FRACTION;
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION:           return GUCEF_DATATYPE_NAME_UINT32T2_LE_FRACTION;
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION:           return GUCEF_DATATYPE_NAME_UINT32T2_BE_FRACTION;
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION:            return GUCEF_DATATYPE_NAME_INT64T2_LE_FRACTION;
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION:            return GUCEF_DATATYPE_NAME_INT64T2_BE_FRACTION;
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION:           return GUCEF_DATATYPE_NAME_UINT64T2_LE_FRACTION;
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION:           return GUCEF_DATATYPE_NAME_UINT64T2_BE_FRACTION;     
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION:        return GUCEF_DATATYPE_NAME_INT64_INT32_LE_FRACTION;
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION:        return GUCEF_DATATYPE_NAME_INT64_INT32_BE_FRACTION;
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION:      return GUCEF_DATATYPE_NAME_UINT64_UINT32_LE_FRACTION;
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION:      return GUCEF_DATATYPE_NAME_UINT64_UINT32_BE_FRACTION;        

        case GUCEF_DATATYPE_MAP:                            return GUCEF_DATATYPE_NAME_MAP;
        case GUCEF_DATATYPE_ENUM:                           return GUCEF_DATATYPE_NAME_ENUM;
        case GUCEF_DATATYPE_SET:                            return GUCEF_DATATYPE_NAME_SET;
        case GUCEF_DATATYPE_ARRAY:                          return GUCEF_DATATYPE_NAME_ARRAY;
        case GUCEF_DATATYPE_OBJECT:                         return GUCEF_DATATYPE_NAME_OBJECT;
        case GUCEF_DATATYPE_NIL:                            return GUCEF_DATATYPE_NAME_NIL;
        case GUCEF_DATATYPE_NULL:                           return GUCEF_DATATYPE_NAME_NULL;

        default:
            return GUCEF_DATATYPE_NAME_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

UInt8
GucefTypeIdForTypeName( const char* typeName )
{
    if ( GUCEF_NULL == typeName )
        return GUCEF_DATATYPE_UNKNOWN;    
    
    if ( 0 == IsANumber( typeName ) )
    {    
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT8, typeName ) ) return GUCEF_DATATYPE_UINT8;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT8, typeName ) ) return GUCEF_DATATYPE_INT8;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_UINT16, typeName ) ) return GUCEF_DATATYPE_LE_UINT16;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_UINT16, typeName ) ) return GUCEF_DATATYPE_BE_UINT16;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_INT16, typeName ) ) return GUCEF_DATATYPE_LE_INT16;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_INT16, typeName ) ) return GUCEF_DATATYPE_BE_INT16;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_UINT32, typeName ) ) return GUCEF_DATATYPE_LE_UINT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_UINT32, typeName ) ) return GUCEF_DATATYPE_BE_UINT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_INT32, typeName ) ) return GUCEF_DATATYPE_LE_INT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_INT32, typeName ) ) return GUCEF_DATATYPE_BE_INT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_UINT64, typeName ) ) return GUCEF_DATATYPE_LE_UINT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_UINT64, typeName ) ) return GUCEF_DATATYPE_BE_UINT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_INT64, typeName ) ) return GUCEF_DATATYPE_LE_INT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_INT64, typeName ) ) return GUCEF_DATATYPE_BE_INT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_FLOAT32, typeName ) ) return GUCEF_DATATYPE_LE_FLOAT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_FLOAT32, typeName ) ) return GUCEF_DATATYPE_BE_FLOAT32;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_FLOAT64, typeName ) ) return GUCEF_DATATYPE_LE_FLOAT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_FLOAT64, typeName ) ) return GUCEF_DATATYPE_BE_FLOAT64;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_NUMERIC, typeName ) ) return GUCEF_DATATYPE_NUMERIC;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_ASCII_STRING, typeName ) ) return GUCEF_DATATYPE_ASCII_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UTF8_STRING, typeName ) ) return GUCEF_DATATYPE_UTF8_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_LE_STRING, typeName ) ) return GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_BE_STRING, typeName ) ) return GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_LE_STRING, typeName ) ) return GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_BE_STRING, typeName ) ) return GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BINARY_BLOB, typeName ) ) return GUCEF_DATATYPE_BINARY_BLOB;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BINARY_BSOB, typeName ) ) return GUCEF_DATATYPE_BINARY_BSOB;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_CONST_DATA_MEMORY_ADDRESS, typeName ) ) return GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_DATA_MEMORY_ADDRESS, typeName ) ) return GUCEF_DATATYPE_DATA_MEMORY_ADDRESS;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_FUNCTION_MEMORY_ADDRESS, typeName ) ) return GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_DATETIME_ISO8601_ASCII_STRING, typeName ) ) return GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_DATETIME_ISO8601_UTF8_STRING, typeName ) ) return GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, typeName ) ) return GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH, typeName ) ) return GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, typeName ) ) return GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH, typeName ) ) return GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT32T2_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT32T2_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT32T2_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT32T2_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT32T2_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT32T2_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT32T2_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT32T2_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT64T2_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT64T2_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT64T2_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT64T2_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT64T2_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT64T2_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT64T2_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT64T2_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT64_INT32_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT64_INT32_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_INT64_INT32_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_INT64_INT32_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT64_UINT32_LE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_UINT64_UINT32_BE_FRACTION, typeName ) ) return GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_MAP, typeName ) ) return GUCEF_DATATYPE_MAP;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_ENUM, typeName ) ) return GUCEF_DATATYPE_ENUM;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_SET, typeName ) ) return GUCEF_DATATYPE_SET;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_ARRAY, typeName ) ) return GUCEF_DATATYPE_ARRAY;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_OBJECT, typeName ) ) return GUCEF_DATATYPE_OBJECT;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_NIL, typeName ) ) return GUCEF_DATATYPE_NIL;
        if ( 0 == strcmp( GUCEF_DATATYPE_NAME_NULL, typeName ) ) return GUCEF_DATATYPE_NULL;
    }
    else
    {           
        return (UInt8) Str_To_Int( typeName );
    }

    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

UInt32 
GucefByteSizeOfFixedSizeType( UInt8 varType )
{
    switch ( varType )
    {
        case GUCEF_DATATYPE_INT8: return sizeof( Int8 );
        case GUCEF_DATATYPE_UINT8: return sizeof( UInt8 );
        case GUCEF_DATATYPE_LE_INT16: return sizeof( Int16 );
        case GUCEF_DATATYPE_BE_INT16: return sizeof( Int16 );
        case GUCEF_DATATYPE_LE_UINT16: return sizeof( UInt16 );
        case GUCEF_DATATYPE_BE_UINT16: return sizeof( UInt16 );
        case GUCEF_DATATYPE_LE_INT32: return sizeof( Int32 );
        case GUCEF_DATATYPE_BE_INT32: return sizeof( Int32 );
        case GUCEF_DATATYPE_LE_UINT32: return sizeof( UInt32 );
        case GUCEF_DATATYPE_BE_UINT32: return sizeof( UInt32 );
        case GUCEF_DATATYPE_LE_INT64: return sizeof( Int64 );
        case GUCEF_DATATYPE_BE_INT64: return sizeof( Int64 );
        case GUCEF_DATATYPE_LE_UINT64: return sizeof( UInt64 );
        case GUCEF_DATATYPE_BE_UINT64: return sizeof( UInt64 );
        case GUCEF_DATATYPE_LE_FLOAT32: return sizeof( Float32 );
        case GUCEF_DATATYPE_BE_FLOAT32: return sizeof( Float32 );
        case GUCEF_DATATYPE_LE_FLOAT64: return sizeof( Float64 );
        case GUCEF_DATATYPE_BE_FLOAT64: return sizeof( Float64 );
        case GUCEF_DATATYPE_BOOLEAN_INT32: return sizeof( Int32 );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return sizeof( UInt64 );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH: return sizeof( UInt64 );
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return sizeof( UInt64 );
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: return sizeof( UInt64 );
        case GUCEF_DATATYPE_INT32T2_LE_FRACTION: return 2*sizeof( Int32 );
        case GUCEF_DATATYPE_INT32T2_BE_FRACTION: return 2*sizeof( Int32 );
        case GUCEF_DATATYPE_UINT32T2_LE_FRACTION: return 2*sizeof( UInt32 );
        case GUCEF_DATATYPE_UINT32T2_BE_FRACTION: return 2*sizeof( UInt32 );
        case GUCEF_DATATYPE_INT64T2_LE_FRACTION: return 2*sizeof( Int64 );
        case GUCEF_DATATYPE_INT64T2_BE_FRACTION: return 2*sizeof( Int64 );
        case GUCEF_DATATYPE_UINT64T2_LE_FRACTION: return 2*sizeof( UInt64 );
        case GUCEF_DATATYPE_UINT64T2_BE_FRACTION: return 2*sizeof( UInt64 );
        case GUCEF_DATATYPE_INT64_INT32_LE_FRACTION: return sizeof( Int64 ) + sizeof( Int32 );
        case GUCEF_DATATYPE_INT64_INT32_BE_FRACTION: return sizeof( Int64 ) + sizeof( Int32 );
        case GUCEF_DATATYPE_UINT64_UINT32_LE_FRACTION: return sizeof( UInt64 ) + sizeof( UInt32 );
        case GUCEF_DATATYPE_UINT64_UINT32_BE_FRACTION: return sizeof( UInt64 ) + sizeof( UInt32 );
        
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS: return sizeof( const void* );
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS: return sizeof( void* );
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS: return sizeof( TDefaultFuncPtr );
        
        case GUCEF_DATATYPE_BINARY_BSOB: return GUCEF_VARIANT_BSOB_SIZE;

        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_NIL:
        default: return 0;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif

/*-------------------------------------------------------------------------*/

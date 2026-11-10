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

#ifndef GUCEF_CORE_CDATATABLECOLUMNDEF_H
#define GUCEF_CORE_CDATATABLECOLUMNDEF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

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
 *  One column of a data table schema.
 *
 *  Serialized as a DataNode object: "name", "type" (the type text of GetTypeText()), and only when they differ from
 *  their defaults "nullable" (false), "encoding" ("plain", "dictionary") and "description".
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableColumnDef : public CIDataNodeSerializable
{
    public:

    CString name;
    TDataTableLogicalType type;
    bool nullable;                          /**< default true */
    UInt8 decimalPrecision;                 /**< 1..18, DECIMAL only, default 18 */
    UInt8 decimalScale;                     /**< 0..precision, DECIMAL only, default 0 */
    TDataTableTimeUnit timeUnit;            /**< TIMESTAMP only, default microseconds */
    bool utcAdjusted;                       /**< TIMESTAMP only, default true */
    TDataTableEncodingHint encodingHint;    /**< AUTO (dictionary for text), PLAIN, DICTIONARY */
    CString description;

    CDataTableColumnDef( void );

    CDataTableColumnDef( const CString& columnName, TDataTableLogicalType columnType, bool isNullable = true );

    CDataTableColumnDef( const CDataTableColumnDef& src );

    virtual ~CDataTableColumnDef() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableColumnDef& operator=( const CDataTableColumnDef& src );

    /** All members equal */
    bool operator==( const CDataTableColumnDef& other ) const;
    bool operator!=( const CDataTableColumnDef& other ) const;

    /** A decimal column of the given precision and scale */
    static CDataTableColumnDef Decimal( const CString& columnName, UInt8 precision, UInt8 scale, bool isNullable = true );

    /** A timestamp column of the given unit */
    static CDataTableColumnDef Timestamp( const CString& columnName, TDataTableTimeUnit unit, bool isUtcAdjusted = true, bool isNullable = true );

    /** Same logical type and type parameters (precision and scale, unit and UTC flag); name, nullability and hints aside */
    bool IsSameType( const CDataTableColumnDef& other ) const;

    /** "bool", "int8" .. "uint64", "float32", "float64", "decimal(18,6)", "date", "timestamp(us,utc)", "timestamp(ns)", "utf8", "binary" */
    CString GetTypeText( void ) const;

    /**
     *  Sets the type and its parameters from the text form of GetTypeText(). Blanks around the parameters are allowed.
     *  "decimal" without parameters is decimal(18,0), "timestamp" without parameters is timestamp(us,utc).
     *  False for unknown text and out of range parameters; the column is unchanged then.
     */
    bool SetTypeFromText( const CString& typeText );

    /** A name and type parameters that can be used: a non-empty name without control characters, precision and scale in range */
    bool IsValid( CString& error ) const;

    /** True for names that can be used: not empty, no control characters (below 0x20) */
    static bool IsValidName( const CString& columnName );

    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLECOLUMNDEF_H ? */

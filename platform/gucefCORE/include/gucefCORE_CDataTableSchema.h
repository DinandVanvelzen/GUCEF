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

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#define GUCEF_CORE_CDATATABLESCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNDEF_H
#include "gucefCORE_CDataTableColumnDef.h"
#define GUCEF_CORE_CDATATABLECOLUMNDEF_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNDEF_H ? */

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
 *  The ordered columns of a data table with its primary key, sort key and schema version.
 *
 *  Immutable once shared through a CDataTableSchemaPtr; copies are independent.
 *
 *  Serialized into the given DataNode as "schemaVersion", "columns" (an array of column objects), and "primaryKey" and
 *  "sortKey" (arrays of column names) when they are set. The definition of a table writes them at its top level.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableSchema : public CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CDataTableColumnDef > TColumnDefVector;
    typedef GUCEF::vector< UInt32 > TColumnIndexVector;

    CDataTableSchema( void );

    CDataTableSchema( const CDataTableSchema& src );

    virtual ~CDataTableSchema() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableSchema& operator=( const CDataTableSchema& src );

    /** Same columns, keys and version */
    bool operator==( const CDataTableSchema& other ) const;
    bool operator!=( const CDataTableSchema& other ) const;

    /**
     *  Appends the column; false on a duplicate or invalid name, or invalid type parameters. An UNSUPPORTED column (one
     *  a codec found in a file and cannot decode) is taken, so file schemas list every column; Validate() refuses it.
     */
    bool AddColumn( const CDataTableColumnDef& column );

    UInt32 GetColumnCount( void ) const;

    /** The column at the index, which must be below GetColumnCount() */
    const CDataTableColumnDef& GetColumn( UInt32 index ) const;

    const TColumnDefVector& GetColumns( void ) const;

    /** -1 when absent, exact (case sensitive) match */
    Int32 FindColumn( const CString& name ) const;

    /** Columns that form the primary key, in key order; false when a name is unknown or repeated. An empty list clears the key */
    bool SetPrimaryKey( const CString::StringVector& columnNames );
    const TColumnIndexVector& GetPrimaryKey( void ) const;

    /** Columns the rows are sorted on, ascending, nulls last; false when a name is unknown or repeated */
    bool SetSortKey( const CString::StringVector& columnNames );
    const TColumnIndexVector& GetSortKey( void ) const;

    UInt32 GetSchemaVersion( void ) const;
    void SetSchemaVersion( UInt32 version );

    /** Removes all columns and keys, the version becomes 1 */
    void Clear( void );

    /**
     *  Checks the schema can be used for a table: at least one column, every column valid, unique names,
     *  and primary key columns that are not nullable
     */
    bool Validate( CString& error ) const;

    /**
     *  Whether files with the given schema can be read as this (table) schema. Columns are matched by name; the file
     *  may hold columns the table does not have. Allowed differences: a column the file lacks that is nullable in the
     *  table (read as all null), INT32 in the file for INT64 in the table, FLOAT32 for FLOAT64, and a decimal of a lower
     *  scale whose integer digits fit the table column (rescaled exactly). A nullable file column for a column that is
     *  not nullable in the table, an UNSUPPORTED file column and every other type difference are refused.
     */
    bool IsReadableAs( const CDataTableSchema& fileSchema, CString& error ) const;

    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool ResolveColumnNames( const CString::StringVector& columnNames, TColumnIndexVector& indices ) const;

    TColumnDefVector m_columns;
    TColumnIndexVector m_primaryKey;
    TColumnIndexVector m_sortKey;
    UInt32 m_schemaVersion;
};

/*-------------------------------------------------------------------------*/

/** A shared, immutable schema: batches hold one reference */
typedef CTSharedPtr< const CDataTableSchema, MT::CMutex >   CDataTableSchemaPtr;

/** A shared, immutable copy of the schema */
GUCEF_CORE_PUBLIC_CPP CDataTableSchemaPtr CreateDataTableSchemaPtr( const CDataTableSchema& schema );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

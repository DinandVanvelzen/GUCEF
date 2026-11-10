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

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#define GUCEF_CORE_CDATATABLEDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#include "gucefCORE_CDataTablePartitionSpec.h"
#define GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#endif /* GUCEF_CORE_CDATATABLEPARTITIONSPEC_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

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
 *  Everything that defines a table: name, schema, partitioning, codec type, writer options and backend options.
 *  What CreateTable takes and a database stores, for example as _tables/<name>.json.
 *
 *  Serialized as one DataNode object: "name", "codec", the schema members at the top level ("schemaVersion",
 *  "columns", "primaryKey", "sortKey"), "partitioning" when the table is partitioned, "storage" (the writer options)
 *  and "backendOptions" when not empty.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableDefinition : public CIDataNodeSerializable
{
    public:

    CString name;                           /**< [A-Za-z0-9_.-], not starting with '.', usable as a file name */
    CDataTableSchema schema;
    CDataTablePartitionSpec partitioning;
    CString codecType;                      /**< registry key of the table codec, default "parquet" */
    CDataTableWriterOptions storage;
    CValueList backendOptions;

    CDataTableDefinition( void );

    CDataTableDefinition( const CDataTableDefinition& src );

    virtual ~CDataTableDefinition() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableDefinition& operator=( const CDataTableDefinition& src );

    bool operator==( const CDataTableDefinition& other ) const;
    bool operator!=( const CDataTableDefinition& other ) const;

    /** The name, the schema, the partitioning against the schema, a codec type and the writer options */
    bool Validate( CString& error ) const;

    /**
     *  True for table names that can be used on every platform: [A-Za-z0-9_.-], not empty, not starting or ending with '.',
     *  and not a device name of Windows, ignoring case, alone or before a '.' (CON, PRN, AUX, NUL, COM0 .. COM9, LPT0 ..
     *  LPT9): the name becomes a file name ("_tables/<name>.json"), and data moves between platforms.
     */
    static bool IsValidTableName( const CString& tableName );

    /** A shared copy of the schema, for batches and scans */
    CDataTableSchemaPtr CreateSchemaPtr( void ) const;

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

#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

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


#ifndef GUCEF_CORE_CDATATABLEINDEXDEF_H
#define GUCEF_CORE_CDATATABLEINDEXDEF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

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
 *  A secondary index hint of a table definition: the columns of the index in order and whether their values are unique
 *  together. Backends that keep indexes (SQL databases) create one per hint; others may ignore the hints or use them
 *  for their layout. A hint never changes what a query returns, only how fast a backend can find it, except that a
 *  unique hint lets a backend that honours it refuse rows that repeat the indexed values.
 *
 *  Serialized as a DataNode object: "name", "columns" (an array of column names) and "unique" when it is true.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableIndexDef : public CIDataNodeSerializable
{
    public:

    /** The longest index name, in bytes: the shortest limit among the common SQL databases */
    static const UInt32 MaxNameLength = 63;

    CString name;                           /**< [A-Za-z0-9_], starting with a letter or '_', at most MaxNameLength bytes */
    CString::StringVector columns;          /**< schema column names in index order, at least one, none repeated */
    bool unique;                            /**< default false */

    CDataTableIndexDef( void );

    CDataTableIndexDef( const CString& indexName, const CString::StringVector& indexColumns, bool isUnique = false );

    CDataTableIndexDef( const CDataTableIndexDef& src );

    virtual ~CDataTableIndexDef() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableIndexDef& operator=( const CDataTableIndexDef& src );

    /** All members equal */
    bool operator==( const CDataTableIndexDef& other ) const;
    bool operator!=( const CDataTableIndexDef& other ) const;

    /** A valid name and at least one column, every column in the schema and none repeated */
    bool Validate( const CDataTableSchema& schema, CString& error ) const;

    /** True for names that can be used: [A-Za-z0-9_], starting with a letter or '_', 1 .. MaxNameLength bytes */
    static bool IsValidIndexName( const CString& indexName );

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

#endif /* GUCEF_CORE_CDATATABLEINDEXDEF_H ? */

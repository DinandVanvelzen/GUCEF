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

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
 *  Options of a table writer: compression, row group and page sizes, dictionary use and statistics.
 *  Part of a table definition ("storage"), passed to CITableCodec::CreateWriter.
 *
 *  Serialized with every member: "compression", "compressionLevel" (only when set), "rowGroupRowCount",
 *  "rowGroupByteLimit", "pageRowCount", "pageByteLimit", "dictionary", "dictionaryByteLimit", "writeStatistics",
 *  "writePageStatistics", "statisticsMaxValueBytes", and the "keyValueMetaData" and "codecOptions" objects when not empty.
 *  Members absent from a DataNode keep their defaults.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableWriterOptions : public CIDataNodeSerializable
{
    public:

    static const Int32 DefaultCompressionLevel = -1;   /**< the compression codec chooses */

    CString compression;                    /**< codec type of the CompressionCodec family: "none" (default), "gzip", ... */
    Int32 compressionLevel;                 /**< DefaultCompressionLevel by default */
    UInt32 rowGroupRowCount;                /**< rows per row group, default 1048576 */
    UInt64 rowGroupByteLimit;               /**< a row group is flushed early at this size, default 512 MiB */
    UInt32 pageRowCount;                    /**< rows per data page, default 16384 */
    UInt32 pageByteLimit;                   /**< a column's page is cut early at this size, default 1 MiB */
    TDataTableDictionaryMode dictionary;    /**< default DATATABLE_DICTIONARY_AUTO */
    UInt32 dictionaryByteLimit;             /**< beyond it a column chunk continues plain, default 1 MiB */
    bool writeStatistics;                   /**< default true */
    bool writePageStatistics;               /**< default true */
    UInt32 statisticsMaxValueBytes;         /**< longer text minimum / maximum values are left out, default 64 */
    CValueList keyValueMetaData;            /**< extra file metadata */
    CValueList codecOptions;                /**< codec specific */

    CDataTableWriterOptions( void );

    CDataTableWriterOptions( const CDataTableWriterOptions& src );

    virtual ~CDataTableWriterOptions() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableWriterOptions& operator=( const CDataTableWriterOptions& src );

    bool operator==( const CDataTableWriterOptions& other ) const;
    bool operator!=( const CDataTableWriterOptions& other ) const;

    /** Non-zero counts and limits, a compression name */
    bool Validate( CString& error ) const;

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

#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

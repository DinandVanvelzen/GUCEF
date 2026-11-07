/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_MACROS_H
#include "tablecodecpluginPARQUET_macros.h"
#define TABLECODECPLUGIN_PARQUET_MACROS_H
#endif /* TABLECODECPLUGIN_PARQUET_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The read options of the parquet codec, from the CValueList given to ReadMetaData() and CreateReader(). Keys the
 *  codec does not know are ignored, so one list can serve several codecs. Every size read from a file is checked
 *  against these bounds before anything is allocated for it.
 *
 *      maxFooterBytes      64 MiB      the Thrift footer
 *      maxMetaDataBytes    256 MiB     memory of the parsed footer structures
 *      maxPageBytes        256 MiB     the uncompressed size of a page, and the decoded values of a page in the batch layout
 *      maxChunkBytes       1 GiB       a column chunk, and one coalesced read of several chunks
 *      ioCoalesceGapBytes  1 MiB       chunks of a row group this close together are read with one read
 *      validateUtf8        false       text values are checked to be UTF-8 (CORRUPT_DATA otherwise)
 */
struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetReadOptions
{
    UInt64 maxFooterBytes;
    UInt64 maxMetaDataBytes;
    UInt64 maxPageBytes;
    UInt64 maxChunkBytes;
    UInt64 ioCoalesceGapBytes;
    bool validateUtf8;

    SParquetReadOptions( void );

    /** False with the error for a value that is not valid; the options stay as they were then */
    bool Parse( const CORE::CValueList& options, CORE::CString& error );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H ? */

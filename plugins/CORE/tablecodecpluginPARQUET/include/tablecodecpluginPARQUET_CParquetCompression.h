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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The compression of the pages of a column chunk, through the CompressionCodec family of the codec registry: the type
 *  is the parquet codec name in lower case ("gzip", "snappy", "zstd", "lz4_raw", "brotli", "lzo"). The codec is looked
 *  up once, by Init(); a reader or writer keeps one object per codec it uses.
 *
 *  Decompression writes into a buffer of exactly the page's uncompressed_page_size: output beyond it is refused, so a
 *  compressed page cannot make the reader produce more than the page header announced (and the caller bounded), and
 *  output short of it is CORRUPT_DATA. One object is used by one thread at a time.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetCompression
{
    public:

    /** The largest expansion of deflate data (RFC 1951 caps it near 1032 : 1), plus room for small pages */
    static const UInt32 GzipMaxExpansion = 1032;

    CParquetCompression( void );

    /** The registry type name of a parquet codec; false for UNCOMPRESSED, the hadoop framed LZ4 and unknown codecs */
    static bool GetCodecTypeName( Int32 parquetCodec, CORE::CString& typeName );

    /** The parquet codec of a writer option: "none" or "uncompressed", "gzip", "snappy", "zstd", "lz4_raw", "brotli", "lzo" */
    static bool ParquetCodecFromName( const CORE::CString& name, Int32& parquetCodec );

    /**
     *  Prepares the codec: UNSUPPORTED (naming it) when no codec of its name is registered or the parquet codec has no
     *  name. level is the codec's "level" parameter for compression, CDataTableWriterOptions::DefaultCompressionLevel
     *  for the codec's default.
     */
    CORE::TDataTableStatus Init( Int32 parquetCodec, Int32 level, CORE::CString& error );

    inline Int32 GetParquetCodec( void ) const { return m_parquetCodec; }
    inline bool IsCompressing( void ) const { return 0 != m_parquetCodec; }

    /** Decompresses exactly outputSize bytes into output; CORRUPT_DATA when the data is not valid or not of that size */
    CORE::TDataTableStatus Decompress( const UInt8* input, UInt32 inputSize, UInt8* output, UInt32 outputSize, CORE::CString& error );

    /** Compresses the input, replacing the content of output */
    CORE::TDataTableStatus Compress( const UInt8* input, UInt32 inputSize, CORE::CDynamicBuffer& output, CORE::CString& error );

    private:

    Int32 m_parquetCodec;
    CORE::CString m_typeName;
    CORE::CCodecRegistry::TICodecPtr m_codec;
    CORE::CValueList m_params;
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

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H ? */

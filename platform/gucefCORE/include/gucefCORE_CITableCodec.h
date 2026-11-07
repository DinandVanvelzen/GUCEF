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

#ifndef GUCEF_CORE_CITABLECODEC_H
#define GUCEF_CORE_CITABLECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

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
 *  A table file format: turns files into batches and back (section 10 of the data tables design).
 *
 *  Codecs are shared and every member is reentrant. CSV is built in; parquet comes as a C++ link-back plugin.
 *  GetTypeName() is the key in the table codec registry.
 */
class GUCEF_CORE_PUBLIC_CPP CITableCodec : public CICloneable
{
    public:

    CITableCodec( void );

    virtual ~CITableCodec() GUCEF_VIRTUAL_OVERRIDE;

    /** "parquet", "csv": the registry key */
    virtual CString GetTypeName( void ) const = 0;

    virtual CString GetName( void ) const = 0;

    virtual CString GetCopyright( void ) const = 0;

    virtual TVersion GetVersion( void ) const = 0;

    /** GUCEF_TABLECODEC_CAPABILITY_* */
    virtual UInt32 GetCapabilities( void ) const = 0;

    /** Lower case, without the dot: "parquet", "csv", "tsv" */
    virtual void GetFileExtensions( CString::StringSet& extensions ) const = 0;

    virtual TDataTableStatus ReadMetaData( CIOAccess& source, const CValueList& options,
                                           CITableFileMetaDataPtr& metaData, CString& error ) const = 0;

    virtual TDataTableStatus CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData,
                                           const CValueList& options, CITableReaderPtr& reader, CString& error ) const = 0;

    virtual TDataTableStatus CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema,
                                           const CDataTableWriterOptions& options, CITableWriterPtr& writer, CString& error ) const = 0;

    private:

    CITableCodec& operator=( const CITableCodec& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CITABLECODEC_H ? */

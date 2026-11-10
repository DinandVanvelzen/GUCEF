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

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#define GUCEF_CORE_CITABLEFILEMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#include "gucefCORE_CDataTableRowGroupInfo.h"
#define GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#endif /* GUCEF_CORE_CDATATABLEROWGROUPINFO_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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
 *  What a table codec read from the metadata of a file (a parquet footer): the file schema, row and row group counts,
 *  row group statistics and key value metadata. Immutable and shared between threads, for example by a footer cache.
 */
class GUCEF_CORE_PUBLIC_CPP CITableFileMetaData
{
    public:

    CITableFileMetaData( void );

    virtual ~CITableFileMetaData();

    /** The schema the file states, or the one the codec inferred (CSV) */
    virtual const CDataTableSchemaPtr& GetSchema( void ) const = 0;

    /** GUCEF_UINT64MAX when unknown (CSV) */
    virtual UInt64 GetRowCount( void ) const = 0;

    /** 1 for formats without row groups */
    virtual UInt32 GetRowGroupCount( void ) const = 0;

    /** False for a row group out of range */
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CDataTableRowGroupInfo& info ) const = 0;

    /** Bytes the object holds, for the budget of a cache */
    virtual UInt64 GetMemoryFootprint( void ) const = 0;

    virtual void GetKeyValueMetaData( CValueList& keyValues ) const = 0;

    /**
     *  Whether VerifySource() can tell if a source still holds this metadata. False by default: metadata of such a codec
     *  must be read again from a file that may have changed, a cache cannot reuse it.
     */
    virtual bool CanVerifySource( void ) const;

    /**
     *  Whether the source holds, byte for byte, the metadata this object was read from (for parquet: the same size, the
     *  same magic and the same footer), so that this object is what the codec would read from the source now. A cache of
     *  metadata asks this of every file it opens again: a file replaced by one of the same size, within the resolution
     *  of a modification time, is never read with the metadata of the file before it. False when the source differs,
     *  cannot be read, or the codec cannot tell (the default). The position of the source is left anywhere.
     */
    virtual bool VerifySource( CIOAccess& source ) const;

    private:

    CITableFileMetaData( const CITableFileMetaData& src );              /**< not implemented */
    CITableFileMetaData& operator=( const CITableFileMetaData& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

/** Shared, immutable file metadata; a codec makes it as a pointer to its own mutable type and shares it as this */
typedef CTSharedPtr< const CITableFileMetaData, MT::CMutex >    CITableFileMetaDataPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

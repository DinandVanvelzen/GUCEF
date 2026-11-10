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

#ifndef GUCEF_CORE_CIDATATABLESCANSOURCE_H
#define GUCEF_CORE_CIDATATABLESCANSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#include "gucefCORE_CDataTableScanStats.h"
#define GUCEF_CORE_CDATATABLESCANSTATS_H
#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

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
 *  One file a scan reads, as listed by a scan source after partition pruning
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableScanFile
{
    public:

    CString path;                           /**< backend specific, for example the VFS path of the partition file */
    UInt32 fileOrdinal;                     /**< position in delivery order, set by the scan */
    CValueList partitionKeys;               /**< key values parsed from the path, keyed by partition field name */
    UInt64 fileSize;                        /**< size the file was listed with, GUCEF_UINT64MAX when not known */
    CDateTime modificationTime;             /**< modification time the file was listed with */
    CITableFileMetaDataPtr metaData;        /**< metadata when the source has it already (a footer cache), else NULL */
    MT::CMutex* sharedFileLock;             /**< held by the readers of this file around every read of their shared input (14.6), GUCEF_NULL when each reader has an input of its own */
    UInt64 backendData;                     /**< free for the source, for example an index into its listing */

    CDataTableScanFile( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  What a backend gives a CDataTableScan: the files to scan and readers on them. Everything else of a scan
 *  (scheduling, ordering, credits, cancellation, row group pruning, exact filtering) is shared by every backend.
 */
class GUCEF_CORE_PUBLIC_CPP CIDataTableScanSource
{
    public:

    CIDataTableScanSource( void );

    virtual ~CIDataTableScanSource();

    /** The next file to scan after partition pruning, in delivery order; END when there are no more. Called by one thread at a time */
    virtual TDataTableStatus NextFile( CDataTableScanFile& file ) = 0;

    /** A reader on the given file, metadata from the cache when possible. May be called from several threads at once */
    virtual TDataTableStatus OpenFile( const CDataTableScanFile& file, CITableReaderPtr& reader ) = 0;

    /** Adds the counters only the source knows (files listed and pruned while listing, footer cache hits); the default adds nothing */
    virtual void AddSourceStats( CDataTableScanStats& stats ) const;

    private:

    CIDataTableScanSource( const CIDataTableScanSource& src );              /**< not implemented */
    CIDataTableScanSource& operator=( const CIDataTableScanSource& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CIDataTableScanSource, MT::CMutex >    CDataTableScanSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIDATATABLESCANSOURCE_H ? */

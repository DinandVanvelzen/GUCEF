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

#ifndef GUCEF_CORE_CDATATABLEPARTITIONINFO_H
#define GUCEF_CORE_CDATATABLEPARTITIONINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  One partition of a table: its path, key values and, when asked for, the statistics of its file.
 *  Event data of CIDataTable::PartitionWrittenEvent and PartitionDeletedEvent.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTablePartitionInfo : public CICloneable
{
    public:

    typedef GUCEF::vector< CDataTablePartitionInfo > TPartitionInfoVector;

    CString path;                           /**< relative path of the partition file */
    CValueList keyValues;                   /**< keyed by partition field name, see CDataTablePartitionSpec */
    bool hasFileStats;                      /**< the members below are set */
    UInt64 fileSize;
    CDateTime modificationTime;
    UInt64 rowCount;                        /**< GUCEF_UINT64MAX when not known */
    UInt32 rowGroupCount;

    CDataTablePartitionInfo( void );

    CDataTablePartitionInfo( const CDataTablePartitionInfo& src );

    virtual ~CDataTablePartitionInfo() GUCEF_VIRTUAL_OVERRIDE;

    CDataTablePartitionInfo& operator=( const CDataTablePartitionInfo& src );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  A list of partitions as one cloneable: the result data of CIDataTable::ListPartitionsAsync
 */
class GUCEF_CORE_PUBLIC_CPP CDataTablePartitionInfoList : public CICloneable
{
    public:

    CDataTablePartitionInfo::TPartitionInfoVector partitions;

    CDataTablePartitionInfoList( void );

    CDataTablePartitionInfoList( const CDataTablePartitionInfoList& src );

    virtual ~CDataTablePartitionInfoList() GUCEF_VIRTUAL_OVERRIDE;

    CDataTablePartitionInfoList& operator=( const CDataTablePartitionInfoList& src );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEPARTITIONINFO_H ? */

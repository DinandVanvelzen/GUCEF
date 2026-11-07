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

#ifndef GUCEF_CORE_CDATATABLEWRITERESULT_H
#define GUCEF_CORE_CDATATABLEWRITERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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
 *  What a write, a delete or a compaction did, per partition (section 7.2). Result data of the async table calls.
 *  Each partition changes atomically; a call over several partitions is not one transaction.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableWriteResult : public CICloneable
{
    public:

    struct GUCEF_CORE_PUBLIC_CPP SPartitionResult
    {
        CString path;
        TDataTableStatus status;
        UInt64 rowsWritten;         /**< rows in the partition file after the change */
        UInt64 rowsReplaced;        /**< rows of the partition replaced by rows with the same primary key */
        UInt64 rowsDeleted;         /**< rows taken out by DeleteRows, or all rows of a deleted partition */
        CString message;            /**< why the partition failed */

        SPartitionResult( void );
    };
    typedef GUCEF::vector< SPartitionResult > TPartitionResultVector;

    TPartitionResultVector partitions;

    CDataTableWriteResult( void );

    CDataTableWriteResult( const CDataTableWriteResult& src );

    virtual ~CDataTableWriteResult() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableWriteResult& operator=( const CDataTableWriteResult& src );

    /** OK when every partition succeeded, else the status of the first that failed */
    TDataTableStatus GetOverallStatus( void ) const;

    UInt64 GetTotalRowsWritten( void ) const;

    void Clear( void );

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

#endif /* GUCEF_CORE_CDATATABLEWRITERESULT_H ? */

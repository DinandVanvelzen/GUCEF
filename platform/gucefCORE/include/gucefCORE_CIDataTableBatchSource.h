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

#ifndef GUCEF_CORE_CIDATATABLEBATCHSOURCE_H
#define GUCEF_CORE_CIDATATABLEBATCHSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHPTR_H
#include "gucefCORE_CDataTableBatchPtr.h"
#define GUCEF_CORE_CDATATABLEBATCHPTR_H
#endif /* GUCEF_CORE_CDATATABLEBATCHPTR_H ? */

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
 *  Something batches are pulled from one at a time: a scan, or a source made for tests and adapters.
 *  CDataTableRowCursor iterates the rows of any batch source. Pulled by one thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CIDataTableBatchSource
{
    public:

    CIDataTableBatchSource( void );

    virtual ~CIDataTableBatchSource();

    /** OK with a batch, END when there are no more batches, CANCELLED, or an error status */
    virtual TDataTableStatus NextBatch( CDataTableBatchPtr& batch ) = 0;

    /** The message of the last failure */
    virtual CString GetLastErrorMessage( void ) const = 0;

    private:

    CIDataTableBatchSource( const CIDataTableBatchSource& src );                /**< not implemented */
    CIDataTableBatchSource& operator=( const CIDataTableBatchSource& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------*/

/** A CDataTableScanPtr converts to this */
typedef CTSharedPtr< CIDataTableBatchSource, MT::CMutex >   CDataTableBatchSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIDATATABLEBATCHSOURCE_H ? */

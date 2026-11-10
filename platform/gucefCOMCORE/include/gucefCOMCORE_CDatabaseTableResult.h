/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#define GUCEF_COMCORE_CDATABASETABLERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The result data of CIDatabaseClient::OpenTableAsync() and CreateTableAsync(): the table that was opened or made.
 */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseTableResult : public CORE::CICloneable
{
    public:

    CORE::CIDataTablePtr table;

    CDatabaseTableResult( void );

    explicit CDatabaseTableResult( const CORE::CIDataTablePtr& openedTable );

    CDatabaseTableResult( const CDatabaseTableResult& src );

    virtual ~CDatabaseTableResult() GUCEF_VIRTUAL_OVERRIDE;

    CDatabaseTableResult& operator=( const CDatabaseTableResult& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    /** False when the future is not completed successfully or does not carry a table */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CORE::CIDataTablePtr& table );
};

/*-------------------------------------------------------------------------*/

/**
 *  The result data of CIDatabaseClient::ListTablesAsync(): the names of the tables.
 */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseTableNamesResult : public CORE::CICloneable
{
    public:

    CORE::CString::StringSet tableNames;

    CDatabaseTableNamesResult( void );

    explicit CDatabaseTableNamesResult( const CORE::CString::StringSet& names );

    CDatabaseTableNamesResult( const CDatabaseTableNamesResult& src );

    virtual ~CDatabaseTableNamesResult() GUCEF_VIRTUAL_OVERRIDE;

    CDatabaseTableNamesResult& operator=( const CDatabaseTableNamesResult& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    /** False when the future is not completed successfully or does not carry names */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CORE::CString::StringSet& tableNames );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

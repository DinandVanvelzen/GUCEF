/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CLANESTABLESCHEMA_H
#define GUCEF_DRNUTILS_CLANESTABLESCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The lanes grid of a scheduler host (contract LaneRow; queuedByPriority flattened to queuedP0..queuedP3); the throttle status of a lane follows as hidden columns throttle.<name> declared by the publisher; default sort lane.
 *  Column names equal the field names of the contract, so the RPC rows and the grids describe one model.
 *  Times are timestamp(ms,utc); enums are their names as text. Grid name: <prefix>/lanes.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CLanesTableSchema
{
    public:

    enum EColumn
    {
        COLUMN_LANE = 0,
        COLUMN_MODE,
        COLUMN_STATE,
        COLUMN_IN_FLIGHT,
        COLUMN_MAX_CONCURRENT,
        COLUMN_QUEUED_P0,
        COLUMN_QUEUED_P1,
        COLUMN_QUEUED_P2,
        COLUMN_QUEUED_P3,
        COLUMN_WAITING_FOR,
        COLUMN_WAITING_UNTIL_UTC,
        COLUMN_UNAVAILABLE_REASON,
        COLUMN_HELD_BY,
        COLUMN_REVISION,
        COLUMN_COUNT
    };

    static const char* TableName;       /**< "lanes" */
    static const char* KeyColumnName;   /**< "lane" */

    static const char* GetColumnName( EColumn column );

    static void GetColumnNames( CORE::CString::StringVector& columnNames );

    /** The schema of the grid: the columns in EColumn order, primary key lane (not nullable, every other column nullable) */
    static void GetSchema( CORE::CDataTableSchema& schema );

    private:

    CLanesTableSchema( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CLANESTABLESCHEMA_H ? */

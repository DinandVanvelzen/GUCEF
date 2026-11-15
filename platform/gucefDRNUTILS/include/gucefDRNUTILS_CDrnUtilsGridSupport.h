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

#ifndef GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H
#define GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#include "gucefDRN_CDRNPublishedTableSchema.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

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
 *  Module-private: what the table publishers of both hosts share: the published schema of a table schema, and grids
 *  in a DRN session made and withdrawn for a CDRNTablePublisher. Called with no lock of the remote layer held: they
 *  take the node lock.
 */
class CDrnUtilsGridSupport
{
    public:

    /**
     *  The published schema: the columns of the table schema (column id = column name) in their order, key column 0,
     *  plus hidden UTF-8 columns of the given names at the end
     */
    static void MakePublishedSchema( const CORE::CDataTableSchema& schema, Int32 defaultSortColumn, bool ascending,
                                     const CORE::CString::StringVector& hiddenTextColumns, DRN::CDRNPublishedTableSchema& published );

    /** A local, reliable grid of the session (registered in CDRNGlobal's session registry) with a publisher on it */
    static bool CreatePublishedGrid( const CORE::CString& sessionName, const CORE::CString& gridName,
                                     DRN::CDRNDataGrid*& grid, DRN::CDRNTablePublisher*& publisher, CORE::CString& error );

    /** Deletes the publisher and withdraws the grid from the session; when the session is no longer registered its grids went with it */
    static void DestroyPublishedGrid( const CORE::CString& sessionName, DRN::CDRNDataGrid*& grid, DRN::CDRNTablePublisher*& publisher );

    private:
    CDrnUtilsGridSupport( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSGRIDSUPPORT_H ? */

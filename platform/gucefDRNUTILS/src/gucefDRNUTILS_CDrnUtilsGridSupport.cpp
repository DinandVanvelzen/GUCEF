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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#include "gucefDRN_CDRNDataGridSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDSETTINGS_H ? */

#include "gucefDRNUTILS_CDrnUtilsGridSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDrnUtilsGridSupport::MakePublishedSchema( const CORE::CDataTableSchema& schema, Int32 defaultSortColumn, bool ascending,
                                           const CORE::CString::StringVector& hiddenTextColumns, DRN::CDRNPublishedTableSchema& published )
{GUCEF_TRACE;

    published.columns.clear();
    UInt32 count = schema.GetColumnCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        const CORE::CDataTableColumnDef& def = schema.GetColumn( i );
        DRN::CDRNPublishedColumn column( def.name, def );
        column.title = def.name;
        published.columns.push_back( column );
    }
    for ( UInt32 i=0; i<hiddenTextColumns.size(); ++i )
    {
        DRN::CDRNPublishedColumn column( hiddenTextColumns[ i ], CORE::CDataTableColumnDef( hiddenTextColumns[ i ], CORE::DATATABLE_TYPE_UTF8, true ) );
        column.hidden = true;
        published.columns.push_back( column );
    }
    published.keyColumn = 0;
    published.defaultSortColumn = defaultSortColumn;
    published.defaultSortAscending = ascending;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsGridSupport::CreatePublishedGrid( const CORE::CString& sessionName, const CORE::CString& gridName,
                                           DRN::CDRNDataGrid*& grid, DRN::CDRNTablePublisher*& publisher, CORE::CString& error )
{GUCEF_TRACE;

    grid = GUCEF_NULL;
    publisher = GUCEF_NULL;
    DRN::CDRNSession* session = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Find( sessionName );
    if ( GUCEF_NULL == session )
    {
        error = "no DRN session \"" + sessionName + "\" is registered in the session registry of gucefDRN";
        return false;
    }
    DRN::CDRNDataGridSettings settings;
    settings.name = gridName;
    settings.reliable = true;
    settings.deltaAgainstAck = true;
    settings.snapshotOnSubscribe = true;
    grid = session->CreateGrid( settings );
    if ( GUCEF_NULL == grid )
    {
        error = "the grid \"" + gridName + "\" could not be made in DRN session \"" + sessionName + "\"";
        return false;
    }
    publisher = GUCEF_NEW DRN::CDRNTablePublisher( *grid, DRN::CDRNTablePublisherSettings() );
    if ( !publisher->IsValid() )
    {
        error = "the grid \"" + gridName + "\" cannot be published (it must be local, reliable and empty)";
        DestroyPublishedGrid( sessionName, grid, publisher );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGridSupport::DestroyPublishedGrid( const CORE::CString& sessionName, DRN::CDRNDataGrid*& grid, DRN::CDRNTablePublisher*& publisher )
{GUCEF_TRACE;

    GUCEF_DELETE publisher;
    publisher = GUCEF_NULL;
    if ( GUCEF_NULL != grid )
    {
        DRN::CDRNSession* session = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Find( sessionName );
        if ( GUCEF_NULL != session )
            session->GetReplicator().DestroyItem( grid );
    }
    grid = GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

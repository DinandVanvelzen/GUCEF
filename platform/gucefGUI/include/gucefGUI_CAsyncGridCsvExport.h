/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CASYNCGRIDCSVEXPORT_H
#define GUCEF_GUI_CASYNCGRIDCSVEXPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_GUI_CGRIDEXPORTSOURCE_H
#include "gucefGUI_CGridExportSource.h"
#define GUCEF_GUI_CGRIDEXPORTSOURCE_H
#endif /* GUCEF_GUI_CGRIDEXPORTSOURCE_H ? */

#ifndef GUCEF_GUI_CGRIDCSVOPTIONS_H
#include "gucefGUI_CGridCsvOptions.h"
#define GUCEF_GUI_CGRIDCSVOPTIONS_H
#endif /* GUCEF_GUI_CGRIDCSVOPTIONS_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#include "gucefGUI_CGridCsvExportReport.h"
#define GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTREPORT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What CGridCsvExporter::ExportAsync() asks a thread of a pool to do, and what came of it */
class GUCEF_HIDDEN CGridCsvExportTaskData : public CORE::CICloneable
{
    public:

    TGridExportSourcePtr source;        /**< shared with the caller: lives until the task ended */
    CORE::IOAccessPtr output;           /**< shared with the caller */
    CGridCsvOptions options;
    CGridCsvExportReport report;        /**< filled when the task ran */

    CGridCsvExportTaskData( void );

    CGridCsvExportTaskData( const CGridCsvExportTaskData& src );

    virtual ~CGridCsvExportTaskData();

    CGridCsvExportTaskData& operator=( const CGridCsvExportTaskData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The task that runs the steps of a CGridCsvExporter for CGridCsvExporter::ExportAsync(); a request to cancel the task stops it between batches */
class GUCEF_HIDDEN CAsyncGridCsvExport : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    CAsyncGridCsvExport( void );

    virtual ~CAsyncGridCsvExport() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the report for CGridCsvExporter::GetAsyncResult() */
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncGridCsvExport( const CAsyncGridCsvExport& src );
    CAsyncGridCsvExport& operator=( const CAsyncGridCsvExport& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CTaskConsumer, CAsyncGridCsvExport, MT::CMutex > TAsyncGridCsvExportTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CASYNCGRIDCSVEXPORT_H ? */

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

#ifndef GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#define GUCEF_GUI_CGRIDCSVEXPORTREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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

/** What came of a CSV export of a grid: written or not, how much, where, and a message for the user */
class GUCEF_GUI_PUBLIC_CPP CGridCsvExportReport
{
    public:

    bool success;               /**< the file is complete at 'path' */
    bool cancelled;             /**< the export was stopped (CancelExport, or the rows of the model changed while it ran) */
    UInt64 rowCount;            /**< rows of data written, without the header row */
    UInt32 columnCount;
    UInt64 byteCount;           /**< bytes of the file */
    CString rootName;           /**< the root of the file save dialog the file went to; empty without a dialog */
    CString path;               /**< the path in that root, or what the caller wrote to */
    CString message;            /**< for the user: what was written, or why not */

    CGridCsvExportReport( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCSVEXPORTREPORT_H ? */

/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The report of a finished job of the pipeline for the report view of gucefGUI: a row per asset of the result of the job (its state and
 *  how long it took), the failures and approximations with the message of the converter, and a first row with the counts. The result is
 *  the document the service keeps of the job (GetJobResult()), the same for a job of this process and of a remote service.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchJobReport
{
    public:

    /** The rows of the report; the number of assets in it */
    static UInt32 BuildEntries( const ASSETSTORE::CAssetServiceDocument& result, GUI::CReportViewForm::TEntryVector& entries );

    /** The paths of the assets that failed in the result, once each */
    static void GetFailedAssets( const ASSETSTORE::CAssetServiceDocument& result, CORE::CString::StringVector& assetPaths );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREPORT_H ? */

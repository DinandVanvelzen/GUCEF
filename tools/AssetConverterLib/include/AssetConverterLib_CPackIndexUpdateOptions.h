/*
 *  AssetConverterLib: what a pack index update does besides the structure
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H
#define ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#include "AssetConverterLib_CPackRedoRequest.h"
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The inputs of CPackIndexUpdater::EnsureIndex() beyond the listing, the profile and the build options */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexUpdateOptions
{
    public:

    enum ELevel
    {
        LEVEL_STRUCTURE = 0 ,   /**< the listing, the sizes and the grouping: seconds */
        LEVEL_ANALYSIS          /**< and the headers of the files */
    };

    ELevel level;
    CPackRedoRequest redo;
    CORE::CString intermediateDir;                  /**< when not empty: the outputs of the cache entries and sidecars are copied into the records */
    const CConversionReport* report;                /**< the report of the run that just ended (optional): failed and refused assets */
    bool licenceRefused;                            /**< the licence record refuses the conversion: the conversion steps are marked refused, the rest is indexed */
    CORE::CString packVfsRoot;                      /**< where the files of the pack can be read (the analysis reads their headers); the listing's root when empty */
    CORE::CString userCacheRoot;                    /**< the VFS root of the mounted user cache (CPackIndexLocator::MountUserCache()): when the index folder given is empty the index lives in the folder of the pack key below it */

    CPackIndexUpdateOptions( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATEOPTIONS_H ? */


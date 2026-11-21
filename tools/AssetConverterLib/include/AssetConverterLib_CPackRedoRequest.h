/*
 *  AssetConverterLib: what the person asked to be done again in a pack index run
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

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/** The redo flags of section 5 of the design: nothing is the default, and each flag names the step it invalidates */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackRedoRequest
{
    public:

    bool reindex;           /**< build the structure and the grouping again (analysis records with valid keys are kept) */
    bool reanalyse;         /**< read the files again for statistics (conversion outputs and thumbnails are kept) */
    bool rethumbnail;       /**< render the thumbnails again */
    bool reimport;          /**< import and export again: the conversion cache is not used */
    bool retryFailed;       /**< analyse the resources that failed before, although nothing changed */
    bool verifyContent;     /**< hash the content of the files to verify the pack (a read of everything) */

    CPackRedoRequest( void );

    bool IsEmpty( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */


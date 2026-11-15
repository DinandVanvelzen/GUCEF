/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H
#define ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A unified diff of two texts by lines, for the previews of the authoring operations: hunks with three lines of
 *  context, "---" and "+++" headers with the names given, line endings ignored for the comparison. The longest common
 *  subsequence is computed for texts of up to 2000 lines each; longer texts give one hunk that replaces all lines. The
 *  output is cut after 'maxLines' lines, which 'truncated' reports.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CTextLineDiff
{
    public:

    static CORE::CString Unified( const CORE::CString& before        ,
                                  const CORE::CString& after         ,
                                  const CORE::CString& beforeName    ,
                                  const CORE::CString& afterName     ,
                                  UInt32 maxLines                    ,
                                  bool& truncated                    );

    /** The lines of a text without their line endings; a text that ends with a line ending has no empty last line */
    static void SplitLines( const CORE::CString& text, CORE::CString::StringVector& lines );

    private:

    CTextLineDiff( void );      /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CTEXTLINEDIFF_H ? */

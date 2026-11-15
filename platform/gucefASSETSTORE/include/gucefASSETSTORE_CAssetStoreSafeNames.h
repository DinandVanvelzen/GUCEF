/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Checks of the names of files that come from a store or from a pack (section 4.4): a name may hold folders, but it
 *  must stay inside the folder it is written to and be usable on every platform. Refused are: an empty name, absolute
 *  paths, drive letters and other colons, the parts "." and "..", empty parts, control characters, the characters
 *  < > " | ? *, parts that end in a dot or a space, the device names of Windows (CON, PRN, AUX, NUL, COM0..9, LPT0..9,
 *  alone or before an extension), parts longer than 255 bytes and names longer than 1024 bytes. Both '/' and '\' are
 *  separators.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreSafeNames
{
    public:

    static const UInt32 MaxNameLength = 1024;
    static const UInt32 MaxPartLength = 255;

    /** Whether the relative name is safe; 'reason' says why not */
    static bool IsSafeRelativeName( const CORE::CString& name, CORE::CString& reason );

    static bool IsSafeRelativeName( const CORE::CString& name );

    /** The name with '/' as the only separator */
    static CORE::CString NormalizeSeparators( const CORE::CString& name );

    /**
     *  A single folder or file name made from free text (a title, a seller): unsafe characters become '_', runs of
     *  blanks become one, at most 'maxLength' bytes, never empty ("_" then) and never a device name of Windows
     */
    static CORE::CString MakeSafePart( const CORE::CString& text, UInt32 maxLength = 80 );

    private:

    static bool IsReservedDeviceName( const CORE::CString& part );

    CAssetStoreSafeNames( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */

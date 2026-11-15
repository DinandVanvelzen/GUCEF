/*
 *  AssetConverterLib: the path arithmetic of VFS paths
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

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#define ASSETCONVERTERLIB_CVFSPATHS_H

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

/**
 *  VFS paths as the converter uses them: '/' between the parts, no '/' at the start or the end. These do not depend
 *  on the separator of the platform, unlike the file path functions of gucefCORE.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CVfsPaths
{
    public:

    /** Backslashes become '/', doubled and outer separators go */
    static CORE::CString Normalize( const CORE::CString& path );

    static CORE::CString Join( const CORE::CString& folder, const CORE::CString& name );

    /** The folder of the path; empty for a name without folder */
    static CORE::CString FolderOf( const CORE::CString& path );

    /** The last part of the path */
    static CORE::CString NameOf( const CORE::CString& path );

    /** The last part without its extension */
    static CORE::CString StemOf( const CORE::CString& path );

    /** The path without the extension of its last part */
    static CORE::CString WithoutExtension( const CORE::CString& path );

    /** The extension of the last part, lower case, without the dot; empty when there is none */
    static CORE::CString ExtensionOf( const CORE::CString& path );

    /** The path relative to the folder; false when it is not below the folder */
    static bool MakeRelative( const CORE::CString& folder, const CORE::CString& path, CORE::CString& relative );

    private:

    CVfsPaths( void );
    ~CVfsPaths();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

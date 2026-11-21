/*
 *  assetpluginUNREAL: An entry of the import table of a package
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#include "assetpluginUNREAL_CUnrealNameTable.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#include "assetpluginUNREAL_CUnrealPackageIndex.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGEINDEX_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealObjectImport
{
    public:

    CORE::CString classPackage;
    CORE::CString className;
    CUnrealPackageIndex outerIndex;
    CORE::CString objectName;
    CORE::CString packageName;          /**< empty when the package does not store it */
    bool isOptional;

    CUnrealObjectImport( void );

    bool Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, UInt32 packageFlags );
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H ? */

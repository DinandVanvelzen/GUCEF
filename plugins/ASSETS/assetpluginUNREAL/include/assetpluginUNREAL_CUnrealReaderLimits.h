/*
 *  assetpluginUNREAL: The memory and size budgets of the Unreal package readers
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/**
 *  Budgets that bound what the readers allocate and read. Every count read from a file is checked against the bytes that
 *  remain before memory is reserved; these limits bound what a well-formed file may ask for.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealReaderLimits
{
    public:

    UInt64 maxHeaderBytes;              /**< bytes of the package header that are read: the summary and all tables (default 64 MiB) */
    UInt64 maxPropertyBytes;            /**< tagged property data of one export that is read (default 256 MiB) */
    UInt32 windowBytes;                 /**< size of the read window of an archive reader (default 1 MiB) */
    UInt32 maxNameLength;               /**< code units of one string (default 1 MiB) */
    UInt32 maxDepth;                    /**< nesting of structs, containers and tags (default 64) */
    UInt64 maxElements;                 /**< elements of all containers of one property read together (default 16 Mi) */
    UInt32 maxTableEntries;             /**< entries of one table of the header (default 16 Mi) */
    UInt64 maxPayloadRawBytes;          /**< decompressed bytes of one payload that are held at once (default 1 GiB) */
    UInt32 maxBlockBytes;               /**< largest block of a compressed buffer that is decompressed (default 16 MiB) */

    CUnrealReaderLimits( void );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

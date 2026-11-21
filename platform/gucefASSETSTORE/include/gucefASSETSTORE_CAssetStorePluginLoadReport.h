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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H

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
 *  What CAssetStorePluginLoader did with a folder of plugins: which libraries it loaded as asset store plugins, which it refused
 *  and why, which it left alone because they are no asset store plugins, and the stores that registered. A person reads it as
 *  lines (GetLines()); nothing in it is a secret.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStorePluginLoadReport
{
    public:

    CORE::CString directory;                    /**< the folder as it was asked for, empty for none */
    bool directoryGiven;                        /**< a folder was set */
    bool directoryFound;                        /**< it is a folder that exists */
    UInt32 libraryCount;                        /**< the libraries in the folder */
    CORE::CString::StringVector loaded;         /**< the file names of the asset store plugins that were loaded */
    CORE::CString::StringVector refused;        /**< "file name: reason" of a library that is a store plugin or is broken, and was not loaded */
    CORE::CString::StringVector ignored;        /**< the file names of the libraries that are no asset store plugins */
    CORE::CString::StringVector stores;         /**< the ids of the stores that the plugins registered */

    CAssetStorePluginLoadReport( void );

    void Clear( void );

    /** Whether a folder was set, exists, and every asset store plugin in it was loaded: the empty folder counts */
    bool IsClean( void ) const;

    /** One line per fact, for a person and for the log: the folder, what was loaded, what was refused and why */
    void GetLines( CORE::CString::StringVector& lines ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H ? */

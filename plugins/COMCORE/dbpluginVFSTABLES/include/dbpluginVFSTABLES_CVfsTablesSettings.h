/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H
#define DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef DBPLUGIN_VFSTABLES_MACROS_H
#include "dbpluginVFSTABLES_macros.h"
#define DBPLUGIN_VFSTABLES_MACROS_H
#endif /* DBPLUGIN_VFSTABLES_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The backend options of a vfstables database (CDatabaseClientSettings::backendOptions, section 14.8 of the data
 *  tables design). Keys the backend does not know are ignored; a value that is not valid fails the open.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsTablesSettings
{
    public:

    static const UInt64 DefaultFooterCacheBytes = 64ULL * 1024 * 1024;
    static const UInt32 DefaultReplaceRetryTimeoutMs = 5000;

    CORE::CString writerId;             /**< names the temporary files of this writer; default host name + process id */
    UInt64 footerCacheBytes;            /**< budget of the footer cache, default 64 MiB; 0 caches nothing */
    UInt32 listingCacheTtlMs;           /**< 0: listings are kept until own writes or Refresh(); else relisted after this time */
    UInt32 replaceRetryTimeoutMs;       /**< how long a refused replace (a sharing violation on Windows) is retried, default 5000 */
    bool syncWrites;                    /**< a new partition file is flushed to storage before it replaces the old one, default true */
    bool skipIncompatibleFiles;         /**< onIncompatibleFile: "fail" (default) or "skip" */
    CORE::CValueList codecOptions;      /**< what reaches the table codec: ioCoalesceGapBytes, maxFooterBytes, maxPageBytes */

    CVfsTablesSettings( void );

    /** Takes the known keys of the backend options; false with the reason for a value that is not valid */
    bool LoadFromBackendOptions( const CORE::CValueList& backendOptions, CORE::CString& error );

    /** The writer id when none is configured: the host name and the process id */
    static CORE::CString GetDefaultWriterId( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSETTINGS_H ? */

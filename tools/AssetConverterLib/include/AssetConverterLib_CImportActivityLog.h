/*
 *  AssetConverterLib: the import activity log of an intermediate folder
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

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H
#define ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H
#include "AssetConverterLib_CImportActivityEntry.h"
#define ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H
#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYENTRY_H ? */


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
 *  The import activity log, <intermediate>/activity.json: per run its UTC time, the pack and an entry per asset (why
 *  it was imported again or not, and how long it took). Each run appends one run to the file. The log is not part of
 *  the intermediate or the cooked output, so its times do not affect their determinism.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CImportActivityLog
{
    public:

    typedef GUCEF::vector< CImportActivityEntry > TEntryVector;

    struct SRun
    {
        CORE::CString time;         /**< ISO 8601, UTC */
        CORE::CString packId;
        TEntryVector entries;
    };
    typedef GUCEF::vector< SRun > TRunVector;

    CImportActivityLog( void );

    const TRunVector& GetRuns( void ) const;

    void AddRun( const SRun& run );

    bool Load( const CORE::CString& vfsPath );
    bool Save( const CORE::CString& vfsPath ) const;

    /** Reads the log at the path (when there is one), adds the run and writes it back */
    static bool AppendRun( const CORE::CString& vfsPath, const SRun& run );

    /** <intermediate>/activity.json */
    static CORE::CString GetLogPath( const CORE::CString& intermediateDir );

    /** The current time, UTC, as ISO 8601 */
    static CORE::CString GetUtcTimeText( void );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    private:

    TRunVector m_runs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIMPORTACTIVITYLOG_H ? */

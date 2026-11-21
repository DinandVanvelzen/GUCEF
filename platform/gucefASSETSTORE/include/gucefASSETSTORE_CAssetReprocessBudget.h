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

#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H
#define GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#include "gucefASSETSTORE_CAssetReprocessSettings.h"
#define GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSSETTINGS_H ? */
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
 *  What the reprocessing of packs may use of the machine: the jobs at a time, the converter threads of each job and the memory
 *  the running jobs may be estimated to need together, from CAssetReprocessSettings and the facts of the machine (its logical
 *  cores and the memory limit of the process). A setting of 0 is automatic:
 *
 *  - jobs at a time: a quarter of the logical cores, at least 1 and at most 8 (28 cores: 7);
 *  - threads per job: the cores less reservedCores (at least 1), divided by the jobs at a time, at least 1 (28 cores, 7 jobs,
 *    2 reserved: 3), so that the busy converter threads of all jobs together stay within the cores the service does not keep;
 *  - memory budget: 70 percent of the memory limit of the container (cgroup v2 memory.max, cgroup v1 memory.limit_in_bytes)
 *    or of the physical memory of the machine when the container has no limit; 0 (no admission by memory) when neither is known.
 *
 *  A number set explicitly is taken as it is. The facts come from Detect*() or are given (the tests give their own).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetReprocessBudget
{
    public:

    UInt32 logicalCores;            /**< the logical cores the computation used */
    UInt64 memoryLimitBytes;        /**< the memory limit the computation used, 0 when not known */
    CORE::CString memoryLimitSource;/**< where it came from: "cgroup v2", "cgroup v1", "machine", "given" or "unknown" */
    UInt32 concurrentJobs;          /**< jobs at a time, at least 1 */
    UInt32 threadsPerJob;           /**< converter threads of one job, at least 1 */
    UInt64 memoryBudgetMB;          /**< 0: no admission by memory */
    bool jobsAutomatic;
    bool threadsAutomatic;
    bool memoryAutomatic;

    CAssetReprocessBudget( void );

    /** The budget of the settings for the facts given */
    void Compute( const CAssetReprocessSettings& settings, UInt32 cores, UInt64 memoryLimit, const CORE::CString& source );

    /**
     *  The memory a job is estimated to need in MiB: 0 when it converts nothing (a rollback, a download again without its
     *  conversion), otherwise memoryPerJobMB or memoryLargestAssetFactor times the largest asset of the pack, whichever is more
     */
    static UInt64 EstimateJobMemoryMB( const CAssetReprocessSettings& settings, bool converts, UInt64 largestAssetBytes );

    /** The numbers as attributes of 'node' (for the status of the reprocessing) */
    void Describe( CORE::CDataNode& node ) const;

    /** "7 jobs x 3 threads of 28 cores, memory budget 22937 MB of 32768 MB (cgroup v2)" */
    CORE::CString DescribeText( void ) const;

    /** The logical cores of the machine, at least 1 */
    static UInt32 DetectLogicalCores( void );

    /** The memory limit of the process: the one of its cgroup, else the physical memory; 0 when neither is known */
    static UInt64 DetectMemoryLimitBytes( CORE::CString& source );

    /**
     *  The memory limit of the cgroup of the process below 'cgroupRoot' (normally "/sys/fs/cgroup"), with 'selfCgroupFile'
     *  (normally "/proc/self/cgroup") naming its group: cgroup v2 memory.max, else cgroup v1 memory/memory.limit_in_bytes, of
     *  the group and else of the root (a container sees its own group as the root). 0 when there is none or it is unlimited.
     */
    static UInt64 ReadCgroupMemoryLimitBytes( const CORE::CString& cgroupRoot, const CORE::CString& selfCgroupFile, CORE::CString& source );

    /** The limit in a cgroup file: false for "max", an empty or unreadable text and the "no limit" values of cgroup v1 */
    static bool ParseCgroupMemoryLimit( const CORE::CString& text, UInt64& bytes );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETREPROCESSBUDGET_H ? */

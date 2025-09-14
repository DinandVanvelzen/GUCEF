/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_DVOSWRAP_H
#define GUCEF_CORE_DVOSWRAP_H

/*
 *      This file contains a set of O/S wrapping functions.
 *      The main idea is that you call these functions instead of the native
 *      O/S functions so that your code can become/stay portable.
 *      macro's are used to direct the function calls to the correct native
 *      O/S functions.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"        /* file & dir I/O */
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_C_POINTERDATA_H
#include "gucefCORE_c_pointerdata.h"
#define GUCEF_CORE_C_POINTERDATA_H
#endif /* GUCEF_CORE_C_POINTERDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

/*
 *      Prevent C++ Name mangeling
 */
#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define OSWRAP_BOOLINT  UInt32
#define OSWRAP_TRUE     1UL
#define OSWRAP_FALSE    0UL

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
typedef UInt32  TProcessId;
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
typedef pid_t   TProcessId;
#else
typedef UInt64  TProcessId;
#endif

/*-------------------------------------------------------------------------*/

struct SProcessInformation
{
    const char* commandLineArgs;
    UInt32 commandLineArgsByteSize;
    const char* imagePath;
    UInt32 imagePathByteSize;
};
typedef struct SProcessInformation TProcessInformation;

/*-------------------------------------------------------------------------*/

/**
 *  Structure holding various bits of memory usage related information
 *  If for the current O/S a field cannot be determined it will be set to 0
 */
struct SProcessMemoryUsageInfo
{
    #ifdef GUCEF_32BIT
    typedef UInt32 TProcMemStatInt;
    #else
    typedef UInt64 TProcMemStatInt;
    #endif

    TProcMemStatInt workingSetSizeInBytes;
    TProcMemStatInt peakWorkingSetSizeInBytes;
    TProcMemStatInt pageFaultCount;
    TProcMemStatInt pageFileUsageInBytes;
    TProcMemStatInt peakPageFileUsageInBytes;
};
typedef struct SProcessMemoryUsageInfo TProcessMemoryUsageInfo;

/*-------------------------------------------------------------------------*/

/**
 *  Structure holding various bits of memory usage related information
 *  If for the current O/S a field cannot be determined it will be set to 0
 */
struct SGlobalMemoryUsageInfo
{
    typedef UInt64 TMemStatInt;

    UInt8 memoryLoadPercentage;
    TMemStatInt totalPhysicalMemoryInBytes;
    TMemStatInt availablePhysicalMemoryInBytes;
    TMemStatInt totalPageFileSizeInBytes;
    TMemStatInt availablePageFileSizeInBytes;
    TMemStatInt totalVirtualMemoryInBytes;
    TMemStatInt availableVirtualMemoryInBytes;
    TMemStatInt availExtendedVirtualMemoryInBytes;
};
typedef struct SGlobalMemoryUsageInfo TGlobalMemoryUsageInfo;

/*-------------------------------------------------------------------------*/

/**
 *  Structure holding various bits of cpu usage related information
 *  If for the current O/S a field cannot be determined it will be set to 0
 */
struct SProcessCpuUsageInfo
{
    UInt64 uptimeInMs;
    Float64 overallCpuConsumptionPercentage;
    UInt32 nrOfThreads;
};
typedef struct SProcessCpuUsageInfo TProcessCpuUsageInfo;

/**
 *  In order to perform CPU usage measurements typically multiple measurements
 *  need to be taken, combining data points.
 *  The following structure is the platform wrapper for such data point storage
 */
struct SProcCpuDataPoint;
typedef struct SProcCpuDataPoint TProcCpuDataPoint;

/*-------------------------------------------------------------------------*/

struct SLogicalCpuStats
{
    Float32 cpuCurrentFrequencyInMhz;
    Float32 cpuSpecMaxFrequencyInMhz;
    Float32 cpuMaxFrequencyInMhz;
    Float64 cpuUsePercentage;
};
typedef struct SLogicalCpuStats TLogicalCpuStats;

/**
 *  Structure holding various bits of cpu usage related information
 *  If for the current O/S a field cannot be determined it will be set to 0
 */
struct SCpuStats
{
    UInt32 logicalCpuCount;
    TLogicalCpuStats* logicalCpuStats;
    Float64 cpuUsePercentage;
};
typedef struct SCpuStats TCpuStats;

/**
 *  In order to perform CPU usage measurements typically multiple measurements
 *  need to be taken, combining data points.
 *  The following structure is the platform wrapper for such data point storage
 */
struct SCpuDataPoint;
typedef struct SCpuDataPoint TCpuDataPoint;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void*
LoadModuleDynamicly( const char* filename );

/*--------------------------------------------------------------------------*/

/**
 *  Gets a pointer to an already loaded module. This function does NOT
 *  increment the reference count of the module referenced. If you do not know
 *  whether the modules is already loaded you should use LoadModuleDynamicly()
 *  instead of this function.
 */
GUCEF_CORE_PUBLIC_C void*
GetModulePointer( const char* moduleName );

/*--------------------------------------------------------------------------*/

/**
 *      Attempts to load the function from the module.
 *
 *      For MS Windows platform:
 *      If the function "functionname" isn't found then an attempt will
 *      be made to locate it using function decorations.
 *      Although C DLL's can be exchanged between different compilers in theory
 *      in practice they use different naming conventions. Unlike C++ the C
 *      exports are compatible if the same calling convention is used but the
 *      names tend to get mucked up.
 */
GUCEF_CORE_PUBLIC_C TAnyPointer
GetFunctionAddress( void *sohandle           ,
                    const char* functionname ,
                    UInt32 parambytes        );

/*--------------------------------------------------------------------------*/

/**
 *      Function that attempts to store the given string in the O/S clipboard
 *      If this functionality is not implemented for you O/S or if it fails then
 *      OSWRAP_FALSE will be returned. Otherwise OSWRAP_TRUE will be returned.
 *
 *      For MS windows:
 *              The handle to the primary window is needed for propper operation
 *              various attempts will be made to obtain it including getting the
 *              "HWND" environment variable. If these attempts fail an attempts will
 *              be made to use the previous clipboard owner HWND. You should set the
 *              "HWND" environment setting after creating your primary window using
 *              the GUCEFSetEnv() function.
 */
GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
StringToClipboard( const char *str );

/*--------------------------------------------------------------------------*/

/**
 *      Function that attempts to retrieve a string from the O/S clipboard
 *      If this functionality is not implemented for you O/S or if it fails then
 *      OSWRAP_FALSE will be returned. Otherwise OSWRAP_TRUE will be returned.
 *      It is assumed that dest has points to size number of allocated bytes.
 *      The actual number of bytes written into the destination buffer is
 *      written into wbytes. If you wish to retrieve all text on the clipboard
 *      you might want to continue calling this function as long as wbytes
 *      equals size. Do note that any other process can empty the clipboard
 *      in between calls.
 *
 *      wbytes must be initialized to 0 !!!
 *
 *      For MS windows:
 *              The handle to the primary window is needed for proper operation
 *              various attempts will be made to obtain it including getting the
 *              "HWND" environment variable. If these attempts fail an attempts will
 *              be made to use the previous clipboard owner HWND. You should set the
 *              "HWND" environment setting after creating your primary window using
 *              the GUCEFSetEnv() function.
 */
GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
StringFromClipboard( char *dest     ,
                     UInt32 size    ,
                     UInt32 *wbytes );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GUCEFSetEnv( const char* key   ,
             const char* value );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C const char*
GUCEFGetEnv( const char* key );

/*--------------------------------------------------------------------------*/

/**
 *      Returns the application tickcount in milliseconds
 */
GUCEF_CORE_PUBLIC_C UInt32
GUCEFGetTickCount( void );

/*--------------------------------------------------------------------------*/

/**
 *      Displays an O/S error message/dialog
 */
GUCEF_CORE_PUBLIC_C void
ShowErrorMessage( const char* message     ,
                  const char* description );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C UInt32
GetPhysicalCPUCount( void );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C UInt32
GetLogicalCPUCount( void );

/*--------------------------------------------------------------------------*/

/**
 *  Obtains a list of Process IDs for all the processes running on the system as visible to the
 *  account under which the software is executing
 */
GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetProcessList( TProcessId** processList ,
                UInt32* processCount     );

/*--------------------------------------------------------------------------*/

/**
 *  To be used with GetProcessList() to iterate the resulting processList
 */
GUCEF_CORE_PUBLIC_C TProcessId
GetProcessIdAtIndex( TProcessId* processList ,
                     UInt32 index            );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
FreeProcessList( TProcessId* processList );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
CheckOnProcessAliveStatus( TProcessId pid, OSWRAP_BOOLINT* status );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetProcessInformation( TProcessId pid            ,
                       TProcessInformation* info );

/*--------------------------------------------------------------------------*/

/**
 *  Attempts to obtain memory usage information for the specified process
 *  @param pid the process id of the process you wish to obtain information for
 *  @param memUseInfo output structure to hold the information that was gathered
 *  @param prevMemUseInfo optional input structure to hold the previous invocation's information. Needed for proper peak/delta determination on some systems
 */
GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetProcessMemoryUsage( TProcessId pid                          ,
                       TProcessMemoryUsageInfo* memUseInfo     ,
                       TProcessMemoryUsageInfo* prevMemUseInfo );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetExeNameForProcessId( TProcessId pid         ,
                        char* outNameBuffer    ,
                        UInt32 nameBufferSize  ,
                        UInt32* usedBufferSize );


/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetGlobalMemoryUsage( TGlobalMemoryUsageInfo* memUseInfo );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TProcCpuDataPoint*
CreateProcCpuDataPoint( TProcessId pid );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TProcCpuDataPoint*
CopyProcCpuDataPoint( TProcCpuDataPoint* srcCpuDataDataPoint ,
                      TProcessId newProcId                   );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C UInt32
GetProcessCpuUsage( TProcessId pid                              ,
                    TProcCpuDataPoint* previousCpuDataDataPoint ,
                    TProcessCpuUsageInfo* cpuUseInfo            );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
FreeProcCpuDataPoint( TProcCpuDataPoint* cpuDataDataPoint );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TCpuDataPoint*
CreateCpuDataPoint( void );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C OSWRAP_BOOLINT
GetCpuStats( TCpuDataPoint* previousCpuDataDataPoint ,
             TCpuStats** cpuStats                    );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
FreeCpuDataPoint( TCpuDataPoint* cpuDataDataPoint );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DVOSWRAP_H ? */


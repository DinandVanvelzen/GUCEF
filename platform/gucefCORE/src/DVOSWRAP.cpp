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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <sstream>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"       /* build configuration */
#define GUCEFCORE_MACROS_H
#endif /* GUCEFCORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  /* Do not use WIN32_LEAN_AND_MEAN because it will remove timeBeginPeriod() etc. */
  #undef  WIN32_LEAN_AND_MEAN
  #include <windows.h>                /* Windows API */
  #include <psapi.h>
  #include <powerbase.h>              /* needed for CPU stats */
  #undef min
  #undef max
  #define MAX_DIR_LENGTH MAX_PATH

  #ifndef GUCEF_CORE_MSWINUTILS_H
  #include "gucefCORE_mswinutils.h"
  #define GUCEF_CORE_MSWINUTILS_H
  #endif /* GUCEF_CORE_MSWINUTILS_H? */

#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

  #include <sys/sysinfo.h>
  #include <sys/times.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <limits.h>                 /* Linux OS limits */
  #include <dlfcn.h>                  /* dynamic linking utilities */
  #include <dirent.h>
  #include <unistd.h>
  #define MAX_DIR_LENGTH PATH_MAX

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

  #include <emscripten.h>
  #include <emscripten/threading.h>
  #include <pthread.h>
  #include <unistd.h>
  #include <sys/time.h>

#endif

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"               /* needed for str to int */
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#include "DVOSWRAP.h"               /* O/S Wrapping function prototypes*/

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
 // defined elsewhere right now
#else
struct SProcessId
{
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    pid_t pid;
    #else
    int foobar_NotSupported;
    #endif
};
#endif

/*--------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

class GUCEF_HIDDEN LinuxCpuFreqInfo
{
    private:

    float ReadFreqFromFile( const std::string& filePath )
    {GUCEF_TRACE;

        std::ifstream file(filePath);
        if (file.is_open()) {
            int freqKHz = 0;
            file >> freqKHz;
            return freqKHz / 1000.0; // Convert kHz to MHz
        }
        return 0;
    }

    public:

    int coreId;
    float scalingMinFreq; // in MHz
    float scalingMaxFreq; // in MHz
    float scalingCurFreq; // in MHz
    float cpuinfoMinFreq; // in MHz
    float cpuinfoMaxFreq; // in MHz

    void ReadAllCpuFreqInfo( void )
    {GUCEF_TRACE;

        std::ostringstream basePath;
        basePath << "/sys/devices/system/cpu/cpu" << coreId << "/cpufreq/";

        scalingMinFreq = ReadFreqFromFile(basePath.str() + "scaling_min_freq");
        scalingMaxFreq = ReadFreqFromFile(basePath.str() + "scaling_max_freq");
        scalingCurFreq = ReadFreqFromFile(basePath.str() + "scaling_cur_freq");
        cpuinfoMinFreq = ReadFreqFromFile(basePath.str() + "cpuinfo_min_freq");
        cpuinfoMaxFreq = ReadFreqFromFile(basePath.str() + "cpuinfo_max_freq");
    }

    void ReadCurrentCpuFreq( void )
    {GUCEF_TRACE;

        std::ostringstream basePath;
        basePath << "/sys/devices/system/cpu/cpu" << coreId << "/cpufreq/";
        scalingMinFreq = ReadFreqFromFile(basePath.str() + "scaling_min_freq");
        scalingMaxFreq = ReadFreqFromFile(basePath.str() + "scaling_max_freq");
        scalingCurFreq = ReadFreqFromFile(basePath.str() + "scaling_cur_freq");
    }

    LinuxCpuFreqInfo( int idOfCore )
        : coreId( idOfCore )
        , scalingMinFreq(0)
        , scalingMaxFreq(0)
        , scalingCurFreq(0)
        , cpuinfoMinFreq(0)
        , cpuinfoMaxFreq(0)
    {GUCEF_TRACE;

        ReadAllCpuFreqInfo();
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN LinuxSysDevicesCpuFreqReader
{
    public:

    typedef GUCEF::vector< LinuxCpuFreqInfo, gucef_allocator< LinuxCpuFreqInfo > >   TLinuxCpuFreqInfoVector;

    TLinuxCpuFreqInfoVector cpuFreqInfos;

    void InitAllCpuFreqInfo( void )
    {GUCEF_TRACE;

        cpuFreqInfos.clear();

        int coreId = 0;
        while (true)
        {
            std::ostringstream path;
            path << "/sys/devices/system/cpu/cpu" << coreId;
            std::ifstream dir( path.str() );
            if ( !dir.good() )
                break; // No more CPUs

            LinuxCpuFreqInfo cpuFreqInfo( coreId );
            cpuFreqInfos.push_back( cpuFreqInfo );

            ++coreId;
        }
    }

    void RefreshCpuFreq( void )
    {
        TLinuxCpuFreqInfoVector::iterator i = cpuFreqInfos.begin();
        while ( i != cpuFreqInfos.end() )
        {
            LinuxCpuFreqInfo& coreInfo = (*i);
            coreInfo.ReadCurrentCpuFreq();
            ++i;
        }
    }

    LinuxSysDevicesCpuFreqReader( void )
        : cpuFreqInfos()
    {GUCEF_TRACE;

        InitAllCpuFreqInfo();
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN LinuxLogicalCpuInfo
{
    public:

    UInt16 processor;
    CString vendor_id;
    UInt32 cpuFamily;
    UInt32 model;
    CString modelName;
    UInt32 stepping;
    Float32 cpuMhz;             // per the docs this is just a snapshot, not a sampling over a time interval
    UInt32 cacheSizeInBytes;
    UInt16 physicalId;
    UInt16 siblings;
    UInt16 coreId;
    UInt16 cpuCores;
    UInt32 apicid;
    UInt32 initialApicid;
    bool fpu;
    bool fpu_exception;
    UInt32 cpuidLevel;
    bool wp;
    CString::StringSet flags;
    CString::StringSet vmxFlags;
    CString::StringSet bugs;
    Float32 bogomips;
    UInt16 clflushSize;
    UInt16 cache_alignment;
    CString addressSizes;

    LinuxLogicalCpuInfo( void )
        : processor( 0 )
        , vendor_id()
        , cpuFamily( 0 )
        , model( 0 )
        , modelName()
        , stepping( 0 )
        , cpuMhz( 0.0 )
        , cacheSizeInBytes( 0 )
        , physicalId( 0 )
        , siblings( 0 )
        , coreId( 0 )
        , cpuCores( 0 )
        , apicid( 0 )
        , initialApicid( 0 )
        , fpu( false )
        , fpu_exception( false )
        , cpuidLevel( 0 )
        , wp( false )
        , flags()
        , vmxFlags()
        , bugs()
        , bogomips( 0.0 )
        , clflushSize( 0 )
        , cache_alignment( 0 )
        , addressSizes()
    {GUCEF_TRACE;

    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN AllLinuxProcCpuInfo
{
    public:

    typedef GUCEF::vector< LinuxLogicalCpuInfo >     LinuxLogicalCpuInfoVector;

    LinuxLogicalCpuInfoVector allCpuInfo;

    bool InitFromOS( void )
    {GUCEF_TRACE;

        FILE* procCpuInfoFile = fopen( "/proc/cpuinfo", "r" );
        if ( GUCEF_NULL != procCpuInfoFile )
        {
            allCpuInfo.clear();

            LinuxLogicalCpuInfo cpuInfo;
            char lineBuffer[ 2048 ];

            while ( GUCEF_NULL != fgets( lineBuffer, 2048, procCpuInfoFile ) )
            {
                // Lines with content have the form <key name><\t><:><value><\n>
                const char* tabChar = strchr( lineBuffer, '\t' );
                if ( GUCEF_NULL != tabChar )
                {
                    const char* colonChar = strchr( tabChar, ':' );
                    UInt32 keyStrLength = 0;
                    if ( GUCEF_NULL != colonChar )
                    {
                        keyStrLength = (UInt32) ( tabChar - lineBuffer );

                        if ( 0 == strncmp( "processor", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.processor = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "vendor_id", lineBuffer, keyStrLength ) )
                        {
                            const char* eolChar = strchr( colonChar+1, '\n' );
                            if ( GUCEF_NULL != eolChar )
                            {
                                cpuInfo.vendor_id.Set( colonChar+1, eolChar-(colonChar+1) );
                                cpuInfo.vendor_id.Trim( true );
                            }
                        }
                        else
                        if ( 0 == strncmp( "cpu family", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.cpuFamily = (UInt32) value;
                        }
                        else
                        if ( 0 == strncmp( "model", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.model = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "model name", lineBuffer, keyStrLength ) )
                        {
                            cpuInfo.modelName = colonChar+1;
                        }
                        else
                        if ( 0 == strncmp( "stepping", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.stepping = (UInt32) value;
                        }
                        else
                        if ( 0 == strncmp( "cpu MHz", lineBuffer, keyStrLength ) )
                        {
                            sscanf( colonChar+1, "%f", &cpuInfo.cpuMhz );
                        }
                        else
                        if ( 0 == strncmp( "cache size", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );

                            // Use old convention of KB == 1024 not 1000
                            cpuInfo.cacheSizeInBytes = (UInt32) value * 1024;
                        }
                        else
                        if ( 0 == strncmp( "physical id", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.physicalId = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "core id", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.coreId = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "cpu cores", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.cpuCores = (UInt16) value;
                        }
                        if ( 0 == strncmp( "apicid", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.apicid = (UInt32) value;
                        }
                        else
                        if ( 0 == strncmp( "initial apicid", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.initialApicid = (UInt32) value;
                        }
                        else
                        if ( 0 == strncmp( "fpu", lineBuffer, keyStrLength ) )
                        {
                            Int32 boolInt = String_To_Boolint( colonChar+1 );
                            cpuInfo.fpu = boolInt > 0;
                        }
                        else
                        if ( 0 == strncmp( "fpu_exception", lineBuffer, keyStrLength ) )
                        {
                            Int32 boolInt = String_To_Boolint( colonChar+1 );
                            cpuInfo.fpu_exception = boolInt > 0;
                        }
                        else
                        if ( 0 == strncmp( "cpuid level", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.cpuidLevel = (UInt32) value;
                        }
                        else
                        if ( 0 == strncmp( "wp", lineBuffer, keyStrLength ) )
                        {
                            Int32 boolInt = String_To_Boolint( colonChar+1 );
                            cpuInfo.wp = boolInt > 0;
                        }
                        else
                        if ( 0 == strncmp( "bogomips", lineBuffer, keyStrLength ) )
                        {
                            sscanf( colonChar+1, "%f", &cpuInfo.bogomips );
                        }
                        else
                        if ( 0 == strncmp( "clflush size", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.clflushSize = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "cache_alignment", lineBuffer, keyStrLength ) )
                        {
                            Int32 value = 0;
                            sscanf( colonChar+1, "%d", &value );
                            cpuInfo.cache_alignment = (UInt16) value;
                        }
                        else
                        if ( 0 == strncmp( "address sizes", lineBuffer, keyStrLength ) )
                        {
                            cpuInfo.addressSizes = colonChar+1;
                        }
                    }
                }
                else
                {
                    // Empty lines seperate the different sections
                    allCpuInfo.push_back( cpuInfo );
                }
            }
            fclose( procCpuInfoFile );
            return true;
        }
        return false;
    }

    AllLinuxProcCpuInfo( void )
        : allCpuInfo()
    {GUCEF_TRACE;

        InitFromOS();
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN LinuxCpuUsageBreakdown
{
    public:

    std::string cpu;
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;

    LinuxCpuUsageBreakdown()
        : user(0)
        , nice(0)
        , system(0)
        , idle(0)
        , iowait(0)
        , irq(0)
        , softirq(0)
        , steal(0)
        , guest(0)
        , guest_nice(0)
    {GUCEF_TRACE;
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN LinuxCpuUtilization
{
    public:

    typedef GUCEF::vector< LinuxCpuUsageBreakdown, gucef_allocator< LinuxCpuUsageBreakdown > >  TLinuxCpuUsageBreakdownVector;

    LinuxCpuUsageBreakdown globalCpuUsage;
    TLinuxCpuUsageBreakdownVector perCoreCpuUsage;

    static void ReadCpuUsage( LinuxCpuUsageBreakdown& globalUsage        ,
                              TLinuxCpuUsageBreakdownVector& cpuUsageVec )
    {GUCEF_TRACE;

        cpuUsageVec.clear();

        std::ifstream file("/proc/stat");
        std::string line;
        bool firstCpuLine = true;

        while ( std::getline( file, line ) )
        {
            if ( line.substr(0, 3) == "cpu" )
            {
                // Note that the first entry should be the overall CPU usage
                // All additional entries would be for the individual cores
                // As such we split those out explicitly to make it easier to consume

                std::istringstream ss(line);
                LinuxCpuUsageBreakdown usage;
                ss >> usage.cpu >> usage.user >> usage.nice >> usage.system >> usage.idle
                   >> usage.iowait >> usage.irq >> usage.softirq >> usage.steal
                   >> usage.guest >> usage.guest_nice;

                if ( firstCpuLine )
                {
                    globalUsage = usage;
                    firstCpuLine = false;
                }
                else
                {
                    cpuUsageVec.push_back( usage );
                }
            }
        }
    }

    static double CalculateCpuUtilization( const LinuxCpuUsageBreakdown& prev, const LinuxCpuUsageBreakdown& curr )
    {GUCEF_TRACE;

        unsigned long long prevIdle = prev.idle + prev.iowait;
        unsigned long long currIdle = curr.idle + curr.iowait;

        unsigned long long prevNonIdle = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;
        unsigned long long currNonIdle = curr.user + curr.nice + curr.system + curr.irq + curr.softirq + curr.steal;

        unsigned long long prevTotal = prevIdle + prevNonIdle;
        unsigned long long currTotal = currIdle + currNonIdle;

        unsigned long long totalDiff = currTotal - prevTotal;
        unsigned long long idleDiff = currIdle - prevIdle;

        return (totalDiff - idleDiff) * 100.0 / totalDiff;
    }

    void Refresh( void )
    {GUCEF_TRACE;

        ReadCpuUsage( globalCpuUsage, perCoreCpuUsage );
    }

    LinuxCpuUtilization( void )
    {GUCEF_TRACE;

        Refresh();
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN CLinuxProcStatusParser
{
    public:

    // Data members for known values in the status file
    CString name;                          // Name of the process
    Int32 pid;                             // Process ID
    Int32 ppid;                            // Parent process ID
    Int32 tracerPid;                       // PID of the process tracing this process (0 if not being traced)
    Int32 uid;                             // User ID of the process owner
    Int32 gid;                             // Group ID of the process owner
    Int32 fdSize;                          // Number of file descriptor slots currently allocated
    Int32 threads;                         // Number of threads in the process
    Int32 voluntaryCtxtSwitches;           // Number of voluntary context switches
    Int32 nonvoluntaryCtxtSwitches;        // Number of non-voluntary context switches
    UInt64 vmPeak;                         // Peak virtual memory size in bytes
    UInt64 vmSize;                         // Virtual memory size in bytes
    UInt64 vmLck;                          // Locked memory size in bytes
    UInt64 vmPin;                          // Pinned memory size in bytes
    UInt64 vmHWM;                          // Peak resident set size ("high water mark") in bytes
    UInt64 vmRSS;                          // Resident set size in bytes
    UInt64 rssAnon;                        // Anonymous resident set size in bytes
    UInt64 rssFile;                        // File resident set size in bytes
    UInt64 rssShmem;                       // Shared memory resident set size in bytes
    UInt64 vmData;                         // Data segment size in bytes
    UInt64 vmStk;                          // Stack size in bytes
    UInt64 vmExe;                          // Text (code) segment size in bytes
    UInt64 vmLib;                          // Shared library code size in bytes
    UInt64 vmPTE;                          // Page table entries size in bytes
    UInt64 vmPMD;                          // Page middle directory size in bytes
    UInt64 vmSwap;                         // Swapped-out virtual memory size in bytes
    CString::StringMap otherValues;        // Catch-all bucket for other values

    private:

    UInt64 NormalizeUnit( const UInt64 value, const CString& unit )
    {GUCEF_TRACE;

        if ( unit == "kB" )
        {
            return value * 1024; // Convert kB to bytes
        }

        // No conversion needed?
        return value;
    }

    CString NormalizeUnit( const CString& value, const CString& unit )
    {GUCEF_TRACE;

        if ( unit == "kB" )
        {
            return ToString( StringToInt64( value, 0 ) * 1024 ); // Convert kB to bytes
        }

        // No conversion needed?
        if ( !unit.IsNULLOrEmpty() )
            return value + " " + unit;
        else
            return value;
    }

    bool ParseFile( const CString& filePath )
    {GUCEF_TRACE;

        std::ifstream statusFile( filePath );
        if ( !statusFile.is_open() )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CLinuxProcStatusParser: Cannot open file: " + filePath );
            return false;
        }

        otherValues.clear();

        size_t parsedValues = 0;
        std::string line;
        while ( std::getline( statusFile, line ) )
        {
            std::istringstream lineStream( line );
            std::string key;
            std::string value;
            std::string unit;

            // Split the line into key, value, and unit
            if ( std::getline( lineStream, key, ':' ) )
            {
                lineStream >> value;

                // Also parse out the 'unit' if the line item has one
                // it will depend on the param type below whether its actually a unit or not
                // the NormalizeUnit function will sort that out.
                lineStream >> unit;

                // Parse known values
                if (key == "Name") {
                    name = value.c_str(); ++parsedValues;
                } else if (key == "Pid") {
                    pid = StringToInt32( value, 0 ); ++parsedValues;
                } else if (key == "PPid") {
                    ppid = StringToInt32( value, 0 ); ++parsedValues;
                } else if (key == "TracerPid") {
                    tracerPid = StringToInt32( value ); ++parsedValues;
                } else if (key == "Uid") {
                    uid = StringToInt32( value ); ++parsedValues;
                } else if (key == "Gid") {
                    gid = StringToInt32( value ); ++parsedValues;
                } else if (key == "FDSize") {
                    fdSize = StringToInt32( value, 0 ); ++parsedValues;
                } else if (key == "Threads") {
                    threads = StringToInt32( value, 0 ); ++parsedValues;
                } else if (key == "VmPeak") {
                    vmPeak = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmSize") {
                    vmSize = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmLck") {
                    vmLck = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmPin") {
                    vmPin = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmHWM") {
                    vmHWM = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmRSS") {
                    vmRSS = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "RssAnon") {
                    rssAnon = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "RssFile") {
                    rssFile = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "RssShmem") {
                    rssShmem = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmData") {
                    vmData = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmStk") {
                    vmStk = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmExe") {
                    vmExe = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmLib") {
                    vmLib = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmPTE") {
                    vmPTE = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmPMD") {
                    vmPMD = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "VmSwap") {
                    vmSwap = NormalizeUnit( StringToUInt64( value, 0 ), unit ); ++parsedValues;
                } else if (key == "voluntary_ctxt_switches") {
                    voluntaryCtxtSwitches = StringToInt32( value, 0 ); ++parsedValues;
                } else if (key == "nonvoluntary_ctxt_switches") {
                    nonvoluntaryCtxtSwitches = StringToInt32( value, 0 ); ++parsedValues;
                }
                else
                {
                    otherValues[ key ] = NormalizeUnit( value, unit );
                }
            }
        }
        parsedValues += otherValues.size();

        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CLinuxProcStatusParser: Parsed " + ToString( parsedValues ) + " values from file: " + filePath );
        return true;
    }

    public:

    bool Parse( pid_t pid )
    {GUCEF_TRACE;

        std::ostringstream procStatusPath;
        procStatusPath << "/proc/" << pid << "/status";
        return ParseFile( procStatusPath.str() );
    }

    void Clear( void )
    {GUCEF_TRACE;

        otherValues.clear();
        name.Clear();
        pid = 0;
        ppid = 0;
        tracerPid = 0;
        uid = 0;
        gid = 0;
        fdSize = 0;
        threads = 0;
        voluntaryCtxtSwitches = 0;
        nonvoluntaryCtxtSwitches = 0;
        vmPeak = 0;
        vmSize = 0;
        vmLck = 0;
        vmPin = 0;
        vmHWM = 0;
        vmRSS = 0;
        rssAnon = 0;
        rssFile = 0;
        rssShmem = 0;
        vmData = 0;
        vmStk = 0;
        vmExe = 0;
        vmLib = 0;
        vmPTE = 0;
        vmPMD = 0;
        vmSwap = 0;
    }

    CLinuxProcStatusParser( pid_t pid )
        : name(), pid(0), ppid(0), tracerPid(0), uid(0), gid(0), fdSize(0), threads(0),
          voluntaryCtxtSwitches(0), nonvoluntaryCtxtSwitches(0), vmPeak(0), vmSize(0), vmLck(0),
          vmPin(0), vmHWM(0), vmRSS(0), rssAnon(0), rssFile(0), rssShmem(0), vmData(0), vmStk(0),
          vmExe(0), vmLib(0), vmPTE(0), vmPMD(0), vmSwap(0)
    {GUCEF_TRACE;

        Parse( pid );
    }

    CLinuxProcStatusParser( void )
        : name(), pid(0), ppid(0), tracerPid(0), uid(0), gid(0), fdSize(0), threads(0),
          voluntaryCtxtSwitches(0), nonvoluntaryCtxtSwitches(0), vmPeak(0), vmSize(0), vmLck(0),
          vmPin(0), vmHWM(0), vmRSS(0), rssAnon(0), rssFile(0), rssShmem(0), vmData(0), vmStk(0),
          vmExe(0), vmLib(0), vmPTE(0), vmPMD(0), vmSwap(0)
    {GUCEF_TRACE;

    }

    CString
    GetOtherValue( const CString& key ) const
    {GUCEF_TRACE;

        CString::StringMap::const_iterator it = otherValues.find( key );
        if ( it != otherValues.end() )
        {
            return it->second;
        }
        return CString::Empty;
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN CLinuxProcStatmParser
{
    public:

    // Data members for values in the statm file
    unsigned long size;     // Total program size
    unsigned long resident; // Resident set size
    unsigned long shared;   // Shared pages
    unsigned long text;     // Text (code)
    unsigned long lib;      // Library (unused in Linux 2.6)
    unsigned long data;     // Data + stack
    unsigned long dirty;    // Dirty pages (unused in Linux 2.6)

    private:

    bool ParseFile( const CString& filePath )
    {GUCEF_TRACE;

        std::ifstream statmFile( filePath );
        if ( !statmFile.is_open() )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CLinuxProcStatmParser: Cannot open file: " + filePath );
            return false;
        }

        statmFile >> size >> resident >> shared >> text >> lib >> data >> dirty;
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CLinuxProcStatmParser: Successfully parsed file: " + filePath );
        return true;
    }

    public:

    void Clear( void )
    {
        size = 0;
        resident = 0;
        shared = 0;
        text = 0;
        lib = 0;
        data = 0;
        dirty = 0;
    }

    bool Parse( pid_t pid )
    {GUCEF_TRACE;

        std::ostringstream procStatmPath;
        procStatmPath << "/proc/" << pid << "/statm";
        return ParseFile( procStatmPath.str() );
    }

    CLinuxProcStatmParser( pid_t pid )
        : size(0), resident(0), shared(0), text(0), lib(0), data(0), dirty(0)
    {GUCEF_TRACE;

        Parse( pid );
    }

    CLinuxProcStatmParser( void )
        : size(0), resident(0), shared(0), text(0), lib(0), data(0), dirty(0)
    {GUCEF_TRACE;
    }
};

/*--------------------------------------------------------------------------*/

class GUCEF_HIDDEN LinuxProcStatParser
{
    public:

    pid_t pid;                          // Process ID
    std::string comm;                   // Filename of the executable, in parentheses
    char state;                         // State (R: running, S: sleeping, D: disk sleep, Z: zombie, T: stopped, t: tracing stop, W: paging, X: dead, x: dead, K: wakekill, W: waking, P: parked)
    pid_t ppid;                         // Parent process ID
    pid_t pgrp;                         // Process group ID
    pid_t session;                      // Session ID
    int tty_nr;                         // Controlling terminal of the process
    pid_t tpgid;                        // ID of the foreground process group of the controlling terminal
    unsigned int flags;                 // Kernel flags word of the process
    unsigned long minflt;               // Number of minor faults the process has made which have not required loading a memory page from disk
    unsigned long cminflt;              // Number of minor faults the process and its children have made
    unsigned long majflt;               // Number of major faults the process has made which have required loading a memory page from disk
    unsigned long cmajflt;              // Number of major faults the process and its children have made
    unsigned long utime;                // Amount of time the process has been scheduled in user mode, measured in clock ticks
    unsigned long stime;                // Amount of time the process has been scheduled in kernel mode, measured in clock ticks
    long cutime;                        // Amount of time the process's children have been scheduled in user mode, measured in clock ticks
    long cstime;                        // Amount of time the process's children have been scheduled in kernel mode, measured in clock ticks
    long priority;                      // Priority value (nice value + 20)
    long nice;                          // Nice value (range: -20 to 19)
    long num_threads;                   // Number of threads in the process
    long itrealvalue;                   // Obsolete, always 0
    unsigned long long starttime;       // Time the process started after system boot, measured in clock ticks
    unsigned long vsize;                // Virtual memory size in bytes
    long rss;                           // Resident Set Size: number of pages the process has in real memory
    unsigned long rsslim;               // Current soft limit in bytes on the RSS of the process
    unsigned long startcode;            // Address above which program text can run
    unsigned long endcode;              // Address below which program text can run
    unsigned long startstack;           // Address of the start of the stack
    unsigned long kstkesp;              // Current value of ESP (stack pointer)
    unsigned long kstkeip;              // Current value of EIP (instruction pointer)
    unsigned long signal;               // Bitmap of pending signals
    unsigned long blocked;              // Bitmap of blocked signals
    unsigned long sigignore;            // Bitmap of ignored signals
    unsigned long sigcatch;             // Bitmap of caught signals
    unsigned long wchan;                // "Channel" in which the process is waiting
    unsigned long nswap;                // Number of pages swapped (not maintained)
    unsigned long cnswap;               // Cumulative nswap for child processes (not maintained)
    int exit_signal;                    // Signal to be sent to parent when the process dies
    int processor;                      // CPU number last executed on
    unsigned int rt_priority;           // Real-time scheduling priority
    unsigned int policy;                // Scheduling policy
    unsigned long long delayacct_blkio_ticks; // Aggregated block I/O delays, measured in clock ticks
    unsigned long guest_time;           // Guest time of the process (time spent running a virtual CPU for a guest operating system), measured in clock ticks
    long cguest_time;                   // Guest time of the process's children, measured in clock ticks
    unsigned long start_data;           // Address above which program data+bss is placed
    unsigned long end_data;             // Address below which program data+bss is placed
    unsigned long start_brk;            // Address above which program heap can be expanded with brk()
    unsigned long arg_start;            // Address above which program command-line arguments (argv) are placed
    unsigned long arg_end;              // Address below which program command-line arguments (argv) are placed
    unsigned long env_start;            // Address above which program environment is placed
    unsigned long env_end;              // Address below which program environment is placed
    int exit_code;                      // The thread's exit status

    private:

    bool ParseFile( const CString& filePath )
    {GUCEF_TRACE;

        std::ifstream statFile( filePath );
        if ( !statFile.is_open() )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "LinuxProcStatParser: Cannot open file: " + filePath );
            return false;
        }

        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);

        iss >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
            >> utime >> stime >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue >> starttime >> vsize >> rss >> rsslim
            >> startcode >> endcode >> startstack >> kstkesp >> kstkeip >> signal >> blocked >> sigignore >> sigcatch >> wchan >> nswap
            >> cnswap >> exit_signal >> processor >> rt_priority >> policy >> delayacct_blkio_ticks >> guest_time >> cguest_time >> start_data
            >> end_data >> start_brk >> arg_start >> arg_end >> env_start >> env_end >> exit_code;


        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "LinuxProcStatParser: Successfully parsed file: " + filePath );
        return true;
    }

    public:

    bool Parse( pid_t pid )
    {GUCEF_TRACE;

        std::ostringstream procStatPath;
        procStatPath << "/proc/" << pid << "/stat";
        return ParseFile( procStatPath.str() );
    }

    LinuxProcStatParser( pid_t pid )
        : pid(pid), comm(), state(0), ppid(0), pgrp(0), session(0), tty_nr(0), tpgid(0), flags(0), minflt(0), cminflt(0), majflt(0), cmajflt(0),
          utime(0), stime(0), cutime(0), cstime(0), priority(0), nice(0), num_threads(0), itrealvalue(0), starttime(0), vsize(0), rss(0), rsslim(0),
          startcode(0), endcode(0), startstack(0), kstkesp(0), kstkeip(0), signal(0), blocked(0), sigignore(0), sigcatch(0), wchan(0), nswap(0),
          cnswap(0), exit_signal(0), processor(0), rt_priority(0), policy(0), delayacct_blkio_ticks(0), guest_time(0), cguest_time(0), start_data(0),
          end_data(0), start_brk(0), arg_start(0), arg_end(0), env_start(0), env_end(0), exit_code(0)
    {GUCEF_TRACE;

        Parse( pid );
    }

    LinuxProcStatParser( void )
        : pid(0), comm(), state(0), ppid(0), pgrp(0), session(0), tty_nr(0), tpgid(0), flags(0), minflt(0), cminflt(0), majflt(0), cmajflt(0),
          utime(0), stime(0), cutime(0), cstime(0), priority(0), nice(0), num_threads(0), itrealvalue(0), starttime(0), vsize(0), rss(0), rsslim(0),
          startcode(0), endcode(0), startstack(0), kstkesp(0), kstkeip(0), signal(0), blocked(0), sigignore(0), sigcatch(0), wchan(0), nswap(0),
          cnswap(0), exit_signal(0), processor(0), rt_priority(0), policy(0), delayacct_blkio_ticks(0), guest_time(0), cguest_time(0), start_data(0),
          end_data(0), start_brk(0), arg_start(0), arg_end(0), env_start(0), env_end(0), exit_code(0)
    {GUCEF_TRACE;

    }
};

/*--------------------------------------------------------------------------*/


#endif

/*--------------------------------------------------------------------------*/
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

int
GetWin32ProcThreadCount( DWORD pid )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer;
    ULONG neededBufferSize = 0;
    NTSTATUS ntStatus = TryNtQuerySystemInformation( SystemProcessInformation ,
                                                     buffer.GetBufferPtr()    ,
                                                     buffer.GetBufferSize()   ,
                                                     &neededBufferSize        );
    if ( STATUS_INFO_LENGTH_MISMATCH == ntStatus )
    {
        // This is expected, we need to allocate sufficient space in the buffer and try again
        if ( buffer.SetBufferSize( neededBufferSize, false, true ) )
        {
            ntStatus = TryNtQuerySystemInformation( SystemProcessInformation ,
                                                    buffer.GetBufferPtr()    ,
                                                    buffer.GetBufferSize()   ,
                                                    &neededBufferSize        );
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetWin32ProcThreadCount: failed to allocate buffer of size " + ToString( (UInt64) neededBufferSize ) );
            return -1;
        }
    }

    if ( WIN32_NT_SUCCESS( ntStatus ) )
    {
        // Successfully retrieved the global process information
        // We will now iterate through the list of processes to find the one we are looking for
        PSYSTEM_PROCESS_INFORMATION processInfo = (PSYSTEM_PROCESS_INFORMATION) buffer.GetBufferPtr();
        while ( processInfo->NextEntryOffset != 0 )
        {
            // In reality process id and thread id is of type HANDLE.
            // this is handled in the special PspCidTable.
            // the win32 layer for some reason defines it as DWORD, but all native api use it as HANDLE
            DWORD entryPid = (DWORD)(ULONG)(ULONG_PTR) processInfo->UniqueProcessId;
            if ( entryPid == pid )
            {
                // We found the process we are looking for among the list of processes
                return processInfo->NumberOfThreads;
            }
            processInfo = (PSYSTEM_PROCESS_INFORMATION) ( (PBYTE)processInfo + processInfo->NextEntryOffset );
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetWin32ProcThreadCount: No entry for PID " + ToString( (UInt64) pid ) );
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetWin32ProcThreadCount: call to TryNtQuerySystemInformation failed with NTSTATUS " + ToString( ntStatus ) );
    }

    // Could not obtain the information
    return -1;
}

#endif

/*--------------------------------------------------------------------------*/

struct SCpuDataPoint
{
    TCpuStats cpuStats;
    UInt32 logicalCpuCount; // we keep a copy here in the private stucture for fast and safe access. Someone could alter the one in cpuStats

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    PPROCESSOR_POWER_INFORMATION cpuPowerInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* cpuPerfInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* prevCpuPerfInfo;
    FILETIME globalUserTime;
    FILETIME globalKernelTime;
    FILETIME globalIdleTime;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    AllLinuxProcCpuInfo* infoFromProcCpu;
    LinuxSysDevicesCpuFreqReader* infoFromSysDevCpuFreq;
    LinuxCpuUtilization* infoFromProcStat;
    #else

    #endif
};

struct SProcCpuDataPoint
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    DWORD pid;
    HANDLE hProcess;
    FILETIME procUserTime;
    FILETIME procKernelTime;
    FILETIME globalUserTime;
    FILETIME globalKernelTime;
    FILETIME globalIdleTime;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    pid_t pid;
    UInt64 procUserModeJiffies;
    UInt64 procKernelModeJiffies;
    UInt64 globalJiffies;
    #else
    int foobar_NotSupported;
    #endif
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void*
LoadModuleDynamicly( const char* filename )
{GUCEF_TRACE;

    // redirect to the C++ version
    CString fName( filename );
    return LoadModuleDynamicly( fName );
}

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void*
GetModulePointer( const char* moduleName )
{GUCEF_TRACE;

    // redirect to the C++ version
    CString mName( moduleName );
    return GetModulePointer( mName );
}

/*--------------------------------------------------------------------------*/

TAnyPointer
GetFunctionAddress( void* sohandle           ,
                    const char* functionname ,
                    UInt32 parambytes        )
{GUCEF_TRACE;

    TAnyPointer fptr;
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == sohandle || GUCEF_NULL == functionname )
    {
        fptr.funcPtr = 0;
        return fptr;
    }

    // redirect to the C++ version
    CString funcName( functionname );
    return GetFunctionAddress( sohandle, funcName, parambytes );
}

/*--------------------------------------------------------------------------*/

#ifdef GUCEF_MSWIN_BUILD

static HWND
GetCurrentHWND( void )
{
        const char* hwndstr;
        HWND whandle = NULL;

        #ifdef SDL_SUPPORT

        /*
         *      If SDL is supported we will try to get the HWND from SDL
         */
        SDL_SysWMinfo winfo;
        SDL_GetWMInfo( &winfo );
        whandle = winfo.window;

        if ( !whandle ) {

        #endif

        #pragma warning( disable: 4047 ) // 'HWND' differs in levels of indirection from 'Int32'

        /*
         *      Try to get the HWND from the process environment settings
         */
        hwndstr = GUCEFGetEnv( "HWND" );
        if ( hwndstr != GUCEF_NULL )
        {

                #ifdef GUCEF_64BIT
                whandle = (HWND) StringToInt64( hwndstr );
                #else
                whandle = (HWND) Str_To_Int( hwndstr );
                #endif
        }
        else
        {
                /*
                 *      If all previous attempts failed then try to use the previous
                 *      clipboard owner (if there is one).
                 */
                whandle = GetClipboardOwner();
        }

        #ifdef SDL_SUPPORT
        }
        #endif

        return whandle;
}

#endif /* GUCEF_MSWIN_BUILD ? */

/*--------------------------------------------------------------------------*/

OSWRAP_BOOLINT
StringToClipboard( const char *str )
{
    #ifdef GUCEF_MSWIN_BUILD

    if ( str )
    {
        UInt32 strlength = (UInt32) strlen( str );

        HWND whandle = GetCurrentHWND();
        UInt32 success = OSWRAP_FALSE;
        HGLOBAL hglbcopy;
        LPTSTR lptstrcopy;

        /*
         *      Open the clipboard with the current task as the owner
         */
        if ( !OpenClipboard( whandle ) ) return OSWRAP_FALSE;

        /*
         *      Try to empty the clipboard so that we can get ownership of the
         *      clipboard which is needed for placing data on it.
         */
        EmptyClipboard();

        /*
         *      Allocate global MS windows managed memory for the text
         */
        hglbcopy = GlobalAlloc( GMEM_MOVEABLE                 ,
                                (strlength+1) * sizeof(TCHAR) );
        if ( hglbcopy == NULL )
        {
                CloseClipboard();
                return OSWRAP_FALSE;
        }

        /*
         *      Now we have to lock the memory we just allocated so that
         *      windows keeps it's paws off of it. After that we can copy
         *      our text into the global memory buffer
         */
        lptstrcopy = (LPTSTR) GlobalLock( hglbcopy );
        memcpy( lptstrcopy, str, strlength+1 );
        GlobalUnlock( hglbcopy );

        /*
         *      We now attempt to set the string in the clipboard.
         *      This will fail if the handle we used in OpenClipboard() is
         *      NULL or incorrect.
         */
        if ( SetClipboardData( CF_TEXT, hglbcopy ) )
        {
            success = OSWRAP_TRUE;
        }
        else
        {
            success = OSWRAP_FALSE;
        }

        /*
         *      Close the clipboard so that other tasks have access again
         */
        CloseClipboard();

        return success;
    }
    return OSWRAP_FALSE;

    #else /* GUCEF_MSWIN_BUILD */

    return OSWRAP_FALSE;

    #endif /* OS WRAPPING */
}

/*--------------------------------------------------------------------------*/

/**
 *      Function that attempts to retrieve a string from the O/S clipboard
 *      If this functionality is not implemented for you O/S or if it fails then
 *      OSWRAP_FALSE will be returned. Otherwise OSWRAP_TRUE will be returned.
 *      It is assumed that dest has points to size number of allocated bytes.
 *      The actual number of bytes written into the destination buffer is
 *      written into wbytes. If you wish to retrieve all text on the clipboard
 *      you might want to continue calling this function as long as wbytes
 *      equals size. Do note that any other proccess can empty the clipboard
 *      in between calls.
 */
OSWRAP_BOOLINT
StringFromClipboard( char *dest     ,
                     UInt32 size    ,
                     UInt32 *wbytes )
{
        #ifdef GUCEF_MSWIN_BUILD

        HWND whandle = GetCurrentHWND();
        HGLOBAL hglb;
        UInt32 success = OSWRAP_FALSE;

        #ifdef SDL_SUPPORT

        SDL_SysWMinfo winfo;

        #endif /* SDL_SUPPORT */

        if ( IsClipboardFormatAvailable( CF_TEXT ) )
        {
                /*
                 *      Open the clipboard with the current task as the owner
                 */
                if ( !OpenClipboard( whandle ) ) return OSWRAP_FALSE;

                /*
                 *      Get a pointer to the data using the global handle we will
                 *      obtain if possible. Then copy the data pointed to by the handle
                 *      into the buffer provided by the user
                 */
                hglb = GetClipboardData( CF_TEXT );
                if ( hglb != NULL )
                {
                        LPTSTR lptstr = (LPTSTR) GlobalLock( hglb );
                        if ( lptstr != NULL )
                        {
                                UInt32 offset = *wbytes;
                                UInt32 dsize = (UInt32) strlen( lptstr );
                                if ( dsize > offset )
                                {
                                        dsize -= offset;
                                        if ( dsize > size-1 ) dsize = size-1;
                                        strncpy( dest, lptstr+offset, dsize );
                                        *wbytes += dsize;

                                        success = OSWRAP_TRUE;
                                }
                                GlobalUnlock( hglb );
                        }
                        else
                        {
                                *wbytes = 0;
                        }
                }

                /*
                 *      Close the clipboard so that other tasks have access again
                 */
                CloseClipboard();

                return success;
        }
        return OSWRAP_FALSE;

        #else /* GUCEF_MSWIN_BUILD */

        return OSWRAP_FALSE;

        #endif /* OS WRAPPING */
}

/*--------------------------------------------------------------------------*/

OSWRAP_BOOLINT
GUCEFSetEnv( const char* key   ,
             const char* value )
{GUCEF_TRACE;

    #ifdef GUCEF_MSWIN_BUILD

    UInt32 retval;
    char* envstr = (char*) malloc( strlen( key ) + strlen( value )+2 );
    sprintf( envstr, "%s=%s", key, value );
    retval = _putenv( envstr );
    free( envstr );
    return retval == 0;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Environment variables are not supported in Emscripten
    return OSWRAP_FALSE;

    #else

    return setenv( key, value, 1 ) == 0;

    #endif
}

/*--------------------------------------------------------------------------*/

const char*
GUCEFGetEnv( const char* key )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // Environment variables are not supported in Emscripten
    return GUCEF_NULL;

    #else

    return getenv( key );

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GUCEFGetTickCount( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    return GetTickCount();

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    #if 1
    struct timespec now;
    clock_gettime( CLOCK_MONOTONIC, &now );
    return (UInt32) ( now.tv_sec*1000000000LL + now.tv_nsec );
    #else
    struct tms timeStorage;
    return (UInt32) times( &timeStorage );
    #endif

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    return static_cast<UInt32>(emscripten_get_now());

    #else
    #error unsupported platform
    #endif
}

/*--------------------------------------------------------------------------*/

void
ShowErrorMessage( const char* message     ,
                  const char* description )
{GUCEF_TRACE;

    #ifdef GUCEF_MSWIN_BUILD
    if ( !IsRunningAsService() )
    {
        MessageBox( GetCurrentHWND()                    ,
                    description                         ,
                    message                             ,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL );
    }
    else
    {
        fprintf( stderr, "%s : %s\n", message, description );
    }
    #else
    fprintf( stderr, "%s : %s\n", message, description );
    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetCPUCountPerPackage( void )
{GUCEF_TRACE;

    /* Number of Logical Cores per Physical Processor */
    UInt32 coreCount = 1;


//::GetLogicalProcessorInformationEx


    /* Initialize to 1 to support older processors. */
    #if ( ( GUCEF_COMPILER == GUCEF_COMPILER_MSVC ) && ( GUCEF_CPU_ARCHITECTURE == GUCEF_CPU_ARCHITECTURE_X86 ) )
    _asm
    {
        mov		eax, 1
        cpuid
        /* Test for HTT bit */
        test	edx, 0x10000000
        jz		Unp
        /* Multi-core or Hyperthreading supported...
        // Read the "# of Logical Processors per Physical Processor" field: */
        mov		eax, ebx
        and		eax, 0x00FF0000 /* Mask the "logical core counter" byte */
        shr		eax, 16 // Shift byte to be least-significant
        mov		coreCount, eax
        /* Uniprocessor (i.e. Pentium III or any AMD CPU excluding their new
        dual-core A64)  */
        Unp:
        /* coreCount will contain 1. */
    }
    #endif

    return coreCount;
}

/*--------------------------------------------------------------------------*/

UInt32
GetPhysicalCPUCount( void )
{GUCEF_TRACE;

    return GetLogicalCPUCount() / GetCPUCountPerPackage();
}

/*--------------------------------------------------------------------------*/

UInt32
GetLogicalCPUCount( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    SYSTEM_INFO systemInfo;
    ::GetSystemInfo( &systemInfo );
    return systemInfo.dwNumberOfProcessors;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    return (UInt32) ::sysconf( _SC_NPROCESSORS_ONLN );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    return emscripten_num_logical_cores();
    #else
    return 1;
    #endif
}

/*--------------------------------------------------------------------------*/

OSWRAP_BOOLINT
GetProcessList( TProcessId** processList ,
                UInt32* processCount     )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL == processCount || GUCEF_NULL == processList )
        return OSWRAP_FALSE;

    // Just call the C++ implementation and convert the result for C
    TProcessIdVector procIds;
    if ( GetProcessList( procIds ) )
    {
        *processList = (TProcessId*) calloc( procIds.size(), sizeof(TProcessId) );
        if ( GUCEF_NULL != *processList )
        {
            for ( size_t i=0; i<procIds.size(); ++i )
            {
                (*processList)[ i ] = procIds[ i ];
                ++i;
            }
            OSWRAP_TRUE;
        }
    }

    #endif

    return OSWRAP_FALSE;
}

/*--------------------------------------------------------------------------*/

void
FreeProcessList( TProcessId* processList )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != processList )
        free( processList );

    #endif
}

/*--------------------------------------------------------------------------*/

TProcessId
GetProcessIdAtIndex( TProcessId* processList ,
                     UInt32 index            )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != processList )
        return processList[ index ];

    #endif

    return 0;
}

/*--------------------------------------------------------------------------*/

OSWRAP_BOOLINT
CheckOnProcessAliveStatus( TProcessId pid, OSWRAP_BOOLINT* status )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != status )
    {
        // Just call the C++ implementation
        bool isAlive = false;
        if ( CheckOnProcessAliveStatus( pid, isAlive ) )
        {
            *status = isAlive ? OSWRAP_TRUE : OSWRAP_FALSE;
            return OSWRAP_TRUE;
        }
    }

    #endif

    return OSWRAP_FALSE;
}

/*--------------------------------------------------------------------------*/

UInt32
GetProcessMemoryUsage( TProcessId pid                          ,
                       TProcessMemoryUsageInfo* memUseInfo     ,
                       TProcessMemoryUsageInfo* prevMemUseInfo )
{GUCEF_TRACE;

    if ( GUCEF_NULL == memUseInfo )
        return OSWRAP_FALSE;

    memset( memUseInfo, 0, sizeof(TProcessMemoryUsageInfo) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    HANDLE hProcess = ::OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                      FALSE,
                                      pid );
    if ( GUCEF_NULL == hProcess )
        return OSWRAP_FALSE;

    PROCESS_MEMORY_COUNTERS pmc;
    if ( ::GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc) ) )
    {
        memUseInfo->pageFaultCount = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.PageFaultCount;
        memUseInfo->pageFileUsageInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.PagefileUsage;
        memUseInfo->peakPageFileUsageInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.PeakPagefileUsage;
        memUseInfo->peakWorkingSetSizeInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.PeakWorkingSetSize;
        //memUseInfo->quotaNonPagedPoolUsageInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.QuotaNonPagedPoolUsage;
        //memUseInfo->quotaPagedPoolUsageInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.QuotaPagedPoolUsage;
        //memUseInfo->quotaPeakNonPagedPoolUsageInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.QuotaPeakNonPagedPoolUsage;
        //memUseInfo->quotaPeakPagedPoolUsageInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.QuotaPeakPagedPoolUsage;
        memUseInfo->workingSetSizeInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) pmc.WorkingSetSize;

        ::CloseHandle( hProcess );
        return OSWRAP_TRUE;
    }

    ::CloseHandle( hProcess );
    return OSWRAP_FALSE;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // pageFaultCountInBytes is available from stat
    //          /proc/<pid>/stat
    LinuxProcStatParser procStatParser;
    if ( procStatParser.Parse( pid ) )
    {
        memUseInfo->pageFaultCount = (TProcessMemoryUsageInfo::TProcMemStatInt) ( procStatParser.minflt + procStatParser.majflt );
    }
    else
    {
        memUseInfo->pageFaultCount = 0;
    }

    // Try to get the other stats from /proc/<pid>/status
    CLinuxProcStatusParser procStatusParser;
    if ( procStatusParser.Parse( pid ) )
    {
        memUseInfo->pageFileUsageInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatusParser.vmSwap;
        memUseInfo->peakWorkingSetSizeInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatusParser.vmHWM;
        memUseInfo->workingSetSizeInBytes  = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatusParser.vmRSS;

        // peakPageFileUsageInBytes is not available
        // we will have no monitor it ourselves through retention of the last vmSwap value and repeated invocation of this function
        // Note that this is merely an approximation over time but its the best we can do on Linux
        if ( GUCEF_NULL != prevMemUseInfo )
        {
            if ( procStatusParser.vmSwap > prevMemUseInfo->peakPageFileUsageInBytes )
                memUseInfo->peakPageFileUsageInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatusParser.vmSwap;
            else
                memUseInfo->peakPageFileUsageInBytes = prevMemUseInfo->peakPageFileUsageInBytes;
        }
        return OSWRAP_TRUE;
    }
    else
    {
        // Try to use /proc/<pid>/statm instead. Note that it doesn't have the same level of detail
        // we will be missing some stats
        CLinuxProcStatmParser procStatmParser;
        if ( procStatmParser.Parse( pid ) )
        {
            long sizeOfPageInBytes = ::sysconf( _SC_PAGESIZE );

            if ( 0 != procStatmParser.dirty ) // <- always 0 in later Linux versions, will only work on old Linux kernels
                memUseInfo->pageFaultCount = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatmParser.dirty * sizeOfPageInBytes;
            memUseInfo->pageFileUsageInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatmParser.shared * sizeOfPageInBytes;
            memUseInfo->workingSetSizeInBytes = (TProcessMemoryUsageInfo::TProcMemStatInt) procStatmParser.resident * sizeOfPageInBytes;

            return OSWRAP_TRUE;
        }

        return OSWRAP_FALSE;
    }

    #else

    return OSWRAP_FALSE;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetGlobalMemoryUsage( TGlobalMemoryUsageInfo* memUseInfo )
{GUCEF_TRACE;

    if ( GUCEF_NULL == memUseInfo )
        return OSWRAP_FALSE;

    memset( memUseInfo, 0, sizeof(TGlobalMemoryUsageInfo) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    ::MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if ( 0 != ::GlobalMemoryStatusEx( &statex ) )
    {
        memUseInfo->memoryLoadPercentage = (UInt8) statex.dwMemoryLoad;
        memUseInfo->totalPhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullTotalPhys;
        memUseInfo->availablePhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullAvailPhys;
        memUseInfo->totalPageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullTotalPageFile;
        memUseInfo->availablePageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullAvailPageFile;
        memUseInfo->totalVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullTotalVirtual;
        memUseInfo->availableVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullAvailVirtual;
        memUseInfo->availExtendedVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) statex.ullAvailExtendedVirtual;

        return OSWRAP_TRUE;
    }
    else
    {
        return OSWRAP_FALSE;
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    // VERBIAGE WARNING:
    //
    // Note that Linux considers memory that is used, but can be made available, as 'Available'
    // Free memory is memory that is not used for any purpose at all including sneaky caching
    // Hence from an application perspective, trying to guard against 'running out of memory' the
    // value you care about on Linux is 'available' memory.
    // The excess bytes of free > available is what Linux uses for things like system managed buffers and disk caches

    #if 1

    struct sysinfo info;
    if ( -1 != ::sysinfo( &info ) )
    {
        memUseInfo->totalPhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( info.totalram * info.mem_unit );
        memUseInfo->availablePhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( ( info.freeram + info.bufferram + info.sharedram ) * info.mem_unit );
        memUseInfo->memoryLoadPercentage = (UInt8) ( 100 - ( memUseInfo->availablePhysicalMemoryInBytes / ( 0.01 * memUseInfo->totalPhysicalMemoryInBytes ) ) );
        memUseInfo->totalPageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( info.totalswap * info.mem_unit );
        memUseInfo->availablePageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( info.freeswap * info.mem_unit );
        memUseInfo->totalVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( (info.totalram + info.totalswap) * info.mem_unit );
        memUseInfo->availableVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( info.freeram * info.mem_unit );
        memUseInfo->availExtendedVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( (info.freeram + info.freeswap) * info.mem_unit );

        return OSWRAP_TRUE;
    }
    return OSWRAP_FALSE;

    #else

    CString memInfo;
    if ( LoadTextFileAsString( "/proc/meminfo", memInfo, true, "\n" ) )
    {
        memInfo = memInfo.CompactRepeatingChar( ' ' );
        memInfo = memInfo.ReplaceSubstr( " kB", CString::Empty );
        CString::StringVector memInfoItems = memInfo.ParseElements( '\n', false );

        TGlobalMemoryUsageInfo::TMemStatInt usedVirtualMemoryInBytes = 0;
        CString::StringVector::iterator i = memInfoItems.begin();
        while ( i != memInfoItems.end() )
        {
            // Note: these values are not in kB but in KiB contrary to what the manual and the file says
            //       hence factor 1024 applies not 1000

            const CString& memInfoEntry = (*i);
            if ( 0 == memInfoEntry.HasSubstr( "MemTotal:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 10, false );
                memUseInfo->totalPhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else  /*  We dont currently have a place to store free vs available
            if ( 0 == memInfoEntry.HasSubstr( "MemFree:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 9, false );
                memUseInfo->freePhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else  */
            if ( 0 == memInfoEntry.HasSubstr( "MemAvailable:", 0, true ) )
            {
                // MemAvailable = MemFree + Buffers + Cached
                CString nrStr = memInfoEntry.SubstrToIndex( 14, false );
                memUseInfo->availablePhysicalMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else
            if ( 0 == memInfoEntry.HasSubstr( "SwapTotal:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 11, false );
                memUseInfo->totalPageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else
            if ( 0 == memInfoEntry.HasSubstr( "SwapFree:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 10, false );
                memUseInfo->availablePageFileSizeInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else
            if ( 0 == memInfoEntry.HasSubstr( "VmallocTotal:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 14, false );
                memUseInfo->totalVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }
            else
            if ( 0 == memInfoEntry.HasSubstr( "VmallocUsed:", 0, true ) )
            {
                CString nrStr = memInfoEntry.SubstrToIndex( 13, false );
                usedVirtualMemoryInBytes = (TGlobalMemoryUsageInfo::TMemStatInt) ( StringToUInt64( nrStr, 0 ) * 1024 );
            }

            ++i;
        }

        if ( memUseInfo->totalVirtualMemoryInBytes > usedVirtualMemoryInBytes )
            memUseInfo->availableVirtualMemoryInBytes = memUseInfo->totalVirtualMemoryInBytes - usedVirtualMemoryInBytes;
        memUseInfo->memoryLoadPercentage = (UInt8) ( 100 - ( memUseInfo->availablePhysicalMemoryInBytes / ( 0.01 * memUseInfo->totalPhysicalMemoryInBytes ) ) );

        return OSWRAP_TRUE;
    }

    return OSWRAP_FALSE;

    #endif // alternate implementation

    #else

    return OSWRAP_FALSE;

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetExeNameForProcessId( TProcessId pid         ,
                        char* outNameBuffer    ,
                        UInt32 nameBufferSize  ,
                        UInt32* usedBufferSize )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL == outNameBuffer || GUCEF_NULL == usedBufferSize )
        return OSWRAP_FALSE;

    // Just call the C++ implementation
    CString outStr;
    if ( GetExeNameForProcessId( pid, outStr ) )
    {
        size_t nrOfBytes = (size_t) GUCEF_SMALLEST( nameBufferSize, outStr.ByteSize() );
        char* srcStr = outStr.C_String();
        if ( GUCEF_NULL != srcStr && nrOfBytes > 0 )
        {
            memcpy( outNameBuffer, srcStr, nrOfBytes );
            srcStr[ nrOfBytes-1 ] = '\0'; // Guarantee a null terminator
        }

        *usedBufferSize = 0;
        return OSWRAP_TRUE;
    }

    #endif

    return OSWRAP_FALSE;
}

/*--------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

Int64
GetFiletimeDelta( LPFILETIME a, LPFILETIME b )
{GUCEF_TRACE;

    ULARGE_INTEGER converterStructA;
    converterStructA.HighPart = a->dwHighDateTime;
    converterStructA.LowPart = a->dwLowDateTime;

    ULARGE_INTEGER converterStructB;
    converterStructB.HighPart = b->dwHighDateTime;
    converterStructB.LowPart = b->dwLowDateTime;

    return converterStructA.QuadPart - converterStructB.QuadPart;
}

/*--------------------------------------------------------------------------*/

Int64
GetFiletimeAsUInt64( LPFILETIME a )
{GUCEF_TRACE;

    ULARGE_INTEGER converterStructA;
    converterStructA.HighPart = a->dwHighDateTime;
    converterStructA.LowPart = a->dwLowDateTime;
    return converterStructA.QuadPart;
}

/*--------------------------------------------------------------------------*/

Int64
GetDurationSinceFiletimeInMs( LPFILETIME since )
{GUCEF_TRACE;

    if ( NULL == since )
        return 0;

    FILETIME systemNowTime;
    ::GetSystemTimeAsFileTime( &systemNowTime );

    Int64 timeDeltaInHundredNanoSecs = GetFiletimeDelta( &systemNowTime, since );

    // 100 nanoseconds is 0.0001 ms
    // so we need to divide by 10000
    return (Int64) ( timeDeltaInHundredNanoSecs / 10000 );
}

#endif

/*--------------------------------------------------------------------------*/

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

bool
GetProcUptimeAndThreads( pid_t pid, UInt64* uptimeInMs, UInt32* nrOfThreads )
{GUCEF_TRACE;

    if ( GUCEF_NULL == uptimeInMs || GUCEF_NULL == nrOfThreads )
        return false;
    *uptimeInMs = 0;
    *nrOfThreads = 0;

    LinuxProcStatParser statParser;
    if ( statParser.Parse( pid ) )
    {
        *nrOfThreads = static_cast< UInt32 >( statParser.num_threads );

        // R  Running
        // S  Sleeping in an interruptible wait
        // D  Waiting in uninterruptible disk sleep
        // Z  Zombie
        // T  Stopped (on a signal) or (before Linux 2.6.33)
        //    trace stopped
        // t  Tracing stop (Linux 2.6.33 onward)
        // W  Paging (only before Linux 2.6.0)
        // X  Dead (from Linux 2.6.0 onward)
        // x  Dead (Linux 2.6.33 to 3.13 only)
        // K  Wakekill (Linux 2.6.33 to 3.13 only)
        // W  Waking (Linux 2.6.33 to 3.13 only)
        // P  Parked (Linux 3.9 to 3.13 only)
        switch ( statParser.state )
        {
            case 'Z':
            case 'X':
            case 'x':
            {
                // Since the proc is dead the uptime should be 0
                return true;
            }
        }

        // The proc appears by all indications still alive
        // we now get the actual uptime
        char procStatPath[ 24 ];
        sprintf( procStatPath, "/proc/%d", pid );
        struct stat statBuffer;
        if ( 0 != ::stat( procStatPath, &statBuffer ) )
        {
            // Proc probaby died while executing this code
            return false;
        }

        time_t nowTime = ::time( NULL );
        *uptimeInMs = ( ( nowTime - statBuffer.st_mtim.tv_sec ) * 1000 ) + ( statBuffer.st_mtim.tv_nsec / 100000000 );
        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------*/

/*

Table 1-3: Contents of the stat files (as of 2.6.22-rc3)

 Field          Content
  pid           process id
  tcomm         filename of the executable
  state         state (R is running, S is sleeping, D is sleeping in an
                uninterruptible wait, Z is zombie, T is traced or stopped)
  ppid          process id of the parent process
  pgrp          pgrp of the process
  sid           session id
  tty_nr        tty the process uses
  tty_pgrp      pgrp of the tty
  flags         task flags
  min_flt       number of minor faults
  cmin_flt      number of minor faults with child's
  maj_flt       number of major faults
  cmaj_flt      number of major faults with child's
  utime         user mode jiffies
  stime         kernel mode jiffies
  cutime        user mode jiffies with child's
  cstime        kernel mode jiffies with child's
*/

bool
GetProcJiffies( pid_t pid, UInt64* userModeJiffies, UInt64* kernelModeJiffies )
{GUCEF_TRACE;

    if ( GUCEF_NULL == userModeJiffies || GUCEF_NULL == kernelModeJiffies )
        return false;

    LinuxProcStatParser statParser;
    if ( statParser.Parse( pid ) )
    {
        *userModeJiffies = static_cast< UInt64 >( statParser.utime );
        *kernelModeJiffies = static_cast< UInt64 >( statParser.stime );
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
GetGlobalJiffies( UInt64* totalJiffies )
{GUCEF_TRACE;

    if ( GUCEF_NULL == totalJiffies )
        return false;

    FILE* fp = NULL;
    if ( ( fp = ::fopen( "/proc/stat", "r" ) ) == NULL )
        return false;

    char cpuLine[ 1024 ];
    int eRead = ::fscanf( fp, "%*s %[^\n]", cpuLine );
    if ( eRead != 1 )
    {
        fclose( fp );
        return false;
    }
    fclose( fp );

    Int64 allJiffies = 0;
    Int64 moreJiffies = 0;
    const char* cpuLineOffset = cpuLine;
    while ( *cpuLineOffset != '\0' && ::sscanf( cpuLineOffset, "%ld", &moreJiffies ) == 1 )
    {
        allJiffies += moreJiffies;

        ++cpuLineOffset;
        while ( *cpuLineOffset != '\0' && *cpuLineOffset != ' ' )
            ++cpuLineOffset;
    }
    *totalJiffies = (UInt64) allJiffies;
    return true;
}

#endif

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TProcCpuDataPoint*
CreateProcCpuDataPoint( TProcessId pid )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( 0 == pid )
        return GUCEF_NULL;

    TProcCpuDataPoint* dataPoint = (TProcCpuDataPoint*) malloc( sizeof( TProcCpuDataPoint ) );
    if ( GUCEF_NULL == dataPoint )
        return GUCEF_NULL;

    memset( dataPoint, 0, sizeof( TProcCpuDataPoint ) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    dataPoint->pid = pid;
    dataPoint->hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION,
                                         FALSE,
                                         pid );
    if ( GUCEF_NULL != dataPoint->hProcess )
    {
        FILETIME dummy;
        ::GetProcessTimes( dataPoint->hProcess, &dummy, &dummy, &dataPoint->procKernelTime, &dataPoint->procUserTime );
        ::GetSystemTimes( &dataPoint->globalIdleTime, &dataPoint->globalKernelTime, &dataPoint->globalUserTime );
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CreateProcCpuDataPoint: Failed to open process handle for pid " + ToString( pid ) );
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    dataPoint->pid = pid;
    GetGlobalJiffies( &dataPoint->globalJiffies );
    GetProcJiffies( pid, &dataPoint->procUserModeJiffies, &dataPoint->procKernelModeJiffies );
    #endif

    return dataPoint;

    #else

    return GUCEF_NULL;

    #endif
}

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TProcCpuDataPoint*
CopyProcCpuDataPoint( TProcCpuDataPoint* srcCpuDataDataPoint ,
                      TProcessId newProcId                   )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL == srcCpuDataDataPoint )
        return GUCEF_NULL;

    TProcCpuDataPoint* dataPoint = (TProcCpuDataPoint*) malloc( sizeof( TProcCpuDataPoint ) );
    if ( GUCEF_NULL == dataPoint )
        return GUCEF_NULL;
    memset( dataPoint, 0, sizeof( TProcCpuDataPoint ) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( 0 == newProcId )
        dataPoint->pid = srcCpuDataDataPoint->pid;
    else
        dataPoint->pid = newProcId;

    dataPoint->hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION,
                                         FALSE,
                                         dataPoint->pid );
    if ( GUCEF_NULL == dataPoint->hProcess )
    {
        free( dataPoint );
        return GUCEF_NULL;
    }

    dataPoint->globalIdleTime = srcCpuDataDataPoint->globalIdleTime;
    dataPoint->globalKernelTime = srcCpuDataDataPoint->globalKernelTime;
    dataPoint->globalUserTime = srcCpuDataDataPoint->globalUserTime;
    dataPoint->procKernelTime = srcCpuDataDataPoint->procKernelTime;
    dataPoint->procUserTime = srcCpuDataDataPoint->procUserTime;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    if ( 0 == newProcId )
        dataPoint->pid = srcCpuDataDataPoint->pid;
    else
        dataPoint->pid = newProcId;

    dataPoint->globalJiffies = srcCpuDataDataPoint->globalJiffies;
    dataPoint->procUserModeJiffies = srcCpuDataDataPoint->procUserModeJiffies;
    dataPoint->procKernelModeJiffies = srcCpuDataDataPoint->procKernelModeJiffies;

    #endif

    return dataPoint;

    #else

    return GUCEF_NULL;

    #endif
}

/*--------------------------------------------------------------------------*/

void
FreeProcCpuDataPoint( TProcCpuDataPoint* cpuDataDataPoint )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != cpuDataDataPoint )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        if ( GUCEF_NULL != cpuDataDataPoint->hProcess )
        {
            ::CloseHandle( cpuDataDataPoint->hProcess );
            cpuDataDataPoint->hProcess = GUCEF_NULL;
        }
        #endif

        free( cpuDataDataPoint );
    }

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetProcessCpuUsage( TProcessId pid                              ,
                    TProcCpuDataPoint* previousCpuDataDataPoint ,
                    TProcessCpuUsageInfo* cpuUseInfo            )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL == previousCpuDataDataPoint || GUCEF_NULL == cpuUseInfo )
        return OSWRAP_FALSE;

    if ( previousCpuDataDataPoint->pid != pid )
        return OSWRAP_FALSE;

    memset( cpuUseInfo, 0, sizeof(TProcessCpuUsageInfo) );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    if ( GUCEF_NULL == previousCpuDataDataPoint->hProcess )
    {
        previousCpuDataDataPoint->hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION,
                                                            FALSE,
                                                            pid );
        if ( GUCEF_NULL == previousCpuDataDataPoint->hProcess )
            return OSWRAP_FALSE;
    }
    else
    {
        // Keeping the handle open between samples also keeps the process object around between CPU samplings
        // As such this may prevent us from properly letting go of the O/S resources
        // We must always check if the process exited if we are to hold onto a process handle
        DWORD exitCode = 0;
        if ( ( ::GetExitCodeProcess( previousCpuDataDataPoint->hProcess, &exitCode ) == FALSE ) || exitCode != STILL_ACTIVE )
        {
            ::CloseHandle( previousCpuDataDataPoint->hProcess );
            previousCpuDataDataPoint->hProcess = GUCEF_NULL;
            return OSWRAP_FALSE;
        }
    }

    FILETIME globalIdleTime;
    FILETIME globalKernelTime;
    FILETIME globalUserTime;
    if ( ::GetSystemTimes( &globalIdleTime, &globalKernelTime, &globalUserTime ) != TRUE )
        return OSWRAP_FALSE;

    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if ( ::GetProcessTimes( previousCpuDataDataPoint->hProcess, &creationTime, &exitTime, &kernelTime, &userTime ) == TRUE )
    {
        cpuUseInfo->uptimeInMs = (UInt64) GetDurationSinceFiletimeInMs( &creationTime );

        UInt64 globalCpuTotal = GetFiletimeAsUInt64( &globalKernelTime ) + GetFiletimeAsUInt64( &globalUserTime );
        UInt64 prevGlobalCpuTotal = GetFiletimeAsUInt64( &previousCpuDataDataPoint->globalKernelTime ) + GetFiletimeAsUInt64( &previousCpuDataDataPoint->globalUserTime );
        Float64 globalCpuUseDelta = (Float64) ( globalCpuTotal - prevGlobalCpuTotal );
        if ( globalCpuUseDelta == 0 )
            globalCpuUseDelta = 0.01;

        UInt64 procCpuTotal = GetFiletimeAsUInt64( &kernelTime ) + GetFiletimeAsUInt64( &userTime );
        UInt64 prevProcCpuTotal = GetFiletimeAsUInt64( &previousCpuDataDataPoint->procKernelTime ) + GetFiletimeAsUInt64( &previousCpuDataDataPoint->procUserTime );
        Float64 procCpuUseDelta = (Float64) ( procCpuTotal - prevProcCpuTotal );

        cpuUseInfo->overallCpuConsumptionPercentage = procCpuUseDelta / ( globalCpuUseDelta / 100.0 );

        // Overwrite the CPU data point making current the previous for next call to this function
        previousCpuDataDataPoint->globalIdleTime = globalIdleTime;
        previousCpuDataDataPoint->globalKernelTime = globalKernelTime;
        previousCpuDataDataPoint->globalUserTime = globalUserTime;
        previousCpuDataDataPoint->procKernelTime = kernelTime;
        previousCpuDataDataPoint->procUserTime = userTime;
    }

    int nrOfThreads = GetWin32ProcThreadCount( pid );
    if ( nrOfThreads > 0 )
        cpuUseInfo->nrOfThreads = (UInt32) nrOfThreads;

    return OSWRAP_TRUE;

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    UInt64 globalJiffies = 0;
    UInt64 procKernelModeJiffies = 0;
    UInt64 procUserModeJiffies = 0;
    if ( GetGlobalJiffies( &globalJiffies ) &&
         GetProcJiffies( pid, &procUserModeJiffies, &procKernelModeJiffies ) )
    {
        // Calculate the % of total CPU jiffies spent used by this proc during the time
        // that has passed between the 2 data sample points in time

        Int64 globalTotalJiffiesDelta = globalJiffies - previousCpuDataDataPoint->globalJiffies;
        if ( globalTotalJiffiesDelta == 0 )
            globalTotalJiffiesDelta = 1;

        Float64 userModeJiffiesDelta = procUserModeJiffies - previousCpuDataDataPoint->procUserModeJiffies;
        Float64 kernelModeJiffiesDelta = procKernelModeJiffies - previousCpuDataDataPoint->procKernelModeJiffies;
        Float64 userModePerc = 100.0 * ( userModeJiffiesDelta / globalTotalJiffiesDelta );
        Float64 kernelModePerc = 100.0 * ( kernelModeJiffiesDelta / globalTotalJiffiesDelta );

        // Overwrite the CPU data point making current the previous for next call to this function
        previousCpuDataDataPoint->procKernelModeJiffies = procKernelModeJiffies;
        previousCpuDataDataPoint->procUserModeJiffies = procUserModeJiffies;
        previousCpuDataDataPoint->globalJiffies = globalJiffies;

        cpuUseInfo->overallCpuConsumptionPercentage = userModePerc + kernelModePerc;
    }
    else
    {
        return OSWRAP_FALSE;
    }

    GetProcUptimeAndThreads( pid, &cpuUseInfo->uptimeInMs, &cpuUseInfo->nrOfThreads );

    return OSWRAP_TRUE;

    #else

    return OSWRAP_FALSE;

    #endif
    #else

    return OSWRAP_FALSE;

    #endif

}

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C TCpuDataPoint*
CreateCpuDataPoint( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    TCpuDataPoint* dataPoint = (TCpuDataPoint*) malloc( sizeof( TCpuDataPoint ) );
    if ( GUCEF_NULL == dataPoint )
        return GUCEF_NULL;
    memset( dataPoint, 0, sizeof( TCpuDataPoint ) );

    dataPoint->cpuStats.logicalCpuCount = GetLogicalCPUCount();
    dataPoint->logicalCpuCount = dataPoint->cpuStats.logicalCpuCount;

    UInt32 cpuStatsDataSize = sizeof( TLogicalCpuStats ) * dataPoint->cpuStats.logicalCpuCount;
    dataPoint->cpuStats.logicalCpuStats = (TLogicalCpuStats*) malloc( cpuStatsDataSize );
    if ( GUCEF_NULL == dataPoint->cpuStats.logicalCpuStats )
    {
        free( dataPoint );
        return GUCEF_NULL;
    }
    memset( dataPoint->cpuStats.logicalCpuStats, 0, cpuStatsDataSize );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    UInt32 cpuPowerInfoDataSize = sizeof( PROCESSOR_POWER_INFORMATION ) * dataPoint->cpuStats.logicalCpuCount;
    dataPoint->cpuPowerInfo = (PPROCESSOR_POWER_INFORMATION) malloc( cpuPowerInfoDataSize );
    if ( GUCEF_NULL == dataPoint->cpuPowerInfo )
    {
        free( dataPoint->cpuStats.logicalCpuStats );
        free( dataPoint );
        return GUCEF_NULL;
    }
    memset( dataPoint->cpuPowerInfo, 0, cpuPowerInfoDataSize );

    UInt32 cpuPerfInfoDataSize = sizeof( SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ) * dataPoint->cpuStats.logicalCpuCount;
    dataPoint->cpuPerfInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*) malloc( cpuPerfInfoDataSize );
    if ( GUCEF_NULL == dataPoint->cpuPerfInfo )
    {
        free( dataPoint->cpuPowerInfo );
        free( dataPoint->cpuStats.logicalCpuStats );
        free( dataPoint );
        return GUCEF_NULL;
    }
    memset( dataPoint->cpuPerfInfo, 0, cpuPerfInfoDataSize );
    dataPoint->prevCpuPerfInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*) malloc( cpuPerfInfoDataSize );
    if ( GUCEF_NULL == dataPoint->prevCpuPerfInfo )
    {
        free( dataPoint->cpuPerfInfo );
        free( dataPoint->cpuPowerInfo );
        free( dataPoint->cpuStats.logicalCpuStats );
        free( dataPoint );
        return GUCEF_NULL;
    }
    memset( dataPoint->prevCpuPerfInfo, 0, cpuPerfInfoDataSize );

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    dataPoint->infoFromProcCpu = new AllLinuxProcCpuInfo();
    dataPoint->infoFromSysDevCpuFreq = new LinuxSysDevicesCpuFreqReader();
    dataPoint->infoFromProcStat = new LinuxCpuUtilization();

    #endif

    return dataPoint;

    #else

    return GUCEF_NULL;

    #endif
}

/*--------------------------------------------------------------------------*/

void
FreeCpuDataPoint( TCpuDataPoint* cpuDataPoint )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL != cpuDataPoint )
    {
        if ( GUCEF_NULL != cpuDataPoint->cpuStats.logicalCpuStats )
            free( cpuDataPoint->cpuStats.logicalCpuStats );

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

        if ( GUCEF_NULL != cpuDataPoint->cpuPowerInfo )
            free( cpuDataPoint->cpuPowerInfo );
        if ( GUCEF_NULL != cpuDataPoint->cpuPerfInfo )
            free( cpuDataPoint->cpuPerfInfo );
        if ( GUCEF_NULL != cpuDataPoint->prevCpuPerfInfo )
            free( cpuDataPoint->prevCpuPerfInfo );

        #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        delete cpuDataPoint->infoFromProcCpu;
        cpuDataPoint->infoFromProcCpu = GUCEF_NULL;
        delete cpuDataPoint->infoFromSysDevCpuFreq;
        cpuDataPoint->infoFromSysDevCpuFreq = GUCEF_NULL;
        delete cpuDataPoint->infoFromProcStat;
        cpuDataPoint->infoFromProcStat = GUCEF_NULL;

        #endif

        free( cpuDataPoint );
    }

    #endif
}

/*--------------------------------------------------------------------------*/

UInt32
GetCpuStats( TCpuDataPoint* previousCpuDataDataPoint ,
             TCpuStats** cpuStats                    )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    if ( GUCEF_NULL == previousCpuDataDataPoint || GUCEF_NULL == cpuStats )
        return OSWRAP_FALSE;
    *cpuStats = GUCEF_NULL;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // For getting the overall CPU usage the recommended way is to "Use GetSystemTimes instead to retrieve this information."
    // Do note that while the below follows said recommendation from Microsoft, it is not the most accurate way to get the CPU usage relative to
    // the per core CPU usage which is also obtained and thus there may be some discrepancies between the two values.
    // Then again the NtQuerySystemInformation call is not guaranteed to work as its an undocumented API call.
    FILETIME globalIdleTime;
    FILETIME globalKernelTime;
    FILETIME globalUserTime;
    if ( ::GetSystemTimes( &globalIdleTime, &globalKernelTime, &globalUserTime ) != TRUE )
        return OSWRAP_FALSE;

    UInt64 globalCpuIdle = GetFiletimeAsUInt64( &globalIdleTime );
    UInt64 prevGlobalCpuIdle = GetFiletimeAsUInt64( &previousCpuDataDataPoint->globalIdleTime );
    UInt64 globalCpuKernel = GetFiletimeAsUInt64( &globalKernelTime );
    UInt64 prevGlobalCpuKernel = GetFiletimeAsUInt64( &previousCpuDataDataPoint->globalKernelTime );
    UInt64 globalCpuUser = GetFiletimeAsUInt64( &globalUserTime );
    UInt64 prevGlobalCpuUser = GetFiletimeAsUInt64( &previousCpuDataDataPoint->globalUserTime );

    UInt64 globalCpuKernelDelta = globalCpuKernel - prevGlobalCpuKernel;
    UInt64 globalCpuUserDelta = globalCpuUser - prevGlobalCpuUser;
    UInt64 globalCpuIdleDelta = globalCpuIdle - prevGlobalCpuIdle;
    UInt64 globalCpuSystemDelta = globalCpuKernelDelta + globalCpuUserDelta;
    if ( 0 == globalCpuSystemDelta )
        globalCpuSystemDelta = 1;

    previousCpuDataDataPoint->cpuStats.cpuUsePercentage = ( (globalCpuSystemDelta - globalCpuIdleDelta) * 100 ) / ( globalCpuSystemDelta * 1.0 );

    // make the current info the 'previous' info to prepare for the next call to this function
    previousCpuDataDataPoint->globalIdleTime = globalIdleTime;
    previousCpuDataDataPoint->globalKernelTime = globalKernelTime;
    previousCpuDataDataPoint->globalUserTime = globalUserTime;

    NTSTATUS ntStatus = TryNtQuerySystemInformation( SystemProcessorPerformanceInformation                                                        ,
                                                     previousCpuDataDataPoint->cpuPerfInfo                                                        ,
                                                     sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * previousCpuDataDataPoint->logicalCpuCount ,
                                                     NULL                                                                                         );
    if ( WIN32_NT_SUCCESS( ntStatus ) )
    {
        for ( UInt32 i=0; i<previousCpuDataDataPoint->logicalCpuCount; ++i )
        {
            SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& corePerfInfo = previousCpuDataDataPoint->cpuPerfInfo[ i ];
            SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION& prevCorePerfInfo = previousCpuDataDataPoint->prevCpuPerfInfo[ i ];

            UInt64 coreIdle = corePerfInfo.IdleTime.QuadPart;
            UInt64 prevCoreIdle = prevCorePerfInfo.IdleTime.QuadPart;
            UInt64 coreKernel = corePerfInfo.KernelTime.QuadPart;
            UInt64 prevCoreKernel = prevCorePerfInfo.KernelTime.QuadPart;
            UInt64 coreUser = corePerfInfo.UserTime.QuadPart;
            UInt64 prevCoreUser = prevCorePerfInfo.UserTime.QuadPart;

            UInt64 coreKernelDelta = coreKernel - prevCoreKernel;
            UInt64 coreUserDelta = coreUser - prevCoreUser;
            UInt64 coreIdleDelta = coreIdle - prevCoreIdle;
            UInt64 coreSystemDelta = coreKernelDelta + coreUserDelta;
            if ( 0 == coreSystemDelta )
                coreSystemDelta = 1;

            Float64 coreUsePercentage = ( (coreSystemDelta - coreIdleDelta) * 100 ) / ( coreSystemDelta * 1.0 );
            previousCpuDataDataPoint->cpuStats.logicalCpuStats[ i ].cpuUsePercentage = coreUsePercentage;
        }

        // make the current info the 'previous' info to prepare for the next call to this function
        memcpy( previousCpuDataDataPoint->prevCpuPerfInfo, previousCpuDataDataPoint->cpuPerfInfo, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * previousCpuDataDataPoint->logicalCpuCount );
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetCpuStats: call to TryNtQuerySystemInformation failed with NTSTATUS " + ToString( ntStatus ) );
    }

    LONG retVal = ::CallNtPowerInformation( ProcessorInformation,
                                            NULL,
                                            0,
                                            previousCpuDataDataPoint->cpuPowerInfo,
                                            sizeof(PROCESSOR_POWER_INFORMATION) * previousCpuDataDataPoint->logicalCpuCount );
    if ( 0 == retVal )
    {
        for ( UInt32 i=0; i<previousCpuDataDataPoint->logicalCpuCount; ++i )
        {
            PROCESSOR_POWER_INFORMATION* powerInfo = &previousCpuDataDataPoint->cpuPowerInfo[ i ];
            TLogicalCpuStats* lCpuStats = &previousCpuDataDataPoint->cpuStats.logicalCpuStats[ i ];

            lCpuStats->cpuCurrentFrequencyInMhz = (Float32) powerInfo->CurrentMhz;
            lCpuStats->cpuMaxFrequencyInMhz = (Float32) powerInfo->MhzLimit;
            lCpuStats->cpuSpecMaxFrequencyInMhz = (Float32) powerInfo->MaxMhz;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "GetCpuStats: call to CallNtPowerInformation failed with status " + ToString( retVal ) );
    }

    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

    previousCpuDataDataPoint->infoFromSysDevCpuFreq->RefreshCpuFreq();
    UInt32 coreMax = GUCEF_SMALLEST( previousCpuDataDataPoint->cpuStats.logicalCpuCount, (UInt32) previousCpuDataDataPoint->infoFromSysDevCpuFreq->cpuFreqInfos.size() );
    for ( UInt32 i=0; i<coreMax; ++i )
    {
        LinuxCpuFreqInfo& coreFreqInfo = previousCpuDataDataPoint->infoFromSysDevCpuFreq->cpuFreqInfos[ i ];
        TLogicalCpuStats* lCpuStats = &previousCpuDataDataPoint->cpuStats.logicalCpuStats[ i ];

        lCpuStats->cpuCurrentFrequencyInMhz = coreFreqInfo.scalingCurFreq;
        lCpuStats->cpuMaxFrequencyInMhz = coreFreqInfo.scalingMaxFreq;
        lCpuStats->cpuSpecMaxFrequencyInMhz = coreFreqInfo.cpuinfoMaxFreq;
    }

    // Get the current CPU jiffies
    // we need to contrast the new jiffies against the old tallies to glean utilization from the delta
    LinuxCpuUtilization currentCpuUsageStats;

    // First calculate the overall global usage %
    previousCpuDataDataPoint->cpuStats.cpuUsePercentage =
        LinuxCpuUtilization::CalculateCpuUtilization( previousCpuDataDataPoint->infoFromProcStat->globalCpuUsage, currentCpuUsageStats.globalCpuUsage );

    // Now do the same per core
    coreMax = GUCEF_SMALLEST( previousCpuDataDataPoint->cpuStats.logicalCpuCount, (UInt32) previousCpuDataDataPoint->infoFromProcStat->perCoreCpuUsage.size() );
    for ( UInt32 i=0; i<coreMax; ++i )
    {
        LinuxCpuUsageBreakdown& currentCoreCpuUsage = currentCpuUsageStats.perCoreCpuUsage[ i ];
        LinuxCpuUsageBreakdown& prevCoreCpuUsage = previousCpuDataDataPoint->infoFromProcStat->perCoreCpuUsage[ i ];

        previousCpuDataDataPoint->cpuStats.logicalCpuStats[ i ].cpuUsePercentage =
            LinuxCpuUtilization::CalculateCpuUtilization( prevCoreCpuUsage, currentCoreCpuUsage );
    }

    // make the current info the 'previous' info to prepare for the next call to this function
    (*previousCpuDataDataPoint->infoFromProcStat) = currentCpuUsageStats;

    #endif

    *cpuStats = &previousCpuDataDataPoint->cpuStats;
    return OSWRAP_TRUE;

    #else

    return OSWRAP_FALSE;

    #endif
}

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

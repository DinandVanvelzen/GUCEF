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

#ifndef GUCEF_CORE_MSWINUTILS_H
#define GUCEF_CORE_MSWINUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* module config macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#ifndef GUCEF_MT_MSWINUTILS_H
#include "gucefMT_mswinutils.h"
#define GUCEF_MT_MSWINUTILS_H
#endif /* GUCEF_MT_MSWINUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif   /* __cplusplus */

#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef STATUS_INFO_LENGTH_MISMATCH
    #define STATUS_INFO_LENGTH_MISMATCH (NTSTATUS) 0xC0000004L
#endif

/*-------------------------------------------------------------------------*/

#define KPRIORITY_LOW_PRIORITY                      0
#define KPRIORITY_LOW_REALTIME_PRIORITY             16
#define KPRIORITY_HIGH_PRIORITY                     31
#define KPRIORITY_MAXIMUM_PRIORITY                  32

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef LONG       KPRIORITY;

/*-------------------------------------------------------------------------*/

typedef enum _KWAIT_REASON
{
	Executive = 0,
	FreePage = 1,
	PageIn = 2,
	PoolAllocation = 3,
	DelayExecution = 4,
	Suspended = 5,
	UserRequest = 6,
	WrExecutive = 7,
	WrFreePage = 8,
	WrPageIn = 9,
	WrPoolAllocation = 10,
	WrDelayExecution = 11,
	WrSuspended = 12,
	WrUserRequest = 13,
	WrEventPair = 14,
	WrQueue = 15,
	WrLpcReceive = 16,
	WrLpcReply = 17,
	WrVirtualMemory = 18,
	WrPageOut = 19,
	WrRendezvous = 20,
	Spare2 = 21,
	Spare3 = 22,
	Spare4 = 23,
	Spare5 = 24,
	WrCalloutStack = 25,
	WrKernel = 26,
	WrResource = 27,
	WrPushLock = 28,
	WrMutex = 29,
	WrQuantumEnd = 30,
	WrDispatchInt = 31,
	WrPreempted = 32,
	WrYieldExecution = 33,
	WrFastMutex = 34,
	WrGuardedMutex = 35,
	WrRundown = 36,
	MaximumWaitReason = 37
} KWAIT_REASON;

/*-------------------------------------------------------------------------*/

typedef struct _CLIENT_ID
{
     PVOID UniqueProcess;
     PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

/*-------------------------------------------------------------------------*/

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessDebugPort = 7,
    ProcessWow64Information = 26,
    ProcessImageFileName = 27,
    ProcessBreakOnTermination = 29
} PROCESSINFOCLASS;

/*-------------------------------------------------------------------------*/

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

/*-------------------------------------------------------------------------*/

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

/*-------------------------------------------------------------------------*/

typedef
VOID
(NTAPI *PPS_POST_PROCESS_INIT_ROUTINE) (
    VOID
    );

/*-------------------------------------------------------------------------*/

typedef struct _PEB_LDR_DATA {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

/*-------------------------------------------------------------------------*/

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID Reserved4[3];
    PVOID AtlThunkSListPtr;
    PVOID Reserved5;
    ULONG Reserved6;
    PVOID Reserved7;
    ULONG Reserved8;
    ULONG AtlThunkSListPtr32;
    PVOID Reserved9[45];
    BYTE Reserved10[96];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved11[128];
    PVOID Reserved12[1];
    ULONG SessionId;
} PEB, *PPEB;

/*-------------------------------------------------------------------------*/

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

/*-------------------------------------------------------------------------*/

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

/*-------------------------------------------------------------------------*/

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemNextEventIdInformation,
    SystemEventIdsInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemPlugPlayBusInformation,
    SystemDockInformation,
    SystemPowerInformation,
    SystemProcessorSpeedInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation

} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

/*-------------------------------------------------------------------------*/

typedef struct
_SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

/*-------------------------------------------------------------------------*/

// Note that this structure definition was accidentally omitted from WinNT.h.
// This error will be corrected in the future. In the meantime, to compile your application,
// include the structure definition contained in this topic in your source code.
// See: https://docs.microsoft.com/en-us/windows/win32/power/processor-power-information-str
typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

/*-------------------------------------------------------------------------*/

typedef struct _SYSTEM_THREAD {
  LARGE_INTEGER           KernelTime;             // Sum of thread's execution time in KernelMode, in native format.
  LARGE_INTEGER           UserTime;               // Sum of thread's execution time in UserMode, in native format.
  LARGE_INTEGER           CreateTime;             // Time of thread creation, in native format.
  ULONG                   WaitTime;               // Sum of thread's waiting time, in native format.                                           
  PVOID                   StartAddress;           // Thread start address.
  CLIENT_ID               ClientId;               // Process and thread identyficators.
  KPRIORITY               Priority;               // Thread priority.
  LONG                    BasePriority;           // Thread base priority.
  ULONG                   ContextSwitchCount;     // Number of context switches executed by thread.
  ULONG                   State;                  // Current thread's state.
  KWAIT_REASON            WaitReason;             // Reason for waiting (if any)

} SYSTEM_THREAD, *PSYSTEM_THREAD;

/*-------------------------------------------------------------------------*/

typedef struct _SYSTEM_THREAD_INFORMATION {
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	LONG BasePriority;
	ULONG ContextSwitchCount;
	ULONG ThreadState;
	KWAIT_REASON WaitReason;
#ifdef _WIN64
	ULONG Reserved[4];
#endif
}SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

/*-------------------------------------------------------------------------*/

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION
{
	SYSTEM_THREAD_INFORMATION ThreadInfo;
	PVOID StackBase;
	PVOID StackLimit;
	PVOID Win32StartAddress;
	PVOID TebAddress; /* This is only filled in on Vista and above */
	ULONG Reserved1;
	ULONG Reserved2;
	ULONG Reserved3;
} SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

/*-------------------------------------------------------------------------*/

typedef struct _VM_COUNTERS
{
    SIZE_T PeakVirtualSize;             // The peak virtual address space size of this process, in bytes.
    SIZE_T VirtualSize;                 // The virtual address space size of this process, in bytes.
    ULONG PageFaultCount;               // The number of page faults.
    SIZE_T PeakWorkingSetSize;          // The peak working set size, in bytes.
    SIZE_T WorkingSetSize;              // The current working set size, in bytes
    SIZE_T QuotaPeakPagedPoolUsage;     // The peak paged pool usage, in bytes.
    SIZE_T QuotaPagedPoolUsage;         // The current paged pool usage, in bytes.
    SIZE_T QuotaPeakNonPagedPoolUsage;  // The peak non-paged pool usage, in bytes.
    SIZE_T QuotaNonPagedPoolUsage;      // The current non-paged pool usage, in bytes.
    SIZE_T PagefileUsage;               // The Commit Charge value in bytes for this process. Commit Charge is the total amount of private memory that the memory manager has committed for a running process.
    SIZE_T PeakPagefileUsage;           // The peak value in bytes of the Commit Charge during the lifetime of this process.
} VM_COUNTERS, *PVM_COUNTERS;

/*-------------------------------------------------------------------------*/

 typedef struct _SYSTEM_PROCESS_INFORMATION
 {
      ULONG NextEntryOffset;                                 // Offset from begining of output buffer to next process entry. On last entry contains zero.
      ULONG NumberOfThreads;                                 // Number of process'es threads.
      LARGE_INTEGER WorkingSetPrivateSize;                   // since VISTA
      ULONG HardFaultCount;                                  // since WIN7
      ULONG NumberOfThreadsHighWatermark;                    // since WIN7
      ULONGLONG CycleTime;                                   // since WIN7
      LARGE_INTEGER CreateTime;                              // Process creation time, in 100-ns units.
      LARGE_INTEGER UserTime;                                // Effective time in User Mode.
      LARGE_INTEGER KernelTime;                              // Effective time in Kernel Mode.
      UNICODE_STRING ImageName;                              // Process name, based on executable file name.
      KPRIORITY BasePriority;                                // Process base priority.
      HANDLE UniqueProcessId;                                // Unique identifier of process.
      HANDLE InheritedFromUniqueProcessId;                   // Creator's identifier.
      ULONG HandleCount;                                     // Nr of open HANDLEs.
      ULONG SessionId;
      ULONG_PTR UniqueProcessKey;                            // since VISTA (requires SystemExtendedProcessInformation)
      VM_COUNTERS VirtualMemoryCounters;                     // Memory performance counters.
      SIZE_T PrivatePageCount;                               // The number of page faults that are demand-zero faults.
      IO_COUNTERS IoCounters;                                // IO performance counters.
      SYSTEM_THREAD_INFORMATION Threads[1];                  // Array of SYSTEM_THREAD structures descripting process's threads
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

/*-------------------------------------------------------------------------*/

typedef struct _SYSTEM_EXTENDED_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER SpareLi2;
	LARGE_INTEGER SpareLi3;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	PVOID PageDirectoryBase;
	VM_COUNTERS VirtualMemoryCounters;
	SIZE_T PrivatePageCount;
	IO_COUNTERS IoCounters;
	SYSTEM_EXTENDED_THREAD_INFORMATION Threads[1];
} SYSTEM_EXTENDED_PROCESS_INFORMATION, *PSYSTEM_EXTENDED_PROCESS_INFORMATION;

/*-------------------------------------------------------------------------*/

/**
 *  Windows implementation of the ProcessId structure
 */
struct SProcessId
{
    DWORD pid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILTIIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** 
 *  Returns the handle of the current module the code is compiled in.
 *  Nowadays in Win32 this means the same as getting the HINSTANCE for the
 *  application. They are only different in 16-bit windows.
 */
GUCEF_CORE_PUBLIC_C HMODULE
GetCurrentModuleHandle( void );

/*-------------------------------------------------------------------------*/

/**
 *  Function to clear the console window.
 *  This is not always available on all windowed platforms thus 
 *  we provide a version here
 */
GUCEF_CORE_PUBLIC_C void 
cls( void );
            
/*-------------------------------------------------------------------------*/

/**
 *  The Windows NT kernel has a function called NtOpenProcess
 *  Since we dont want to hard link against ntdll with all the issues that creates (dll hell)
 *  we have our own TryNtOpenProcess version which will attempt to use said function if it exists / can be accessed.
 *  
 *  Use WIN32_NT_SUCCESS to evaluate the return value
 *  Returns NTSTATUS_NOT_SUPPORTED if the function could not be dynamicallly linked and is thus unsupported
 */
GUCEF_CORE_PUBLIC_C NTSTATUS 
TryNtOpenProcess( PHANDLE ProcessHandle               ,
                  ACCESS_MASK DesiredAccess           ,
                  POBJECT_ATTRIBUTES ObjectAttributes ,
                  PCLIENT_ID ClientId                 );

/*-------------------------------------------------------------------------*/

/**
 *  The Windows NT kernel has an unofficial function called NtQueryInformationProcess
 *  This provides some access to process information that is hard/impossible to access otherwise
 *  Since this is not officially a public function we have our own NtQueryInformationProcess version
 *  which will attempt to use said function if it exists / can be accessed.
 *  
 *  Use WIN32_NT_SUCCESS to evaluate the return value
 *  Returns NTSTATUS_NOT_SUPPORTED if the function could not be dynamicallly linked and is thus unsupported
 */
GUCEF_CORE_PUBLIC_C NTSTATUS
TryNtQueryInformationProcess( HANDLE ProcessHandle                     ,
                              PROCESSINFOCLASS ProcessInformationClass ,
                              PVOID ProcessInformation                 ,
                              ULONG ProcessInformationLength           ,
                              PULONG ReturnLength                      );

/*-------------------------------------------------------------------------*/

/**
 *  The Windows NT kernel has an unofficial function called NtQuerySystemInformation
 *  This provides some access to process information that is hard/impossible to access otherwise
 *  Since this is not officially a public function we have our own NtQuerySystemInformation version
 *  which will attempt to use said function if it exists / can be accessed.
 *  
 *  Use WIN32_NT_SUCCESS to evaluate the return value
 *  Returns NTSTATUS_NOT_SUPPORTED if the function could not be dynamicallly linked and is thus unsupported
 */
GUCEF_CORE_PUBLIC_C NTSTATUS 
TryNtQuerySystemInformation( SYSTEM_INFORMATION_CLASS SystemInformationClass ,
                             PVOID SystemInformation                         ,
                             ULONG SystemInformationLength                   ,
                             PULONG ReturnLength                             );

/*-------------------------------------------------------------------------*/

/**
 *  The Windows NT kernel has an unofficial function called NtReadVirtualMemory
 *  This provides some access to process information that is hard/impossible to access otherwise
 *  Since this is not officially a public function we have our own TryNtReadVirtualMemory version
 *  which will attempt to use said function if it exists / can be accessed.
 *  
 *  Use WIN32_NT_SUCCESS to evaluate the return value
 *  Returns NTSTATUS_NOT_SUPPORTED if the function could not be dynamicallly linked and is thus unsupported
 */
GUCEF_CORE_PUBLIC_C NTSTATUS 
TryNtReadVirtualMemory( HANDLE ProcessHandle       ,
                        PVOID BaseAddress          ,
                        PVOID Buffer               ,
                        SIZE_T NumberOfBytesToRead ,
                        PSIZE_T NumberOfBytesRead  );

/*-------------------------------------------------------------------------*/

#ifndef RtlOffsetToPointer
  #define RtlOffsetToPointer( B,O )  ( (PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O)) ) )
#endif


/*-------------------------------------------------------------------------*/

/**
 *  From ntdef.h
 *  To avoid including that header everywhere causing issues
 */
#ifndef InitializeObjectAttributes
    #define InitializeObjectAttributes( p, n, a, r, s ) {   \
        (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
        (p)->RootDirectory = r;                             \
        (p)->Attributes = a;                                \
        (p)->ObjectName = n;                                \
        (p)->SecurityDescriptor = s;                        \
        (p)->SecurityQualityOfService = NULL;               \
        }
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

#endif /* GUCEF_CORE_MSWINUTILS_H ? */


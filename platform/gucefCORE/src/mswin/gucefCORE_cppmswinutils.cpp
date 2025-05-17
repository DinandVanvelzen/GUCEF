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

#include "gucefCORE_cppmswinutils.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <pdh.h>

#include "gucefMT_CScopeMutex.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h" 
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

const GUID CLSID_WbemLocator = 
    { 0x4590F811, 0x1D3A, 0x11D0, { 0x89, 0x1F, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };
const GUID IID_IWbemLocator = 
    { 0xDC12A687, 0x737F, 0x11CF, { 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CWindowsComponentAccess* CWindowsComponentAccess::g_instance = GUCEF_NULL;
MT::CMutex CWindowsComponentAccess::g_instanceLock;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CWmiAccessImpl
{
    private:

    bool m_comInitialized;
    bool m_comSecurityInitialized;
    bool m_wmiSecurityInitialized;
    IWbemLocator* m_pLocator;
    IWbemServices* m_pServices;
    
    private:

    Int32 Initialize( void );

    void Clear( void );

    public:

    CWmiAccessImpl( void );     
    ~CWmiAccessImpl();

    Int32 TryLoadWMI( void );
    Int32 UnloadWMI( void );

    Int32 GetPhysicalDeviceIdsFromWMI( const CString& volumeGuid              ,
                                       CStringMap& partitionId2physicalDiskId );
    bool IsValid( void ) const;
    bool IsCOMInitialized( void ) const;
    IWbemLocator* GetLocator( void ) const;
    IWbemServices* GetServices( void ) const;
};

/*-------------------------------------------------------------------------*/

#define PDH_NO_DATA  0x800007D5

class GUCEF_HIDDEN CPdhAccessImpl
{
    private:
    friend class CPdhDevicePerfStatAccessImpl;

    typedef PDH_STATUS (WINAPI *PdhOpenQueryFunc)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
    typedef PDH_STATUS (WINAPI *PdhAddCounterAFunc)(PDH_HQUERY, LPCSTR, DWORD_PTR, PDH_HCOUNTER*);
    typedef PDH_STATUS (WINAPI *PdhCollectQueryDataFunc)(PDH_HQUERY);
    typedef PDH_STATUS (WINAPI *PdhGetFormattedCounterValueFunc)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
    typedef PDH_STATUS (WINAPI *PdhCloseQueryFunc)(PDH_HQUERY);

    PdhOpenQueryFunc m_pdhOpenQuery;
    PdhAddCounterAFunc m_pdhAddCounterA;
    PdhCollectQueryDataFunc m_pdhCollectQueryData;
    PdhGetFormattedCounterValueFunc m_pdhGetFormattedCounterValue;
    PdhCloseQueryFunc m_pdhCloseQuery;
    HMODULE m_pdhLibrary;
    PDH_HQUERY m_realtimeQuery;
    UInt64 m_lastCollectTimeInTicks;
    
    public:
    
    CPdhAccessImpl( void );     
    
    ~CPdhAccessImpl();
    
    Int32 TryLoadPDH( void );

    Int32 UnloadPDH( void );

    Int32 GetPhysicalDevicePerfStats( CPdhAccess::CPdhDevicePerfStatAccess& devicePerfStatAcces ,
                                      CStorageDeviceInformation& devicePerfStats                );

    bool IsValid( void ) const;

    /**
     *  Pulls latest performance data from the PDH API
     *  
     */
    bool CollectStats( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CPdhDevicePerfStatAccessImpl
{
    private:
    friend class CPdhAccess::CPdhDevicePerfStatAccess;
    friend class CPdhAccessImpl;

    struct CounterInfo
    {
        std::string name;
        PDH_HCOUNTER handle;
        LONGLONG value;
    };

    typedef std::vector< CounterInfo, gucef_allocator< CounterInfo > > TCounterInfoVector;
    
    TCounterInfoVector m_counters;
    CPdhAccessImpl* m_pdhApi;
    bool m_isInitialized;

    bool InitForDeviceId( UInt32 deviceIndex, CPdhAccessImpl* pdhApi );

    bool IsInitialized( void ) const;

    bool CollectStats( CStorageDeviceInformation& devicePerfStats );

    void Clear( void );

    CPdhDevicePerfStatAccessImpl( void );

    ~CPdhDevicePerfStatAccessImpl();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWmiAccessImpl::CWmiAccessImpl( void ) 
    : m_comInitialized( false )
    , m_comSecurityInitialized( false )
    , m_wmiSecurityInitialized( false )
    , m_pLocator( GUCEF_NULL )
    , m_pServices( GUCEF_NULL )
{GUCEF_TRACE;

    Initialize();
}  

/*-------------------------------------------------------------------------*/

CWmiAccessImpl::~CWmiAccessImpl() 
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CWmiAccessImpl::Clear( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_pServices ) 
    {
        m_pServices->Release();
        m_pServices = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_pLocator ) 
    {
        m_pLocator->Release();
        m_pLocator = GUCEF_NULL;
    }

    if ( m_comInitialized )
    {
        CoUninitialize();
        m_comInitialized = false;
    }

    m_comInitialized = false;
    m_comSecurityInitialized = false;
    m_wmiSecurityInitialized = false;
}

/*-------------------------------------------------------------------------*/

bool 
CWmiAccessImpl::IsValid( void ) const
{GUCEF_TRACE;
        
    // Check if the WMI locator, services, and enumerator are valid
    return ( m_comInitialized &&
             m_comSecurityInitialized &&
             m_wmiSecurityInitialized &&
             GUCEF_NULL != m_pLocator && 
             GUCEF_NULL != m_pServices );
}

/*-------------------------------------------------------------------------*/

bool 
CWmiAccessImpl::IsCOMInitialized( void ) const
{GUCEF_TRACE;

    // Check if COM is initialized
    return m_comInitialized && m_comSecurityInitialized;                          
}                           

/*-------------------------------------------------------------------------*/

IWbemLocator* 
CWmiAccessImpl::GetLocator( void ) const
{GUCEF_TRACE;
    // Return the WMI locator
    return m_pLocator;
}

/*-------------------------------------------------------------------------*/

IWbemServices* 
CWmiAccessImpl::GetServices( void ) const
{GUCEF_TRACE;
    // Return the WMI services
    return m_pServices;
}

/*-------------------------------------------------------------------------*/

Int32 
CWmiAccessImpl::Initialize( void ) 
{GUCEF_TRACE;

    HRESULT hres = 0;
        
    // Initialize COM library
    if ( !m_comInitialized )
    {
        hres = CoInitializeEx( NULL, COINIT_MULTITHREADED );
        if ( FAILED( hres ) ) 
            return -2;
        m_comInitialized = true;
    }

    // Initialize COM security
    if ( !m_comSecurityInitialized )
    {
        hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                                    RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
        if (FAILED(hres)) 
            return -3;
        m_comSecurityInitialized = true;
    }

    if ( GUCEF_NULL == m_pLocator )
    {
        hres = CoCreateInstance( CLSID_WbemLocator,         // The CLSID of the WbemLocator class
                                 NULL,                      // No context
                                 CLSCTX_INPROC_SERVER,      // Run in the same process
                                 IID_IWbemLocator,          // The interface to obtain (IWbemLocator)
                                 (LPVOID*)&m_pLocator);     // Pointer to store the interface pointer
        if (FAILED(hres)) 
            return -4;
    }

    if ( GUCEF_NULL == m_pServices )
    {
        hres = m_pLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, NULL,
                                            0, NULL, NULL, &m_pServices);
        if ( FAILED( hres ) ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to connect to WMI service. Error code: " + ToString( hres ) );
            return -5;
        }
    }

    if ( !m_wmiSecurityInitialized )
    {
        // Set security levels on WMI connection
        hres = CoSetProxyBlanket( m_pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                                    RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if ( FAILED( hres ) ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to set security levels on WMI connection. Error code: " + ToString( hres ) );
            return -6;
        }
        m_wmiSecurityInitialized = true;
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

Int32
CWmiAccessImpl::TryLoadWMI( void )
{GUCEF_TRACE;

    try
    {        
        return Initialize();
    }
    catch ( const std::exception& )
    {
        return -1;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CWmiAccessImpl::UnloadWMI( void )
{GUCEF_TRACE;

    try
    {
        Clear();
        return 0;
    }
    catch ( const std::exception& )
    {
        return -1;
    }
}

/*-------------------------------------------------------------------------*/

Int32 
CWmiAccessImpl::GetPhysicalDeviceIdsFromWMI( const CString& volumeGuid              ,
                                             CStringMap& partitionId2physicalDiskId )
{GUCEF_TRACE;

    HRESULT hres = 0;

    try
    {
        if ( GUCEF_NULL == m_pServices )
            return -2;

        // Note that the use a LIKE statement because the GUID is sufficiently unique in of itself
        // and matching an exact volume path in WMI is super brittle due to all the escaping
        std::wstring wVolumeGuid = ToWString( volumeGuid );
        std::wstring wqlVolQuery =
            L"SELECT DeviceID, DriveLetter "
            L"FROM Win32_Volume "
            // single-layer escaping for WQL: each '\' → '\\' in the literal
            L"WHERE DeviceID LIKE '%" + wVolumeGuid + L"%'";

        // Execute WMI Query to get the drive letter

        IEnumWbemClassObject* pVolEnum  = GUCEF_NULL;
        hres = m_pServices->ExecQuery(bstr_t(L"WQL"), bstr_t(wqlVolQuery.c_str()),
                             WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                             NULL, &pVolEnum );
        if ( FAILED( hres ) ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI query to get drive letter for volume GUID path failed. Error code: " + ToString( hres ) );
            return -3;
        }
        if ( GUCEF_NULL == pVolEnum ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI query did not result in a drive letter enumerator" );
            return -4;
        }

        // Fetch the one (or zero) volume object

        IWbemClassObject* pVolObj = GUCEF_NULL;
        ULONG returned = 0;
        hres = pVolEnum->Next( WBEM_INFINITE, 1, &pVolObj, &returned );
        if ( 0 == returned || GUCEF_NULL == pVolObj ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetPhysicalDriveInfoFromWMI: No drive letter found for volume " + volumeGuid );
            pVolEnum->Release();
            if ( GUCEF_NULL != pVolObj )
                pVolObj->Release();    
            return 1;
        }

        // Read the DriveLetter

        VARIANT varDrive;
        VariantInit( &varDrive );
        hres = pVolObj->Get( L"DriveLetter", 0, &varDrive, NULL, NULL );
        if ( FAILED( hres ) || varDrive.vt != VT_BSTR ) 
        {
            // maybe the volume is mounted only by GUID, not a drive letter
            // this is currently not supported
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "GetPhysicalDriveInfoFromWMI: No drive letter data found for volume " + volumeGuid );
            return 2;
        }
        std::wstring driveLetter = varDrive.bstrVal;  // e.g. L"F:"
        VariantClear( &varDrive );
        pVolObj->Release();
        pVolObj = GUCEF_NULL;
        pVolEnum->Release();
        pVolEnum = GUCEF_NULL;

        // Now we cam map the LogicalDisk to the DiskPartition using the drive letter attribute

        std::wstring q1 =
            L"ASSOCIATORS OF {Win32_LogicalDisk.DeviceID=\"" + driveLetter + L"\"} "
            L"WHERE AssocClass = Win32_LogicalDiskToPartition";

        IEnumWbemClassObject* pPartEnum = GUCEF_NULL;
        hres = m_pServices->ExecQuery(
                                    bstr_t( L"WQL" ),
                                    bstr_t( q1.c_str() ),
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL,
                                    &pPartEnum);
        if ( FAILED( hres ) ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI LogicalDisk to Partition query failed. Error code: " + ToString( hres ) );
            return -5;
        }

        // There may be one or more partitions; typically you'll get exactly one.
        IWbemClassObject* pPartObj = GUCEF_NULL;
        ULONG partCount = 0;
        while (SUCCEEDED(pPartEnum->Next(WBEM_INFINITE, 1, &pPartObj, &partCount)) && partCount) 
        {
            // Grab the __RELPATH so we can use it in the next query
            VARIANT varRel;
            VariantInit(&varRel);
            hres = pPartObj->Get(L"__RELPATH", 0, &varRel, NULL, NULL);
            if ( FAILED( hres ) ) 
            {
                pPartObj->Release();
                pPartObj = GUCEF_NULL;
                continue;
            }
            
            _bstr_t partPath = varRel.bstrVal;
            CString partitionId;

            VARIANT varDiskIndex, varPartitionIndex;
            VariantInit(&varDiskIndex); VariantInit(&varPartitionIndex);
            HRESULT hrDisk = pPartObj->Get(L"DiskIndex", 0, &varDiskIndex, NULL, NULL);
            HRESULT hrIndex = pPartObj->Get(L"Index", 0, &varPartitionIndex, NULL, NULL);

            if ( SUCCEEDED(hrDisk) && SUCCEEDED(hrIndex) &&
                  varDiskIndex.vt == VT_I4 && varPartitionIndex.vt == VT_I4) 
            {
                int diskIndex = varDiskIndex.intVal;
                int partitionIndex = varPartitionIndex.intVal;

                // Construct the standardized partition identifier
                partitionId = "Disk #" + ToString( diskIndex ) + ", Partition #" + ToString( partitionIndex );
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI: for volume \"" + volumeGuid + "\" found partitionId: " + partitionId );

            } 
            else 
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI Failed to retrieve DiskIndex or Index properties. Error codes: " + ToString( hrDisk ) + ", " + ToString( hrIndex ) );
            }

            VariantClear(&varRel);
            pPartObj->Release();
            pPartObj = GUCEF_NULL;

            // Map DiskPartition to DiskDrive

            std::wstring q2 =
                L"ASSOCIATORS OF {" + std::wstring(partPath) + L"} "
                L"WHERE AssocClass = Win32_DiskDriveToDiskPartition";

            IEnumWbemClassObject* pDriveEnum = GUCEF_NULL;
            hres = m_pServices->ExecQuery(
                                        bstr_t(L"WQL"),
                                        bstr_t(q2.c_str()),
                                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                        NULL,
                                        &pDriveEnum);
            if ( FAILED( hres ) ) 
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI query to map Partition to Drive failed. Error code: " + ToString( hres ) );
                continue;
            }

            // You may get multiple drives for spanned volumes; loop through them
            IWbemClassObject* pDriveObj = GUCEF_NULL;
            ULONG driveCount = 0;
            while ( SUCCEEDED(pDriveEnum->Next(WBEM_INFINITE, 1, &pDriveObj, &driveCount)) && driveCount) 
            {
                VARIANT varDevID;
                VariantInit(&varDevID);
                hres = pDriveObj->Get( L"DeviceID", 0, &varDevID, NULL, NULL );
                if ( SUCCEEDED(hres) && varDevID.vt == VT_BSTR) 
                {
                    CString deviceId = ToString( varDevID.bstrVal );
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "WMI: for volume \"" + volumeGuid + "\" found deviceId: " + deviceId );
                    partitionId2physicalDiskId[ partitionId ] = deviceId;
                }
                VariantClear(&varDevID);
                pDriveObj->Release();
                pDriveObj = GUCEF_NULL;
            }
            pDriveEnum->Release();
            pDriveEnum = GUCEF_NULL;
        }
        pPartEnum->Release();
        pPartEnum = GUCEF_NULL;
   
    }
    catch ( const std::exception& )
    {
        return -1;
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

CWmiAccess::CWmiAccess( void )
    : m_impl( GUCEF_NEW CWmiAccessImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWmiAccess::~CWmiAccess()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
    {
        delete m_impl;
        m_impl = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

Int32
CWmiAccess::TryLoadWMI( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->TryLoadWMI();
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CWmiAccess::UnloadWMI( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->UnloadWMI();
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CWmiAccess::GetPhysicalDeviceIdsFromWMI( const CString& volumeGuid              ,
                                         CStringMap& partitionId2physicalDiskId )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetPhysicalDeviceIdsFromWMI( volumeGuid, partitionId2physicalDiskId );
    return -1;
}

/*-------------------------------------------------------------------------*/

bool 
CWmiAccess::IsValid( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->IsValid();
    return false;
}

/*-------------------------------------------------------------------------*/

CPdhDevicePerfStatAccessImpl::CPdhDevicePerfStatAccessImpl( void )
    : m_counters()
    , m_pdhApi( GUCEF_NULL )
    , m_isInitialized( false )
{GUCEF_TRACE;

    m_counters.reserve( 6 ); 
}

/*-------------------------------------------------------------------------*/

CPdhDevicePerfStatAccessImpl::~CPdhDevicePerfStatAccessImpl()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CPdhDevicePerfStatAccessImpl::Clear( void )
{GUCEF_TRACE;

    // Clear the PDH counters
    // This is important to avoid memory leaks and dangling pointers
    if ( GUCEF_NULL != m_pdhApi )
    {
        TCounterInfoVector::iterator i = m_counters.begin();
        while ( i != m_counters.end() )
        {
            CounterInfo& counter = (*i);
            if ( GUCEF_NULL != counter.handle )
            {
                m_pdhApi->m_pdhCloseQuery( counter.handle );
                counter.handle = GUCEF_NULL;
            }

            ++i;
        }
        
        m_pdhApi = GUCEF_NULL;
    }

    m_isInitialized = false;
}

/*-------------------------------------------------------------------------*/

bool
CPdhDevicePerfStatAccessImpl::IsInitialized( void ) const
{GUCEF_TRACE;
 
    return m_isInitialized;
}

/*-------------------------------------------------------------------------*/

bool
CPdhDevicePerfStatAccessImpl::InitForDeviceId( UInt32 deviceIndex     , 
                                               CPdhAccessImpl* pdhApi )
{GUCEF_TRACE;

    Clear();

    m_pdhApi = pdhApi;
    CString deviceIndexStr = ToString( deviceIndex );
    
    // Initialize the PDH counters for the specified device ID
    m_counters = {
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Disk Read Bytes/sec",           NULL, 0 },
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Disk Write Bytes/sec",          NULL, 0 },
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Avg. Disk sec/Read",            NULL, 0 },
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Avg. Disk sec/Write",           NULL, 0 },
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Current Disk Queue Length",     NULL, 0 },
        { "\\PhysicalDisk(" + deviceIndexStr + ")\\Split IO/Sec",                  NULL, 0 }
    };

    if ( GUCEF_NULL == pdhApi || !pdhApi->IsValid() )
    {
        return false;
    }

    for ( size_t i=0; i<m_counters.size(); ++i )
    {
        if ( pdhApi->m_pdhAddCounterA( pdhApi->m_realtimeQuery, 
                                       m_counters[ i ].name.c_str(), 
                                       0, 
                                       &m_counters[ i ].handle ) != ERROR_SUCCESS )
        {
            Clear();
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPdhDevicePerfStatAccessImpl::CollectStats( CStorageDeviceInformation& devicePerfStats )
{GUCEF_TRACE;
    
    devicePerfStats.perfStats.Clear();
    devicePerfStats.hasPerfStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_pdhApi || !m_pdhApi->IsValid() || m_counters.size() < 6 )
        return false;

    devicePerfStats.perfStats.hasBytesReadPerSec = true;
    devicePerfStats.perfStats.bytesReadPerSec = static_cast< UInt64 >( m_counters[0].value );
    devicePerfStats.perfStats.hasBytesWrittenPerSec = true;
    devicePerfStats.perfStats.bytesWrittenPerSec = static_cast< UInt64 >( m_counters[1].value );
    devicePerfStats.perfStats.hasAvgBytesReadPerSec = true;
    devicePerfStats.perfStats.avgBytesReadPerSec = static_cast< UInt64 >( m_counters[2].value );
    devicePerfStats.perfStats.hasAvgBytesWrittenPerSec = true;
    devicePerfStats.perfStats.avgBytesWrittenPerSec = static_cast< UInt64 >( m_counters[3].value );
    devicePerfStats.perfStats.hasRequestQueueDepth = true;
    devicePerfStats.perfStats.requestQueueDepth = static_cast< UInt64 >( m_counters[4].value );
    devicePerfStats.perfStats.hasRequestSplitCountPerSec = true;
    devicePerfStats.perfStats.requestSplitCountPerSec = static_cast< UInt64 >( m_counters[5].value );

    devicePerfStats.hasPerfStats = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CPdhAccess::CPdhDevicePerfStatAccess::CPdhDevicePerfStatAccess( void )
    : m_impl( GUCEF_NEW CPdhDevicePerfStatAccessImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPdhAccess::CPdhDevicePerfStatAccess::~CPdhDevicePerfStatAccess()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
    {
        GUCEF_DELETE m_impl;
        m_impl = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CPdhAccessImpl::CPdhAccessImpl( void )
    : m_pdhOpenQuery( GUCEF_NULL )
    , m_pdhAddCounterA( GUCEF_NULL )
    , m_pdhCollectQueryData( GUCEF_NULL )
    , m_pdhGetFormattedCounterValue( GUCEF_NULL )
    , m_pdhCloseQuery( GUCEF_NULL )
    , m_pdhLibrary( GUCEF_NULL )
    , m_realtimeQuery( GUCEF_NULL )
    , m_lastCollectTimeInTicks( 0 )
{GUCEF_TRACE;

    TryLoadPDH();
}

/*-------------------------------------------------------------------------*/

CPdhAccessImpl::~CPdhAccessImpl()
{GUCEF_TRACE;

    UnloadPDH();
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::TryLoadPDH( void )
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL == m_pdhLibrary )
        {
            m_pdhLibrary = ::LoadLibraryA( "pdh.dll" );
            if ( GUCEF_NULL == m_pdhLibrary )
            {
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "Failed to load PDH library" );
                return -2;
            }
        }

        m_pdhOpenQuery = (PdhOpenQueryFunc) ::GetProcAddress( m_pdhLibrary, "PdhOpenQueryW" );
        m_pdhAddCounterA = (PdhAddCounterAFunc) ::GetProcAddress( m_pdhLibrary, "PdhAddCounterA" );
        m_pdhCollectQueryData = (PdhCollectQueryDataFunc) ::GetProcAddress( m_pdhLibrary, "PdhCollectQueryData" );
        m_pdhGetFormattedCounterValue = (PdhGetFormattedCounterValueFunc) ::GetProcAddress( m_pdhLibrary, "PdhGetFormattedCounterValue" );
        m_pdhCloseQuery = (PdhCloseQueryFunc) ::GetProcAddress( m_pdhLibrary, "PdhCloseQuery");

        if ( GUCEF_NULL == m_pdhOpenQuery   || 
             GUCEF_NULL == m_pdhAddCounterA || 
             GUCEF_NULL == m_pdhCollectQueryData ||
             GUCEF_NULL == m_pdhGetFormattedCounterValue || 
             GUCEF_NULL == m_pdhCloseQuery )
        {
            ::FreeLibrary( m_pdhLibrary );
            m_pdhLibrary = GUCEF_NULL;
            return -3;
        }

        if ( m_pdhOpenQuery( GUCEF_NULL, 0, &m_realtimeQuery ) != ERROR_SUCCESS )
        {
            FreeLibrary( m_pdhLibrary );
            m_pdhLibrary = GUCEF_NULL;
            return -4;
        }
    }
    catch ( const std::exception& )
    {
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CPdhAccessImpl::IsValid( void ) const
{GUCEF_TRACE;

    // Check if the PDH library and functions are valid
    return ( GUCEF_NULL != m_pdhLibrary &&
             GUCEF_NULL != m_pdhOpenQuery &&
             GUCEF_NULL != m_pdhAddCounterA &&
             GUCEF_NULL != m_pdhCollectQueryData &&
             GUCEF_NULL != m_pdhGetFormattedCounterValue &&
             GUCEF_NULL != m_pdhCloseQuery );
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::UnloadPDH( void )
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL != m_realtimeQuery )
        {
            m_pdhCloseQuery( m_realtimeQuery );
            m_realtimeQuery = GUCEF_NULL;
        }
        if ( GUCEF_NULL != m_pdhLibrary )
        {
            ::FreeLibrary( m_pdhLibrary );
            m_pdhLibrary = GUCEF_NULL;
        }
    }
    catch ( const std::exception& )
    {
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool 
CPdhAccessImpl::CollectStats( void )
{GUCEF_TRACE;

    UInt64 currentTimeInTicks = ::GetTickCount64();
    if ( currentTimeInTicks - m_lastCollectTimeInTicks < 900 )
    {
        // We only want to collect data with a minimum interval of 900ms
        // to avoid flooding the PDH API with requests and/or invalidating the calculations
        return true;
    }

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_pdhLibrary && 
                           GUCEF_NULL != m_pdhCollectQueryData &&
                           GUCEF_NULL != m_realtimeQuery )
    {
        // Collect the PDH data
        PDH_STATUS status = m_pdhCollectQueryData( m_realtimeQuery );
        if ( status != ERROR_SUCCESS )
        {
            if ( PDH_NO_DATA != status )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data: error code: " + ToString( status ) );
                return false;
            }
            else
            {
                // No data available, but this is not an error
                // We can continue to collect data and the situation may resolve itself
                GUCEF_DEBUG_LOG_EVERYTHING( "PdhAccess: No data available" );
            }
        }
        m_lastCollectTimeInTicks = currentTimeInTicks;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::GetPhysicalDevicePerfStats( CPdhAccess::CPdhDevicePerfStatAccess& devicePerfStatAcces ,
                                            CStorageDeviceInformation& devicePerfStats                )
{GUCEF_TRACE;

    devicePerfStats.perfStats.Clear();
    devicePerfStats.hasPerfStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == devicePerfStatAcces.m_impl )
        return -100;
    if GUCEF_PREDICT_FALSE( !devicePerfStats.hasDeviceIndex )
        return -101;
    
    if GUCEF_PREDICT_FALSE( !devicePerfStatAcces.m_impl->IsInitialized() )
    {
        // Initialize the PDH counters for the specified device ID
        if ( !devicePerfStatAcces.m_impl->InitForDeviceId( devicePerfStats.deviceIndex, this ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to initialize PDH device perf counters for device ID: " + devicePerfStats.deviceId );
            return -102;
        }
    }

    // Collect the PDH data
    if ( CollectStats() )
    {
        if GUCEF_PREDICT_FALSE( !devicePerfStatAcces.m_impl->CollectStats( devicePerfStats ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data for device Id: " + devicePerfStats.deviceId );
            return -104;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data " );
        return -103;
    }
    
    return 0;
}

/*-------------------------------------------------------------------------*/

CPdhAccess::CPdhAccess( void )
    : m_impl( GUCEF_NEW CPdhAccessImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPdhAccess::~CPdhAccess()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
    {
        GUCEF_DELETE m_impl;
        m_impl = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CPdhAccess::IsValid( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->IsValid();
    return false;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::TryLoadPDH( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->TryLoadPDH();
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CPdhAccess::UnloadPDH( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->UnloadPDH();
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetPhysicalDevicePerfStats( CPdhDevicePerfStatAccess& devicePerfStatAcces ,
                                        CStorageDeviceInformation& devicePerfStats    )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetPhysicalDevicePerfStats( devicePerfStatAcces, devicePerfStats );
    return -1;
}

/*-------------------------------------------------------------------------*/

CWindowsComponentAccess::CWindowsComponentAccess( void )
    : m_wmiAccess( GUCEF_NULL )
    , m_pdhAccess( GUCEF_NULL )
{GUCEF_TRACE;

    // We do not want to intitialize the windows components here 
    // utilize a lazy-load approach instead to only use what we need
}

/*-------------------------------------------------------------------------*/

CWindowsComponentAccess::~CWindowsComponentAccess()
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_instanceLock );
    
    if ( GUCEF_NULL != m_wmiAccess )
    {
        delete m_wmiAccess;
        m_wmiAccess = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_pdhAccess )
    {
        delete m_pdhAccess;
        m_pdhAccess = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CWindowsComponentAccess* 
CWindowsComponentAccess::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex scopeLock( g_instanceLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CWindowsComponentAccess();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void 
CWindowsComponentAccess::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_instanceLock );
    if ( GUCEF_NULL != g_instance )
    {
        GUCEF_DELETE g_instance;
        g_instance = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CWmiAccess* 
CWindowsComponentAccess::GetWmiAccess( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_instanceLock );
    
    if ( GUCEF_NULL != m_wmiAccess )
        return m_wmiAccess;
    
    // Lazy-load dependency only if needed
    m_wmiAccess = GUCEF_NEW CWmiAccess();
    return m_wmiAccess;
}

/*-------------------------------------------------------------------------*/

CPdhAccess* 
CWindowsComponentAccess::GetPdhAccess( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_instanceLock );
    
    if ( GUCEF_NULL != m_pdhAccess )
        return m_pdhAccess;
    
    // Lazy-load dependency only if needed
    m_pdhAccess = GUCEF_NEW CPdhAccess();
    return m_pdhAccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

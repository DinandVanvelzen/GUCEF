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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h" 
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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

#define PDH_NO_DATA            0x800007D5
#define PDH_MORE_DATA          0x800007D2
#define PDH_CSTATUS_NO_COUNTER ( (PDH_STATUS) -1073738820 )  // 0xC0000BC4
#define PDH_INVALID_DATA       ( (PDH_STATUS) -1073738810 )  // 0xC0000BC6 // specified counter does not contain valid data or a successful status code.

class GUCEF_HIDDEN CPdhAccessImpl
{
    private:
    friend class CPdhDevicePerfStatAccessImpl;
    friend class CPdhVolumePerfStatAccessImpl;

    typedef PDH_STATUS (WINAPI *PdhOpenQueryFunc)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
    typedef PDH_STATUS (WINAPI *PdhAddCounterWFunc)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
    typedef PDH_STATUS (WINAPI *PdhCollectQueryDataFunc)(PDH_HQUERY);
    typedef PDH_STATUS (WINAPI *PdhGetFormattedCounterValueFunc)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
    typedef PDH_STATUS (WINAPI *PdhCloseQueryFunc)(PDH_HQUERY);    
    typedef PDH_STATUS (WINAPI *PdhEnumObjectItemsWFunc)(LPCWSTR szDataSource, LPCWSTR szMachineName, LPCWSTR szObjectName, PZZWSTR mszCounterList, LPDWORD pcchCounterListLength, PZZWSTR mszInstanceList, LPDWORD pcchInstanceListLength, DWORD dwDetailLevel, DWORD dwFlags );  
    typedef PDH_STATUS (WINAPI *PdhEnumObjectsWFunc)( LPCWSTR szDataSource, LPCWSTR szMachineName, PZZWSTR mszObjectList, LPDWORD pcchBufferSize, DWORD dwDetailLevel, BOOL bRefresh ); 

    PdhOpenQueryFunc m_pdhOpenQuery;
    PdhAddCounterWFunc m_pdhAddCounterW;
    PdhCollectQueryDataFunc m_pdhCollectQueryData;
    PdhGetFormattedCounterValueFunc m_pdhGetFormattedCounterValue;
    PdhEnumObjectItemsWFunc m_pdhEnumObjectItemsW;
    PdhEnumObjectsWFunc m_pdhEnumObjectsW;
    PdhCloseQueryFunc m_pdhCloseQuery;
    HMODULE m_pdhLibrary;
    PDH_HQUERY m_realtimeQuery;
    UInt64 m_lastCollectTimeInTicks;
    
    public:
    
    CPdhAccessImpl( void );     
    
    ~CPdhAccessImpl();
    
    Int32 TryLoadPDH( void );

    Int32 UnloadPDH( void );

    Int32 GetListOfStatCategories( CStringSet& statCatagories ) const;

    Int32 GetListOfStatCountersInCategory( const CString& statCategory ,
                                           CStringSet& statsInCategory ) const;

    Int32 GetListOfStatInstancesInCategory( const CString& statCategory ,
                                            CStringSet& statsInCategory ) const;

    Int32 GetPhysicalDevicePerfStats( CPdhAccess::CPdhDevicePerfStatAccess& devicePerfStatAcces ,
                                      CStorageDeviceInformation& devicePerfStats                ,
                                      bool& hasNewStats                                         );

    Int32 GetLogicalVolumePerfStats( CPdhAccess::CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                     CStorageVolumeInformation& volumePerfStats                 ,
                                     bool& hasNewStats                                          );

    Int32 GetTotalLogicalVolumePerfStats( CPdhAccess::CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                          CStorageVolumeInformation& volumePerfStats                 ,
                                          bool& hasNewStats                                          );

    bool IsValid( void ) const;

    /**
     *  Pulls latest performance data from the PDH API
     */
    bool CollectStats( UInt64& lastCollectTimeInTicks );
};

/*-------------------------------------------------------------------------*/

/**
 *  Hidden implementation of the CPdhDevicePerfStatAccess class
 *  It stores the actual references to the relative performance counters
 *  It also handles the initialization and collection of performance data
 *  and the copying of the resulting data to the CStorageDeviceInformation class
 */
class GUCEF_HIDDEN CPdhDevicePerfStatAccessImpl
{
    private:
    friend class CPdhAccess::CPdhDevicePerfStatAccess;
    friend class CPdhAccessImpl;

    class CounterInfo
    {
        public: 
        std::wstring name;
        PDH_HCOUNTER handle;
        CVariant value;
        DWORD formatType;
        bool hasUpdatedValue;

        CounterInfo( void );
    };

    typedef GUCEF::vector< CounterInfo, gucef_allocator< CounterInfo > > TCounterInfoVector;
    typedef GUCEF::map< CString, TCounterInfoVector >                    TVolumePathToCounterInfoVector;
    
    TVolumePathToCounterInfoVector m_counters;
    UInt64 m_lastCounterUpdateTimeInTicks;
    CPdhAccessImpl* m_pdhApi;
    bool m_isInitialized;

    bool InitForDeviceId( UInt32 deviceIndex, CPdhAccessImpl* pdhApi );

    bool IsInitialized( void ) const;

    /**
     *  Retrieves the performance counter values after a successful system query
     *  Note that a successful system query doesn't mean these specific counters had an update
     * 
     *  @param hasNewStats whether there are any stat values that were updated as a result of the query vs just 'last known' values
     *  @return whether the underlying operations experienced an error or not. true = no error
     */
    bool CollectStats( CStorageDeviceInformation& devicePerfStats ,
                       UInt64 rtQueryLastCollectTimeInTicks       ,
                       bool& hasNewStats                          );

    void Clear( void );

    CPdhDevicePerfStatAccessImpl( void );

    ~CPdhDevicePerfStatAccessImpl();
};

/*-------------------------------------------------------------------------*/

/**
 *  Hidden implementation of the CPdhVolumePerfStatAccess class
 *  It stores the actual references to the relative performance counters
 *  It also handles the initialization and collection of performance data
 *  and the copying of the resulting data to the CStorageVolumeInformation class
 */
class GUCEF_HIDDEN CPdhVolumePerfStatAccessImpl
{
    private:
    friend class CPdhAccess::CPdhVolumePerfStatAccess;
    friend class CPdhAccessImpl;

    class CounterInfo
    {
        public: 
        std::wstring name;
        PDH_HCOUNTER handle;
        CVariant value;
        DWORD formatType;
        bool hasUpdatedValue;

        CounterInfo( void );
    };

    typedef GUCEF::vector< CounterInfo, gucef_allocator< CounterInfo > > TCounterInfoVector;
    
    TCounterInfoVector m_counters;
    UInt64 m_lastCounterUpdateTimeInTicks;
    CPdhAccessImpl* m_pdhApi;
    bool m_isInitialized;

    bool InitForMountPath( const CString& mountPath, CPdhAccessImpl* pdhApi );

    bool IsInitialized( void ) const;

    /**
     *  Retrieves the performance counter values after a successful system query
     *  Note that a successful system query doesn't mean these specific counters had an update
     * 
     *  @param hasNewStats whether there are any stat values that were updated as a result of the query vs just 'last known' values
     *  @return whether the underlying operations experienced an error or not. true = no error
     */
    bool CollectStats( CStorageVolumeInformation& volumePerfStats ,
                       UInt64 rtQueryLastCollectTimeInTicks       ,
                       bool& hasNewStats                          );

    void Clear( void );

    CPdhVolumePerfStatAccessImpl( void );

    ~CPdhVolumePerfStatAccessImpl();
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

            // not having a drive letter is not an error. its a perfectly valid usecase
            return 0;
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

CPdhDevicePerfStatAccessImpl::CounterInfo::CounterInfo( void )
    : name()
    , handle( GUCEF_NULL )
    , value()
    , formatType( PDH_FMT_LARGE ) // Default to large format for bytes/sec counters
    , hasUpdatedValue( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPdhDevicePerfStatAccessImpl::CPdhDevicePerfStatAccessImpl( void )
    : m_counters()
    , m_lastCounterUpdateTimeInTicks( 0 )
    , m_pdhApi( GUCEF_NULL )
    , m_isInitialized( false )
{GUCEF_TRACE;

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
        TVolumePathToCounterInfoVector::iterator n = m_counters.begin();
        while ( n != m_counters.end() )
        {
            TCounterInfoVector& counterInfo = (*n).second;
            TCounterInfoVector::iterator i = counterInfo.begin();
            while ( i != counterInfo.end() )
            {
                CounterInfo& counter = (*i);
                if ( GUCEF_NULL != counter.handle )
                {
                    m_pdhApi->m_pdhCloseQuery( counter.handle );
                    counter.handle = GUCEF_NULL;
                }

                ++i;
            }
            counterInfo.clear();
            ++n;
        }
        m_counters.clear();
        m_pdhApi = GUCEF_NULL;
    }

    m_lastCounterUpdateTimeInTicks = 0;
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

    if ( GUCEF_NULL == pdhApi || !pdhApi->IsValid() )
    {
        return false;
    }

    // Windows performance counters does not provide a way to actually get the device stats
    // best we can do here is to aggregate for all the mounted volumes on the relative device
    // it would not include non-mounted volumes and thus is technically not really the entire device
    // for most purposes this should still be fine since on Windows it needs to be mounted to be
    // used by the user to begin with

    CStringSet deviceAndMountedVolumeCombos;
    Int32 errorCode = m_pdhApi->GetListOfStatInstancesInCategory( "PhysicalDisk", deviceAndMountedVolumeCombos );
    if ( 0 == errorCode )
    {
        CString deviceIndexStr = ToString( deviceIndex );
        CStringSet::iterator i = deviceAndMountedVolumeCombos.begin();
        while ( i != deviceAndMountedVolumeCombos.end() )
        {
            const CString& deviceAndMountEntry = (*i);
            if ( deviceAndMountEntry.StartsWith( deviceIndexStr ) )
            {
                const std::wstring& wDeviceAndMountEntry = ToWString( deviceAndMountEntry );
                TCounterInfoVector& counterInfo = m_counters[ deviceAndMountEntry ];
                
                // Initialize the PDH counters for the specified device ID
                counterInfo.resize( 6 );
                counterInfo[ 0 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Disk Read Bytes/sec";
                counterInfo[ 0 ].formatType = PDH_FMT_LARGE;
                counterInfo[ 1 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Disk Write Bytes/sec";
                counterInfo[ 1 ].formatType = PDH_FMT_LARGE;
                counterInfo[ 2 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Avg. Disk sec/Read";
                counterInfo[ 2 ].formatType = PDH_FMT_DOUBLE;
                counterInfo[ 3 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Avg. Disk sec/Write";
                counterInfo[ 3 ].formatType = PDH_FMT_DOUBLE;
                counterInfo[ 4 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Current Disk Queue Length";
                counterInfo[ 4 ].formatType = PDH_FMT_LARGE;
                counterInfo[ 5 ].name = L"\\PhysicalDisk(" + wDeviceAndMountEntry + L")\\Split IO/Sec";
                counterInfo[ 5 ].formatType = PDH_FMT_LARGE;

                for ( size_t n=0; n<counterInfo.size(); ++n )
                {
                    PDH_STATUS status = pdhApi->m_pdhAddCounterW( pdhApi->m_realtimeQuery, 
                                                                  counterInfo[ n ].name.c_str(), 
                                                                  GUCEF_NULL, 
                                                                  &counterInfo[ n ].handle );
                    if ( status != ERROR_SUCCESS )
                    {
                        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Failed to add PDH counter: " + 
                            ToString( counterInfo[ n ].name ) + ". Error code: " + ToString( status ) );
                    }
                }
            }
            ++i;
        }

        m_isInitialized = true;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CPdhDevicePerfStatAccessImpl::CollectStats( CStorageDeviceInformation& devicePerfStats ,
                                            UInt64 rtQueryLastCollectTimeInTicks       ,
                                            bool& hasNewStats                          )
{GUCEF_TRACE;
    
    hasNewStats = false;
    devicePerfStats.perfStats.Clear();
    devicePerfStats.hasPerfStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_pdhApi || !m_pdhApi->IsValid() || m_counters.empty() )
        return false;

    bool hadErrors = false;

    // Check if we need to obtain new stats from the PDH API
    // This depends on when the last global rt query was made
    if ( m_lastCounterUpdateTimeInTicks < rtQueryLastCollectTimeInTicks )
    {
        // Collect stats in a best effort manner
        TVolumePathToCounterInfoVector::iterator n = m_counters.begin();
        while ( n != m_counters.end() )
        {
            TCounterInfoVector& counterInfo = (*n).second;
            bool mountHasNewStats = false;

            for ( size_t i=0; i<counterInfo.size(); ++i )
            {
                counterInfo[i].hasUpdatedValue = false;

                // Retrieve the counter value
                PDH_FMT_COUNTERVALUE counterValue;
                DWORD counterType = 0;
                PDH_STATUS status = m_pdhApi->m_pdhGetFormattedCounterValue(
                                                                            counterInfo[i].handle,           // The PDH_HCOUNTER handle
                                                                            counterInfo[i].formatType,       // Format: PDH_FMT_LONG, PDH_FMT_DOUBLE, or PDH_FMT_LARGE
                                                                            &counterType,                    // Receives the counter type
                                                                            &counterValue                    // Receives the value
                                                                        );
                if ( status == ERROR_SUCCESS )
                {
                    counterInfo[i].hasUpdatedValue = true;
                    mountHasNewStats = true;

                    switch ( counterInfo[i].formatType )
                    {
                        case PDH_FMT_LARGE:
                            counterInfo[i].value = counterValue.longValue;
                            break;
                        case PDH_FMT_DOUBLE:
                            counterInfo[i].value = counterValue.doubleValue;
                            break;
                        default:
                        {
                            counterInfo[i].hasUpdatedValue = false; // Unsupported format type, skip this counter
                            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Unsupported format type for counter: " + 
                                ToString( counterInfo[i].name ) + ". Format type: " + ToString( (UInt64) counterInfo[i].formatType ) );
                            hadErrors = true; // Mark as error for the entire operation
                            break;
                        }
                    }
                }
                else
                if ( status == PDH_CSTATUS_NO_COUNTER )
                {
                    // Most likely the counter is not available on this system, or the mount path is invalid
                    // However, it can also mean that the counter is not available for the specific mount path AT THIS TIME
                    // we have to account for removable media that may not be present at the time of the query
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Counter: " + 
                        ToString (counterInfo[i].name ) + " is not available" );
                }
                else
                if ( status == PDH_INVALID_DATA )
                {
                    // Some counters, like rate counters, require two data points to compute a value, and you might not have collected enough data before querying
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Data not available (yet?) for counter: " + 
                        ToString( counterInfo[i].name ) + ". Error code: " + ToString( status ) );
                }
                else
                {
                    GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Failed to get formatted value for counter: " + 
                        ToString( counterInfo[i].name ) + ". Error code: " + ToString( status ) );
                    hadErrors = true; // Mark as error for the entire operation
                }
            }
        
            if ( mountHasNewStats )
            {
                // Aggregate the values for the given device id - mount combo into the total
                // We are emulating device wide stats here so we don't want the per-volume stats

                devicePerfStats.perfStats.hasBytesReadPerSec = counterInfo[0].hasUpdatedValue || devicePerfStats.perfStats.hasBytesReadPerSec;
                if ( counterInfo[0].hasUpdatedValue )
                    devicePerfStats.perfStats.bytesReadPerSec += counterInfo[0].value.AsUInt64();
                devicePerfStats.perfStats.hasBytesWrittenPerSec = counterInfo[1].hasUpdatedValue || devicePerfStats.perfStats.hasBytesWrittenPerSec;
                if ( counterInfo[1].hasUpdatedValue )
                    devicePerfStats.perfStats.bytesWrittenPerSec += counterInfo[1].value.AsUInt64();
                devicePerfStats.perfStats.hasAvgReadTimePerOperationInMs = counterInfo[2].hasUpdatedValue || devicePerfStats.perfStats.hasAvgReadTimePerOperationInMs;
                if ( counterInfo[2].hasUpdatedValue )
                    devicePerfStats.perfStats.avgReadTimePerOperationInMs += counterInfo[2].value.AsFloat32() * 1000.0f;
                devicePerfStats.perfStats.hasAvgWriteTimePerOperationInMs = counterInfo[3].hasUpdatedValue || devicePerfStats.perfStats.hasAvgWriteTimePerOperationInMs;
                if ( counterInfo[3].hasUpdatedValue )
                    devicePerfStats.perfStats.avgWriteTimePerOperationInMs += counterInfo[3].value.AsFloat32() * 1000.0f;
                devicePerfStats.perfStats.hasRequestQueueDepth = counterInfo[4].hasUpdatedValue || devicePerfStats.perfStats.hasRequestQueueDepth;
                if ( counterInfo[4].hasUpdatedValue )
                    devicePerfStats.perfStats.requestQueueDepth += counterInfo[4].value.AsUInt64();
                devicePerfStats.perfStats.hasRequestSplitCountPerSec = counterInfo[5].hasUpdatedValue || devicePerfStats.perfStats.hasRequestSplitCountPerSec;
                if ( counterInfo[5].hasUpdatedValue )
                    devicePerfStats.perfStats.requestSplitCountPerSec += counterInfo[5].value.AsUInt64();

                hasNewStats = devicePerfStats.hasPerfStats = mountHasNewStats || hasNewStats;
            }

            ++n;
        }
    }
    else
    if ( 0 != m_lastCounterUpdateTimeInTicks )
    {
        // Use the last known values from the PDH counters
        // the last known values are not guaranteed to be updated since the last query
        TVolumePathToCounterInfoVector::iterator n = m_counters.begin();
        while ( n != m_counters.end() )
        {
            TCounterInfoVector& counterInfo = (*n).second;

            for ( size_t i=0; i<counterInfo.size(); ++i )
            {
                devicePerfStats.perfStats.hasBytesReadPerSec = counterInfo[0].hasUpdatedValue || devicePerfStats.perfStats.hasBytesReadPerSec;
                if ( counterInfo[0].hasUpdatedValue )
                    devicePerfStats.perfStats.bytesReadPerSec += counterInfo[0].value.AsUInt64();
                devicePerfStats.perfStats.hasBytesWrittenPerSec = counterInfo[1].hasUpdatedValue || devicePerfStats.perfStats.hasBytesWrittenPerSec;
                if ( counterInfo[1].hasUpdatedValue )
                    devicePerfStats.perfStats.bytesWrittenPerSec += counterInfo[1].value.AsUInt64();
                devicePerfStats.perfStats.hasAvgReadTimePerOperationInMs = counterInfo[2].hasUpdatedValue || devicePerfStats.perfStats.hasAvgReadTimePerOperationInMs;
                if ( counterInfo[2].hasUpdatedValue )
                    devicePerfStats.perfStats.avgReadTimePerOperationInMs += counterInfo[2].value.AsFloat32() * 1000.0f;
                devicePerfStats.perfStats.hasAvgWriteTimePerOperationInMs = counterInfo[3].hasUpdatedValue || devicePerfStats.perfStats.hasAvgWriteTimePerOperationInMs;
                if ( counterInfo[3].hasUpdatedValue )
                    devicePerfStats.perfStats.avgWriteTimePerOperationInMs += counterInfo[3].value.AsFloat32() * 1000.0f;
                devicePerfStats.perfStats.hasRequestQueueDepth = counterInfo[4].hasUpdatedValue || devicePerfStats.perfStats.hasRequestQueueDepth;
                if ( counterInfo[4].hasUpdatedValue )
                    devicePerfStats.perfStats.requestQueueDepth += counterInfo[4].value.AsUInt64();
                devicePerfStats.perfStats.hasRequestSplitCountPerSec = counterInfo[5].hasUpdatedValue || devicePerfStats.perfStats.hasRequestSplitCountPerSec;
                if ( counterInfo[5].hasUpdatedValue )
                    devicePerfStats.perfStats.requestSplitCountPerSec += counterInfo[5].value.AsUInt64();
            }
            ++n;
        }
    }

    return !hadErrors;
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

CPdhVolumePerfStatAccessImpl::CounterInfo::CounterInfo( void )
    : name()
    , handle( GUCEF_NULL )
    , value()
    , formatType( PDH_FMT_LARGE ) // Default to large format for bytes/sec counters
    , hasUpdatedValue( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPdhVolumePerfStatAccessImpl::CPdhVolumePerfStatAccessImpl( void )
    : m_counters()
    , m_lastCounterUpdateTimeInTicks( 0 )
    , m_pdhApi( GUCEF_NULL )
    , m_isInitialized( false )
{GUCEF_TRACE;

    m_counters.reserve( 6 ); 
}

/*-------------------------------------------------------------------------*/

CPdhVolumePerfStatAccessImpl::~CPdhVolumePerfStatAccessImpl()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CPdhVolumePerfStatAccessImpl::Clear( void )
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

    m_lastCounterUpdateTimeInTicks = 0;
    m_isInitialized = false;
}

/*-------------------------------------------------------------------------*/

bool
CPdhVolumePerfStatAccessImpl::IsInitialized( void ) const
{GUCEF_TRACE;
 
    return m_isInitialized;
}

/*-------------------------------------------------------------------------*/

bool
CPdhVolumePerfStatAccessImpl::InitForMountPath( const CString& mountPath , 
                                                CPdhAccessImpl* pdhApi   )
{GUCEF_TRACE;

    Clear();

    std::wstring wMountPath = ToWString( mountPath );
    m_pdhApi = pdhApi;
    
    // Initialize the PDH counters for the specified volume mount path
    // example: "C:" thus \\LogicalDisk(C:)\\Disk Read Bytes/sec
    m_counters.resize( 6 );
    m_counters[ 0 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Disk Read Bytes/sec";
    m_counters[ 0 ].formatType = PDH_FMT_LARGE;
    m_counters[ 1 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Disk Write Bytes/sec";
    m_counters[ 1 ].formatType = PDH_FMT_LARGE;
    m_counters[ 2 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Avg. Disk sec/Read";
    m_counters[ 2 ].formatType = PDH_FMT_DOUBLE;
    m_counters[ 3 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Avg. Disk sec/Write";
    m_counters[ 3 ].formatType = PDH_FMT_DOUBLE;
    m_counters[ 4 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Current Disk Queue Length";
    m_counters[ 4 ].formatType = PDH_FMT_LARGE;
    m_counters[ 5 ].name = L"\\LogicalDisk(" + wMountPath + L")\\Split IO/Sec";
    m_counters[ 5 ].formatType = PDH_FMT_LARGE;

    if ( GUCEF_NULL == pdhApi || !pdhApi->IsValid() )
    {
        return false;
    }

    for ( size_t i=0; i<m_counters.size(); ++i )
    {
        if ( pdhApi->m_pdhAddCounterW( pdhApi->m_realtimeQuery, 
                                       m_counters[ i ].name.c_str(), 
                                       0, 
                                       &m_counters[ i ].handle ) != ERROR_SUCCESS )
        {
            Clear();
            return false;
        }
    }

    m_isInitialized = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CPdhVolumePerfStatAccessImpl::CollectStats( CStorageVolumeInformation& volumePerfStats ,
                                            UInt64 rtQueryLastCollectTimeInTicks       ,
                                            bool& hasNewStats                          )
{GUCEF_TRACE;
    
    hasNewStats = false;
    volumePerfStats.perfStats.Clear();
    volumePerfStats.hasPerfStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == m_pdhApi || !m_pdhApi->IsValid() || m_counters.size() < 6 )
        return false;

    bool hadErrors = false;

    // Check if we need to obtain new stats from the PDH API
    // This depends on when the last global rt query was made
    if ( m_lastCounterUpdateTimeInTicks < rtQueryLastCollectTimeInTicks )
    {
        for ( size_t i=0; i<m_counters.size(); ++i )
        {
            m_counters[i].hasUpdatedValue = false;

            // Retrieve the counter value
            PDH_FMT_COUNTERVALUE counterValue;
            DWORD counterType = 0;
            PDH_STATUS status = m_pdhApi->m_pdhGetFormattedCounterValue(
                                                                        m_counters[i].handle,            // The PDH_HCOUNTER handle
                                                                        m_counters[i].formatType,        // Format: PDH_FMT_LONG, PDH_FMT_DOUBLE, or PDH_FMT_LARGE
                                                                        &counterType,                    // Receives the counter type
                                                                        &counterValue                    // Receives the value
                                                                    );

            if ( status == ERROR_SUCCESS )
            {
                m_counters[i].hasUpdatedValue = true;
                hasNewStats = true;

                switch ( m_counters[i].formatType )
                {
                    case PDH_FMT_LARGE:
                        m_counters[i].value = counterValue.longValue;
                        break;
                    case PDH_FMT_DOUBLE:
                        m_counters[i].value = counterValue.doubleValue;
                        break;
                    default:
                    {
                        m_counters[i].hasUpdatedValue = false; // Unsupported format type, skip this counter
                        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhDevicePerfStatAccessImpl: Unsupported format type for counter: " + 
                            ToString( m_counters[i].name ) + ". Format type: " + ToString( (UInt64) m_counters[i].formatType ) );
                        hadErrors = true; // Mark as error for the entire operation
                        break;
                    }
                }
            }
            else
            if ( status == PDH_CSTATUS_NO_COUNTER )
            {
                // Most likely the counter is not available on this system, or the mount path is invalid
                // However, it can also mean that the counter is not available for the specific mount path AT THIS TIME
                // we have to account for removable media that may not be present at the time of the query
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhVolumePerfStatAccessImpl: Counter: " + 
                    ToString( m_counters[i].name ) + " is not available" );
            }
            else
            if ( status == PDH_INVALID_DATA )
            {
                // Some counters, like rate counters, require two data points to compute a value, and you might not have collected enough data before querying
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhVolumePerfStatAccessImpl: Data not available (yet?) for counter: " + 
                    ToString( m_counters[i].name ) + ". Error code: " + ToString( status ) );
            }
            else
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhVolumePerfStatAccessImpl: Failed to get formatted value for counter: " + 
                    ToString( m_counters[i].name ) + ". Error code: " + ToString( status ) );
                hadErrors = true;
            }
        }
    }

    volumePerfStats.perfStats.hasBytesReadPerSec = m_counters[0].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasBytesReadPerSec )
        volumePerfStats.perfStats.bytesReadPerSec = m_counters[0].value.AsUInt64();
    volumePerfStats.perfStats.hasBytesWrittenPerSec = m_counters[1].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasBytesWrittenPerSec )
        volumePerfStats.perfStats.bytesWrittenPerSec = m_counters[1].value.AsUInt64();
    volumePerfStats.perfStats.hasAvgReadTimePerOperationInMs = m_counters[2].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasAvgReadTimePerOperationInMs )
        volumePerfStats.perfStats.avgReadTimePerOperationInMs = m_counters[2].value.AsFloat32() * 1000.0f;
    volumePerfStats.perfStats.hasAvgWriteTimePerOperationInMs = m_counters[3].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasAvgWriteTimePerOperationInMs )
        volumePerfStats.perfStats.avgWriteTimePerOperationInMs = m_counters[3].value.AsFloat32() * 1000.0f;
    volumePerfStats.perfStats.hasRequestQueueDepth = m_counters[4].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasRequestQueueDepth )
        volumePerfStats.perfStats.requestQueueDepth = m_counters[4].value.AsUInt64();
    volumePerfStats.perfStats.hasRequestSplitCountPerSec = m_counters[5].hasUpdatedValue;
    if ( volumePerfStats.perfStats.hasRequestSplitCountPerSec )
        volumePerfStats.perfStats.requestSplitCountPerSec = m_counters[5].value.AsUInt64();

    volumePerfStats.hasPerfStats = true;

    return !hadErrors;
}

/*-------------------------------------------------------------------------*/

CPdhAccess::CPdhVolumePerfStatAccess::CPdhVolumePerfStatAccess( void )
    : m_impl( GUCEF_NEW CPdhVolumePerfStatAccessImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPdhAccess::CPdhVolumePerfStatAccess::~CPdhVolumePerfStatAccess()
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
    , m_pdhAddCounterW( GUCEF_NULL )
    , m_pdhCollectQueryData( GUCEF_NULL )
    , m_pdhGetFormattedCounterValue( GUCEF_NULL )
    , m_pdhEnumObjectItemsW( GUCEF_NULL )
    , m_pdhEnumObjectsW( GUCEF_NULL ) 
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
        m_pdhAddCounterW = (PdhAddCounterWFunc) ::GetProcAddress( m_pdhLibrary, "PdhAddCounterW" );
        m_pdhCollectQueryData = (PdhCollectQueryDataFunc) ::GetProcAddress( m_pdhLibrary, "PdhCollectQueryData" );
        m_pdhGetFormattedCounterValue = (PdhGetFormattedCounterValueFunc) ::GetProcAddress( m_pdhLibrary, "PdhGetFormattedCounterValue" );
        m_pdhEnumObjectItemsW = (PdhEnumObjectItemsWFunc) ::GetProcAddress( m_pdhLibrary, "PdhEnumObjectItemsW" );
        m_pdhEnumObjectsW = (PdhEnumObjectsWFunc) ::GetProcAddress( m_pdhLibrary, "PdhEnumObjectsW" );
        m_pdhCloseQuery = (PdhCloseQueryFunc) ::GetProcAddress( m_pdhLibrary, "PdhCloseQuery");

        if ( GUCEF_NULL == m_pdhOpenQuery   || 
             GUCEF_NULL == m_pdhAddCounterW || 
             GUCEF_NULL == m_pdhCollectQueryData ||
             GUCEF_NULL == m_pdhGetFormattedCounterValue || 
             GUCEF_NULL == m_pdhEnumObjectItemsW ||
             GUCEF_NULL == m_pdhEnumObjectsW ||
             GUCEF_NULL == m_pdhCloseQuery )
        {
            ::FreeLibrary( m_pdhLibrary );
            m_pdhLibrary = GUCEF_NULL;
            return -3;
        }

        if ( m_pdhOpenQuery( GUCEF_NULL, 0, &m_realtimeQuery ) != ERROR_SUCCESS )
        {
            ::FreeLibrary( m_pdhLibrary );
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
             GUCEF_NULL != m_pdhAddCounterW &&
             GUCEF_NULL != m_pdhCollectQueryData &&
             GUCEF_NULL != m_pdhGetFormattedCounterValue &&
             GUCEF_NULL != m_pdhEnumObjectItemsW &&
             GUCEF_NULL != m_pdhEnumObjectsW &&
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

Int32 
CPdhAccessImpl::GetListOfStatCategories( CStringSet& statCategories ) const
{GUCEF_TRACE;

    try
    {    
        if ( GUCEF_NULL == m_pdhLibrary || GUCEF_NULL == m_pdhEnumObjectsW )
            return -1;

        DWORD dwInstanceSize = 0;
        LPWSTR szCounterList = NULL;
        LPWSTR szInstanceList = NULL;
    
        // Get buffer sizes
        DWORD requiredBufferSize = 0;
        DWORD dwDetailLevel = PERF_DETAIL_WIZARD; // PERF_DETAIL_NOVICE; // PERF_DETAIL_WIZARD; // Adjust detail level as needed
        PDH_STATUS status = m_pdhEnumObjectsW( GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, &requiredBufferSize, dwDetailLevel, TRUE);
        if ( 0 == requiredBufferSize || ( status != ERROR_SUCCESS && status != PDH_MORE_DATA  ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to obtain size of storage need for stat list. error code: " + ToString( status ) );
            return -2;
        }        

        // Allocate memory for object names and load them into the buffer
        CDynamicBuffer objectNamesBuffer( requiredBufferSize * sizeof(wchar_t), true );
        objectNamesBuffer.SetDataSize( objectNamesBuffer.GetBufferSize() );
        status = m_pdhEnumObjectsW( GUCEF_NULL, GUCEF_NULL, objectNamesBuffer.AsTypePtr< wchar_t >(), &requiredBufferSize, dwDetailLevel, FALSE );
        if ( status != ERROR_SUCCESS ) 
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to enumerate performance objects. error code: " + ToString( status ) );
            return -3;
        }
        objectNamesBuffer.SetDataSize( (UInt32) ( requiredBufferSize * sizeof(wchar_t) ) );

        wchar_t separatorCodePoint = 0;
        objectNamesBuffer.ParseUniqueWStringElements( separatorCodePoint, statCategories, false );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl:GetListOfStatCategories: exception: " + ToString( e.what() ) );
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::GetListOfStatCountersInCategory( const CString& statCategory ,
                                                 CStringSet& statsInCategory ) const
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL == m_pdhLibrary || GUCEF_NULL == m_pdhEnumObjectItemsW )
            return -1;

        DWORD dwInstanceSize = 0;
        LPWSTR szCounterList = NULL;
        LPWSTR szInstanceList = NULL;
    
        // Get buffer sizes
        DWORD requiredBufferSize = 0;
        DWORD dwDetailLevel = PERF_DETAIL_WIZARD; //PERF_DETAIL_NOVICE; // PERF_DETAIL_WIZARD; // Adjust detail level as needed
        PDH_STATUS status = m_pdhEnumObjectItemsW( GUCEF_NULL, GUCEF_NULL, ToWString( statCategory ).c_str(), GUCEF_NULL, &requiredBufferSize, GUCEF_NULL, &dwInstanceSize, dwDetailLevel, 0 );
        if ( 0 == requiredBufferSize || ( status != ERROR_SUCCESS && status != PDH_MORE_DATA ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to obtain size of storage need for stat list. error code: " + ToString( status ) );
            return -2;
        }        
        // Allocate memory for counter names and instance names
        CDynamicBuffer counterNamesBuffer( requiredBufferSize * sizeof(wchar_t), true );
        counterNamesBuffer.SetDataSize( counterNamesBuffer.GetBufferSize() );
        
        dwInstanceSize = 0;
        status = m_pdhEnumObjectItemsW( GUCEF_NULL, GUCEF_NULL, ToWString( statCategory ).c_str(), counterNamesBuffer.AsTypePtr< wchar_t >(), &requiredBufferSize, GUCEF_NULL, &dwInstanceSize, dwDetailLevel, 0 );
        
        if ( 0 == requiredBufferSize || ( status != ERROR_SUCCESS && status != PDH_MORE_DATA ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to enumerate performance counters in category '" + statCategory + "'. error code: " + ToString( status ) );
            return -3;
        }
        
        counterNamesBuffer.SetDataSize( (UInt32) ( requiredBufferSize * sizeof(wchar_t) ) );
        
        wchar_t separatorCodePoint = 0;
        counterNamesBuffer.ParseUniqueWStringElements( separatorCodePoint, statsInCategory, false );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl:GetListOfStatCategories: exception: " + ToString( e.what() ) );
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::GetListOfStatInstancesInCategory( const CString& statCategory ,
                                                  CStringSet& statsInCategory ) const
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL == m_pdhLibrary || GUCEF_NULL == m_pdhEnumObjectItemsW )
            return -1;

        DWORD dwInstanceSize = 0;
        LPWSTR szCounterList = NULL;
        LPWSTR szInstanceList = NULL;
    
        // Get buffer sizes
        DWORD counterBufferSize = 0;
        DWORD dwDetailLevel = PERF_DETAIL_WIZARD; //PERF_DETAIL_NOVICE; // PERF_DETAIL_WIZARD; // Adjust detail level as needed
        PDH_STATUS status = m_pdhEnumObjectItemsW( GUCEF_NULL, GUCEF_NULL, ToWString( statCategory ).c_str(), GUCEF_NULL, &counterBufferSize, GUCEF_NULL, &dwInstanceSize, dwDetailLevel, 0 );
        if ( status != ERROR_SUCCESS && status != PDH_MORE_DATA )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to obtain size of storage need for stat list. error code: " + ToString( status ) );
            return -2;
        }        

        // Allocate memory for counter names and instance names
        CDynamicBuffer instanceNamesBuffer( dwInstanceSize * sizeof(wchar_t), true );
        instanceNamesBuffer.SetDataSize( instanceNamesBuffer.GetBufferSize() );
        
        counterBufferSize = 0;
        status = m_pdhEnumObjectItemsW( GUCEF_NULL, GUCEF_NULL, ToWString( statCategory ).c_str(), GUCEF_NULL, &counterBufferSize, instanceNamesBuffer.AsTypePtr< wchar_t >(), &dwInstanceSize, dwDetailLevel, 0 );
        
        if ( status != ERROR_SUCCESS && status != PDH_MORE_DATA )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl: Failed to enumerate performance counters in category '" + statCategory + "'. error code: " + ToString( status ) );
            return -3;
        }
        
        instanceNamesBuffer.SetDataSize( (UInt32) ( dwInstanceSize * sizeof(wchar_t) ) );
        
        wchar_t separatorCodePoint = 0;
        instanceNamesBuffer.ParseUniqueWStringElements( separatorCodePoint, statsInCategory, false );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "PdhAccessImpl:GetListOfStatCategories: exception: " + ToString( e.what() ) );
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool 
CPdhAccessImpl::CollectStats( UInt64& lastCollectTimeInTicks )
{GUCEF_TRACE;

    lastCollectTimeInTicks = m_lastCollectTimeInTicks;
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
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_BELOW_NORMAL, "PdhAccessImpl: Collected stats" );
        }
        lastCollectTimeInTicks = m_lastCollectTimeInTicks = currentTimeInTicks;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::GetPhysicalDevicePerfStats( CPdhAccess::CPdhDevicePerfStatAccess& devicePerfStatAccess ,
                                            CStorageDeviceInformation& devicePerfStats                 ,
                                            bool& hasNewStats                                          )
{GUCEF_TRACE;

    devicePerfStats.perfStats.Clear();
    devicePerfStats.hasPerfStats = false;
    hasNewStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == devicePerfStatAccess.m_impl )
        return -100;
    if GUCEF_PREDICT_FALSE( !devicePerfStats.hasDeviceIndex )
        return -101;
    
    if GUCEF_PREDICT_FALSE( !devicePerfStatAccess.m_impl->IsInitialized() )
    {
        // Initialize the PDH counters for the specified device ID
        if ( !devicePerfStatAccess.m_impl->InitForDeviceId( devicePerfStats.deviceIndex, this ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to initialize PDH device perf counters for device ID: " + devicePerfStats.deviceId );
            return -102;
        }
    }

    // Collect the PDH data
    UInt64 rtQueryLastCollectTimeInTicks = 0;
    if ( CollectStats( rtQueryLastCollectTimeInTicks ) )
    {
        if GUCEF_PREDICT_FALSE( !devicePerfStatAccess.m_impl->CollectStats( devicePerfStats, rtQueryLastCollectTimeInTicks, hasNewStats ) )
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

Int32 
CPdhAccessImpl::GetLogicalVolumePerfStats( CPdhAccess::CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                           CStorageVolumeInformation& volumePerfStats                 ,
                                           bool& hasNewStats                                          )
{GUCEF_TRACE;

    volumePerfStats.perfStats.Clear();
    volumePerfStats.hasPerfStats = false;
    hasNewStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == volumePerfStatAccess.m_impl )
        return -100;
    if GUCEF_PREDICT_FALSE( !volumePerfStats.hasVolumeId || volumePerfStats.volumeId.IsNULLOrEmpty() )
        return -101;

    if GUCEF_PREDICT_FALSE( !volumePerfStats.hasPaths || volumePerfStats.paths.empty() )
    {
        if ( GetAllFileSystemPathNamesForVolume( volumePerfStats.volumeId, volumePerfStats.paths ) )
        {
            volumePerfStats.hasPaths = true;
            if ( volumePerfStats.paths.empty() )
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "No file system path names found for volume ID: " + volumePerfStats.volumeId );
                return -102;
            }
            else
            {
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Retrieved file system path names for volume ID: " + volumePerfStats.volumeId );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to retrieve file system path names for volume ID: " + volumePerfStats.volumeId );
            return -102;
        }
    }
    
    if GUCEF_PREDICT_FALSE( !volumePerfStatAccess.m_impl->IsInitialized() )
    {
        // Initialize the PDH counters for the specified device ID
        const CString& fullMountPath = *volumePerfStats.paths.begin(); 
        CString mountPath = fullMountPath.SubstrToChar( ':', true, false ) + ':';

        if ( !volumePerfStatAccess.m_impl->InitForMountPath( mountPath, this ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to initialize PDH volume perf counters for volume ID: " + volumePerfStats.volumeId );
            return -103;
        }
    }

    // Collect the PDH data
    UInt64 rtQueryLastCollectTimeInTicks = 0;
    if ( CollectStats( rtQueryLastCollectTimeInTicks ) )
    {
        if GUCEF_PREDICT_FALSE( !volumePerfStatAccess.m_impl->CollectStats( volumePerfStats, rtQueryLastCollectTimeInTicks, hasNewStats ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data for volume Id: " + volumePerfStats.volumeId );
            return -104;
        }
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data " );
        return -105;
    }
    
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccessImpl::GetTotalLogicalVolumePerfStats( CPdhAccess::CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                                CStorageVolumeInformation& volumePerfStats                 ,
                                                bool& hasNewStats                                          )
{GUCEF_TRACE;

    volumePerfStats.perfStats.Clear();
    volumePerfStats.hasPerfStats = false;
    hasNewStats = false;
    
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == volumePerfStatAccess.m_impl )
        return -100;
    if GUCEF_PREDICT_FALSE( volumePerfStats.hasVolumeId ) // for the total we should NOT have a volume id
        return -101;
    
    if GUCEF_PREDICT_FALSE( !volumePerfStatAccess.m_impl->IsInitialized() )
    {
        // Initialize the PDH counters for the total 
        if ( !volumePerfStatAccess.m_impl->InitForMountPath( "_Total", this ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to initialize PDH volume perf counters for system wide volume stats" );
            return -102;
        }
    }

    // Collect the PDH data
    UInt64 rtQueryLastCollectTimeInTicks = 0;
    if ( CollectStats( rtQueryLastCollectTimeInTicks ) )
    {
        if GUCEF_PREDICT_FALSE( !volumePerfStatAccess.m_impl->CollectStats( volumePerfStats, rtQueryLastCollectTimeInTicks, hasNewStats ) )
        {
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "Failed to collect PDH data for system wide volume stats" );
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
CPdhAccess::GetListOfStatCategories( CStringSet& statCategories ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetListOfStatCategories( statCategories );
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetListOfStatCountersInCategory( const CString& statCategory ,
                                             CStringSet& statsInCategory ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetListOfStatCountersInCategory( statCategory, statsInCategory );
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetListOfStatInstancesInCategory( const CString& statCategory ,
                                              CStringSet& statsInCategory ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetListOfStatInstancesInCategory( statCategory, statsInCategory );
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetPhysicalDevicePerfStats( CPdhDevicePerfStatAccess& devicePerfStatAccess ,
                                        CStorageDeviceInformation& devicePerfStats     ,
                                        bool& hasNewStats                              )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetPhysicalDevicePerfStats( devicePerfStatAccess, devicePerfStats, hasNewStats );
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetLogicalVolumePerfStats( CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                       CStorageVolumeInformation& volumePerfStats     ,
                                       bool& hasNewStats                              )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetLogicalVolumePerfStats( volumePerfStatAccess, volumePerfStats, hasNewStats );
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32 
CPdhAccess::GetTotalLogicalVolumePerfStats( CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                            CStorageVolumeInformation& volumePerfStats     ,
                                            bool& hasNewStats                              )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_impl )
        return m_impl->GetTotalLogicalVolumePerfStats( volumePerfStatAccess, volumePerfStats, hasNewStats );
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

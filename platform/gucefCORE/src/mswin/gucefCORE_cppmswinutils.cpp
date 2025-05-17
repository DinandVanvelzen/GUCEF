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

#include "gucefCORE_Logging.h" 

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
//      UTILTIIES                                                          //
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

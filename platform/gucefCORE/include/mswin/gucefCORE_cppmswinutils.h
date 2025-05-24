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

#ifndef GUCEF_CORE_CPPMSWINUTILS_H
#define GUCEF_CORE_CPPMSWINUTILS_H

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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"             //  we need the data classes
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWindowsComponentAccess;

class CWmiAccessImpl;

class GUCEF_CORE_PUBLIC_CPP CWmiAccess 
{
    public:

    /**
     *  Attempts to load and link the WMI API
     *  Important: 
     *      - This function will return a failure if the WMI API is not available on the system
     *  @return 0 is success, any other value is an error code
     */
    Int32 TryLoadWMI( void );

    /**
     *  Attempts to unload and unlink the WMI API
     *  Expected to be called when the WMI API is no longer needed and after a successful call to TryLoadWMI()
     *  @return 0 is success, any other value is an error code
     */
    Int32 UnloadWMI( void );

    /**
     *  Attempts to obtain physical device ids via WMI given a volume id
     * 
     *  This produces a map since a logical volume can span multiple partitions and thus also devices
     *  That is less typical though. More typically a volume will relate to 1 partition which is by definition on 1 device
     */
    Int32 GetPhysicalDeviceIdsFromWMI( const CString& volumeGuid              ,
                                       CStringMap& partitionId2physicalDiskId );
    
    bool IsValid( void ) const;
    
    private:
    friend class CWindowsComponentAccess;
    
    CWmiAccess( void );

    ~CWmiAccess();

    private:

    CWmiAccessImpl* m_impl;
};  

/*-------------------------------------------------------------------------*/

class CPdhAccessImpl;
class CPdhDevicePerfStatAccessImpl;
class CPdhVolumePerfStatAccessImpl;

/**
 *  Class which acts as a wrapper for the PDH API (Performance Data Helper API)
 * 
 *  PDH is a high-level API that provides a way to access and manage performance counter data. 
 *  Performance counters are the actual data points that represent system metrics (like CPU usage, memory usage, etc.). 
 *  Windows Performance Counters are the underlying data source, and PDH provides a way for applications to interact with them. 
 * 
 *  It is provided by pdh.dll and may not be available on all Windows systems
 */
class GUCEF_CORE_PUBLIC_CPP CPdhAccess 
{
    public:

    /**
     *  Class that hold private data needed to access device performance data
     *  Intended to be reused for multiple calls to GetPhysicalDevicePerfStats()
     */
    class GUCEF_CORE_PUBLIC_CPP CPdhDevicePerfStatAccess
    {
        public:

        CPdhDevicePerfStatAccess( void );

        ~CPdhDevicePerfStatAccess();        
        
        private:
        CPdhDevicePerfStatAccess( const CPdhDevicePerfStatAccess& src );  /**< do not copy */
        CPdhDevicePerfStatAccess& operator=( const CPdhDevicePerfStatAccess& src );  /**< do not assign */
        
        private:
        friend class CPdhAccessImpl;

        CPdhDevicePerfStatAccessImpl* m_impl;
    };

    /**
     *  Class that hold private data needed to access storage volume performance data
     *  Intended to be reused for multiple calls to GetLogicalVolumePerfStats()
     */
    class GUCEF_CORE_PUBLIC_CPP CPdhVolumePerfStatAccess
    {
        public:

        CPdhVolumePerfStatAccess( void );

        ~CPdhVolumePerfStatAccess();        
        
        private:
        CPdhVolumePerfStatAccess( const CPdhVolumePerfStatAccess& src );  /**< do not copy */
        CPdhVolumePerfStatAccess& operator=( const CPdhVolumePerfStatAccess& src );  /**< do not assign */
        
        private:
        friend class CPdhAccessImpl;

        CPdhVolumePerfStatAccessImpl* m_impl;
    };

    /**
     *  Attempts to load and link the PDH API
     *  Important: 
     *      - This function will return a failure if the PDH API is not available on the system
     *  @return 0 is success, any other value is an error code
     */
    Int32 TryLoadPDH( void );

    /**
     *  Attempts to unload and unlink the WMI API
     *  Expected to be called when the WMI API is no longer needed and after a successful call to TryLoadWMI()
     *  @return 0 is success, any other value is an error code
     */
    Int32 UnloadPDH( void );

    /**
     *  Attempts to the list of all available system stat categories
     *  
     *  @param statCatagories the list of all available system stat categories 
     */
    Int32 GetListOfStatCategories( CStringSet& statCategories ) const;

    /**
     *  Attempts to the list of all available stat counters for the given category
     *  
     *  @param statCategory the stat category for which to obtain the stats
     *  @param statsInCategory the list of all available stats in the given stat category
     */
    Int32 GetListOfStatCountersInCategory( const CString& statCategory ,
                                           CStringSet& statsInCategory ) const;

    /**
     *  Attempts to the list of all available stat instances for the given category
     *  
     *  @param statCategory the stat category for which to obtain the stats
     *  @param statsInCategory the list of all available stats in the given stat category
     */
    Int32 GetListOfStatInstancesInCategory( const CString& statCategory ,
                                            CStringSet& statsInCategory ) const;

    /**
     *  Attempts to obtain physical device performance stats via PDH
     *  The device index needs to be known and set
     *  
     *  @param devicePerfStatAccess the object that will hold data needed to efficiently gather the stats repeatedly 
     *  @param devicePerfStats the object that will hold the performance stats
     *  @param hasNewStats true if new stats were obtained, false if the stats are the same as before
     */
    Int32 GetPhysicalDevicePerfStats( CPdhDevicePerfStatAccess& devicePerfStatAccess ,
                                      CStorageDeviceInformation& devicePerfStats     ,
                                      bool& hasNewStats                              );
    
    /**
     *  Attempts to obtain logical volume performance stats via PDH
     *  The volume needs to have mount paths. These can be resolved from the GUID if needed on first hit, however
     *  not all volumes have mount paths. If the volume does not have mount paths then it is not possible to obtain the stats
     *  
     *  @param volumePerfStatAccess the object that will hold data needed to efficiently gather the stats repeatedly 
     *  @param volumePerfStats the object that will hold the performance stats
     *  @param hasNewStats true if new stats were obtained, false if the stats are the same as before
     */
    Int32 GetLogicalVolumePerfStats( CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                     CStorageVolumeInformation& volumePerfStats     ,
                                     bool& hasNewStats                              );

    /**
     *  Attempts to obtain system total storage performance stats via PDH
     *  
     *  @param volumePerfStatAccess the object that will hold data needed to efficiently gather the stats repeatedly 
     *  @param totalPerfStats the object that will hold the performance stats
     *  @param hasNewStats true if new stats were obtained, false if the stats are the same as before
     */
    Int32 GetTotalLogicalVolumePerfStats( CPdhVolumePerfStatAccess& volumePerfStatAccess ,
                                          CStorageVolumeInformation& volumePerfStats     ,
                                          bool& hasNewStats                              );

    bool IsValid( void ) const;
    
    private:
    friend class CWindowsComponentAccess;
    
    CPdhAccess( void );

    ~CPdhAccess();

    private:

    CPdhAccessImpl* m_impl;
};  

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CWindowsComponentAccess 
{
    public:

    static CWindowsComponentAccess* Instance( void );

    static void Deinstance( void );

    CWmiAccess* GetWmiAccess( void );

    CPdhAccess* GetPdhAccess( void );

    private:

    CWindowsComponentAccess( void );                                           /**< singleton thus private */
    CWindowsComponentAccess( const CWindowsComponentAccess& src );             /**< singleton, not implemented */
    CWindowsComponentAccess& operator=( const CWindowsComponentAccess& src );  /**< singleton, not implemented */
    ~CWindowsComponentAccess();
    
    private:
    
    CWmiAccess* m_wmiAccess;
    CPdhAccess* m_pdhAccess;
    static CWindowsComponentAccess* g_instance;
    static MT::CMutex g_instanceLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

#endif /* GUCEF_CORE_CPPMSWINUTILS_H ? */


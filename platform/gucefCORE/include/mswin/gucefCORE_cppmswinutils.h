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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

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
    
    CWmiAccess( void );

    virtual ~CWmiAccess();

    private:

    CWmiAccessImpl* m_impl;
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


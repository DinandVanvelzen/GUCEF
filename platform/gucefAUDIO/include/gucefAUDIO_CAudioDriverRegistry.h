/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H
#define GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAudioGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  All audio drivers that can be used, by name. Drivers from plugins register themselves here when the
 *  plugin is loaded. A driver can also be registered directly from code, which is how a statically linked build
 *  (WebAssembly) gets its drivers. Reach it through CAudioGlobal.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioDriverRegistry : public CORE::CTGlobalONRegistry< CAudioDriver, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CAudioDriver, MT::CMutex >::TRegisteredObjPtr   TAudioDriverPtr;

    /** Finds a driver by name, without regard for case */
    bool TryFindDriver( const CString& driverName, TAudioDriverPtr& driver ) const;

    /** The registered drivers, best (highest priority) first */
    void GetDriversByPriority( GUCEF::vector< TAudioDriverPtr >& drivers ) const;

    /**
     *  Opens a device with the named driver, or when the name is empty with the first driver (best first) that manages it.
     *  The name of the driver that was used is given back. False when no driver could open the device.
     */
    bool OpenDevice( const CString& driverName, const CAudioDeviceParams& params, IAudioDeviceCallback* callback, TAudioDevicePtr& device, CString& usedDriverName ) const;

    private:
    friend class CAudioGlobal;

    CAudioDriverRegistry( void );

    virtual ~CAudioDriverRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CAudioDriverRegistry( const CAudioDriverRegistry& src );            /**< not implemented, don't use */
    CAudioDriverRegistry& operator=( const CAudioDriverRegistry& src ); /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIODRIVERREGISTRY_H ? */

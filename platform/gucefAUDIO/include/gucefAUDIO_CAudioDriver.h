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

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#define GUCEF_AUDIO_CAUDIODRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#include "gucefAUDIO_CAudioDeviceParams.h"
#define GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEPARAMS_H ? */

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

/**
 *  The way to the audio hardware of one kind of system: ALSA, WASAPI, CoreAudio, the browser, or no hardware at all.
 *  Drivers are registered in the CAudioDriverRegistry by name. The ones for real platforms come from driver plugins,
 *  the null driver that is built in plays to nowhere (useful for tests, servers and machines without sound).
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioDriver
{
    public:

    CAudioDriver( void );

    virtual ~CAudioDriver();

    /** Short name that selects the driver ("alsa", "wasapi", "null") */
    virtual CString GetName( void ) const = 0;

    virtual CString GetDescription( void ) const;

    /**
     *  When no driver is named the one with the highest priority that can open the device is used.
     *  Platform drivers are 0 and up (a driver for a sound server can be above a driver for the bare hardware),
     *  the null driver is far below everything.
     */
    virtual Int32 GetPriority( void ) const;

    /** Lists the devices of the given direction that the system has right now */
    virtual bool EnumerateDevices( TAudioDirection direction, TAudioDeviceInfoVector& devices ) = 0;

    /** The device used when nothing is chosen. The default implementation takes the one that EnumerateDevices() marks as default, or else the first */
    virtual bool GetDefaultDevice( TAudioDirection direction, CAudioDeviceInfo& device );

    /**
     *  Opens a device (stopped). The callback is called by the device once it is started and has to stay valid
     *  until the device is gone. False when the device does not exist or cannot be opened as asked.
     */
    virtual bool OpenDevice( const CAudioDeviceParams& params, IAudioDeviceCallback* callback, TAudioDevicePtr& device ) = 0;

    private:

    CAudioDriver( const CAudioDriver& src );            /**< not implemented */
    CAudioDriver& operator=( const CAudioDriver& src ); /**< not implemented */
};

typedef CORE::CTSharedPtr< CAudioDriver, MT::CMutex > TAudioDriverPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

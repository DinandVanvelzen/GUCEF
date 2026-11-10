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

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#define GUCEF_AUDIO_CAUDIODEVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEINFO_H
#include "gucefAUDIO_CAudioDeviceInfo.h"
#define GUCEF_AUDIO_CAUDIODEVICEINFO_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEINFO_H ? */

#ifndef GUCEF_AUDIO_IAUDIODEVICECALLBACK_H
#include "gucefAUDIO_IAudioDeviceCallback.h"
#define GUCEF_AUDIO_IAUDIODEVICECALLBACK_H
#endif /* GUCEF_AUDIO_IAUDIODEVICECALLBACK_H ? */

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

class CNullAudioDevice;
class CAudioPushDevice;

/**
 *  An audio device that has been opened: it plays or records in one format until it is stopped or closed.
 *  Drivers make these, everything else only uses them. The device does nothing until Start() and
 *  closes when the last reference to it goes away.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioDevice
{
    public:

    typedef CORE::CTSharedPtr< CAudioDevice, MT::CMutex > TAudioDevicePtr;

    CAudioDevice( const CAudioDeviceInfo& info, IAudioDeviceCallback* callback );

    virtual ~CAudioDevice();

    const CAudioDeviceInfo& GetDeviceInfo( void ) const;

    TAudioDirection GetDirection( void ) const;

    /** The format the device really works in. It can differ from what was asked for, the one that uses the device converts */
    virtual const CAudioFormat& GetFormat( void ) const = 0;

    /** Starts calling the callback. True if the device runs afterwards (also when it already did) */
    virtual bool Start( void ) = 0;

    /** Stops calling the callback. When this returns the callback is not running any more, so it is safe to destroy what it points at */
    virtual bool Stop( void ) = 0;

    virtual bool IsStarted( void ) const = 0;

    /** Frames the device asks for per callback, 0 if it varies or is not known */
    virtual UInt32 GetPeriodSizeInFrames( void ) const;

    /** Frames between giving audio to the device and hearing it, as far as the driver knows, else 0 */
    virtual UInt32 GetLatencyInFrames( void ) const;

    /** This device as the null device, NULL when it is none; no run time type information needed */
    virtual CNullAudioDevice* AsNullDevice( void );

    /** This device as a push device (also a driver's own kind of it), NULL when it is none; no run time type information needed */
    virtual CAudioPushDevice* AsPushDevice( void );

    protected:

    IAudioDeviceCallback* GetCallback( void ) const;

    private:

    CAudioDevice( const CAudioDevice& src );            /**< not implemented */
    CAudioDevice& operator=( const CAudioDevice& src ); /**< not implemented */

    private:

    CAudioDeviceInfo m_info;
    IAudioDeviceCallback* m_callback;
};

typedef CAudioDevice::TAudioDevicePtr TAudioDevicePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

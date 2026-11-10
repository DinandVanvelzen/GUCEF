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

#ifndef GUCEF_AUDIO_CAUDIODEVICEINFO_H
#define GUCEF_AUDIO_CAUDIODEVICEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

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
 *  What a driver tells about one of its audio devices: a speaker, a headset, a microphone, a virtual output.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioDeviceInfo
{
    public:

    CAudioDeviceInfo( void );

    CAudioDeviceInfo( const CString& id, const CString& name, TAudioDirection direction );

    CAudioDeviceInfo( const CAudioDeviceInfo& src );

    CAudioDeviceInfo& operator=( const CAudioDeviceInfo& src );

    ~CAudioDeviceInfo();

    /** Identifies the device to its driver, stable between runs where the platform allows. Empty means the default device */
    const CString& GetId( void ) const;
    void SetId( const CString& id );

    /** Name for people to read */
    const CString& GetName( void ) const;
    void SetName( const CString& name );

    TAudioDirection GetDirection( void ) const;
    void SetDirection( TAudioDirection direction );

    /** True for the device the system uses when nothing else is chosen */
    bool IsDefault( void ) const;
    void SetIsDefault( bool isDefault );

    /** The format the device works with natively, the one that needs no conversion in the driver. Invalid when the driver does not know */
    const CAudioFormat& GetPreferredFormat( void ) const;
    void SetPreferredFormat( const CAudioFormat& format );

    /** Most channels the device can play or record, 0 when unknown */
    UInt32 GetMaxChannelCount( void ) const;
    void SetMaxChannelCount( UInt32 channels );

    /** Anything else the driver wants to say about the device, as name / value pairs */
    const CORE::CValueList& GetProperties( void ) const;
    CORE::CValueList& GetProperties( void );

    private:

    CString m_id;
    CString m_name;
    TAudioDirection m_direction;
    bool m_isDefault;
    CAudioFormat m_preferredFormat;
    UInt32 m_maxChannelCount;
    CORE::CValueList m_properties;
};

typedef GUCEF::vector< CAudioDeviceInfo > TAudioDeviceInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIODEVICEINFO_H ? */

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

#ifndef GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H
#define GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The unit a parameter value is expressed in, for editors and configuration */
enum EAudioParameterUnit
{
    AUDIOPARAMUNIT_NONE = 0     ,
    AUDIOPARAMUNIT_DECIBEL      ,
    AUDIOPARAMUNIT_HERTZ        ,
    AUDIOPARAMUNIT_MILLISECONDS ,
    AUDIOPARAMUNIT_SECONDS      ,
    AUDIOPARAMUNIT_RATIO        ,
    AUDIOPARAMUNIT_PERCENT      ,
    AUDIOPARAMUNIT_LINEAR_GAIN
};
typedef enum EAudioParameterUnit TAudioParameterUnit;

/** How the range of a parameter is best presented and smoothed: evenly, or evenly per octave/decade (frequencies, times) */
enum EAudioParameterScale
{
    AUDIOPARAMSCALE_LINEAR = 0 ,
    AUDIOPARAMSCALE_LOGARITHMIC
};
typedef enum EAudioParameterScale TAudioParameterScale;

/** What kind of values a parameter takes. All are stored as Float32; integral kinds hold integral values */
enum EAudioParameterKind
{
    AUDIOPARAMKIND_CONTINUOUS = 0 ,
    AUDIOPARAMKIND_INTEGER        ,
    AUDIOPARAMKIND_BOOLEAN        ,     /**< 0 or 1 */
    AUDIOPARAMKIND_ENUMERATION          /**< 0 .. label count - 1, each value has a label */
};
typedef enum EAudioParameterKind TAudioParameterKind;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Describes one parameter of an effect: its id (the index of the parameter in its effect, an enumeration value of the
 *  concrete effect class), a stable name used as configuration key (lowerCamelCase), unit, scale, range, default and kind.
 *  An effect class keeps one table of these for all its instances; the descriptors do not change after construction.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioEffectParameterInfo
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    CAudioEffectParameterInfo( void );

    /** A continuous, integer or boolean parameter */
    CAudioEffectParameterInfo( UInt32 id                         ,
                               const CString& name               ,
                               TAudioParameterUnit unit          ,
                               TAudioParameterScale scale        ,
                               Float32 minimum                   ,
                               Float32 maximum                   ,
                               Float32 defaultValue              ,
                               TAudioParameterKind kind = AUDIOPARAMKIND_CONTINUOUS );

    /** An enumeration with the given labels, values 0 .. labelCount - 1 */
    CAudioEffectParameterInfo( UInt32 id                 ,
                               const CString& name       ,
                               const char* const* labels ,
                               UInt32 labelCount         ,
                               UInt32 defaultIndex       );

    CAudioEffectParameterInfo( const CAudioEffectParameterInfo& src );

    CAudioEffectParameterInfo& operator=( const CAudioEffectParameterInfo& src );

    ~CAudioEffectParameterInfo();

    UInt32 GetId( void ) const;
    const CString& GetName( void ) const;
    TAudioParameterUnit GetUnit( void ) const;
    TAudioParameterScale GetScale( void ) const;
    TAudioParameterKind GetKind( void ) const;
    Float32 GetMinimum( void ) const;
    Float32 GetMaximum( void ) const;
    Float32 GetDefaultValue( void ) const;

    UInt32 GetEnumLabelCount( void ) const;

    /** The label of an enumeration value, empty when out of range */
    const CString& GetEnumLabel( UInt32 index ) const;

    /** The value of an enumeration label, without regard for case */
    bool TryGetEnumValue( const CString& label, Float32& value ) const;

    /** The value made valid: limited to the range, rounded for the integral kinds. Not a number gives the default */
    Float32 Clamp( Float32 value ) const;

    private:

    UInt32 m_id;
    CString m_name;
    TAudioParameterUnit m_unit;
    TAudioParameterScale m_scale;
    TAudioParameterKind m_kind;
    Float32 m_minimum;
    Float32 m_maximum;
    Float32 m_defaultValue;
    TStringVector m_enumLabels;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOEFFECTPARAMETERINFO_H ? */

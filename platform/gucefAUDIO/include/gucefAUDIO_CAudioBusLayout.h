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

#ifndef GUCEF_AUDIO_CAUDIOBUSLAYOUT_H
#define GUCEF_AUDIO_CAUDIOBUSLAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIO_ETYPES_H
#include "gucefAUDIO_ETypes.h"
#define GUCEF_AUDIO_ETYPES_H
#endif /* GUCEF_AUDIO_ETYPES_H ? */

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
 *  A description of the bus tree of an audio engine, as a value: the buses with their parent, volume and mute, the sends
 *  of every bus and its insert effects, each with its type, slot, switch, wet and dry level and its own configuration
 *  (the parameters of the effect). It can be captured from a running engine (CAudioEngine::GetBusLayout()), applied to
 *  an engine (CAudioEngine::ApplyBusLayout(), which has the exact rules) and read from or written to a configuration
 *  tree, so a game keeps its mixing set-up in a settings file.
 *
 *  The master bus is a bus like the others, named "master", and has no parent. A bus without a parent name is a child of
 *  the master bus. Volumes and send levels are in decibels, wet and dry levels are linear.
 *
 *  Serialized form (the node given to Serialize() is the section; Deserialize() also finds the section "AudioBusLayout"
 *  as a child of the node it is given):
 *  <pre>
 *  AudioBusLayout
 *      Bus  name="sfx" parent="master" volumeDb="-3" muted="false"
 *          Send   target="reverb" levelDb="-10" preFader="false"
 *          Effect type="compressor" slot="0" enabled="true" wet="1" dry="0"
 *              Options        (construction options, only for effects that have them)
 *              Parameters     threshold="-20" ratio="3" ...
 *  </pre>
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioBusLayout : public CORE::CIDataNodeSerializable
{
    public:

    static const CString ConfigSectionName;         /**< "AudioBusLayout" */
    static const CString MasterBusName;             /**< "master", the name of the top of the tree */

    /** A send from the bus that holds it to the bus 'target' */
    struct GUCEF_AUDIO_PUBLIC_CPP SSend
    {
        CString target;
        Float32 levelInDecibels;
        bool preFader;                              /**< taken before the fader of the bus (true) or after it (false, the default) */

        SSend( void );
        SSend( const CString& target, Float32 levelInDecibels, bool preFader = false );
    };
    typedef GUCEF::vector< SSend > TSendVector;

    /** An effect in an insert slot of the bus that holds it */
    struct GUCEF_AUDIO_PUBLIC_CPP SEffect
    {
        UInt32 slot;
        CString typeName;                           /**< the name the effect is registered under in CAudioEffectRegistry, "reverb", "compressor", ... */
        bool enabled;
        Float32 wet;                                /**< linear */
        Float32 dry;                                /**< linear */
        CORE::CDataNode config;                     /**< the configuration of the effect itself as CAudioEffect::SaveConfig() writes it: "Parameters" and, if any, "Options" */

        SEffect( void );
        SEffect( UInt32 slot, const CString& typeName );
        SEffect( const SEffect& src );
        SEffect& operator=( const SEffect& src );

        /** Sets a parameter of the effect by name, in the configuration */
        void SetParameter( const CString& name, const CORE::CVariant& value );

        /** Sets a construction option of the effect by name, in the configuration */
        void SetOption( const CString& name, const CORE::CVariant& value );
    };
    typedef GUCEF::vector< SEffect > TEffectVector;

    struct GUCEF_AUDIO_PUBLIC_CPP SBus
    {
        CString name;
        CString parent;                             /**< empty for a child of the master bus, and for the master bus itself */
        Float32 volumeInDecibels;
        bool muted;
        TSendVector sends;
        TEffectVector effects;

        SBus( void );
        SBus( const CString& name, const CString& parent = CString::Empty, Float32 volumeInDecibels = 0.0f, bool muted = false );

        /** Adds a send and gives it back */
        SSend& AddSend( const CString& target, Float32 levelInDecibels, bool preFader = false );

        /** Adds an effect and gives it back */
        SEffect& AddEffect( UInt32 slot, const CString& typeName );
    };
    typedef GUCEF::vector< SBus > TBusVector;

    CAudioBusLayout( void );

    CAudioBusLayout( const CAudioBusLayout& src );

    virtual ~CAudioBusLayout() GUCEF_VIRTUAL_OVERRIDE;

    CAudioBusLayout& operator=( const CAudioBusLayout& src );

    void Clear( void );

    bool IsEmpty( void ) const;

    /** Adds a bus and gives it back. The reference is good until the next bus is added */
    SBus& AddBus( const CString& name, const CString& parent = CString::Empty, Float32 volumeInDecibels = 0.0f, bool muted = false );

    /** The bus with that name, null when there is none */
    const SBus* FindBus( const CString& name ) const;
    SBus* FindBus( const CString& name );

    const TBusVector& GetBuses( void ) const;
    TBusVector& GetBuses( void );

    /**
     *  Whether two layouts describe the same tree. Buses are matched by name, not by position; the effects of a bus by slot;
     *  the sends of a bus as a set. Decibel values are the same within 0.001 dB (a volume lives as a linear factor in
     *  the engine, so a volume that went through it comes back a hair different), other numbers are the same within one part in
     *  a million. Effect parameters are compared by name and value, not by how the value is written
     */
    bool operator==( const CAudioBusLayout& other ) const;
    bool operator!=( const CAudioBusLayout& other ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Replaces the content with what the node holds. False, with what could be read kept, when a bus has no name or the
     *  node holds no layout at all. Missing values get their defaults: 0 dB, not muted, enabled, wet 1, dry 0, post fader
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    TBusVector m_buses;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOBUSLAYOUT_H ? */

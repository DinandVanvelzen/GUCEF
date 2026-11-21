/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H
#define GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the service says about one of its settings that a client may read and that an administrator may change while the
 *  service runs (gucefASSETSTORE_RemoteService_Design.md 18): the key (the dotted path below the section of the service in
 *  the site settings, "reprocess.maxConcurrentJobs"), its type with its range or allowed values, its default, a description
 *  for people, whether a change applies at once ("live") or at the next start ("restart"), who may read its value
 *  ("public", or "owner-only-read": the devices with the role owner and the site administrator) and the group a form shows
 *  it in. Values travel as text in the canonical form Normalize() gives.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingDescriptor : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString TypeBool;                    /**< "bool": true or false */
    static const CORE::CString TypeUInt;                    /**< "uint": a whole number from 0 */
    static const CORE::CString TypeInt;                     /**< "int": a whole number */
    static const CORE::CString TypeFloat;                   /**< "float": a number */
    static const CORE::CString TypeString;                  /**< "string": a text without control characters */
    static const CORE::CString TypeEnum;                    /**< "enum": one of allowedValues */
    static const CORE::CString ApplyLive;                   /**< "live": applied at once */
    static const CORE::CString ApplyRestart;                /**< "restart": kept and applied at the next start */
    static const CORE::CString SensitivityPublic;           /**< "public": every client may read the value */
    static const CORE::CString SensitivityOwnerOnlyRead;    /**< "owner-only-read": only devices with the role owner and the site administrator read the value */

    CORE::CString key;                                      /**< the dotted path, letters, digits, '.', '_' and '-' */
    CORE::CString group;                                    /**< the group a form shows the setting in */
    CORE::CString type;                                     /**< one of the types above */
    CORE::CString minimum;                                  /**< numbers: the lowest value, empty for none */
    CORE::CString maximum;                                  /**< numbers: the highest value, empty for none */
    CORE::CString::StringVector allowedValues;              /**< enum: the values in the order a form offers them */
    UInt32 maxLength;                                       /**< string: the longest text, 0 for no maximum */
    CORE::CString defaultValue;                             /**< the value without a site settings file */
    CORE::CString description;                              /**< for people */
    CORE::CString unit;                                     /**< numbers: what the number counts ("MB", "ms"), may be empty */
    CORE::CString applyMode;                                /**< ApplyLive or ApplyRestart */
    CORE::CString sensitivity;                              /**< SensitivityPublic or SensitivityOwnerOnlyRead */

    CAssetSettingDescriptor( void );
    virtual ~CAssetSettingDescriptor() GUCEF_VIRTUAL_OVERRIDE;

    /** A setting of the type bool, live and public */
    static CAssetSettingDescriptor MakeBool( const CORE::CString& key, const CORE::CString& group, bool defaultValue, const CORE::CString& description );

    /** A setting of the type uint within [lowest, highest], live and public */
    static CAssetSettingDescriptor MakeUInt( const CORE::CString& key, const CORE::CString& group, UInt64 lowest, UInt64 highest, UInt64 defaultValue,
                                         const CORE::CString& unit, const CORE::CString& description );

    /** A setting of the type int within [lowest, highest], live and public */
    static CAssetSettingDescriptor MakeInt( const CORE::CString& key, const CORE::CString& group, Int64 lowest, Int64 highest, Int64 defaultValue,
                                        const CORE::CString& unit, const CORE::CString& description );

    /** A setting of the type float within [lowest, highest], live and public */
    static CAssetSettingDescriptor MakeFloat( const CORE::CString& key, const CORE::CString& group, Float64 lowest, Float64 highest, Float64 defaultValue,
                                          const CORE::CString& unit, const CORE::CString& description );

    /** A setting of the type string of at most maxLength characters (0: no maximum), live and public */
    static CAssetSettingDescriptor MakeString( const CORE::CString& key, const CORE::CString& group, UInt32 maxLength, const CORE::CString& defaultValue,
                                           const CORE::CString& description );

    /** A setting of the type enum, live and public */
    static CAssetSettingDescriptor MakeEnum( const CORE::CString& key, const CORE::CString& group, const CORE::CString::StringVector& allowedValues,
                                         const CORE::CString& defaultValue, const CORE::CString& description );

    /** The same descriptor, applied at the next start */
    CAssetSettingDescriptor AppliedAtRestart( void ) const;

    /** The same descriptor, its value readable by the devices with the role owner only */
    CAssetSettingDescriptor OwnerOnlyRead( void ) const;

    bool IsPublic( void ) const;

    bool AppliesLive( void ) const;

    /** Whether the descriptor can be used: a valid key, a known type, apply mode and sensitivity, a default that fits; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    /**
     *  Whether the text is a value of the setting: its type, its range or allowed values, its length. 'normalized' receives the
     *  canonical text ("true" or "false", a number without leading zeros or blanks, an allowed value as it is listed), 'problem'
     *  the reason when it is not a value
     */
    bool Normalize( const CORE::CString& text, CORE::CString& normalized, CORE::CString& problem ) const;

    /** What a value may be, for people: "0 to 256", "true or false", "one of newest, oldest" */
    CORE::CString GetRangeText( void ) const;

    /** Whether the key has the form of a key: letters, digits, '.', '_' and '-', not empty, at most 128 characters, no empty part */
    static bool IsValidKey( const CORE::CString& key );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of the result of a future or of the data of an event without RTTI (CICloneable::CloneableAs()) */
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H ? */

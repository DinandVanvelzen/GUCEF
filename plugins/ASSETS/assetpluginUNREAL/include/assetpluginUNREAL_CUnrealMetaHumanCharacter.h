/*
 *  assetpluginUNREAL: The root asset of a MetaHuman character package: properties and the texture payloads
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#include "assetpluginUNREAL_CIUnrealOodleProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#include "assetpluginUNREAL_CUnrealPayloadReference.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The export of the class MetaHumanCharacter of a MetaHuman package, as a typed record. The tagged properties give the name,
 *  the viewport settings (camera frame, light rotation, environment), the wardrobe and the settings of skin, eyes, makeup and
 *  head model; the settings are kept as a compact tree (scalars as attributes, structs as children) for the sidecar. The
 *  textures of the character (nine face and eleven body maps) are named by two maps of texture information in the properties;
 *  their pixels are payloads of the package trailer that the native data after the properties lists as editor bulk data
 *  records, in the order of the two maps. A payload that is stored without compression is a PNG file; the others are
 *  compressed (Oodle) and need the oodle provider.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealMetaHumanCharacter
{
    public:

    /** One texture of the character: what the property maps say and where its payload is */
    struct GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP STexture
    {
        CORE::CString group;            /**< "face" or "body" */
        CORE::CString type;             /**< the enumerator without its type, for example "Basecolor" or "Body_Normal" */
        UInt32 width;                   /**< the sizes of the texture information */
        UInt32 height;
        UInt32 slices;
        Int32 format;
        Int32 gammaSpace;
        Int32 record;                   /**< index in the records of the payloads, -1 when the character names no payload for the texture */
        bool hasPayload;
        CUnrealPayloadReference payload;
        Int32 method;                   /**< the method of the compressed buffer of the payload, -1 when it was not read */
        bool isPng;                     /**< the payload is stored and holds a PNG file */
        UInt32 pngWidth;
        UInt32 pngHeight;
        UInt64 pngSize;                 /**< bytes of the PNG file */

        STexture( void );
        bool IsExtractable( void ) const;
        bool IsOodle( void ) const;     /**< the payload is assigned and compressed with Oodle (it is read through the provider) */
    };
    typedef GUCEF::vector< STexture > TTextureVector;

    /** One editor bulk data record behind the properties, with what the payload header says */
    struct SRecord
    {
        CUnrealPayloadReference payload;
        Int32 method;                   /**< -1 when the payload could not be opened */
        bool isPng;
        UInt32 pngWidth;
        UInt32 pngHeight;
        UInt64 pngSize;
        Int32 texture;                  /**< the texture the record was assigned to, -1 for the state data of the character */

        SRecord( void );
    };
    typedef GUCEF::vector< SRecord > TRecordVector;

    CUnrealMetaHumanCharacter( void );

    void Clear( void );

    /** Reads the export (the class must be MetaHumanCharacter) of an opened package */
    bool Read( CUnrealPackage& package, UInt32 exportIndex );

    const CORE::CString& GetError( void ) const;

    /** Opens the payload of a texture and reads the PNG file (after the 64 byte header of the compressed buffer) into the buffer */
    bool ReadPng( CUnrealPackage& package, const STexture& texture, CORE::CDynamicBuffer& png ) const;

    /**
     *  Reads the PNG file of a texture whether it is stored or compressed with Oodle; the compressed ones go through the provider.
     *  The sizes of the PNG file are checked against the texture information.
     *  @param providerMissing set when the failure is that the oodle tool is not there
     */
    bool ReadPngThrough( CUnrealPackage& package, const STexture& texture, CIUnrealOodleProvider* provider, const ASSETCONVERTER::CConversionReport* cancellation, CORE::CDynamicBuffer& png, CORE::CString& error, bool& providerMissing ) const;

    CORE::CString name;                         /**< the object name of the export */
    CORE::CString packageName;                  /**< the package, for example /Game/EditableCharacters/Name/Name */
    CORE::CString nameOverride;                 /**< AssemblySettings.NameOverride */
    CORE::CString cameraFrame;                  /**< ViewportSettings.CameraFrame without the type, empty when the asset leaves it at the default */
    CORE::CString characterEnvironment;         /**< ViewportSettings.CharacterEnvironment without the type, empty for the default */
    CORE::CString previewMaterial;
    bool hasLightRotation;
    Float32 lightRotation;                      /**< ViewportSettings.LightRotation in degrees */
    bool hasHighResolutionTextures;
    bool hasFaceDnaBlendshapes;
    CORE::CString::StringVector wardrobeItems;  /**< the soft object paths of the wardrobe items of the character */
    CORE::CString::StringVector presetCharacters;
    CORE::CDataNode settings;                   /**< compact copy of SkinSettings, EyesSettings, MakeupSettings and HeadModelSettings */
    TTextureVector textures;
    TRecordVector records;                      /**< the editor bulk data records behind the properties, in file order */
    UInt32 payloadsAssigned;

    private:

    void AssignPayloads( void );

    CUnrealMetaHumanCharacter( const CUnrealMetaHumanCharacter& src );              /**< not implemented */
    CUnrealMetaHumanCharacter& operator=( const CUnrealMetaHumanCharacter& src );   /**< not implemented */

    CORE::CString m_error;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALMETAHUMANCHARACTER_H ? */

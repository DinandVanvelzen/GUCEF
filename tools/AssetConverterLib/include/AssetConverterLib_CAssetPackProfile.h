/*
 *  AssetConverterLib: what a pack recogniser knows about a recognised pack: engine, entry points, grouping and noise
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

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The record a pack recogniser (CIAssetImporter::RecognisePack()) fills for a folder or archive it knows: what kind of pack it is,
 *  the engine and its version when the pack says so, and the profile of the pack. The profile holds the entry points that the
 *  workbench uses to show the pack without clicks (the best candidate first: CPackPresentationResolver of the workbench), the grouping of
 *  its contents and the files that are noise.
 *
 *  Paths are relative to the root of the pack, with '/' as separator.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPackProfile
{
    public:

    CAssetPackProfile( void );

    void Clear( void );

    /** Whether a recogniser recognised something: the pack type is set */
    bool IsRecognised( void ) const;

    /** Whether a path is noise in this pack: it matches one of the noise patterns ('*' is the wildcard, letter case does not matter) */
    bool IsNoise( const CORE::CString& packPath ) const;

    /** The identifier of the pack type: "unity-project", "unity-package", "godot-project", "unreal-project", "metahuman-package", "model-pack" */
    CORE::CString packType;
    /** What the people see: "Unity project", "Godot project" and so on */
    CORE::CString packTypeName;
    /** "Unity", "Godot", "Unreal Engine", empty when there is none */
    CORE::CString engine;
    /** As the pack states it, for example "2022.3.10f1", "4.2" or "5.3"; empty when it does not say or is unreadable */
    CORE::CString engineVersion;
    /** The folder of the listing that holds the pack, empty when it is the root */
    CORE::CString root;
    /** The plugin that recognised it */
    CORE::CString recognisedBy;
    /** How sure the recogniser is, 0 to 100; the most sure recogniser wins when several recognise the same pack */
    UInt32 confidence;
    /** What opens first, best candidate first: the default scene of the project, demo and sample scenes, the model files of a model pack */
    CORE::CString::StringVector showcaseCandidates;
    /** The folders that group the contents of the pack, such as "Assets/Models" */
    CORE::CString::StringVector groupingHints;
    /** Wildcard patterns of the files that are no content: "*.meta", "*.import" */
    CORE::CString::StringVector noisePatterns;
};

typedef GUCEF::vector< CAssetPackProfile > TAssetPackProfileVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

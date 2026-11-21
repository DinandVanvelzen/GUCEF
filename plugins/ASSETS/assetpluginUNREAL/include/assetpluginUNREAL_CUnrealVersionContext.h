/*
 *  assetpluginUNREAL: The file versions and custom versions of the package being read
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSION_H
#include "assetpluginUNREAL_CUnrealCustomVersion.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSION_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSION_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

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
 *  What a package says about itself: legacy, UE4, UE5 and licensee file versions and the custom version container. Every
 *  reader receives the context of the package it reads; there is no global version. The context also decides whether the
 *  package may be read at all (section 3.1 of the design): a version that is newer than the newest supported is refused
 *  with the value found and the supported maximum.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealVersionContext
{
    public:

    typedef GUCEF::vector< CUnrealCustomVersion > TCustomVersionVector;

    CUnrealVersionContext( void );

    void Clear( void );

    void SetFileVersions( Int32 legacyFileVersion, Int32 legacyUe3Version, Int32 ue4Version, Int32 ue5Version, Int32 licenseeVersion );

    Int32 GetLegacyFileVersion( void ) const;

    Int32 GetLegacyUe3Version( void ) const;

    Int32 GetUe4Version( void ) const;

    Int32 GetUe5Version( void ) const;

    Int32 GetLicenseeVersion( void ) const;

    bool IsAtLeast( EUnrealUe4Version version ) const;

    bool IsAtLeast( EUnrealUe5Version version ) const;

    /** Adds a custom version, or replaces the value of one that is present */
    void SetCustomVersion( const CUnrealGuid& key, Int32 version );

    /** The value of a custom version, or -1 when the package does not state it */
    Int32 GetCustomVersion( const CUnrealGuid& key ) const;

    bool HasCustomVersion( const CUnrealGuid& key ) const;

    const TCustomVersionVector& GetCustomVersions( void ) const;

    /** Whether the package may be read; when not, text names the value found and what is supported */
    bool CheckSupported( CORE::CString& text ) const;

    /** The name of the engine release a pair of file versions belongs to, "4.14" to "5.8", or an empty string */
    static CORE::CString GuessEngineRelease( Int32 ue4Version, Int32 ue5Version );

    private:

    Int32 m_legacyFileVersion;
    Int32 m_legacyUe3Version;
    Int32 m_ue4Version;
    Int32 m_ue5Version;
    Int32 m_licenseeVersion;
    TCustomVersionVector m_customVersions;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

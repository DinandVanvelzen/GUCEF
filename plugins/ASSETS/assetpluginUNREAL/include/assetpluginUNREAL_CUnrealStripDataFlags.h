/*
 *  assetpluginUNREAL: The two strip flag bytes that start most native object data
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

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
 *  The strip data flags: one byte of global flags and one of class specific flags. Editor packages saved by the engine
 *  strip nothing, so both bytes are zero there; the flags tell which parts follow in cooked packages.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealStripDataFlags
{
    public:

    CUnrealStripDataFlags( void );

    bool Read( CUnrealArchiveReader& reader );

    UInt8 GetGlobalFlags( void ) const;

    UInt8 GetClassFlags( void ) const;

    /** Data only the editor uses (source data, editor only vertex data) is not there */
    bool IsEditorDataStripped( void ) const;

    /** Data a dedicated server does not need (render data) is not there */
    bool IsDataStrippedForServer( void ) const;

    private:

    UInt8 m_globalFlags;
    UInt8 m_classFlags;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H ? */

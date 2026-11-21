/*
 *  assetpluginUNREAL: One property tag in either layout
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H
#include "assetpluginUNREAL_CUnrealPropertyTypeName.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#include "assetpluginUNREAL_CUnrealGuid.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H ? */

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
 *  What precedes the value of a property: its name, type, size, array index and the flags. Both layouts are read into the
 *  same members: the classic layout (type name, size, array index, then the type specific fields and the property GUID)
 *  and the complete type name layout of UE5 file version 1012 and later (type tree, size, flags byte, optional fields).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPropertyTag
{
    public:

    CORE::CString name;
    CUnrealPropertyTypeName type;
    Int32 size;                     /**< bytes of the value that follow the tag */
    Int32 arrayIndex;
    UInt8 flags;                    /**< the flags byte of the complete type name layout, synthesised for the classic layout */
    bool hasPropertyGuid;
    CUnrealGuid propertyGuid;
    bool boolValue;                 /**< value of a boolean property: in the tag, not after it */
    bool hasBinaryOrNativeSerialize;
    bool skippedSerialize;
    CORE::CString structName;       /**< StructProperty */
    CUnrealGuid structGuid;         /**< classic layout only */
    CORE::CString enumName;         /**< ByteProperty and EnumProperty with an enum, empty otherwise */
    UInt8 extension;
    bool isClassic;

    CUnrealPropertyTag( void );

    void Clear( void );

    /** Reads one tag; isNone is set for the "None" terminator, which has no other fields */
    bool Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealReaderLimits& limits, bool& isNone );

    private:

    bool ReadExtensions( CUnrealArchiveReader& reader );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H ? */

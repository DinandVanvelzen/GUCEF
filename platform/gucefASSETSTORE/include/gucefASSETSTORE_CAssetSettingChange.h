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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H
#define GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H

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
 *  One setting that a Settings/Set or Settings/Reset changed (gucefASSETSTORE_RemoteService_Design.md 18.2): the key, the value before
 *  and after, and whether the change applied at once, applies at the next start, or changed nothing. The values of a setting that is
 *  owner-only-read are left out of what goes to every client (valuesShown false).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingChange : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString AppliedLive;         /**< "live": in effect now */
    static const CORE::CString AppliedRestart;      /**< "restart": kept, in effect after the next start */
    static const CORE::CString AppliedUnchanged;    /**< "unchanged": the value was the value already */

    CORE::CString key;
    CORE::CString oldValue;                         /**< the value in effect before, or the one a restart would have applied */
    CORE::CString newValue;
    CORE::CString applied;                          /**< AppliedLive, AppliedRestart or AppliedUnchanged */
    bool valuesShown;                               /**< false: the values are left out (a setting that is owner-only-read) */

    CAssetSettingChange( void );
    virtual ~CAssetSettingChange() GUCEF_VIRTUAL_OVERRIDE;

    /** Leaves the values out */
    void HideValues( void );

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

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H ? */

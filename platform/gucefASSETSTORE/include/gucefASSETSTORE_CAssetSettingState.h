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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H
#include "gucefASSETSTORE_CAssetSettingDescriptor.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H ? */

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
 *  One setting as the service describes it to a caller (Settings/Describe, gucefASSETSTORE_RemoteService_Design.md 18.2): its
 *  descriptor, the value in effect, the value a restart will apply when that differs (a change of a setting that applies at
 *  the next start), the value of the site settings file and where the value comes from.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingState : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString SourceSite;          /**< "site": the value of the site settings file, which differs from the default */
    static const CORE::CString SourceOverridden;    /**< "overridden": a value set by a client or an administrator, kept over a restart */
    static const CORE::CString SourceDefault;       /**< "default": the site settings give the default */

    CAssetSettingDescriptor descriptor;
    CORE::CString value;                            /**< the value in effect */
    bool hasPendingValue;                           /**< a restart applies pendingValue */
    CORE::CString pendingValue;                     /**< the value the next start applies, when it differs from value */
    CORE::CString siteValue;                        /**< the value of the site settings file (the default when it has none) */
    CORE::CString source;                           /**< SourceSite, SourceOverridden or SourceDefault */

    CAssetSettingState( void );
    virtual ~CAssetSettingState() GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H ? */

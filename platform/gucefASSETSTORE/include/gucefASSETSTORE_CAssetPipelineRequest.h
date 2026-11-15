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

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H

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
 *  An operation of the converter pipeline or of the render path that the asset store service hands to the converter hook
 *  (CIAssetConverterHook::RunPipelineOperation()) or the render hook (CIAssetRenderHook). The service does not know the
 *  types of the converter: what is specific to an operation is in 'arguments'. The operation names are those of
 *  CAssetPipelineOperations.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPipelineRequest : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString operation;                    /**< a name of CAssetPipelineOperations */
    CORE::CString packId;                       /**< a converter pack id */
    CORE::CString assetPath;                    /**< pack relative */
    CORE::CString sourcePath;                   /**< a rooted path "<rootId>:<relative>" */
    CORE::CString storeId;
    CORE::CString itemId;
    CORE::CString storeAssetId;
    CORE::CString::StringVector assets;         /**< globs on pack relative asset paths */
    CORE::CString::StringVector cookTargets;
    CORE::CString::StringVector profiles;
    CORE::CString section;
    bool force;
    bool reimport;
    bool apply;                                 /**< a write is carried out, not previewed */
    CORE::CString changeToken;                  /**< of the preview the write applies */
    CORE::CString pageToken;
    UInt32 maxEntries;                          /**< 0: the default */
    CORE::CDataNode arguments;                  /**< what is specific to the operation */

    CAssetPipelineRequest( void );
    virtual ~CAssetPipelineRequest() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

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
 *  The task data of a job of the asset tools on the job scheduler (section 6.3 of the asset tools design): the kind
 *  (the name of the tool that started it), the command id the service gets, and the request of the service operation
 *  (a CAssetPipelineRequest, a CAssetIngestRequest or an ingest id) as the payload of the job definition. The task type
 *  name is that of the tool set the job belongs to ("GUCEF::ASSETSTORE::AssetToolJob.<tool set name>").
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolJobTaskData : public CORE::CIDataNodeSerializableTaskData
{
    public:

    static const CORE::CString TaskTypePrefix;      /**< "GUCEF::ASSETSTORE::AssetToolJob." */

    CORE::CString kind;
    CORE::CString commandId;
    CORE::CDataNode request;

    explicit CAssetToolJobTaskData( const CORE::CString& taskTypeName = CORE::CString::Empty );

    CAssetToolJobTaskData( const CAssetToolJobTaskData& src );

    virtual ~CAssetToolJobTaskData() GUCEF_VIRTUAL_OVERRIDE;

    void SetTaskTypeName( const CORE::CString& taskTypeName );

    /** The payload of a job definition with these members */
    void ToPayload( CORE::CDataNode& payload ) const;

    virtual const CORE::CString& GetTaskTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetThreadPoolName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetOnlyUseExistingThreadPool( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetTaskCanBeQueued( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetTaskId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_taskTypeName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATA_H ? */

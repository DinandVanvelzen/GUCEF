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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H
#define GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#include "gucefASSETSTORE_CAssetToolRoot.h"
#define GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLROOT_H ? */

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
 *  The settings of the asset tool set (CAssetToolSet, section "AssetToolSet"), read strictly: an unknown member, or a
 *  value of another form or out of its range, refuses the whole section and changes nothing (LoadStrict()).
 *
 *  - name: the name of the tool set in the process; the job scheduler of its jobs is "assettools.<name>"
 *  - roots: the folders tools may name (CAssetToolRoot), for the paths the tool layer itself resolves (store_ingest);
 *    the converter hook of the service has its own roots for the pipeline operations
 *  - exposureFile: the exposure file (a path of the file system, read without the VFS); empty: the closed defaults
 *  - exposureCheckIntervalMs: how often the exposure file is looked at for a change (2000)
 *  - jobStateFile: where the job handles are kept across a restart (file system, owner only); empty: not kept
 *  - confirmWrites: "auto" (a form elicitation confirms an apply when the client can show one), "always" (an apply
 *    needs that confirmation), "never" (the change token is the confirmation)
 *  - result limits: defaultLimit (25), maxLimit (100), maxResultBytes (32768), maxResourceBytes (1048576),
 *    maxUntrustedTextChars (500)
 *  - jobs: defaultWaitMs (20000), maxWaitMs (60000), jobPollIntervalMs (100), maxQueuedPerPrincipal (8),
 *    resultTtlHours (168), requestKeyTtlMs (86400000)
 *  - changeTokenTtlMs (600000): the life of a change token of the previews the tool layer makes itself
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolSetSettings : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString SectionName;         /**< "AssetToolSet" */
    static const CORE::CString ConfirmAuto;         /**< "auto" */
    static const CORE::CString ConfirmAlways;       /**< "always" */
    static const CORE::CString ConfirmNever;        /**< "never" */

    CORE::CString name;
    TAssetToolRootVector roots;
    CORE::CString exposureFile;
    UInt32 exposureCheckIntervalMs;
    CORE::CString jobStateFile;
    CORE::CString confirmWrites;
    UInt32 defaultLimit;
    UInt32 maxLimit;
    UInt32 maxResultBytes;
    UInt32 maxResourceBytes;
    UInt32 maxUntrustedTextChars;
    UInt32 defaultWaitMs;
    UInt32 maxWaitMs;
    UInt32 jobPollIntervalMs;
    UInt32 maxQueuedPerPrincipal;
    UInt32 resultTtlHours;
    UInt64 requestKeyTtlMs;
    UInt32 changeTokenTtlMs;

    CAssetToolSetSettings( void );

    virtual ~CAssetToolSetSettings() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the values are usable together; 'problem' says why not */
    bool IsValid( CORE::CString& problem ) const;

    /**
     *  Reads the section strictly: every member is optional, an unknown member or a value out of range refuses it.
     *  On a refusal nothing changes and 'problem' names the member
     */
    bool LoadStrict( const CORE::CDataNode& node, CORE::CString& problem );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Not strict: takes the members that are there (LoadStrict() for settings files) */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H ? */

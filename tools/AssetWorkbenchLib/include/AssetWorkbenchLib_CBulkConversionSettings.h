/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H
#define ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a bulk conversion does (feedback item F8): the scope (a selection, a folder, a pack, a group of the resource view or a saved
 *  search), the targets (the intermediate glTF with its sidecar, the runtime files of the Ogre cooker, a cook profile) and the
 *  options (convert again whatever is up to date, validate only, cook only from the intermediate, an asset filter, analyse the pack
 *  index too). The settings are what a job template keeps under a name in the preferences, and what turns into the request of the
 *  conversion service: the operation convert_pack, cook_pack or validate_pack (convert_plan for the plan) of the pipeline of the
 *  asset store service, with the asset globs, the cook targets and the options of the request.
 *
 *  Scope: an explicit list of the paths of the assets in the pack ('assets': a selection or the members of a group), or all of a pack,
 *  or the assets below a folder of the pack ('folder' becomes the glob "<folder>/*"); the asset filter, a glob, narrows what the scope
 *  names. A saved search makes the packs of its results the scope (the service resolves it, see CBulkConversionController).
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CBulkConversionSettings : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ScopeSelection;      /**< "selection" */
    static const CORE::CString ScopeFolder;         /**< "folder" */
    static const CORE::CString ScopePack;           /**< "pack" */
    static const CORE::CString ScopeGroup;          /**< "group" */
    static const CORE::CString ScopeSavedSearch;    /**< "savedSearch" */
    static const CORE::CString OgreTargetName;      /**< "gucef-ogre3d": the cooker of the runtime files of Ogre; the store keeps its output below gucef-ogre3d/<file>/<profile> */

    CORE::CString name;                         /**< the name of the template; empty for settings that are not kept */
    CORE::CString scopeKind;                    /**< one of the Scope... names */
    CORE::CString packId;
    CORE::CString folder;                       /**< pack relative, for the scope folder */
    CORE::CString groupId;                      /**< the group of the resource view, for the scope group (the members are in 'assets') */
    CORE::CString savedSearchId;
    CORE::CString::StringVector assets;         /**< pack relative paths of the assets (selection, group) */
    bool intermediate;                          /**< the intermediate glTF and its sidecar (always made by a convert run) */
    bool ogreRuntime;                           /**< the runtime files of Ogre (gucef-ogre3d) */
    CORE::CString cookProfile;                  /**< a cook profile of the pack; empty for all of its profiles */
    bool force;                                 /**< convert again, also what is up to date (reimport) */
    bool validateOnly;
    bool cookOnly;                              /**< cook from the intermediate that exists */
    bool analyse;                               /**< analyse the files for the pack index while converting */
    CORE::CString assetFilter;                  /**< a glob on the pack relative paths that narrows the scope; empty for none */

    CBulkConversionSettings( void );
    virtual ~CBulkConversionSettings() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    /** The operation of the pipeline the settings ask for: validate_pack, cook_pack or convert_pack */
    CORE::CString GetOperation( void ) const;

    /** Whether the settings can be started; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    /**
     *  The request for the pack: the operation (the plan operation when 'forPlan'), the globs of the scope narrowed by the asset filter,
     *  the cook targets and the profile and the flags. 'scopeAssets' is what the scope names in that pack (empty for all of it); false
     *  when the filter leaves nothing of an explicit list
     */
    bool MakeRequest( const CORE::CString& pack, const CORE::CString::StringVector& scopeAssets, bool forPlan, ASSETSTORE::CAssetPipelineRequest& request ) const;

    /** The globs the scope names in its own pack: the selection or group members, the folder as a glob, or none for the whole pack */
    void GetScopeGlobs( CORE::CString::StringVector& globs ) const;

    /** The scope and the targets in words, for the job name and the confirmation */
    CORE::CString Describe( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CBulkConversionSettings > TBulkConversionSettingsVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONSETTINGS_H ? */

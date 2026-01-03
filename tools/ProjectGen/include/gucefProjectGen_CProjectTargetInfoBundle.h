/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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
 
#ifndef GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H
#define GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <map>
#include <vector>

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CVERSIONRANGE_H
#include "gucefCORE_CVersionRange.h"
#define GUCEF_CORE_CVERSIONRANGE_H
#endif /* GUCEF_CORE_CVERSIONRANGE_H ? */

#ifndef GUCEF_PROJECTGEN_MACROS_H
#include "gucefProjectGen_macros.h"
#define GUCEF_PROJECTGEN_MACROS_H
#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H
#include "gucefProjectGen_CProjectTargetInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTTARGETINFO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class which holds a bundle of information related to all denoted 'targets' in the overall source project
 *  
 */
class GUCEF_PROJECTGEN_PUBLIC_CPP CProjectTargetInfoBundle : public CORE::CIDataNodeSerializable ,
                                                             public CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef GUCEF::map< CORE::CString, CProjectTargetInfoPtr >      TProjectTargetInfoPtrMap;     // maps a given target platform name, for example 'win32' to everything linked/needed for a given auto-generated target
    typedef GUCEF::map< CORE::CString, TProjectTargetInfoPtrMap >   TProjectTargetInfoPtrMapMap;  // maps a auto-generated target project name to another map which maps on a per target platform basis
    typedef typename CORE::CTSharedObjCreator< CProjectTargetInfoBundle, MT::CMutex >::TBasicSharedPtrType    CProjectTargetInfoBundlePtr;

    /**
     *  Attempts to retrieve the project target for with the given name for the given platform
     */
    CProjectTargetInfoPtr
    GetPlatformProjectTarget( const CORE::CString& targetName   ,
                              const CORE::CString& platformName ) const;

    CProjectTargetInfoPtr
    GetOrCreatePlatformProjectTarget( const CORE::CString& targetName   ,
                                      const CORE::CString& platformName );

    TProjectTargetInfoPtrMap&
    GetOrCreateTargetEntry( const CORE::CString& targetName );

    const TProjectTargetInfoPtrMapMap& GetAllTargets( void ) const;

    const CORE::CString*
    GetTargetMainModuleName( const CORE::CString& targetName            ,
                             const CORE::CString& targetPlatform        ,
                             CModuleInfoPtr* outModuleInfo = GUCEF_NULL ) const;

    void CollapseRedundantPlatformTargets( void );

    void IdentifyRedundantPlatformTargets( CORE::CStringMapMapSet& redundantPlatformsPerProjectTarget ) const;

    static void IdentifyRedundantPlatformTargetsForPlatform( const TProjectTargetInfoPtrMap& targetByPlatform   ,
                                                             const CORE::CString& platformName                  ,
                                                             CORE::CStringMapSet& redundantPlatformsPerPlatform );

    void Clear( void );

    CProjectTargetInfoBundle( void );

    CProjectTargetInfoBundle( const CProjectTargetInfoBundle& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CProjectTargetInfoBundle( CProjectTargetInfoBundle&& src ) GUCEF_NOEXCEPT;
    #endif

    virtual ~CProjectTargetInfoBundle() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Across multiple platforms targets can have multiple names                                                                                                                                                                                                 
     *  This allows for coming up with a consensus name across the various platforms
     *  for a given target.
     *  Note that this considers module linker target names and module names
     *
     *  Not all target origins cause a main module to be defined.
     *  As such its perfectly possible be unable to define a consensus target name because the concept does not apply
     *  to the collection of targets due to the origins of the collection
     */ 
    static CORE::CString GetConsensusTargetName( const TProjectTargetInfoPtrMap& targetPlatforms                    ,
                                                 const CORE::CString& targetPlatform = KnownPlatforms::AllPlatforms );

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     */
    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to serialize the object to a DOM created out of DataNode objects
     *
     *  @param domRootNode Node that acts as root of the DOM data tree from which to deserialize
     *  @return whether deserializing the object data from the given DOM was successful.
     */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SyncObjectsToNames( void );

    void SyncNamesToObjects( const CProjectInfo& projectInfo );

    private:

    TProjectTargetInfoPtrMapMap m_projects;
};

typedef CProjectTargetInfoBundle::CProjectTargetInfoBundlePtr CProjectTargetInfoBundlePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CPROJECTTARGETINFOBUNDLE_H ? */

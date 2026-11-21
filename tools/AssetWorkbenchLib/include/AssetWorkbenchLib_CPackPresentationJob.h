/*
 *  AssetWorkbenchLib: the work of showing the presentation of a pack in the background: convert or prepare on a pool thread, show in slices on the thread of the renderer
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

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATIONJOB_H
#define ASSETWORKBENCHLIB_CPACKPRESENTATIONJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#include "AssetWorkbenchLib_CIAssetLoadJob.h"
#define ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#endif /* ASSETWORKBENCHLIB_CIASSETLOADJOB_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKPRESENTATION_H
#include "AssetWorkbenchLib_CPackPresentation.h"
#define ASSETWORKBENCHLIB_CPACKPRESENTATION_H
#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATION_H ? */

#ifndef ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H
#include "AssetWorkbenchLib_CPackAssetConverter.h"
#define ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H
#endif /* ASSETWORKBENCHLIB_CPACKASSETCONVERTER_H ? */

#ifndef ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#include "AssetWorkbenchLib_CCookedOutputResolver.h"
#define ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H
#endif /* ASSETWORKBENCHLIB_CCOOKEDOUTPUTRESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CMODELVIEW_H
#include "AssetWorkbenchLib_CModelView.h"
#define ASSETWORKBENCHLIB_CMODELVIEW_H
#endif /* ASSETWORKBENCHLIB_CMODELVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

class CWorkbenchShell;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Carries out a CPackPresentation as a request of the CAssetLoader, so that it runs in the background with the activity indicator
 *  after 300 ms, can be cancelled, and is dropped when another request came meanwhile.
 *
 *  Prepare() runs on a thread of a pool: the conversion of an asset of an engine pack (CPackAssetConverter), and the preparation of the
 *  files of the models (CModelView::PrepareMesh()). Step() runs on the thread of the renderer: one model of the showcase per step (so
 *  the slice of 8 ms of the loader is kept), then the camera, the light and the panels. The shell is a friend: the job uses the same
 *  pieces of the shell that opening one asset uses.
 *
 *  The cooked Ogre output (gucef-ogre3d, CCookedOutputResolver) comes first when the preference says so: the constructor looks for it
 *  (the layout of a store for the assets of a pack, or the cache folder of an earlier conversion of the workbench for the asset of a
 *  converter), Prepare() compares it with the source, and a current output is loaded from its files instead of converting the pack to glTF
 *  or reading the originals through the Assimp codec. Without one the quick look is shown and the presentation says what the cooked output
 *  is. Convert for Ogre (the shell) makes the job convert and cook the asset into the cache folder and show the result.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackPresentationJob : public CIAssetLoadJob
{
    public:

    /** 'packSource' is the place of the pack on the disk (the archive file or the folder); 'cacheFolder' is where conversions go; both may be empty */
    CPackPresentationJob( CWorkbenchShell& shell, const CPackPresentation& presentation, const CORE::CString& packSource, const CORE::CString& cacheFolder );
    virtual ~CPackPresentationJob() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReportsProgress( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Prepare( CAssetLoadControl& control ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EStepResult Step( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Discard( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetProblem( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The presentation as it is after the work: the hints of the character and the reasons of fallbacks are in it */
    const CPackPresentation& GetPresentation( void ) const;

    private:

    struct SModel
    {
        CORE::CString path;             /**< the path in the pack */
        CORE::CString vfsPath;
        CORE::CString group;
        CORE::CString relative;
        CModelView::SPreparedMesh prepared;
        bool ok;
        CCookedOutputResolver::SCookedOutput cooked;
        bool probed;                    /**< the cooked output of the model was found (current or not: Prepare() compares it with the source) */
        bool useCooked;
        CORE::CString cookedGroup;
        CORE::CString cookedRelative;
        SModel( void );
    };
    typedef GUCEF::vector< SModel > TModelVector;

    bool PrepareConverted( CAssetLoadControl& control );
    bool PrepareCookedPack( CAssetLoadControl& control );
    void ProbeCookedPack( const CORE::CString& packSource, const CORE::CString& cacheFolder );
    void ProbeCookedModels( void );
    EStepResult StepCookedScene( void );
    void NoteCooked( const CCookedOutputResolver::SCookedOutput& cooked );
    void NoteCookedModels( UInt32 cooked, UInt32 prepared );
    bool PrepareModelFiles( SModel& model, CAssetLoadControl& control, bool& fromCooked );
    void ReleaseCookMount( void );
    void TakeOverShownConversion( const CPackAssetConverter::SRequest& own );
    bool PrepareGrid( CAssetLoadControl& control );
    EStepResult StepConvertedModel( void );
    EStepResult StepConvertedScene( void );
    EStepResult StepGrid( void );
    EStepResult StepSingle( void );
    EStepResult Done( void );
    EStepResult Failed( const CORE::CString& problem );
    static CORE::CString MakeId( const CORE::CString& text );
    void ReleaseOwnMount( void );

    CPackPresentationJob( const CPackPresentationJob& src );
    CPackPresentationJob& operator=( const CPackPresentationJob& src );

    private:

    CWorkbenchShell& m_shell;
    CPackPresentation m_presentation;
    CORE::CString m_cacheFolder;
    CPackAssetConverter::SRequest m_request;
    CPackAssetConverter::SResult m_result;
    bool m_converts;
    TModelVector m_models;
    CORE::CString m_mainVfs;            /**< the glTF or the cooked scene of a conversion */
    CORE::CString m_mainGroup;
    CORE::CString m_mainRelative;
    CModelView::SPreparedMesh m_mainPrepared;
    UInt32 m_step;
    UInt32 m_attached;
    bool m_kept;
    CORE::CString m_problem;
    bool m_prefersCooked;               /**< the preference: look for cooked output */
    bool m_forceCook;                   /**< Convert for Ogre: the asset is converted and cooked, and the cooked scene is shown */
    CPackAssetConverter::SRequest m_cookRequest;
    bool m_cookMounted;                 /**< the cache folder of the cooked conversion is mounted (and adopted by the mounter) */
    bool m_cookKept;
    CCookedOutputResolver::SCookedOutput m_cooked;      /**< the cooked output of the asset of a converter */
    bool m_cookedProbed;
    bool m_useCooked;                   /**< the cooked output was found current: nothing is converted */
    bool m_cookedResult;                /**< the conversion of this job cooked for Ogre: the scene of it is shown */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKPRESENTATIONJOB_H ? */

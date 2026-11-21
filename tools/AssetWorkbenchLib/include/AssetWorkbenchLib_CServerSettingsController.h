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

#ifndef ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H
#define ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */
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
 *  The run-time settings of the asset store service a workbench is connected to (gucefASSETSTORE_RemoteService_Design.md 18.5): the
 *  description of the settings (Settings/Describe: the values, where they come from, how a change applies, whether this device may
 *  change them), the sync profiles of the online stores (Sync/GetProfile, Sync/SetProfile: the existing operations of the library
 *  sync), and the changes the person makes (Settings/Set, Settings/Reset, Sync/SetProfile). Every call is asynchronous: the answers are
 *  taken by Update() between two frames, so the interface never waits for the network. A change of the settings by any client
 *  (SettingsChangedEvent of the service) reads the description again.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CServerSettingsController : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CServerSettingsController > TEventCallback;

    static const CORE::CEvent SettingsReadEvent;        /**< no data: the description or a sync profile was read again */
    static const CORE::CEvent OutcomeEvent;             /**< TCloneableString: the outcome of a change, for people */

    static void RegisterEvents( void );

    /** The sync profile of an online store of the service */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SStoreProfile
    {
        CORE::CString storeId;
        CORE::CString displayName;
        bool known;                                     /**< the profile was read */
        ASSETSTORE::CAssetLibrarySyncProfile profile;

        SStoreProfile( void );
    };
    typedef GUCEF::vector< SStoreProfile > TStoreProfileVector;

    explicit CServerSettingsController( ASSETSTORE::CIAssetStoreService& service );

    virtual ~CServerSettingsController() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the description of the settings and the sync profiles of the stores again; false when it cannot be asked */
    bool Refresh( void );

    /** Asks the service to change a setting; the outcome comes with OutcomeEvent. False with GetProblem() when it cannot be asked */
    bool SetValue( const CORE::CString& key, const CORE::CString& value );

    /** Asks the service to put a setting back to the value of its site settings */
    bool ResetValue( const CORE::CString& key );

    /** Changes a member of the sync profile of a store (the attribute of CAssetLibrarySyncProfile) and sends the profile */
    bool SetProfileMember( const CORE::CString& storeId, const CORE::CString& member, const CORE::CString& value );

    /** Takes the answers of the service; call it between two frames */
    void Update( void );

    /** Whether answers of the service are awaited */
    bool IsWaiting( void ) const;

    bool HasDescription( void ) const;

    const ASSETSTORE::CAssetSettingsDescription& GetDescription( void ) const;

    const TStoreProfileVector& GetProfiles( void ) const;

    const SStoreProfile* FindProfile( const CORE::CString& storeId ) const;

    /** Why the last request could not be asked or was refused, empty when it was not */
    const CORE::CString& GetProblem( void ) const;

    /** The outcome of the last change, for people */
    const CORE::CString& GetLastOutcome( void ) const;

    /** How often a description was taken (a change by another client makes one more) */
    UInt32 GetDescriptionCount( void ) const;

    /** The outcome of a change for people: what changed and how it applies, or why nothing changed */
    static CORE::CString DescribeOutcome( const ASSETSTORE::CAssetSettingsChangeResult& outcome );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EPendingKind
    {
        PENDING_DESCRIBE = 0    ,
        PENDING_STORES          ,
        PENDING_PROFILE         ,
        PENDING_CHANGE          ,
        PENDING_PROFILE_CHANGE
    };

    struct SPending
    {
        CORE::CFutureResult future;
        EPendingKind kind;
        CORE::CString storeId;

        SPending( void );
    };
    typedef GUCEF::vector< SPending > TPendingVector;

    CServerSettingsController( const CServerSettingsController& src );              /**< not implemented */
    CServerSettingsController& operator=( const CServerSettingsController& src );   /**< not implemented */

    void OnSettingsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool Ask( const CORE::CFutureResult& future, EPendingKind kind, const CORE::CString& storeId );
    bool IsPending( EPendingKind kind ) const;
    void TakeOutcome( const CORE::CString& text );
    CORE::CString NextCommandId( void );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    ASSETSTORE::CAssetSettingsDescription m_description;
    bool m_hasDescription;
    TStoreProfileVector m_profiles;
    TPendingVector m_pending;
    CORE::CString m_problem;
    CORE::CString m_lastOutcome;
    UInt32 m_descriptionCount;
    UInt32 m_commandCounter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H ? */

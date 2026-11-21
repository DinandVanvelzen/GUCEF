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

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#define GUCEF_ASSETSTORE_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_WEB_ETYPES_H
#include "gucefWEB_ETypes.h"
#define GUCEF_WEB_ETYPES_H
#endif /* GUCEF_WEB_ETYPES_H ? */
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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcome of an operation of an asset store, a download or the primary store. Values are never renumbered,
 *  new ones are only added. The names of AssetStoreStatusToString() are what persisted data and logs hold.
 */
enum EAssetStoreStatus
{
    ASSETSTORE_STATUS_OK                    = 0     ,   /**< done */
    ASSETSTORE_STATUS_FAILED                = 10    ,   /**< another failure */
    ASSETSTORE_STATUS_INVALID_ARGUMENT      = 11    ,   /**< a bad request, setting or name */
    ASSETSTORE_STATUS_NOT_FOUND             = 12    ,   /**< an item, file, pack, job or provider that is not there */
    ASSETSTORE_STATUS_NOT_SIGNED_IN         = 13    ,   /**< the store needs a sign in first */
    ASSETSTORE_STATUS_SESSION_EXPIRED       = 14    ,   /**< the store refused the session and renewing it did not help: sign in again */
    ASSETSTORE_STATUS_URL_EXPIRED           = 15    ,   /**< a signed download address is no longer valid */
    ASSETSTORE_STATUS_RATE_LIMITED          = 16    ,   /**< the store asked to slow down (HTTP 429) */
    ASSETSTORE_STATUS_NETWORK_ERROR         = 17    ,   /**< no response: name lookup, connection, time out */
    ASSETSTORE_STATUS_SERVER_ERROR          = 18    ,   /**< the store answered with a 5xx status */
    ASSETSTORE_STATUS_HTTP_ERROR            = 19    ,   /**< the store answered with another status that is not success */
    ASSETSTORE_STATUS_DIGEST_MISMATCH       = 20    ,   /**< a downloaded file does not have the digest the store gave */
    ASSETSTORE_STATUS_SIZE_MISMATCH         = 21    ,   /**< a downloaded file does not have the size the store gave */
    ASSETSTORE_STATUS_UNSAFE_NAME           = 22    ,   /**< a file name that could leave its folder or that a platform reserves */
    ASSETSTORE_STATUS_IO_ERROR              = 23    ,   /**< the storage refused a read or a write */
    ASSETSTORE_STATUS_CANCELLED             = 24    ,   /**< cancelled */
    ASSETSTORE_STATUS_PAUSED                = 25    ,   /**< stopped to be resumed later */
    ASSETSTORE_STATUS_UNSUPPORTED           = 26    ,   /**< the store or the backend cannot do it */
    ASSETSTORE_STATUS_DUPLICATE             = 27    ,   /**< the pack is in the catalogue already */
    ASSETSTORE_STATUS_UNAVAILABLE           = 28    ,   /**< a storage location that is not reachable now */
    ASSETSTORE_STATUS_DATABASE_ERROR        = 29    ,   /**< the catalogue database refused */
    ASSETSTORE_STATUS_ALREADY_EXISTS        = 30    ,   /**< a name or id that is taken */
    ASSETSTORE_STATUS_REFUSED               = 31    ,   /**< a conversion that was refused (the licence or the policy does not allow it): not a failure, the pack stays as original */
    ASSETSTORE_STATUS_TIMED_OUT             = 40    ,   /**< a wait ran out of time, such as a sign in that was not finished in time */
    ASSETSTORE_STATUS_OWNER_ACTION_NEEDED   = 41    ,   /**< only a person can go on (a captcha, a code sent by e-mail, a second factor) and none is there to do it */
    ASSETSTORE_STATUS_BROWSER_UNAVAILABLE   = 42        /**< no browser to sign in with: none was found, or it did not start */
};
typedef enum EAssetStoreStatus TAssetStoreStatus;

/** The states of a download job (section 4.4 of the design) */
enum EAssetDownloadJobState
{
    ASSETDOWNLOAD_STATE_QUEUED = 0  ,
    ASSETDOWNLOAD_STATE_PLANNING    ,
    ASSETDOWNLOAD_STATE_RUNNING     ,
    ASSETDOWNLOAD_STATE_PAUSED      ,
    ASSETDOWNLOAD_STATE_VERIFYING   ,
    ASSETDOWNLOAD_STATE_DONE        ,
    ASSETDOWNLOAD_STATE_FAILED      ,
    ASSETDOWNLOAD_STATE_CANCELLED
};
typedef enum EAssetDownloadJobState TAssetDownloadJobState;

/** Whether the packs of a source enter the primary store by themselves */
enum EAssetIngestMode
{
    ASSETINGEST_MODE_MANUAL = 0     ,   /**< a pack is offered with "add to primary store"; nothing enters on its own */
    ASSETINGEST_MODE_AUTOMATIC          /**< every completed download of the source is ingested */
};
typedef enum EAssetIngestMode TAssetIngestMode;

/** What the primary store keeps of a pack */
enum EAssetKeepPolicy
{
    ASSETKEEP_ORIGINAL = 0  ,
    ASSETKEEP_CONVERTED     ,
    ASSETKEEP_BOTH              /**< the default */
};
typedef enum EAssetKeepPolicy TAssetKeepPolicy;

/** What happens to a pack that is in the catalogue already */
enum EAssetDuplicatePolicy
{
    ASSETDUPLICATE_SKIP = 0 ,   /**< reported and not ingested again, the default */
    ASSETDUPLICATE_KEEP         /**< ingested as a pack of its own; equal files share their digest */
};
typedef enum EAssetDuplicatePolicy TAssetDuplicatePolicy;

/** The steps of the ingest pipeline in their order (section 8 of the design) */
enum EAssetIngestStep
{
    ASSETINGEST_STEP_NONE = 0   ,
    ASSETINGEST_STEP_STAGE      ,
    ASSETINGEST_STEP_IDENTIFY   ,
    ASSETINGEST_STEP_NORMALISE  ,
    ASSETINGEST_STEP_ANALYSE    ,
    ASSETINGEST_STEP_CONVERT    ,
    ASSETINGEST_STEP_PLACE      ,
    ASSETINGEST_STEP_INDEX      ,
    ASSETINGEST_STEP_RECORD     ,
    ASSETINGEST_STEP_DONE
};
typedef enum EAssetIngestStep TAssetIngestStep;

/** The state of a pack in the catalogue */
enum EAssetPackState
{
    ASSETPACK_STATE_STAGED = 0          ,   /**< in the staging area, the ingest has not finished */
    ASSETPACK_STATE_INDEXING            ,   /**< its rows are being written; a pack left in this state is repaired at the next open */
    ASSETPACK_STATE_INGESTED            ,
    ASSETPACK_STATE_CONVERSION_PENDING  ,   /**< ingested, the conversion that the keep policy asks for has not been done */
    ASSETPACK_STATE_FAILED              ,   /**< an ingest step failed: the pack stays staged with the reason */
    ASSETPACK_STATE_CONVERSION_REFUSED      /**< ingested, the converter refused to convert it (its licence record does not allow it): kept as original, the reason is in the state reason */
};
typedef enum EAssetPackState TAssetPackState;

/** How a store signs in: not at all, through a provider of gucefAUTH, or through its web site (section 12.1 of the design) */
enum EAssetStoreSignInKind
{
    ASSETSTORE_SIGNIN_NONE = 0          ,
    ASSETSTORE_SIGNIN_AUTH_PROVIDER     ,   /**< GetAuthProviderName() names the provider of gucefAUTH */
    ASSETSTORE_SIGNIN_WEB_SESSION           /**< a browser signs in on the web site, CWebSessionSignIn keeps the cookies */
};
typedef enum EAssetStoreSignInKind TAssetStoreSignInKind;

/** Where the sign in of a store stands, as the service lists it (the remote design, section 5.7) */
enum EAssetStoreSignInState
{
    ASSETSTORE_SIGNINSTATE_SIGNED_OUT = 0   ,   /**< no session: never signed in, signed out, or a sign in that cannot be done here (the message says why) */
    ASSETSTORE_SIGNINSTATE_IN_PROGRESS      ,   /**< a sign in runs and waits for the person or for the store */
    ASSETSTORE_SIGNINSTATE_SIGNED_IN        ,   /**< a session that works */
    ASSETSTORE_SIGNINSTATE_EXPIRED          ,   /**< the session ended and has to be made again */
    ASSETSTORE_SIGNINSTATE_FAILED               /**< the last sign in ended without a session (the message says why) */
};
typedef enum EAssetStoreSignInState TAssetStoreSignInState;

/*
 *  Deprecated: the mode of a web session sign in and the state of a page element are types of gucefWEB
 *  (gucefWEB_ETypes.h). These names in GUCEF::ASSETSTORE are kept for one release so old code still compiles;
 *  use the GUCEF::WEB names.
 */
using WEB::EWebSignInMode;
using WEB::TWebSignInMode;
using WEB::WEBSIGNIN_MODE_INTERACTIVE;
using WEB::WEBSIGNIN_MODE_UNATTENDED;
using WEB::EWebElementState;
using WEB::TWebElementState;
using WEB::WEBELEMENT_STATE_NO_ANSWER;
using WEB::WEBELEMENT_STATE_INVALID_SELECTOR;
using WEB::WEBELEMENT_STATE_ABSENT;
using WEB::WEBELEMENT_STATE_PRESENT;
using WEB::WEBELEMENT_STATE_VISIBLE;

/** The states of a job of the asset store service: a download, an ingest, a conversion, a pipeline run, ... (section 4.3 of the service design) */
enum EAssetJobState
{
    ASSETJOB_STATE_QUEUED = 0   ,   /**< waiting for its turn */
    ASSETJOB_STATE_RUNNING      ,
    ASSETJOB_STATE_PAUSED       ,
    ASSETJOB_STATE_FINISHING    ,   /**< the data is there, it is checked or processed (verifying, indexing) */
    ASSETJOB_STATE_DONE         ,
    ASSETJOB_STATE_FAILED       ,
    ASSETJOB_STATE_CANCELLED
};
typedef enum EAssetJobState TAssetJobState;

/** What can be asked of a job of the asset store service */
enum EAssetJobAction
{
    ASSETJOB_ACTION_PAUSE = 0   ,
    ASSETJOB_ACTION_RESUME      ,
    ASSETJOB_ACTION_CANCEL      ,
    ASSETJOB_ACTION_RETRY       ,
    ASSETJOB_ACTION_REMOVE          /**< takes a job that ended out of the list */
};
typedef enum EAssetJobAction TAssetJobAction;

/** What changed in the catalogue, an entry of the change journal of the asset store service */
enum EAssetCatalogueChangeKind
{
    ASSETCHANGE_PACK_ADDED = 0  ,
    ASSETCHANGE_PACK_UPDATED    ,
    ASSETCHANGE_PACK_REMOVED    ,
    ASSETCHANGE_VOCABULARY      ,
    ASSETCHANGE_SAVED_SEARCHES  ,
    ASSETCHANGE_RESYNC          ,   /**< the changes since a revision are no longer known: everything that is cached is to be read again */
    ASSETCHANGE_PACK_INDEX_UPDATED  /**< the pack index of a pack (its groups and resources) was loaded or changed: a view of the resources reads it again */
};
typedef enum EAssetCatalogueChangeKind TAssetCatalogueChangeKind;

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** "ok", "failed", "invalidArgument", ... */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetStoreStatusToString( TAssetStoreStatus status );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetStoreStatus AssetStoreStatusFromString( const CORE::CString& text );

/** "queued", "planning", "running", "paused", "verifying", "done", "failed", "cancelled" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetDownloadJobStateToString( TAssetDownloadJobState state );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetDownloadJobState AssetDownloadJobStateFromString( const CORE::CString& text );

/** True for done, failed and cancelled */
GUCEF_ASSETSTORE_PUBLIC_CPP bool AssetDownloadJobStateIsFinal( TAssetDownloadJobState state );

/** "manual", "automatic" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetIngestModeToString( TAssetIngestMode mode );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetIngestMode AssetIngestModeFromString( const CORE::CString& text, TAssetIngestMode defaultMode = ASSETINGEST_MODE_MANUAL );

/** "original", "converted", "both" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetKeepPolicyToString( TAssetKeepPolicy keep );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetKeepPolicy AssetKeepPolicyFromString( const CORE::CString& text, TAssetKeepPolicy defaultKeep = ASSETKEEP_BOTH );

/** "skip", "keep" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetDuplicatePolicyToString( TAssetDuplicatePolicy policy );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetDuplicatePolicy AssetDuplicatePolicyFromString( const CORE::CString& text, TAssetDuplicatePolicy defaultPolicy = ASSETDUPLICATE_SKIP );

/** "none", "stage", "identify", "normalise", "analyse", "convert", "place", "index", "record", "done" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetIngestStepToString( TAssetIngestStep step );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetIngestStep AssetIngestStepFromString( const CORE::CString& text );

/** "staged", "indexing", "ingested", "conversionPending", "failed", "conversionRefused" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetPackStateToString( TAssetPackState state );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetPackState AssetPackStateFromString( const CORE::CString& text );

/** True for the states of a pack that is in the catalogue as a usable pack: ingested, conversion pending and conversion refused */
GUCEF_ASSETSTORE_PUBLIC_CPP bool AssetPackStateIsIngested( TAssetPackState state );
GUCEF_ASSETSTORE_PUBLIC_CPP bool AssetPackStateIsIngested( const CORE::CString& stateText );

/** "none", "authProvider", "webSession" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetStoreSignInKindToString( TAssetStoreSignInKind kind );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetStoreSignInKind AssetStoreSignInKindFromString( const CORE::CString& text );

/** "signedOut", "inProgress", "signedIn", "expired", "failed" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetStoreSignInStateToString( TAssetStoreSignInState state );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetStoreSignInState AssetStoreSignInStateFromString( const CORE::CString& text );

/* Deprecated, kept for one release: the texts of the mode of a web session sign in are those of gucefWEB */
using WEB::WebSignInModeToString;
using WEB::WebSignInModeFromString;

/**
 *  The outcome of a web session sign in (WEB::TWebSignInStatus) as the outcome of an operation of a store: the same
 *  value and name. The other way around a status that is no outcome of a sign in becomes WEB::WEBSIGNIN_STATUS_FAILED.
 */
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetStoreStatus AssetStoreStatusFromWebSignInStatus( WEB::TWebSignInStatus status );
GUCEF_ASSETSTORE_PUBLIC_CPP WEB::TWebSignInStatus WebSignInStatusFromAssetStoreStatus( TAssetStoreStatus status );

/** "queued", "running", "paused", "finishing", "done", "failed", "cancelled" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetJobStateToString( TAssetJobState state );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetJobState AssetJobStateFromString( const CORE::CString& text );

/** True for done, failed and cancelled */
GUCEF_ASSETSTORE_PUBLIC_CPP bool AssetJobStateIsFinal( TAssetJobState state );

/** "pause", "resume", "cancel", "retry", "remove" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetJobActionToString( TAssetJobAction action );

/** False for a text that names no action */
GUCEF_ASSETSTORE_PUBLIC_CPP bool AssetJobActionFromString( const CORE::CString& text, TAssetJobAction& action );

/** "packAdded", "packUpdated", "packRemoved", "vocabulary", "savedSearches", "resync" */
GUCEF_ASSETSTORE_PUBLIC_CPP const char* AssetCatalogueChangeKindToString( TAssetCatalogueChangeKind kind );
GUCEF_ASSETSTORE_PUBLIC_CPP TAssetCatalogueChangeKind AssetCatalogueChangeKindFromString( const CORE::CString& text );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

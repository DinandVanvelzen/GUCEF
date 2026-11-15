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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefASSETSTORE_ETypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SNamedValue
{
    int value;
    const char* name;
};

const SNamedValue g_statusNames[] =
{
    { ASSETSTORE_STATUS_OK, "ok" },
    { ASSETSTORE_STATUS_FAILED, "failed" },
    { ASSETSTORE_STATUS_INVALID_ARGUMENT, "invalidArgument" },
    { ASSETSTORE_STATUS_NOT_FOUND, "notFound" },
    { ASSETSTORE_STATUS_NOT_SIGNED_IN, "notSignedIn" },
    { ASSETSTORE_STATUS_SESSION_EXPIRED, "sessionExpired" },
    { ASSETSTORE_STATUS_URL_EXPIRED, "urlExpired" },
    { ASSETSTORE_STATUS_RATE_LIMITED, "rateLimited" },
    { ASSETSTORE_STATUS_NETWORK_ERROR, "networkError" },
    { ASSETSTORE_STATUS_SERVER_ERROR, "serverError" },
    { ASSETSTORE_STATUS_HTTP_ERROR, "httpError" },
    { ASSETSTORE_STATUS_DIGEST_MISMATCH, "digestMismatch" },
    { ASSETSTORE_STATUS_SIZE_MISMATCH, "sizeMismatch" },
    { ASSETSTORE_STATUS_UNSAFE_NAME, "unsafeName" },
    { ASSETSTORE_STATUS_IO_ERROR, "ioError" },
    { ASSETSTORE_STATUS_CANCELLED, "cancelled" },
    { ASSETSTORE_STATUS_PAUSED, "paused" },
    { ASSETSTORE_STATUS_UNSUPPORTED, "unsupported" },
    { ASSETSTORE_STATUS_DUPLICATE, "duplicate" },
    { ASSETSTORE_STATUS_UNAVAILABLE, "unavailable" },
    { ASSETSTORE_STATUS_DATABASE_ERROR, "databaseError" },
    { ASSETSTORE_STATUS_ALREADY_EXISTS, "alreadyExists" },
    { ASSETSTORE_STATUS_REFUSED, "refused" },
    { ASSETSTORE_STATUS_TIMED_OUT, "timedOut" },
    { ASSETSTORE_STATUS_OWNER_ACTION_NEEDED, "ownerActionNeeded" },
    { ASSETSTORE_STATUS_BROWSER_UNAVAILABLE, "browserUnavailable" }
};

const SNamedValue g_jobStateNames[] =
{
    { ASSETDOWNLOAD_STATE_QUEUED, "queued" },
    { ASSETDOWNLOAD_STATE_PLANNING, "planning" },
    { ASSETDOWNLOAD_STATE_RUNNING, "running" },
    { ASSETDOWNLOAD_STATE_PAUSED, "paused" },
    { ASSETDOWNLOAD_STATE_VERIFYING, "verifying" },
    { ASSETDOWNLOAD_STATE_DONE, "done" },
    { ASSETDOWNLOAD_STATE_FAILED, "failed" },
    { ASSETDOWNLOAD_STATE_CANCELLED, "cancelled" }
};

const SNamedValue g_modeNames[] =
{
    { ASSETINGEST_MODE_MANUAL, "manual" },
    { ASSETINGEST_MODE_AUTOMATIC, "automatic" }
};

const SNamedValue g_keepNames[] =
{
    { ASSETKEEP_ORIGINAL, "original" },
    { ASSETKEEP_CONVERTED, "converted" },
    { ASSETKEEP_BOTH, "both" }
};

const SNamedValue g_duplicateNames[] =
{
    { ASSETDUPLICATE_SKIP, "skip" },
    { ASSETDUPLICATE_KEEP, "keep" }
};

const SNamedValue g_stepNames[] =
{
    { ASSETINGEST_STEP_NONE, "none" },
    { ASSETINGEST_STEP_STAGE, "stage" },
    { ASSETINGEST_STEP_IDENTIFY, "identify" },
    { ASSETINGEST_STEP_NORMALISE, "normalise" },
    { ASSETINGEST_STEP_ANALYSE, "analyse" },
    { ASSETINGEST_STEP_CONVERT, "convert" },
    { ASSETINGEST_STEP_PLACE, "place" },
    { ASSETINGEST_STEP_INDEX, "index" },
    { ASSETINGEST_STEP_RECORD, "record" },
    { ASSETINGEST_STEP_DONE, "done" }
};

const SNamedValue g_packStateNames[] =
{
    { ASSETPACK_STATE_STAGED, "staged" },
    { ASSETPACK_STATE_INDEXING, "indexing" },
    { ASSETPACK_STATE_INGESTED, "ingested" },
    { ASSETPACK_STATE_CONVERSION_PENDING, "conversionPending" },
    { ASSETPACK_STATE_FAILED, "failed" },
    { ASSETPACK_STATE_CONVERSION_REFUSED, "conversionRefused" }
};

const SNamedValue g_signInKindNames[] =
{
    { ASSETSTORE_SIGNIN_NONE, "none" },
    { ASSETSTORE_SIGNIN_AUTH_PROVIDER, "authProvider" },
    { ASSETSTORE_SIGNIN_WEB_SESSION, "webSession" }
};

const SNamedValue g_webSignInModeNames[] =
{
    { WEBSIGNIN_MODE_INTERACTIVE, "interactive" },
    { WEBSIGNIN_MODE_UNATTENDED, "unattended" }
};

const SNamedValue g_serviceJobStateNames[] =
{
    { ASSETJOB_STATE_QUEUED, "queued" },
    { ASSETJOB_STATE_RUNNING, "running" },
    { ASSETJOB_STATE_PAUSED, "paused" },
    { ASSETJOB_STATE_FINISHING, "finishing" },
    { ASSETJOB_STATE_DONE, "done" },
    { ASSETJOB_STATE_FAILED, "failed" },
    { ASSETJOB_STATE_CANCELLED, "cancelled" }
};

const SNamedValue g_jobActionNames[] =
{
    { ASSETJOB_ACTION_PAUSE, "pause" },
    { ASSETJOB_ACTION_RESUME, "resume" },
    { ASSETJOB_ACTION_CANCEL, "cancel" },
    { ASSETJOB_ACTION_RETRY, "retry" },
    { ASSETJOB_ACTION_REMOVE, "remove" }
};

const SNamedValue g_changeKindNames[] =
{
    { ASSETCHANGE_PACK_ADDED, "packAdded" },
    { ASSETCHANGE_PACK_UPDATED, "packUpdated" },
    { ASSETCHANGE_PACK_REMOVED, "packRemoved" },
    { ASSETCHANGE_VOCABULARY, "vocabulary" },
    { ASSETCHANGE_SAVED_SEARCHES, "savedSearches" },
    { ASSETCHANGE_RESYNC, "resync" }
};

const char*
NameOf( const SNamedValue* table, UInt32 count, int value, const char* unknown )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( table[ i ].value == value )
            return table[ i ].name;
    return unknown;
}

int
ValueOf( const SNamedValue* table, UInt32 count, const CORE::CString& text, int defaultValue )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( text.Equals( table[ i ].name, false ) )
            return table[ i ].value;
    return defaultValue;
}

#define GUCEF_ASSETSTORE_COUNT( table ) ( (UInt32) ( sizeof( table ) / sizeof( table[ 0 ] ) ) )

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
AssetStoreStatusToString( TAssetStoreStatus status )
{GUCEF_TRACE;

    return NameOf( g_statusNames, GUCEF_ASSETSTORE_COUNT( g_statusNames ), status, "failed" );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
AssetStoreStatusFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetStoreStatus) ValueOf( g_statusNames, GUCEF_ASSETSTORE_COUNT( g_statusNames ), text, ASSETSTORE_STATUS_FAILED );
}

/*-------------------------------------------------------------------------*/

const char*
AssetDownloadJobStateToString( TAssetDownloadJobState state )
{GUCEF_TRACE;

    return NameOf( g_jobStateNames, GUCEF_ASSETSTORE_COUNT( g_jobStateNames ), state, "queued" );
}

/*-------------------------------------------------------------------------*/

TAssetDownloadJobState
AssetDownloadJobStateFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetDownloadJobState) ValueOf( g_jobStateNames, GUCEF_ASSETSTORE_COUNT( g_jobStateNames ), text, ASSETDOWNLOAD_STATE_QUEUED );
}

/*-------------------------------------------------------------------------*/

bool
AssetDownloadJobStateIsFinal( TAssetDownloadJobState state )
{GUCEF_TRACE;

    return ASSETDOWNLOAD_STATE_DONE == state || ASSETDOWNLOAD_STATE_FAILED == state || ASSETDOWNLOAD_STATE_CANCELLED == state;
}

/*-------------------------------------------------------------------------*/

const char*
AssetIngestModeToString( TAssetIngestMode mode )
{GUCEF_TRACE;

    return NameOf( g_modeNames, GUCEF_ASSETSTORE_COUNT( g_modeNames ), mode, "manual" );
}

/*-------------------------------------------------------------------------*/

TAssetIngestMode
AssetIngestModeFromString( const CORE::CString& text, TAssetIngestMode defaultMode )
{GUCEF_TRACE;

    return (TAssetIngestMode) ValueOf( g_modeNames, GUCEF_ASSETSTORE_COUNT( g_modeNames ), text, defaultMode );
}

/*-------------------------------------------------------------------------*/

const char*
AssetKeepPolicyToString( TAssetKeepPolicy keep )
{GUCEF_TRACE;

    return NameOf( g_keepNames, GUCEF_ASSETSTORE_COUNT( g_keepNames ), keep, "both" );
}

/*-------------------------------------------------------------------------*/

TAssetKeepPolicy
AssetKeepPolicyFromString( const CORE::CString& text, TAssetKeepPolicy defaultKeep )
{GUCEF_TRACE;

    return (TAssetKeepPolicy) ValueOf( g_keepNames, GUCEF_ASSETSTORE_COUNT( g_keepNames ), text, defaultKeep );
}

/*-------------------------------------------------------------------------*/

const char*
AssetDuplicatePolicyToString( TAssetDuplicatePolicy policy )
{GUCEF_TRACE;

    return NameOf( g_duplicateNames, GUCEF_ASSETSTORE_COUNT( g_duplicateNames ), policy, "skip" );
}

/*-------------------------------------------------------------------------*/

TAssetDuplicatePolicy
AssetDuplicatePolicyFromString( const CORE::CString& text, TAssetDuplicatePolicy defaultPolicy )
{GUCEF_TRACE;

    return (TAssetDuplicatePolicy) ValueOf( g_duplicateNames, GUCEF_ASSETSTORE_COUNT( g_duplicateNames ), text, defaultPolicy );
}

/*-------------------------------------------------------------------------*/

const char*
AssetIngestStepToString( TAssetIngestStep step )
{GUCEF_TRACE;

    return NameOf( g_stepNames, GUCEF_ASSETSTORE_COUNT( g_stepNames ), step, "none" );
}

/*-------------------------------------------------------------------------*/

TAssetIngestStep
AssetIngestStepFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetIngestStep) ValueOf( g_stepNames, GUCEF_ASSETSTORE_COUNT( g_stepNames ), text, ASSETINGEST_STEP_NONE );
}

/*-------------------------------------------------------------------------*/

const char*
AssetPackStateToString( TAssetPackState state )
{GUCEF_TRACE;

    return NameOf( g_packStateNames, GUCEF_ASSETSTORE_COUNT( g_packStateNames ), state, "staged" );
}

/*-------------------------------------------------------------------------*/

TAssetPackState
AssetPackStateFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetPackState) ValueOf( g_packStateNames, GUCEF_ASSETSTORE_COUNT( g_packStateNames ), text, ASSETPACK_STATE_STAGED );
}


/*-------------------------------------------------------------------------*/

bool
AssetPackStateIsIngested( TAssetPackState state )
{GUCEF_TRACE;

    return ASSETPACK_STATE_INGESTED == state || ASSETPACK_STATE_CONVERSION_PENDING == state || ASSETPACK_STATE_CONVERSION_REFUSED == state;
}

/*-------------------------------------------------------------------------*/

bool
AssetPackStateIsIngested( const CORE::CString& stateText )
{GUCEF_TRACE;

    return AssetPackStateIsIngested( AssetPackStateFromString( stateText ) );
}

/*-------------------------------------------------------------------------*/

const char*
AssetStoreSignInKindToString( TAssetStoreSignInKind kind )
{GUCEF_TRACE;

    return NameOf( g_signInKindNames, GUCEF_ASSETSTORE_COUNT( g_signInKindNames ), kind, "none" );
}

/*-------------------------------------------------------------------------*/

TAssetStoreSignInKind
AssetStoreSignInKindFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetStoreSignInKind) ValueOf( g_signInKindNames, GUCEF_ASSETSTORE_COUNT( g_signInKindNames ), text, ASSETSTORE_SIGNIN_NONE );
}

/*-------------------------------------------------------------------------*/

const char*
WebSignInModeToString( TWebSignInMode mode )
{GUCEF_TRACE;

    return NameOf( g_webSignInModeNames, GUCEF_ASSETSTORE_COUNT( g_webSignInModeNames ), mode, "interactive" );
}

/*-------------------------------------------------------------------------*/

TWebSignInMode
WebSignInModeFromString( const CORE::CString& text, TWebSignInMode defaultMode )
{GUCEF_TRACE;

    return (TWebSignInMode) ValueOf( g_webSignInModeNames, GUCEF_ASSETSTORE_COUNT( g_webSignInModeNames ), text, defaultMode );
}

/*-------------------------------------------------------------------------*/

const char*
AssetJobStateToString( TAssetJobState state )
{GUCEF_TRACE;

    return NameOf( g_serviceJobStateNames, GUCEF_ASSETSTORE_COUNT( g_serviceJobStateNames ), state, "queued" );
}

/*-------------------------------------------------------------------------*/

TAssetJobState
AssetJobStateFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetJobState) ValueOf( g_serviceJobStateNames, GUCEF_ASSETSTORE_COUNT( g_serviceJobStateNames ), text, ASSETJOB_STATE_QUEUED );
}

/*-------------------------------------------------------------------------*/

bool
AssetJobStateIsFinal( TAssetJobState state )
{GUCEF_TRACE;

    return ASSETJOB_STATE_DONE == state || ASSETJOB_STATE_FAILED == state || ASSETJOB_STATE_CANCELLED == state;
}

/*-------------------------------------------------------------------------*/

const char*
AssetJobActionToString( TAssetJobAction action )
{GUCEF_TRACE;

    return NameOf( g_jobActionNames, GUCEF_ASSETSTORE_COUNT( g_jobActionNames ), action, "remove" );
}

/*-------------------------------------------------------------------------*/

bool
AssetJobActionFromString( const CORE::CString& text, TAssetJobAction& action )
{GUCEF_TRACE;

    const int value = ValueOf( g_jobActionNames, GUCEF_ASSETSTORE_COUNT( g_jobActionNames ), text, -1 );
    if ( value < 0 )
        return false;
    action = (TAssetJobAction) value;
    return true;
}

/*-------------------------------------------------------------------------*/

const char*
AssetCatalogueChangeKindToString( TAssetCatalogueChangeKind kind )
{GUCEF_TRACE;

    return NameOf( g_changeKindNames, GUCEF_ASSETSTORE_COUNT( g_changeKindNames ), kind, "resync" );
}

/*-------------------------------------------------------------------------*/

TAssetCatalogueChangeKind
AssetCatalogueChangeKindFromString( const CORE::CString& text )
{GUCEF_TRACE;

    return (TAssetCatalogueChangeKind) ValueOf( g_changeKindNames, GUCEF_ASSETSTORE_COUNT( g_changeKindNames ), text, ASSETCHANGE_RESYNC );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

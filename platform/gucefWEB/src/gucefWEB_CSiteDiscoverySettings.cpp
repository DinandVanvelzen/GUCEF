/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefWEB_CSiteDiscoverySettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString::StringVector
ToList( const char* const* items )
{
    CORE::CString::StringVector list;
    for ( UInt32 i=0; GUCEF_NULL != items[ i ]; ++i )
        list.push_back( items[ i ] );
    return list;
}

/*-------------------------------------------------------------------------*/

/** Reads a list when the configuration names it; the list then replaces the current one, also when it is empty */
static void
LoadList( const CORE::CDataNode& cfg, const char* name, CORE::CString::StringVector& list )
{
    CORE::CString::StringVector configured;
    if ( cfg.GetStringListByName( name, configured ) )
        list = configured;
}

/*-------------------------------------------------------------------------*/

static void
SaveList( CORE::CDataNode& cfg, const char* name, const CORE::CString::StringVector& list )
{
    cfg.SetValuesOfChildByName( name, list );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CSiteDiscoverySettings::MinKnownSecretLengthFloor;

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultNeverRecordPathPatterns( void )
{GUCEF_TRACE;

    static const char* const parts[] = { "password", "passwd", "signin", "sign-in", "sign_in", "login", "logon", "credential", "oauth", "/security/",
                                         "step-up", "stepup", "/verify", "authenticat", "/2fa", "/mfa", "/otp", "passcode", "/token", "/account/",
                                         "/secret", "/session", "saml", "/sso", "/authorize", "webauthn", "passkey", "/recover", "/identity/",
                                         "/purchase", "/checkout", "/payment", "/billing", "/paypal", "/card", GUCEF_NULL };
    return ToList( parts );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultNeverRecordHosts( void )
{GUCEF_TRACE;

    static const char* const hosts[] = { "accounts.google.com", "login.microsoftonline.com", "login.live.com", "appleid.apple.com", "*.okta.com",
                                         "*.auth0.com", "login.*", "signin.*", "sso.*", "auth.*", "payment*", "pay.*", "checkout.*", "billing.*", "pci*", GUCEF_NULL };
    return ToList( hosts );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultSecretNamePatterns( void )
{GUCEF_TRACE;

    static const char* const names[] = { "*csrf*", "*xsrf*", "code", "state", "nonce", "ticket", "saml*", "assertion", "*hmac*", "x-amz-*",
                                         "pin", "ssn", "cvv", "cvc", "*accountnumber*", "*account_number*", "*routing*", "*iban*", GUCEF_NULL };
    return ToList( names );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultValueQueryNames( void )
{GUCEF_TRACE;

    static const char* const names[] = { "q", "query", "search", "searchterm", "term", "keyword", "keywords", "text", GUCEF_NULL };
    return ToList( names );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultValuePathSegmentsAfter( void )
{GUCEF_TRACE;

    static const char* const names[] = { "search", "lookup", "autocomplete", "suggest", "typeahead", GUCEF_NULL };
    return ToList( names );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CSiteDiscoverySettings::GetDefaultNonSensitiveValues( void )
{GUCEF_TRACE;

    static const char* const words[] = { "OK", "ID", "PDF", "CSV", "TXT", "XLS", "XLSX", "JSON", "XML", "HTML", "URL", "API", "FAQ", "AM", "PM", "UTC", "GMT", GUCEF_NULL };
    return ToList( words );
}

/*-------------------------------------------------------------------------*/

CSiteDiscoverySettings::CSiteDiscoverySettings( void )
    : CORE::CIConfigurable()
    , hostFilter()
    , allowPathPrefixes()
    , neverRecordPathPatterns( GetDefaultNeverRecordPathPatterns() )
    , neverRecordHosts( GetDefaultNeverRecordHosts() )
    , noteNeverRecorded( true )
    , secretNamePatterns( GetDefaultSecretNamePatterns() )
    , secretValueMode( SECRET_VALUE_HASH )
    , recordExampleValues( false )
    , recordEnumQueryValues( false )
    , knownSecretMinLength( 12 )
    , knownSecretMinEntropyBits( 32 )
    , ignoredSecretFields()
    , tripwireMaxLineShare( 20 )
    , maskIdLikeMemberNames( true )
    , maskTokenLikeParameterNames( true )
    , maxParameterNameLength( 40 )
    , valueQueryNames( GetDefaultValueQueryNames() )
    , enumQueryNames()
    , valuePathSegmentsAfter( GetDefaultValuePathSegmentsAfter() )
    , maskUppercaseTokens( false )
    , uppercaseTokenMaxLength( 5 )
    , nonSensitiveValues( GetDefaultNonSensitiveValues() )
    , sensitiveValues()
    , sensitiveValuePatterns()
    , maskFileNameWords( true )
    , recordResponseShapes( true )
    , saveResponseBodies( false )
    , maxResponseBodyBytes( 4ULL * 1024ULL * 1024ULL )
    , responseBodyContentTypes()
    , outputDirectory()
    , journalFileName( "requests.jsonl" )
    , reportFileName( "endpoints.md" )
    , bodiesDirectoryName( "bodies" )
    , continueJournal( true )
    , disableBrowserCache( false )
    , reportTableCellControlNames( true )
    , captureClicks( false )
    , actionWindowMs( 3000 )
    , actionMaxDurationMs( 15000 )
    , quietGapMs( 1500 )
    , maxActions( 500 )
    , maxShapeDepth( 5 )
    , maxShapeLines( 150 )
    , maxShapesPerEndpoint( 12 )
    , browserRunMode( BROWSER_RUNMODE_ATTACH )
    , browserDebugPort( 9222 )
    , browserUserDataDirs()
    , browserExecutablePath()
    , browserProfileDirectory()
    , browserStartUrl()
    , browserHeadless( false )
    , browserConnectTimeoutMs( 90000 )
    , closeLaunchedBrowser( false )
{GUCEF_TRACE;

    responseBodyContentTypes.push_back( "json" );
    responseBodyContentTypes.push_back( "xml" );
    responseBodyContentTypes.push_back( "text/plain" );
}

/*-------------------------------------------------------------------------*/

CSiteDiscoverySettings::CSiteDiscoverySettings( const CSiteDiscoverySettings& src )
    : CORE::CIConfigurable( src )
    , hostFilter( src.hostFilter )
    , allowPathPrefixes( src.allowPathPrefixes )
    , neverRecordPathPatterns( src.neverRecordPathPatterns )
    , neverRecordHosts( src.neverRecordHosts )
    , noteNeverRecorded( src.noteNeverRecorded )
    , secretNamePatterns( src.secretNamePatterns )
    , secretValueMode( src.secretValueMode )
    , recordExampleValues( src.recordExampleValues )
    , recordEnumQueryValues( src.recordEnumQueryValues )
    , knownSecretMinLength( src.knownSecretMinLength )
    , knownSecretMinEntropyBits( src.knownSecretMinEntropyBits )
    , ignoredSecretFields( src.ignoredSecretFields )
    , tripwireMaxLineShare( src.tripwireMaxLineShare )
    , maskIdLikeMemberNames( src.maskIdLikeMemberNames )
    , maskTokenLikeParameterNames( src.maskTokenLikeParameterNames )
    , maxParameterNameLength( src.maxParameterNameLength )
    , valueQueryNames( src.valueQueryNames )
    , enumQueryNames( src.enumQueryNames )
    , valuePathSegmentsAfter( src.valuePathSegmentsAfter )
    , maskUppercaseTokens( src.maskUppercaseTokens )
    , uppercaseTokenMaxLength( src.uppercaseTokenMaxLength )
    , nonSensitiveValues( src.nonSensitiveValues )
    , sensitiveValues( src.sensitiveValues )
    , sensitiveValuePatterns( src.sensitiveValuePatterns )
    , maskFileNameWords( src.maskFileNameWords )
    , recordResponseShapes( src.recordResponseShapes )
    , saveResponseBodies( src.saveResponseBodies )
    , maxResponseBodyBytes( src.maxResponseBodyBytes )
    , responseBodyContentTypes( src.responseBodyContentTypes )
    , outputDirectory( src.outputDirectory )
    , journalFileName( src.journalFileName )
    , reportFileName( src.reportFileName )
    , bodiesDirectoryName( src.bodiesDirectoryName )
    , continueJournal( src.continueJournal )
    , disableBrowserCache( src.disableBrowserCache )
    , reportTableCellControlNames( src.reportTableCellControlNames )
    , captureClicks( src.captureClicks )
    , actionWindowMs( src.actionWindowMs )
    , actionMaxDurationMs( src.actionMaxDurationMs )
    , quietGapMs( src.quietGapMs )
    , maxActions( src.maxActions )
    , maxShapeDepth( src.maxShapeDepth )
    , maxShapeLines( src.maxShapeLines )
    , maxShapesPerEndpoint( src.maxShapesPerEndpoint )
    , browserRunMode( src.browserRunMode )
    , browserDebugPort( src.browserDebugPort )
    , browserUserDataDirs( src.browserUserDataDirs )
    , browserExecutablePath( src.browserExecutablePath )
    , browserProfileDirectory( src.browserProfileDirectory )
    , browserStartUrl( src.browserStartUrl )
    , browserHeadless( src.browserHeadless )
    , browserConnectTimeoutMs( src.browserConnectTimeoutMs )
    , closeLaunchedBrowser( src.closeLaunchedBrowser )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSiteDiscoverySettings::~CSiteDiscoverySettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSiteDiscoverySettings&
CSiteDiscoverySettings::operator=( const CSiteDiscoverySettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        hostFilter = src.hostFilter;
        allowPathPrefixes = src.allowPathPrefixes;
        neverRecordPathPatterns = src.neverRecordPathPatterns;
        neverRecordHosts = src.neverRecordHosts;
        noteNeverRecorded = src.noteNeverRecorded;
        secretNamePatterns = src.secretNamePatterns;
        secretValueMode = src.secretValueMode;
        recordExampleValues = src.recordExampleValues;
        recordEnumQueryValues = src.recordEnumQueryValues;
        knownSecretMinLength = src.knownSecretMinLength;
        knownSecretMinEntropyBits = src.knownSecretMinEntropyBits;
        ignoredSecretFields = src.ignoredSecretFields;
        tripwireMaxLineShare = src.tripwireMaxLineShare;
        maskIdLikeMemberNames = src.maskIdLikeMemberNames;
        maskTokenLikeParameterNames = src.maskTokenLikeParameterNames;
        maxParameterNameLength = src.maxParameterNameLength;
        valueQueryNames = src.valueQueryNames;
        enumQueryNames = src.enumQueryNames;
        valuePathSegmentsAfter = src.valuePathSegmentsAfter;
        maskUppercaseTokens = src.maskUppercaseTokens;
        uppercaseTokenMaxLength = src.uppercaseTokenMaxLength;
        nonSensitiveValues = src.nonSensitiveValues;
        sensitiveValues = src.sensitiveValues;
        sensitiveValuePatterns = src.sensitiveValuePatterns;
        maskFileNameWords = src.maskFileNameWords;
        recordResponseShapes = src.recordResponseShapes;
        saveResponseBodies = src.saveResponseBodies;
        maxResponseBodyBytes = src.maxResponseBodyBytes;
        responseBodyContentTypes = src.responseBodyContentTypes;
        outputDirectory = src.outputDirectory;
        journalFileName = src.journalFileName;
        reportFileName = src.reportFileName;
        bodiesDirectoryName = src.bodiesDirectoryName;
        continueJournal = src.continueJournal;
        disableBrowserCache = src.disableBrowserCache;
        reportTableCellControlNames = src.reportTableCellControlNames;
        captureClicks = src.captureClicks;
        actionWindowMs = src.actionWindowMs;
        actionMaxDurationMs = src.actionMaxDurationMs;
        quietGapMs = src.quietGapMs;
        maxActions = src.maxActions;
        maxShapeDepth = src.maxShapeDepth;
        maxShapeLines = src.maxShapeLines;
        maxShapesPerEndpoint = src.maxShapesPerEndpoint;
        browserRunMode = src.browserRunMode;
        browserDebugPort = src.browserDebugPort;
        browserUserDataDirs = src.browserUserDataDirs;
        browserExecutablePath = src.browserExecutablePath;
        browserProfileDirectory = src.browserProfileDirectory;
        browserStartUrl = src.browserStartUrl;
        browserHeadless = src.browserHeadless;
        browserConnectTimeoutMs = src.browserConnectTimeoutMs;
        closeLaunchedBrowser = src.closeLaunchedBrowser;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoverySettings::SecretValueModeToString( ESecretValueMode mode )
{GUCEF_TRACE;

    return SECRET_VALUE_STRIP == mode ? "strip" : "hash";
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySettings::SecretValueModeFromString( const CORE::CString& text, ESecretValueMode& mode )
{GUCEF_TRACE;

    const CORE::CString lower = text.Lowercase();
    if ( lower == "hash" )
        mode = SECRET_VALUE_HASH;
    else
    if ( lower == "strip" )
        mode = SECRET_VALUE_STRIP;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoverySettings::BrowserRunModeToString( EBrowserRunMode mode )
{GUCEF_TRACE;

    return BROWSER_RUNMODE_RECORDED_SESSION == mode ? "recordedSession" : "attach";
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySettings::BrowserRunModeFromString( const CORE::CString& text, EBrowserRunMode& mode )
{GUCEF_TRACE;

    const CORE::CString lower = text.Lowercase();
    if ( lower == "attach" )
        mode = BROWSER_RUNMODE_ATTACH;
    else
    if ( lower == "recordedsession" )
        mode = BROWSER_RUNMODE_RECORDED_SESSION;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    SaveList( cfg, "hostFilter", hostFilter );
    SaveList( cfg, "allowPathPrefixes", allowPathPrefixes );
    SaveList( cfg, "neverRecordPathPatterns", neverRecordPathPatterns );
    SaveList( cfg, "neverRecordHosts", neverRecordHosts );
    cfg.SetAttribute( "noteNeverRecorded", noteNeverRecorded );
    SaveList( cfg, "secretNamePatterns", secretNamePatterns );
    cfg.SetAttribute( "secretValueMode", CORE::CVariant( SecretValueModeToString( secretValueMode ) ) );
    cfg.SetAttribute( "recordExampleValues", recordExampleValues );
    cfg.SetAttribute( "recordEnumQueryValues", recordEnumQueryValues );
    cfg.SetAttribute( "knownSecretMinLength", knownSecretMinLength );
    cfg.SetAttribute( "knownSecretMinEntropyBits", knownSecretMinEntropyBits );
    SaveList( cfg, "ignoredSecretFields", ignoredSecretFields );
    cfg.SetAttribute( "tripwireMaxLineShare", tripwireMaxLineShare );
    cfg.SetAttribute( "maskIdLikeMemberNames", maskIdLikeMemberNames );
    cfg.SetAttribute( "maskTokenLikeParameterNames", maskTokenLikeParameterNames );
    cfg.SetAttribute( "maxParameterNameLength", maxParameterNameLength );
    SaveList( cfg, "valueQueryNames", valueQueryNames );
    SaveList( cfg, "enumQueryNames", enumQueryNames );
    SaveList( cfg, "valuePathSegmentsAfter", valuePathSegmentsAfter );
    cfg.SetAttribute( "maskUppercaseTokens", maskUppercaseTokens );
    cfg.SetAttribute( "uppercaseTokenMaxLength", uppercaseTokenMaxLength );
    SaveList( cfg, "nonSensitiveValues", nonSensitiveValues );
    SaveList( cfg, "sensitiveValues", sensitiveValues );
    SaveList( cfg, "sensitiveValuePatterns", sensitiveValuePatterns );
    cfg.SetAttribute( "maskFileNameWords", maskFileNameWords );
    cfg.SetAttribute( "recordResponseShapes", recordResponseShapes );
    cfg.SetAttribute( "saveResponseBodies", saveResponseBodies );
    cfg.SetAttribute( "maxResponseBodyBytes", maxResponseBodyBytes );
    SaveList( cfg, "responseBodyContentTypes", responseBodyContentTypes );
    cfg.SetAttribute( "outputDirectory", CORE::CVariant( outputDirectory ) );
    cfg.SetAttribute( "journalFileName", CORE::CVariant( journalFileName ) );
    cfg.SetAttribute( "reportFileName", CORE::CVariant( reportFileName ) );
    cfg.SetAttribute( "bodiesDirectoryName", CORE::CVariant( bodiesDirectoryName ) );
    cfg.SetAttribute( "continueJournal", continueJournal );
    cfg.SetAttribute( "disableBrowserCache", disableBrowserCache );
    cfg.SetAttribute( "reportTableCellControlNames", reportTableCellControlNames );
    cfg.SetAttribute( "captureClicks", captureClicks );
    cfg.SetAttribute( "actionWindowMs", actionWindowMs );
    cfg.SetAttribute( "actionMaxDurationMs", actionMaxDurationMs );
    cfg.SetAttribute( "quietGapMs", quietGapMs );
    cfg.SetAttribute( "maxActions", maxActions );
    cfg.SetAttribute( "maxShapeDepth", maxShapeDepth );
    cfg.SetAttribute( "maxShapeLines", maxShapeLines );
    cfg.SetAttribute( "maxShapesPerEndpoint", maxShapesPerEndpoint );
    cfg.SetAttribute( "browserRunMode", CORE::CVariant( BrowserRunModeToString( browserRunMode ) ) );
    cfg.SetAttribute( "browserDebugPort", (UInt32) browserDebugPort );
    SaveList( cfg, "browserUserDataDirs", browserUserDataDirs );
    cfg.SetAttribute( "browserExecutablePath", CORE::CVariant( browserExecutablePath ) );
    cfg.SetAttribute( "browserProfileDirectory", CORE::CVariant( browserProfileDirectory ) );
    cfg.SetAttribute( "browserStartUrl", CORE::CVariant( browserStartUrl ) );
    cfg.SetAttribute( "browserHeadless", browserHeadless );
    cfg.SetAttribute( "browserConnectTimeoutMs", browserConnectTimeoutMs );
    cfg.SetAttribute( "closeLaunchedBrowser", closeLaunchedBrowser );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    LoadList( cfg, "hostFilter", hostFilter );
    LoadList( cfg, "allowPathPrefixes", allowPathPrefixes );
    LoadList( cfg, "neverRecordPathPatterns", neverRecordPathPatterns );
    LoadList( cfg, "neverRecordHosts", neverRecordHosts );
    noteNeverRecorded = cfg.GetAttributeValueOrChildValueByName( "noteNeverRecorded" ).AsBool( noteNeverRecorded, true );
    LoadList( cfg, "secretNamePatterns", secretNamePatterns );

    const CORE::CString mode = cfg.GetAttributeValueOrChildValueByName( "secretValueMode" ).AsString( CORE::CString::Empty, true );
    if ( !mode.IsNULLOrEmpty() && !SecretValueModeFromString( mode, secretValueMode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: secretValueMode must be hash or strip, not " << mode );
        return false;
    }

    recordExampleValues = cfg.GetAttributeValueOrChildValueByName( "recordExampleValues" ).AsBool( recordExampleValues, true );
    recordEnumQueryValues = cfg.GetAttributeValueOrChildValueByName( "recordEnumQueryValues" ).AsBool( recordEnumQueryValues, true );
    knownSecretMinLength = cfg.GetAttributeValueOrChildValueByName( "knownSecretMinLength" ).AsUInt32( knownSecretMinLength, true );
    if ( knownSecretMinLength < MinKnownSecretLengthFloor )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: knownSecretMinLength " << knownSecretMinLength << " is below the minimum of " << MinKnownSecretLengthFloor );
        return false;
    }
    knownSecretMinEntropyBits = cfg.GetAttributeValueOrChildValueByName( "knownSecretMinEntropyBits" ).AsUInt32( knownSecretMinEntropyBits, true );
    LoadList( cfg, "ignoredSecretFields", ignoredSecretFields );
    tripwireMaxLineShare = cfg.GetAttributeValueOrChildValueByName( "tripwireMaxLineShare" ).AsUInt32( tripwireMaxLineShare, true );
    if ( tripwireMaxLineShare > 100 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: tripwireMaxLineShare is a percentage, not " << tripwireMaxLineShare );
        return false;
    }
    maskIdLikeMemberNames = cfg.GetAttributeValueOrChildValueByName( "maskIdLikeMemberNames" ).AsBool( maskIdLikeMemberNames, true );
    maskTokenLikeParameterNames = cfg.GetAttributeValueOrChildValueByName( "maskTokenLikeParameterNames" ).AsBool( maskTokenLikeParameterNames, true );
    maxParameterNameLength = cfg.GetAttributeValueOrChildValueByName( "maxParameterNameLength" ).AsUInt32( maxParameterNameLength, true );
    if ( 0 == maxParameterNameLength )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: maxParameterNameLength must be at least 1" );
        return false;
    }
    LoadList( cfg, "valueQueryNames", valueQueryNames );
    LoadList( cfg, "enumQueryNames", enumQueryNames );
    LoadList( cfg, "valuePathSegmentsAfter", valuePathSegmentsAfter );
    maskUppercaseTokens = cfg.GetAttributeValueOrChildValueByName( "maskUppercaseTokens" ).AsBool( maskUppercaseTokens, true );
    uppercaseTokenMaxLength = cfg.GetAttributeValueOrChildValueByName( "uppercaseTokenMaxLength" ).AsUInt32( uppercaseTokenMaxLength, true );
    LoadList( cfg, "nonSensitiveValues", nonSensitiveValues );
    LoadList( cfg, "sensitiveValues", sensitiveValues );
    LoadList( cfg, "sensitiveValuePatterns", sensitiveValuePatterns );
    maskFileNameWords = cfg.GetAttributeValueOrChildValueByName( "maskFileNameWords" ).AsBool( maskFileNameWords, true );
    recordResponseShapes = cfg.GetAttributeValueOrChildValueByName( "recordResponseShapes" ).AsBool( recordResponseShapes, true );
    saveResponseBodies = cfg.GetAttributeValueOrChildValueByName( "saveResponseBodies" ).AsBool( saveResponseBodies, true );
    maxResponseBodyBytes = cfg.GetAttributeValueOrChildValueByName( "maxResponseBodyBytes" ).AsUInt64( maxResponseBodyBytes, true );
    LoadList( cfg, "responseBodyContentTypes", responseBodyContentTypes );
    outputDirectory = cfg.GetAttributeValueOrChildValueByName( "outputDirectory" ).AsString( outputDirectory, true );
    journalFileName = cfg.GetAttributeValueOrChildValueByName( "journalFileName" ).AsString( journalFileName, true );
    reportFileName = cfg.GetAttributeValueOrChildValueByName( "reportFileName" ).AsString( reportFileName, true );
    bodiesDirectoryName = cfg.GetAttributeValueOrChildValueByName( "bodiesDirectoryName" ).AsString( bodiesDirectoryName, true );
    continueJournal = cfg.GetAttributeValueOrChildValueByName( "continueJournal" ).AsBool( continueJournal, true );
    disableBrowserCache = cfg.GetAttributeValueOrChildValueByName( "disableBrowserCache" ).AsBool( disableBrowserCache, true );
    reportTableCellControlNames = cfg.GetAttributeValueOrChildValueByName( "reportTableCellControlNames" ).AsBool( reportTableCellControlNames, true );
    captureClicks = cfg.GetAttributeValueOrChildValueByName( "captureClicks" ).AsBool( captureClicks, true );
    actionWindowMs = cfg.GetAttributeValueOrChildValueByName( "actionWindowMs" ).AsUInt32( actionWindowMs, true );
    actionMaxDurationMs = cfg.GetAttributeValueOrChildValueByName( "actionMaxDurationMs" ).AsUInt32( actionMaxDurationMs, true );
    quietGapMs = cfg.GetAttributeValueOrChildValueByName( "quietGapMs" ).AsUInt32( quietGapMs, true );
    maxActions = cfg.GetAttributeValueOrChildValueByName( "maxActions" ).AsUInt32( maxActions, true );
    maxShapeDepth = cfg.GetAttributeValueOrChildValueByName( "maxShapeDepth" ).AsUInt32( maxShapeDepth, true );
    maxShapeLines = cfg.GetAttributeValueOrChildValueByName( "maxShapeLines" ).AsUInt32( maxShapeLines, true );
    maxShapesPerEndpoint = cfg.GetAttributeValueOrChildValueByName( "maxShapesPerEndpoint" ).AsUInt32( maxShapesPerEndpoint, true );

    const CORE::CString runMode = cfg.GetAttributeValueOrChildValueByName( "browserRunMode" ).AsString( CORE::CString::Empty, true );
    if ( !runMode.IsNULLOrEmpty() && !BrowserRunModeFromString( runMode, browserRunMode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: browserRunMode must be attach or recordedSession, not " << runMode );
        return false;
    }
    const UInt32 port = cfg.GetAttributeValueOrChildValueByName( "browserDebugPort" ).AsUInt32( browserDebugPort, true );
    if ( 0 == port || port > 65535 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySettings: browserDebugPort " << port << " is not a valid port" );
        return false;
    }
    browserDebugPort = (UInt16) port;
    LoadList( cfg, "browserUserDataDirs", browserUserDataDirs );
    browserExecutablePath = cfg.GetAttributeValueOrChildValueByName( "browserExecutablePath" ).AsString( browserExecutablePath, true );
    browserProfileDirectory = cfg.GetAttributeValueOrChildValueByName( "browserProfileDirectory" ).AsString( browserProfileDirectory, true );
    browserStartUrl = cfg.GetAttributeValueOrChildValueByName( "browserStartUrl" ).AsString( browserStartUrl, true );
    browserHeadless = cfg.GetAttributeValueOrChildValueByName( "browserHeadless" ).AsBool( browserHeadless, true );
    browserConnectTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "browserConnectTimeoutMs" ).AsUInt32( browserConnectTimeoutMs, true );
    closeLaunchedBrowser = cfg.GetAttributeValueOrChildValueByName( "closeLaunchedBrowser" ).AsBool( closeLaunchedBrowser, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSiteDiscoverySettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CSiteDiscoverySettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

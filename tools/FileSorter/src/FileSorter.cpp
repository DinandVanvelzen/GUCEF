/*
 *  FileSorter: service which sorts files
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#include "gucefCOMCORE_CGeoLocationLookupService.h"
#define GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H
#endif /* GUCEF_COMCORE_CGEOLOCATIONLOOKUPSERVICE_H ? */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CHTTPENCODINGTYPES_H
#include "gucefWEB_CHttpEncodingTypes.h"
#define GUCEF_WEB_CHTTPENCODINGTYPES_H
#endif /* GUCEF_WEB_CHTTPENCODINGTYPES_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#include "FileSorter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

RestApiFileSorterInfoResource::RestApiFileSorterInfoResource( FileSorter* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiFileSorterInfoResource::~RestApiFileSorterInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiFileSorterInfoResource::Serialize( CORE::CDataNode& output             ,
                                          const CORE::CString& representation )
{GUCEF_TRACE;

    output.SetName( "info" );
    output.SetAttribute( "application", "FileSorter" );
    output.SetAttribute( "buildDateTime", __TIMESTAMP__ );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", "false" );
    #else
    output.SetAttribute( "isReleaseBuild", "true" );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiFileSorterConfigResource::RestApiFileSorterConfigResource( FileSorter* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiFileSorterConfigResource::~RestApiFileSorterConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiFileSorterConfigResource::Serialize( CORE::CDataNode& output             ,
                                            const CORE::CString& representation )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const FileSorterConfig& loadedConfig = m_app->GetAppConfig();
        return loadedConfig.SaveConfig( output );
    }

    const CORE::CDataNode& globalConfig = m_app->GetGlobalConfig();
    output.Copy( globalConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString FileTypeConfig::ClassTypeName = "FileTypeConfig";

/*-------------------------------------------------------------------------*/

FileTypeConfig::FileTypeConfig( void )
    : CORE::CIConfigurable()
    , m_fileExtToTypeMap()
    , m_typeToSortFolderMap()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

FileTypeConfig::~FileTypeConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
FileTypeConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
FileTypeConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

 CORE::CString
FileTypeConfig::GetStringForFileType( const TFileType fileType )
{GUCEF_TRACE;

    switch ( fileType )
    {
        case TFileType::FILETYPE_TEXT: return "text";
        case TFileType::FILETYPE_BINARY: return "binary";
        case TFileType::FILETYPE_IMAGE: return "image";
        case TFileType::FILETYPE_AUDIO: return "audio";
        case TFileType::FILETYPE_VIDEO: return "video";
        case TFileType::FILETYPE_ARCHIVE: return "archive";
        case TFileType::FILETYPE_DOCUMENT: return "document";
        case TFileType::FILETYPE_PRESENTATION: return "presentation";
        case TFileType::FILETYPE_SPREADSHEET: return "spreadsheet";
        case TFileType::FILETYPE_DATABASE: return "database";
        case TFileType::FILETYPE_EXECUTABLE: return "executable";
        case TFileType::FILETYPE_SCRIPT: return "script";
        case TFileType::FILETYPE_WEBPAGE: return "webpage";
        case TFileType::FILETYPE_SOURCECODE: return "sourcecode";
        case TFileType::FILETYPE_CONFIGURATION: return "configuration";
        case TFileType::FILETYPE_LOG: return "log";
        case TFileType::FILETYPE_TEMPORARY: return "temporary";
        case TFileType::FILETYPE_COMPRESSED: return "compressed";
        case TFileType::FILETYPE_OTHER: return "other";
        case TFileType::FILETYPE_UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

TFileType 
FileTypeConfig::GetFileTypeForString( const CORE::CString& fileTypeStr )
{GUCEF_TRACE;

    const CORE::CString& fileTypeStrLc = fileTypeStr.Lowercase();

    if ( "text" == fileTypeStrLc )
        return FILETYPE_TEXT;
    if ( "binary" == fileTypeStrLc )
        return FILETYPE_BINARY;
    if ( "image" == fileTypeStrLc )
        return FILETYPE_IMAGE;
    if ( "audio" == fileTypeStrLc )
        return TFileType::FILETYPE_AUDIO;
    if ( "video" == fileTypeStrLc )
        return TFileType::FILETYPE_VIDEO;
    if ( "archive" == fileTypeStrLc )
        return TFileType::FILETYPE_ARCHIVE;
    if ( "document" == fileTypeStrLc )
        return TFileType::FILETYPE_DOCUMENT;
    if ( "presentation" == fileTypeStrLc )
        return TFileType::FILETYPE_PRESENTATION;
    if ( "spreadsheet" == fileTypeStrLc )
        return TFileType::FILETYPE_SPREADSHEET;
    if ( "database" == fileTypeStrLc )
        return TFileType::FILETYPE_DATABASE;
    if ( "database" == fileTypeStrLc )
        return TFileType::FILETYPE_DATABASE;
    if ( "executable" == fileTypeStrLc )
        return TFileType::FILETYPE_EXECUTABLE;
    if ( "script" == fileTypeStrLc )
        return TFileType::FILETYPE_SCRIPT;
    if ( "webpage" == fileTypeStrLc )
        return TFileType::FILETYPE_WEBPAGE;
    if ( "sourcecode" == fileTypeStrLc )
        return TFileType::FILETYPE_SOURCECODE;
    if ( "configuration" == fileTypeStrLc )
        return TFileType::FILETYPE_CONFIGURATION;
    if ( "log" == fileTypeStrLc )
        return TFileType::FILETYPE_LOG;
    if ( "temporary" == fileTypeStrLc )
        return TFileType::FILETYPE_TEMPORARY;
    if ( "compressed" == fileTypeStrLc )
        return TFileType::FILETYPE_COMPRESSED;
    if ( "other" == fileTypeStrLc )
        return TFileType::FILETYPE_OTHER;
    if ( "unknown" == fileTypeStrLc )
        return TFileType::FILETYPE_UNKNOWN;

    return TFileType::FILETYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

TFileTypeSet 
FileTypeConfig::GetFileTypesForStringSet( const CORE::CString::StringSet& fileTypeStrs )
{GUCEF_TRACE;

    TFileTypeSet fileTypes;
    CORE::CString::StringSet::const_iterator i = fileTypeStrs.begin();
    while ( i != fileTypeStrs.end() )
    {
        const CORE::CString& fileTypeStr = (*i);
        fileTypes.insert( GetFileTypeForString( fileTypeStr ) );
        ++i;
    }
    return fileTypes;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet 
FileTypeConfig::GetStringSetForFileTypes( const TFileTypeSet& fileTypes )
{GUCEF_TRACE;

    CORE::CString::StringSet fileTypeStrs;
    TFileTypeSet::const_iterator i = fileTypes.begin();
    while ( i != fileTypes.end() )
    {
        TFileType fileType = (*i);
        fileTypeStrs.insert( GetStringForFileType( fileType ) );
        ++i;
    }
    return fileTypeStrs;
}

/*-------------------------------------------------------------------------*/

bool
FileTypeConfig::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CORE::CDataNode::TConstDataNodeSet fileTypeNodes = cfg.FindChildrenOfType( "FileType", true, false );
    CORE::CDataNode::TConstDataNodeSet::const_iterator i = fileTypeNodes.begin();
    while ( i != fileTypeNodes.end() )
    {
        const CORE::CDataNode& fileTypeNode = *(*i);
        
        CORE::CString fileTypeStr = fileTypeNode.GetAttributeValueOrChildValueByName( "type" ).AsString();
        CORE::CString::StringSet fileExts = fileTypeNode.GetAttributeValueOrChildValueByName( "extensions" ).AsString().ParseUniqueElements( ',', false );
        CORE::CString sortFolderName = fileTypeNode.GetAttributeValueOrChildValueByName( "sortFolderName" ).AsString();
        
        TFileType fileType = FileTypeConfig::GetFileTypeForString( fileTypeStr );
        if ( !sortFolderName.IsNULLOrEmpty() )
        {
            m_typeToSortFolderMap[ fileType ] = sortFolderName;

            CORE::CString::StringSet::const_iterator j = fileExts.begin();
            while ( j != fileExts.end() )
            {
                const CORE::CString& fileExt = (*j);
                m_fileExtToTypeMap[ fileExt.Lowercase() ] = fileType;
                ++j;
            }
        }
        ++i;
    }

    return !m_typeToSortFolderMap.empty() && !m_fileExtToTypeMap.empty();
}

/*-------------------------------------------------------------------------*/

TFileType 
FileTypeConfig::GetFileTypeForFileExtension( const CORE::CString& fileExt ) const
{GUCEF_TRACE;

    TStringToFileTypeMap::const_iterator i = m_fileExtToTypeMap.find( fileExt.Lowercase() );
    if ( i != m_fileExtToTypeMap.end() )
        return (*i).second;
    return TFileType::FILETYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
FileTypeConfig::GetSortRootFolderForFileType( TFileType fileType ) const
{GUCEF_TRACE;

    TFileTypeToStringMap::const_iterator i = m_typeToSortFolderMap.find( fileType );
    if ( i != m_typeToSortFolderMap.end() )
        return (*i).second;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
FileTypeConfig::GetSortRootFolderForFileExtension( const CORE::CString& fileExt ) const
{GUCEF_TRACE;

    return GetSortRootFolderForFileType( GetFileTypeForFileExtension( fileExt ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString FileSorterConfig::ClassTypeName = "FileSorterConfig";

/*-------------------------------------------------------------------------*/

FileSorterConfig::FileSorterConfig( void )
    : CORE::CGloballyConfigurable()
    , vfsInboxPath()
    , vfsInboxPathScanIntervalInMs( 5 * 60 * 1000 ) // default is 5 minutes
    , vfsSortSourceRootPath()
    , vfsSortedTargetRootPath()
    , useDateTimeFolderStructure( true )
    , dirStructureUtcOffsetInMins( 0 )  // default keeps filesystem structure in UTC
    , yearFolderDecoration( "-== {year} ==-" )
    , useMonthFolder( false )
    , monthFolderDecoration( "-== {month}. {monthName} ==-" )
    , useDayFolder( true )
    , dayFolderDecoration( "{month}.{day}.{year}" )
    , sortAllFileTypes( false )
    , fileTypesToSort()
    , tryToGetMetaDataInfoFromImages( false ) 
    , useMediaPropertyEarliestDt( true )
    , performReverseGeolocationLookups( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

FileSorterConfig::~FileSorterConfig()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

bool
FileSorterConfig::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter:SaveConfig" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
FileSorterConfig::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter:LoadConfig" );
    
    bool totalSuccess = true;

    const CORE::CDataNode* cfg = globalConfig.Find( "FileSorterConfig" );
    if ( GUCEF_NULL == cfg )
        return false;

    vfsInboxPath = cfg->GetAttributeValueOrChildValueByName( "vfsInboxPath", vfsInboxPath ).AsString( vfsInboxPath, true );
    vfsInboxPathScanIntervalInMs = cfg->GetAttributeValueOrChildValueByName( "vfsInboxPathScanIntervalInMs", vfsInboxPathScanIntervalInMs ).AsUInt32( vfsInboxPathScanIntervalInMs, true );
    vfsSortSourceRootPath = cfg->GetAttributeValueOrChildValueByName( "vfsSortSourceRootPath", vfsSortSourceRootPath ).AsString( vfsSortSourceRootPath, true );
    vfsSortedTargetRootPath = cfg->GetAttributeValueOrChildValueByName( "vfsSortedTargetRootPath", vfsSortedTargetRootPath ).AsString( vfsSortedTargetRootPath, true );
    useDateTimeFolderStructure = cfg->GetAttributeValueOrChildValueByName( "useDateTimeFolderStructure", useDateTimeFolderStructure ).AsBool( useDateTimeFolderStructure, true );
    dirStructureUtcOffsetInMins = cfg->GetAttributeValueOrChildValueByName( "dirStructureUtcOffsetInMins", dirStructureUtcOffsetInMins ).AsInt16( dirStructureUtcOffsetInMins, true );
    yearFolderDecoration = cfg->GetAttributeValueOrChildValueByName( "yearFolderDecoration", yearFolderDecoration ).AsString( yearFolderDecoration, true );
    useMonthFolder = cfg->GetAttributeValueOrChildValueByName( "useMonthFolder", useMonthFolder ).AsBool( useMonthFolder, true );
    monthFolderDecoration = cfg->GetAttributeValueOrChildValueByName( "monthFolderDecoration", monthFolderDecoration ).AsString( monthFolderDecoration, true );
    useDayFolder = cfg->GetAttributeValueOrChildValueByName( "useDayFolder", useDayFolder ).AsBool( useDayFolder, true );
    dayFolderDecoration = cfg->GetAttributeValueOrChildValueByName( "dayFolderDecoration", dayFolderDecoration ).AsString( dayFolderDecoration, true );
    sortAllFileTypes = cfg->GetAttributeValueOrChildValueByName( "sortAllFileTypes", useDayFolder ).AsBool( sortAllFileTypes, true );
        
    CORE::CString::StringSet fileTypesToSortStrSet = FileTypeConfig::GetStringSetForFileTypes( fileTypesToSort );
    CORE::CString fileTypesToSortStr = CORE::ToString( fileTypesToSortStrSet );
    fileTypesToSortStr = cfg->GetAttributeValueOrChildValueByName( "fileTypesToSort", fileTypesToSortStr ).AsString( fileTypesToSortStr, true );
    fileTypesToSortStrSet = CORE::StringToStringSet( fileTypesToSortStr );

    tryToGetMetaDataInfoFromImages = cfg->GetAttributeValueOrChildValueByName( "tryToGetMetaDataInfoFromImages", tryToGetMetaDataInfoFromImages ).AsBool( tryToGetMetaDataInfoFromImages, true );
    useMediaPropertyEarliestDt = cfg->GetAttributeValueOrChildValueByName( "useMediaPropertyEarliestDt", useMediaPropertyEarliestDt ).AsBool( useMediaPropertyEarliestDt, true );
    fileTypesToSort = FileTypeConfig::GetFileTypesForStringSet( fileTypesToSortStrSet );
    performReverseGeolocationLookups = cfg->GetAttributeValueOrChildValueByName( "performReverseGeolocationLookups", performReverseGeolocationLookups ).AsBool( performReverseGeolocationLookups, true );
    
    const CORE::CDataNode* fileTypeConfigNode = cfg->FindChild( "FileTypeConfig" );
    if ( GUCEF_NULL != fileTypeConfigNode )
    {
        totalSuccess = fileTypeConfig.LoadConfig( *fileTypeConfigNode ) && totalSuccess;
    }
    
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
FileSorterConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

MediaMetaData::MediaMetaData( void )
    : geoLocation()
    , hasGeoLocation( false )
    , earliestMediaPropertyDt()
    , country()
    , stateOrProvice()
    , city()
    , street()
    , streetNr()
    , zipOrPostalCode()
    , timeZoneOffsetInMins( 0 )
    , hasTimeZoneOffsetInMins( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
FileSorter::GetMediaMetaDataFromImage( const CORE::CString& vfsFilePath, MediaMetaData& mediaMetaData )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString fileExt = CORE::ExtractFileExtention( vfsFilePath );
    
    if ( 0 == vfs.GetFileSize( vfsFilePath ) )
        return false;

    IMAGE::CImage img;
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsFilePath, "rb", false );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr ioAccess = file->GetAccess();
        if ( !ioAccess.IsNULL() )
        {
            if ( img.Load( *ioAccess, fileExt, false, true ) )
            {                                    
                if ( img.TryGetGeoLocationFromMetaData( mediaMetaData.geoLocation ) )
                {
                    mediaMetaData.hasGeoLocation = true;

                    if ( m_appConfig.performReverseGeolocationLookups )
                    {
                        // Attempt to obtain other location information based on the geo location
                        // This will require service providers to be registered capable of looking up geo location info
                        COMCORE::CComCoreGlobal* comms = COMCORE::CComCoreGlobal::Instance();
                        COMCORE::CGeoLocationLookupService& geoLookupService = comms->GetGeoLocationLookupService();
                        if ( geoLookupService.TryLookupLocation( mediaMetaData.geoLocation, 
                                                                 mediaMetaData.country,
                                                                 mediaMetaData.stateOrProvice,
                                                                 mediaMetaData.city,
                                                                 mediaMetaData.street,
                                                                 mediaMetaData.streetNr,
                                                                 mediaMetaData.zipOrPostalCode,
                                                                 mediaMetaData.timeZoneOffsetInMins,
                                                                 mediaMetaData.hasTimeZoneOffsetInMins ) )
                        {
                        }
                    }
                }   
                    
                bool isTimezoneAware = false;
                CORE::CDateTime originalImageCreationDt;
                if ( img.TryGetOriginalImageCreationDtFromMetaData( originalImageCreationDt, isTimezoneAware ) )
                {
                    if ( !isTimezoneAware )
                    {
                        // Use the timezone offset obtained from geo lookup, if available
                        if ( mediaMetaData.hasTimeZoneOffsetInMins )
                        {
                            originalImageCreationDt.SetTimeZoneUTCOffsetInMins( mediaMetaData.timeZoneOffsetInMins );
                            originalImageCreationDt = originalImageCreationDt.ToUTC();
                        }
                        else
                        {
                            // Add the default filesystem timezone offset to the date time in the absence of timezone info
                            // Its more likely to be 'less wrong' closer to what the filesystem offset is vs assuming UTC
                            // this reinterprets the datetime as-is in a different timezone thus assuming that the media was created in that timezone
                            originalImageCreationDt.SetTimeZoneUTCOffsetInMins( m_appConfig.dirStructureUtcOffsetInMins ); 
                            originalImageCreationDt = originalImageCreationDt.ToUTC();
                        }
                    }
                        
                    if ( mediaMetaData.earliestMediaPropertyDt == CORE::CDateTime::Empty ||
                         mediaMetaData.earliestMediaPropertyDt > originalImageCreationDt  )
                    {
                        mediaMetaData.earliestMediaPropertyDt = originalImageCreationDt;
                    }
                }

                CORE::CDateTime imageLastModifiedDt;
                if ( img.TryGetImageLastModifiedDtFromMetaData( imageLastModifiedDt, isTimezoneAware ) )
                {
                    if ( !isTimezoneAware )
                    {
                        // Use the timezone offset obtained from geo lookup, if available
                        if ( mediaMetaData.hasTimeZoneOffsetInMins )
                        {
                            originalImageCreationDt.SetTimeZoneUTCOffsetInMins( mediaMetaData.timeZoneOffsetInMins );
                            originalImageCreationDt = originalImageCreationDt.ToUTC();
                        }
                        else
                        {                            
                            // Add the default filesystem timezone offset to the date time in the absence of timezone info
                            // Its more likely to be 'less wrong' closer to what the filesystem offset is vs assuming UTC
                            // this reinterprets the datetime as-is in a different timezone thus assuming that the media was created in that timezone
                            imageLastModifiedDt.SetTimeZoneUTCOffsetInMins( m_appConfig.dirStructureUtcOffsetInMins ); 
                            imageLastModifiedDt = imageLastModifiedDt.ToUTC();
                        }
                    }

                    if ( mediaMetaData.earliestMediaPropertyDt == CORE::CDateTime::Empty ||
                         mediaMetaData.earliestMediaPropertyDt > imageLastModifiedDt      )
                    {
                        mediaMetaData.earliestMediaPropertyDt = imageLastModifiedDt;
                    }
                }
                
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
FileSorter::SortFile( const CORE::CString& currentVfsFilePath )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        
    CORE::CString fileExt = CORE::ExtractFileExtention( currentVfsFilePath );
    TFileType fileType = m_appConfig.fileTypeConfig.GetFileTypeForFileExtension( fileExt );

    if ( !m_appConfig.sortAllFileTypes )
    {
        TFileTypeSet::iterator i = m_appConfig.fileTypesToSort.find( fileType );
        if ( i == m_appConfig.fileTypesToSort.end() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: File \"" + currentVfsFilePath + "\" is of type \"" + 
                FileTypeConfig::GetStringForFileType( fileType ) + "\" and is not in the list of file types to sort. Skipping" );
            return true;
        }
    }

    CORE::CString filename = CORE::ExtractFilename( currentVfsFilePath );
    const CORE::CString& typeSortRootFolder = m_appConfig.fileTypeConfig.GetSortRootFolderForFileType( fileType );    
        
    CORE::CString targetVfsPath = m_appConfig.vfsSortedTargetRootPath + "/" + typeSortRootFolder;
    
    // Some variables we may or may not be able to source from resource meta-data
    MediaMetaData mediaMetaData;
    if ( fileType == TFileType::FILETYPE_IMAGE &&
         m_appConfig.tryToGetMetaDataInfoFromImages )
    {
       GetMediaMetaDataFromImage( currentVfsFilePath, mediaMetaData );
    }
    
    if ( m_appConfig.useDateTimeFolderStructure )    
    {                               
        CORE::CResourceMetaData fileMetaData;
        if ( vfs.GetFileMetaData( currentVfsFilePath, fileMetaData ) )
        {
            CORE::CDateTime earliestUtcFileDt;
            if ( !fileMetaData.GetEarliestUtcDt( earliestUtcFileDt ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Failed to get earliest date time for : " + currentVfsFilePath );
                return false;
            }

            if ( m_appConfig.useMediaPropertyEarliestDt )
            {
                // if we have a media property date time, we use that as the earliest date time
                // if it is in fact earlier than the file's earliest date time
                if ( mediaMetaData.earliestMediaPropertyDt != CORE::CDateTime::Empty &&
                     mediaMetaData.earliestMediaPropertyDt < earliestUtcFileDt        )
                {
                    earliestUtcFileDt = mediaMetaData.earliestMediaPropertyDt.ToUTC();
                }
            }
            
            CORE::CDateTime sortPathDt = earliestUtcFileDt;
            sortPathDt.AddMinutes( m_appConfig.dirStructureUtcOffsetInMins ); // this applies a timezone conversion

            CORE::CString sortPathYearStr = m_appConfig.yearFolderDecoration.ReplaceSubstr( "{year}", CORE::ToString( sortPathDt.GetYear() ) );

            targetVfsPath += "/" + sortPathYearStr;
            
            if ( m_appConfig.useMonthFolder )
            {
                CORE::CString sortPathMonthStr = m_appConfig.monthFolderDecoration.ReplaceSubstr( "{month}", CORE::ToString( sortPathDt.GetMonth() ) );
                sortPathMonthStr = sortPathMonthStr.ReplaceSubstr( "{monthName}", CORE::ToString( sortPathDt.GetMonthName() ) );

                targetVfsPath += "/" + sortPathMonthStr;
            }            
        
            if ( m_appConfig.useDayFolder )
            {
                CORE::CString sortPathDayStr = m_appConfig.dayFolderDecoration.ReplaceSubstr( "{day}", CORE::ToString( sortPathDt.GetDay() ) );
                sortPathDayStr = sortPathDayStr.ReplaceSubstr( "{year}", CORE::ToString( sortPathDt.GetYear() ) );
                sortPathDayStr = sortPathDayStr.ReplaceSubstr( "{month}", CORE::ToString( sortPathDt.GetMonth() ) );
                sortPathDayStr = sortPathDayStr.ReplaceSubstr( "{monthName}", CORE::ToString( sortPathDt.GetMonthName() ) );

                targetVfsPath += "/" + sortPathDayStr;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Failed to get file meta data for : " + currentVfsFilePath );
            return false;
        }
    }

    targetVfsPath += "/" + filename;
    
    if ( vfs.FileExists( targetVfsPath ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: File already exists at target location \"" + targetVfsPath + "\". Will add suffix to compensate" );

        CORE::CString targetVfsPathWithoutExt = CORE::StripFileExtention( targetVfsPath );

        bool foundAlternate = false;
        for ( CORE::UInt32 i=2; i<1000; ++i )
        {
            CORE::CString testTargetVfsPath = targetVfsPathWithoutExt + "_" + CORE::ToString( i ) + "." + fileExt;
            if ( !vfs.FileExists( testTargetVfsPath ) )
            {
                targetVfsPath = testTargetVfsPath;
                foundAlternate = true;
                break;
            }
        }
        
        if ( !foundAlternate )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Failed to find alternate target location for file \"" + targetVfsPath + "\"" );
            return false;
        }
    }    

    if ( vfs.MoveFile( currentVfsFilePath, targetVfsPath, false ) ) // for safety we NEVER overwrite
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Successfully moved file from \"" + currentVfsFilePath + "\" to \"" + targetVfsPath + "\"" );    
        return true;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Failed to move file from \"" + currentVfsFilePath + "\" to \"" + targetVfsPath + "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
FileSorter::SortFilesInVfsPath( const CORE::CString& vfsRootPath )
{GUCEF_TRACE;

    bool totalSuccess = true;
    if ( vfsRootPath.IsNULLOrEmpty() )
        return true;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.IsInitialized() )
        return false;
    
    CORE::CString::StringVector filesToSort;
    if ( vfs.GetFileList( filesToSort, vfsRootPath, true, true, CORE::CString::Empty ) )
    {
        CORE::CString::StringVector::const_iterator i = filesToSort.begin();
        while ( i != filesToSort.end() )
        {
            const CORE::CString& fileToSort = (*i);
            totalSuccess = SortFile( fileToSort ) && totalSuccess;
            ++i;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
FileSorter::SortInitialRootPaths( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Commencing sorting of initial startup root paths" );
    bool result = SortFilesInVfsPath( m_appConfig.vfsInboxPath ) && SortFilesInVfsPath( m_appConfig.vfsSortSourceRootPath );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Finished sorting of initial startup root paths" );
    return result;
}

/*-------------------------------------------------------------------------*/

void
FileSorter::OnAppStarted( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter:OnAppStarted" );
    
    SortInitialRootPaths();

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    
    if ( vfs.AddDirToWatch( m_appConfig.vfsInboxPath, true ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Added VFS path  \"" + m_appConfig.vfsInboxPath + "\" to watch list" );
    }
    else
    {
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "FileSorter: Failed to add VFS path \"" + m_appConfig.vfsInboxPath + "\" to watch list" );
	}

    m_inboxWatchTimer.SetInterval( m_appConfig.vfsInboxPathScanIntervalInMs );
    m_inboxWatchTimer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void 
FileSorter::OnVfsWatchedInboxDirChange( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // we dont even care what the change is or to what file, we just sort everything in the inbox
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Detected changes in 'inbox' VFS path  \"" + m_appConfig.vfsInboxPath + "\"" );
    SortFilesInVfsPath( m_appConfig.vfsInboxPath );
}

/*-------------------------------------------------------------------------*/

void
FileSorter::OnInboxWatchTimerCycle( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_appConfig.vfsInboxPath.IsNULLOrEmpty() )
    {
        m_inboxWatchTimer.SetEnabled( false ); // stop the timer while we process just in case it takes a while

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Performing periodic scan of VFS path  \"" + m_appConfig.vfsInboxPath + "\"" );
        SortFilesInVfsPath( m_appConfig.vfsInboxPath );

        m_inboxWatchTimer.SetEnabled( true );
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "FileSorter: Cannot perform periodic scan of 'inbox' VFS path since none is configured" );
    }
}

/*-------------------------------------------------------------------------*/

void 
FileSorter::RegisterEventHandlers( void )
{GUCEF_TRACE;

    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    // Set our starting conditions for work since we depend on multiple sub-systems
    m_appWorkStartTrigger.SubscribeAndAddEventToTriggerCriterea( &app, CORE::CGUCEFApplication::FirstCycleEvent );
    m_appWorkStartTrigger.SubscribeAndAddEventToTriggerCriterea( &pluginControl, CORE::CPluginControl::RegisteringOfAllPluginsFinishedEvent );
    m_appWorkStartTrigger.SubscribeAndAddEventToTriggerCriterea( &vfs, VFS::CVFS::VfsInitializationCompletedEvent );
    
    TEventCallback callback( this, &FileSorter::OnAppStarted );
    SubscribeTo( &m_appWorkStartTrigger                              ,
                 CORE::CEventAggregateTrigger::AggregateTriggerEvent ,
                 callback                                            );
    
    TEventCallback callback3( this, &FileSorter::OnVfsWatchedInboxDirChange );
    SubscribeTo( &vfs                        ,
                 VFS::CVFS::FileCreatedEvent ,
                 callback3                   );
    TEventCallback callback4( this, &FileSorter::OnVfsWatchedInboxDirChange );
    SubscribeTo( &vfs                         ,
                 VFS::CVFS::FileModifiedEvent ,
                 callback4                    );
    TEventCallback callback5( this, &FileSorter::OnVfsWatchedInboxDirChange );
    SubscribeTo( &vfs                        ,
                 VFS::CVFS::FileRenamedEvent ,
                 callback5                   );

    TEventCallback callback6( this, &FileSorter::OnInboxWatchTimerCycle );
    SubscribeTo( &m_inboxWatchTimer             ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

FileSorter::FileSorter( void )
    : CORE::CTSGNotifier()
    , m_httpServer()
    , m_httpRouter()
    , m_taskManagementRsc()
    , m_inboxWatchTimer()
    , m_globalConfig()
    , m_appArgs()
    , m_appConfig()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

FileSorter::~FileSorter()
{GUCEF_TRACE;
    
    m_httpServer.Close();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

bool
FileSorter::Start( void )
{GUCEF_TRACE;

    bool totalSuccess = true;

    if ( m_httpServer.Listen() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "FileSorter: Opened REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
        return totalSuccess;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "FileSorter: Failed to open REST API on port " + CORE::UInt16ToString( m_httpServer.GetPort() ) );
    return false;

}

/*-------------------------------------------------------------------------*/

bool
FileSorter::LoadConfig( const CORE::CValueList& appArgs     ,
                        const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appArgs.GetValueAlways( "RestApiPort", "10000" ) ) ) );
    m_httpRouter.SetResourceMapping( "/info", RestApiFileSorterInfoResource::THTTPServerResourcePtr( new RestApiFileSorterInfoResource( this ) )  );
    m_httpRouter.SetResourceMapping( "/config/appargs", RestApiFileSorterInfoResource::THTTPServerResourcePtr( new RestApiFileSorterConfigResource( this, true ) )  );
    m_httpRouter.SetResourceMapping( "/config", RestApiFileSorterInfoResource::THTTPServerResourcePtr( new RestApiFileSorterConfigResource( this, false ) )  );
    m_httpRouter.SetResourceMapping(  appArgs.GetValueAlways( "RestBasicHealthUri", "/health/basic" ), RestApiFileSorterInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

    if ( !m_taskManagementRsc.ConnectHttpRouting( m_httpRouter ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "FileSorter: Failed to set up task management API" );
    }

    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );

    m_appArgs = appArgs;
    m_globalConfig.Copy( globalConfig );

    return m_appConfig.LoadConfig( globalConfig );
}

/*-------------------------------------------------------------------------*/

const FileSorterConfig&
FileSorter::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
FileSorter::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/

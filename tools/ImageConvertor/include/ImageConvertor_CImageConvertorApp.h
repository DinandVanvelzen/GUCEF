/*
 *  ImageConvertor: converts images between the formats of the loaded image codecs
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
#ifndef IMAGECONVERTOR_CIMAGECONVERTORAPP_H
#define IMAGECONVERTOR_CIMAGECONVERTORAPP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H
#include "gucefIMAGE_CImageConversionSettings.h"
#define GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H
#endif /* GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGECONVERTOR {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The command line of the image convertor: which files to convert to which format, and the run of those conversions.
 *  The conversions themselves are those of IMAGE::CImageConverter; this class finds the files, names the outputs, runs
 *  the conversions on the threads of a pool and reports.
 */
class CImageConvertorApp
{
    public:

    static const CORE::CString AppName;

    /** The exit codes: all files converted or skipped, a wrong command line, files that failed, a configuration that could not be loaded */
    enum EExitCode
    {
        ExitOk = 0      ,
        ExitUsage = 1   ,
        ExitFailed = 2  ,
        ExitConfig = 3
    };

    CImageConvertorApp( int argc, char** argv );

    ~CImageConvertorApp();

    /** The options of the command line on the console */
    static void LogUsage( void );

    /** False when the command line is wrong or asks for the help: the usage is to be shown */
    bool ParseCommandLine( void );

    const CORE::CString& GetBootstrapConfigFile( void ) const;

    /** The lowest log level that is shown: --verbose and --quiet change it */
    Int32 GetMinimalLogLevel( void ) const;

    /** Loads the plugins of --plugin, lists the codecs for --list-codecs, converts the files and reports. Gives an exit code */
    Int32 Run( void );

    private:

    CImageConvertorApp( const CImageConvertorApp& src );
    CImageConvertorApp& operator=( const CImageConvertorApp& src );

    private:

    int m_argc;
    char** m_argv;
    CORE::CString m_bootstrapConfigFile;
    Int32 m_minimalLogLevel;
    CORE::CString::StringVector m_inputs;
    CORE::CString::StringVector m_plugins;
    CORE::CString m_output;
    CORE::CString m_toType;
    CORE::CString m_fromType;
    bool m_recursive;
    bool m_mirror;
    bool m_overwrite;
    bool m_dryRun;
    bool m_listCodecs;
    bool m_quiet;
    Int32 m_jobs;
    IMAGE::CImageConversionSettings m_settings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGECONVERTOR */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* IMAGECONVERTOR_CIMAGECONVERTORAPP_H ? */

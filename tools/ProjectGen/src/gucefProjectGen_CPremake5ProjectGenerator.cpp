/*
 *  ProjectGenerator: Tool to generate module/project files
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefProjectGen_CPremake5ProjectGenerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const CORE::CString AllPlatforms = "all";
static CORE::CStringMap premake5AdditionTemplates;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const TStringSetMap&
GetSupportedPlatformDirMap( void )

{GUCEF_TRACE;

    static TStringSetMap platformMap;
    if ( platformMap.empty() )
    {
        platformMap[ "WIN32" ].insert( "mswin" );
        platformMap[ "WIN32" ].insert( "win32" );
        platformMap[ "WIN64" ].insert( "mswin" );
        platformMap[ "WIN64" ].insert( "win64" );
        platformMap[ "UNIX" ].insert( "linux" );
        platformMap[ "UNIX" ].insert( "unix" );
        platformMap[ "IPHONEOS" ].insert( "iphone" );
        platformMap[ "SYMBIAN" ].insert( "symbian" );
        platformMap[ "OSX" ].insert( "osx" );
        platformMap[ "OSX" ].insert( "mac" );
        platformMap[ "ANDROID" ].insert( "android" );
        platformMap[ "GLX" ].insert( "glx" );
        platformMap[ "GTK" ].insert( "gtk" );
        platformMap[ "SDL" ].insert( "sdl" );
    }
    return platformMap;
}

/*-------------------------------------------------------------------------*/

static const TStringSet&
GetSupportedPlatforms( void )
{GUCEF_TRACE;

    static TStringSet platforms;
    if ( platforms.empty() )
    {
        const TStringSetMap& dirMap = GetSupportedPlatformDirMap();
        TStringSetMap::const_iterator i = dirMap.begin();
        while ( i != dirMap.end() )
        {
            platforms.insert( (*i).first );
            ++i;
        }
    }
    return platforms;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ConvertEnvVarStrings( const CORE::CString& inStr )
{GUCEF_TRACE;

    return inStr.ReplaceEnvelopingSubstr( "$ENVVAR:", "$", "os.getenv( ", " )" );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
GetPremake5FileHeader( bool addCompileDate = false )
{GUCEF_TRACE;

    static bool initialized = false;
    static CORE::CString fileHeader;
    static CORE::CString fileHeaderWithCompileDate;

    if ( !initialized )
    {
        fileHeader =
        "--------------------------------------------------------------------\n"
        "-- This file was automatically generated by ProjectGenerator\n"
        "-- which is tooling part the build system designed for GUCEF\n"
        "--     (Galaxy Unlimited Framework)\n"
        "-- For the latest info, see http://www.VanvelzenSoftware.com/\n"
        "--\n"
        "-- The contents of this file are placed in the public domain. Feel\n"
        "-- free to make use of it in any way you like.\n"
        "--------------------------------------------------------------------\n"
        "--\n";

        fileHeaderWithCompileDate = fileHeader +
        "--\n"
        "-- The ProjectGenerator tool used was compiled on: " __DATE__ " @ " __TIME__ "\n"
        "--\n"
        "--\n"
        "--\n";

        initialized = true;
    }

    if ( addCompileDate )
    {
        return fileHeaderWithCompileDate;
    }
    return fileHeader;
}

/*---------------------------------------------------------------------------*/
/*
CORE::CString
GenerateAutoGenertedTemplatedAdditionSeperator( bool end )
{GUCEF_TRACE;

    CORE::CString seperator = "\n\n\n#********";
    if ( end )
    {
        seperator += " END OF AUTOGENERATED TEMPLATED ADDITION CONTENT ";
    }
    else
    {
        seperator += " START OF AUTOGENERATED TEMPLATED ADDITION CONTENT ";
    }
    seperator += "********\n\n\n";
    return seperator;
}

/*---------------------------------------------------------------------------*/

bool
LoadPremake5FileAdditionTemplates( const CORE::CString& templateDir )
{GUCEF_TRACE;

    CORE::CString pathToTemplates = CORE::RelativePath( templateDir );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Attempting to load Premake5 file addition templates from: " + pathToTemplates );

    CORE::SDI_Data* sdiData = CORE::DI_First_Dir_Entry( pathToTemplates.C_String() );
    if ( NULL != sdiData )
    {
        do
        {
            if ( 0 != DI_Is_It_A_File( sdiData ) )
            {
                CORE::CString filename = pathToTemplates;
                CORE::AppendToPath( filename, DI_Name( sdiData ) );

                // Filter 'bad' files based on extention in case someone uses a non-dedicated template dir
                CORE::CString fileExt = CORE::ExtractFileExtention( filename ).Lowercase();
                if ( fileExt == "pm4t" )
                {
                    // Use the filename as the template name
                    CORE::CString templateName = DI_Name( sdiData );
                    templateName = templateName.CutChars( fileExt.Length()+1, false ).Lowercase();

                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found template with name \"" + templateName + "\" in dir " + pathToTemplates );

                    CORE::CString templateText;
                    if ( CORE::LoadTextFileAsString( filename, templateText ) )
                    {
                        premake5AdditionTemplates[ templateName ] = templateText;
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded template with name \"" + templateName + "\" from dir " + pathToTemplates );
                    }
                    else
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to load template with name \"" + templateName + "\" from dir " + pathToTemplates );
                    }
                }
            }
        }
        while ( 0 != DI_Next_Dir_Entry( sdiData ) );
        DI_Cleanup( sdiData );
    }
    return true;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5TemplatedAdditionSection( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                          const CORE::CString& platformName          )
{GUCEF_TRACE;

    const CModuleInfo* moduleInfo = NULL;
    const CORE::CString* moduleName = GetModuleName( moduleInfoEntry, platformName, &moduleInfo );

    if ( ( NULL != moduleName ) && ( NULL != moduleInfo ) )
    {
        // Use the module type as the index to find a template.
        // We can make this more advanced later if needed but for now differentiating based on module
        // type provides enough flexibility since you can do the rest in CMake utility functions
        CORE::CString moduleTypeSttr = ModuleTypeToString( GetModuleType( moduleInfoEntry, platformName ) ).Lowercase();
        CORE::CStringMap::iterator i = premake5AdditionTemplates.find( moduleTypeSttr );
        if ( i != premake5AdditionTemplates.end() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processing template with name \"" + moduleTypeSttr + "\" for module with name " + *moduleName + " using platform " + platformName );

            const CORE::CString& templateContent = (*i).second;

            // Now perform the main task of a template,.. variable substitution
            CORE::CString sectionContent = templateContent.ReplaceSubstr( "$#$MODULENAME$#$", "${MODULE_NAME}" );

            // --> Later we could add additional replacements here as needed

            return sectionContent;
        }
    }

    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5TemplatedAdditionSection( const CModuleInfoEntryPtr& moduleInfoEntry )

{GUCEF_TRACE;

    CORE::CString sectionContent;     /*
    bool platformAdded = false;
    bool allPlatformsSectionAdded = false;

    TModuleInfoMap::const_iterator i = moduleInfoEntry.modulesPerPlatform.begin();
    while ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;
        if ( AllPlatforms != platformName )
        {
            if ( !platformAdded )
            {
                CORE::CString templatedContent = GenerateCMakeTemplatedAdditionSection( moduleInfoEntry, platformName );
                if ( !templatedContent.IsNULLOrEmpty() )
                {
                    sectionContent += "\n\nif (" + platformName.Uppercase() + ")\n" + templatedContent;
                    platformAdded = true;
                }
            }
            else
            {
                CORE::CString templatedContent = GenerateCMakeTemplatedAdditionSection( moduleInfoEntry, platformName );
                if ( !templatedContent.IsNULLOrEmpty() )
                {
                    sectionContent += "\nelseif (" + platformName.Uppercase() + ")\n" + templatedContent;
                }
            }
        }
        ++i;
    }

    i = moduleInfoEntry.modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry.modulesPerPlatform.end() )
    {
        CORE::CString templatedContent = GenerateCMakeTemplatedAdditionSection( moduleInfoEntry, AllPlatforms );
        if ( !templatedContent.IsNULLOrEmpty() )
        {
            if ( platformAdded )
            {
                sectionContent += "\nelse()\n" + templatedContent;
            }
            else
            {
                sectionContent = templatedContent;
            }
            allPlatformsSectionAdded = true;
        }
    }

    if ( platformAdded )
    {
        sectionContent += "\nendif()\n";
    }
    if ( platformAdded || allPlatformsSectionAdded )
    {
        sectionContent = GenerateAutoGenertedTemplatedAdditionSeperator( false ) +
                         sectionContent                                          +
                         GenerateAutoGenertedTemplatedAdditionSeperator( true );
    }
               */
    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5FileSection( const CORE::CString& sectionContent ,
                             const TStringSetMap& fileMap        )
{GUCEF_TRACE;

    CORE::CString newSectionContent = sectionContent;

    newSectionContent = "files( {\n";
    bool first = true;
    TStringSetMap::const_iterator i = fileMap.begin();
    while ( i != fileMap.end() )
    {
        TStringSet::const_iterator n = (*i).second.begin();
        while ( n != (*i).second.end() )
        {
            CORE::CString path = (*i).first;
            CORE::AppendToPath( path, (*n) );
            path = ConvertEnvVarStrings( path );

            path = path.ReplaceChar( '\\', '/' );

            if ( first )
            {
                newSectionContent += "  \"" + path + '\"';
                first = false;
            }
            else
            {
                newSectionContent += ",\n  \"" + path + '\"';
            }
            ++n;
        }
        ++i;
    }

    newSectionContent += "\n } )\n\n";
    return newSectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5FileIncludeSection( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                    const CORE::CString& consensusModuleName   )
{GUCEF_TRACE;

    CORE::CString sectionContent;
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const TStringSetMap& includeFiles = (*i).second.includeDirs;
        if ( !includeFiles.empty() )
        {
            sectionContent = "\n\nconfiguration( {} )\nvpaths { [\"Headers\"] = { \"**.h\", \"**.hpp\", \"**.hxx\" } }\n";
            sectionContent += GeneratePremake5FileSection( sectionContent, includeFiles );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed " + CORE::ToString( includeFiles.size() ) + " include file dirs for module " + consensusModuleName );
            return sectionContent;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The module " + consensusModuleName + " does not have include file directories which apply to all platforms, skipping general include section" );
        }
    }

    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5FileSrcSection( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                const CORE::CString& consensusModuleName   )
{GUCEF_TRACE;

    CORE::CString sectionContent;
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const TStringSetMap& srcFiles = (*i).second.sourceDirs;
        if ( !srcFiles.empty() )
        {
            sectionContent = "\n\nconfiguration( {} )\nvpaths { [\"Source\"] = { \"**.c\", \"**.cpp\", \"**.cs\", \"**.asm\" } }\n";
            sectionContent += GeneratePremake5FileSection( sectionContent, srcFiles );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Processed " + CORE::ToString( srcFiles.size() ) + " source dirs for module " + consensusModuleName );
            return sectionContent;
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "The module " + consensusModuleName + " does not have source directories which apply to all platforms, skipping general source section" );
        }
    }
    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

void
GeneratePremake5FilePlatformFilesSection( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                          const CORE::CString& platformName          ,
                                          CORE::CString& headerSection               ,
                                          CORE::CString& sourceSection               ,
                                          bool& hasPlatformHeaderFiles               ,
                                          bool& hasPlatformSourceFiles               )
{GUCEF_TRACE;

    TModuleInfoMap::const_iterator m = moduleInfoEntry->modulesPerPlatform.find( platformName );
    if ( m != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const TStringSetMap& platformHeaderFiles = (*m).second.includeDirs;
        if ( !platformHeaderFiles.empty() )
        {
            hasPlatformHeaderFiles = true;
            headerSection = "    vpaths { [\"Platform Headers\"] = { \"**.h\", \"**.hpp\", \"**.hxx\" } }\n    files( {\n";

            bool first = true;
            TStringSetMap::const_iterator n = platformHeaderFiles.begin();
            while ( n != platformHeaderFiles.end() )
            {
                const TStringSet& platformHeaderFilesDir = (*n).second;
                TStringSet::const_iterator i = platformHeaderFilesDir.begin();
                while ( i != platformHeaderFilesDir.end() )
                {
                    CORE::CString path = (*n).first;
                    CORE::AppendToPath( path, (*i) );
                    path = path.ReplaceChar( '\\', '/' );

                    if ( first )
                    {
                        headerSection += "      \"" + path + '\"';
                        first = false;
                    }
                    else
                    {
                        headerSection += ",\n      \"" + path + '\"';
                    }
                    ++i;
                }
                ++n;
            }
            headerSection += "\n    } )\n\n";

        }

        const TStringSetMap& platformSourceFiles = (*m).second.sourceDirs;
        if ( !platformSourceFiles.empty() )
        {
            hasPlatformSourceFiles = true;
            sourceSection = "    vpaths { [\"Platform Source\"] = { \"**.c\", \"**.cpp\", \"**.cs\", \"**.asm\" } }\n    files( {\n";

            bool first = true;
            TStringSetMap::const_iterator n = platformSourceFiles.begin();
            while ( n != platformSourceFiles.end() )
            {
                const TStringSet& platformSourceFilesDir = (*n).second;
                TStringSet::const_iterator i = platformSourceFilesDir.begin();
                while ( i != platformSourceFilesDir.end() )
                {
                    CORE::CString path = (*n).first;
                    CORE::AppendToPath( path, (*i) );
                    path = path.ReplaceChar( '\\', '/' );

                    if ( first )
                    {
                        sourceSection += "      \"" + path + '\"';
                        first = false;
                    }
                    else
                    {
                        sourceSection += ",\n      \"" + path + '\"';
                    }
                    ++i;
                }
                ++n;
            }
            sourceSection += "\n    } )\n\n";
        }
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5FilePlatformFilesSection( const CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    bool hasPlatformHeaderFiles = false;
    bool hasPlatformSourceFiles = false;

    CORE::CString sectionContent;
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.begin();
    while ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        CORE::CString headerSection;
        CORE::CString sourceSection;
        const CORE::CString& platformName = (*i).first;

        if ( AllPlatforms != platformName && !platformName.IsNULLOrEmpty() )
        {
            GeneratePremake5FilePlatformFilesSection( moduleInfoEntry        ,
                                                      platformName           ,
                                                      headerSection          ,
                                                      sourceSection          ,
                                                      hasPlatformHeaderFiles ,
                                                      hasPlatformSourceFiles );

            if ( !headerSection.IsNULLOrEmpty() || !sourceSection.IsNULLOrEmpty() )
            {
                sectionContent += "\n\nconfiguration( { \"" + platformName.Uppercase() + "\" } )\n" + headerSection + sourceSection;
            }
        }
        ++i;
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleIncludesSection( const CModuleInfo& moduleInfo ,
                                       const CORE::CString& rootDir  )
{GUCEF_TRACE;

    // Add include dirs for each dependency we know about
    bool first = true;
    CORE::CString allRelDependencyPaths;
    const TStringSet& includeDirs = moduleInfo.dependencyIncludeDirs;
    TStringSet::const_iterator i = includeDirs.begin();
    while ( i != includeDirs.end() )
    {
        if ( first )
        {
            allRelDependencyPaths += " \"" + ConvertEnvVarStrings( (*i) ) + '\"';
            first = false;
        }
        else
        {
            allRelDependencyPaths += ", \"" + ConvertEnvVarStrings( (*i) ) + '\"';
        }
        ++i;
    }

    // Add all the regular include dirs for this module
    TStringSetMap::const_iterator n = moduleInfo.includeDirs.begin();
    while ( n != moduleInfo.includeDirs.end() )
    {
        CORE::CString includeDir = ConvertEnvVarStrings( (*n).first ).ReplaceChar( '\\', '/' );
        if ( 0 != includeDir.Length() )
        {
            if ( first )
            {
                allRelDependencyPaths += " \"" + includeDir + '\"';
                first = false;
            }
            else
            {
                allRelDependencyPaths += ", \"" + includeDir + '\"';
            }
        }
        else
        {
            // Check if there is more then one include dir
            // If so we have create an include for an empty include dir
            // to ensure files in subdirs can include the file with the zero length
            // subdir.
            if ( 1 < moduleInfo.includeDirs.size() )
            {
                if ( first )
                {
                    allRelDependencyPaths += " \"../" + CORE::LastSubDir( rootDir ) + '\"';
                    first = false;
                }
                else
                {
                    allRelDependencyPaths += ", \"../" + CORE::LastSubDir( rootDir ) + '\"';
                }
            }
        }
        ++n;
    }

    CORE::CString sectionContent;
    if ( allRelDependencyPaths.Length() > 0 )
    {
        sectionContent = "includedirs( {" + allRelDependencyPaths + " } )\n";
    }
    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleIncludesSection( const CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    CORE::CString sectionContent;

    // First add the include section which applies to all platforms
    // it should not have an 'if' check around it
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        sectionContent += "\nconfiguration( {} )\n";
        sectionContent += GeneratePremake5ModuleIncludesSection( (*i).second, moduleInfoEntry->rootDir );
    }

    // Now add the include paths which are platform specific
    i = moduleInfoEntry->modulesPerPlatform.begin();
    while ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;
        if ( platformName != AllPlatforms )
        {
            CORE::CString platformSection = GeneratePremake5ModuleIncludesSection( (*i).second, moduleInfoEntry->rootDir );
            if ( platformSection.Length() > 0 )
            {
                sectionContent += "\nconfiguration( { \"" + platformName.Uppercase() + "\" } )\n" + platformSection;
            }
        }
        ++i;
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5AutoGenertedSeperator( bool end )
{GUCEF_TRACE;

    CORE::CString seperator = "\n\n\n--********";
    if ( end )
    {
        seperator += " END OF AUTOGENERATED CONTENT ";
    }
    else
    {
        seperator += " START OF AUTOGENERATED CONTENT ";
    }
    seperator += "**********\n\n\n";
    return seperator;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleDescriptionLine( const CModuleInfo& moduleInfo     ,
                                       const CORE::CString& moduleName   ,
                                       const CORE::CString& platformName )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 module description line for module " + moduleName + " and platform " + platformName );

    CORE::CString platformPrefix;
    if ( platformName == AllPlatforms )
    {
        platformPrefix = "\n\nconfiguration( {} )\n";
    }
    else
    {
        platformPrefix = "\n\nconfiguration( { " + platformName.Uppercase() + " } )\n";
    }

    switch ( moduleInfo.moduleType )
    {
        case MODULETYPE_EXECUTABLE:
        {
            if ( platformName == "win32" || platformName == "win64" )
            {
                return platformPrefix + "kind( \"WindowedApp\" )\n";
            }
            else
            {
                return platformPrefix + "kind( \"ConsoleApp\" )\n";
            }
        }
        case MODULETYPE_SHARED_LIBRARY:
        {
            return platformPrefix + "kind( \"SharedLib\" )\n";
        }
        case MODULETYPE_STATIC_LIBRARY:
        {
            return platformPrefix + "kind( \"StaticLib\" )\n";
        }
        case MODULETYPE_UNKNOWN:
        case MODULETYPE_UNDEFINED:
        default:
        {
            return CORE::CString();
        }
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleDependenciesLine( const CProjectInfo& projectInfo   ,
                                        const CModuleInfo& moduleInfo     ,
                                        const CORE::CString& moduleName   ,
                                        const CORE::CString& platformName )
{GUCEF_TRACE;

    if ( !moduleInfo.dependencies.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 module dependencies line for module " + moduleName + " and platform " + platformName );

        TStringSet dependencies;
        TStringSet::const_iterator i = moduleInfo.dependencies.begin();
        while ( i != moduleInfo.dependencies.end() )
        {
            // We add all dependencies except for header include locations which are not real modules
            // and Premake5 will not be using a make file for those.
            const CModuleInfoEntryPtr dependencyModule = GetModuleInfoEntry( projectInfo, (*i), AllPlatforms );
            if ( !dependencyModule.IsNULL() )
            {
                TModuleType moduleType = GetModuleType( dependencyModule, AllPlatforms );
                if ( ( MODULETYPE_HEADER_INCLUDE_LOCATION != moduleType )   &&
                     ( MODULETYPE_HEADER_INTEGRATE_LOCATION != moduleType ) &&
                     ( MODULETYPE_CODE_INTEGRATE_LOCATION != moduleType )    )
                {
                    dependencies.insert( (*i) );
                }
            }
            else
            {
                dependencies.insert( (*i) );
            }
            ++i;
        }

        if ( !dependencies.empty() )
        {
            bool first = true;
            CORE::CString sectionContent;
            if ( platformName == AllPlatforms )
            {
                sectionContent = "\nconfiguration( {} )\nlinks( {";
            }
            else
            {
                sectionContent = "\nconfiguration( { " + platformName.Uppercase() + " } )\nlinks( {";
            }

            // The reason we first put the dependency strings in a set is to sort them alphabetically.
            // this way the output remains the same regardless of what order the modules were processed in.
            TStringSet::iterator n = dependencies.begin();
            while ( n != dependencies.end() )
            {
                if ( first )
                {
                    sectionContent += " \"" + ConvertEnvVarStrings( (*n) ) + '\"';
                    first = false;
                }
                else
                {
                    sectionContent += ", \"" + ConvertEnvVarStrings( (*n) ) + '\"';
                }
                ++n;
            }
            sectionContent += " } )\n";
            return sectionContent;
        }
    }

    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleLinkerLine( const CModuleInfo& moduleInfo     ,
                                  const CORE::CString& moduleName   ,
                                  const CORE::CString& platformName )
{GUCEF_TRACE;

    if ( !moduleInfo.linkerSettings.linkedLibraries.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 module linker line for module " + moduleName + " and platform " + platformName );

        bool first = true;
        CORE::CString sectionContent = "links( {";
        TLinkedLibrarySettingsMap::const_iterator i = moduleInfo.linkerSettings.linkedLibraries.begin();
        while ( i != moduleInfo.linkerSettings.linkedLibraries.end() )
        {
            if ( first )
            {
                sectionContent += " \"" + ConvertEnvVarStrings( (*i).first ) + '\"';
                first = false;
            }
            else
            {
                sectionContent += ", \"" + ConvertEnvVarStrings( (*i).first ) + '\"';
            }
            ++i;
        }
        sectionContent += " } )\n";
        return sectionContent;
    }

    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleDefinesLine( const CModuleInfo& moduleInfo     ,
                                   const CORE::CString& moduleName   ,
                                   const CORE::CString& platformName )
{GUCEF_TRACE;

    if ( !moduleInfo.preprocessorSettings.defines.empty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 module preprocessor defines for module " + moduleName + " and platform " + platformName );

        CORE::CString sectionContent;
        if ( platformName == AllPlatforms )
        {
            sectionContent = "\n\nconfiguration( {} )\ndefines( { ";
        }
        else
        {
            sectionContent = "\n\nconfiguration( { " + platformName.Uppercase() + " } )\ndefines( { ";
        }

        bool first = true;
        TStringSet::const_iterator i = moduleInfo.preprocessorSettings.defines.begin();
        while ( i != moduleInfo.preprocessorSettings.defines.end() )
        {
            if ( first )
            {
                sectionContent += "\"" + ConvertEnvVarStrings( (*i) ) + "\"";
                first = false;
            }
            else
            {
                sectionContent += ", \"" + ConvertEnvVarStrings( (*i) ) + "\"";
            }
            ++i;
        }
        sectionContent += " } )\n";
        return sectionContent;
    }

    return CORE::CString();
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleNameSection( const CModuleInfoEntryPtr& moduleInfoEntry )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 section which defines the module name depending on the platform flags" );

    CORE::CString sectionContent;

    // First get a list of platforms for which a platform specific module name is defined
    TConstModuleInfoPtrMap moduleNameMap;
    GetModuleInfoWithUniqueModuleNames( moduleInfoEntry, moduleNameMap );

    // Now we will use this list to define the module name CMake variable
    bool platformAdded = false;
    TConstModuleInfoPtrMap::iterator i = moduleNameMap.begin();
    while ( i != moduleNameMap.end() )
    {
        const CORE::CString& platformName = (*i).first;
        const CORE::CString& moduleName = (*i).second->name;

        if ( platformName != AllPlatforms )
        {
            sectionContent += "\nconfiguration( { \"" +  platformName.Uppercase() + "\" } )\n  project( \"" + moduleName + "\" )\n";
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "module name = " + moduleName + " for platform " + platformName );
            platformAdded = true;
        }
        ++i;
    }

    i = moduleNameMap.find( AllPlatforms );
    if ( i != moduleNameMap.end() )
    {
        const CORE::CString& platformName = (*i).first;
        const CORE::CString& moduleName = (*i).second->name;

        if ( platformAdded )
        {
            sectionContent += "\nconfiguration( {} )\n  project( \"" + moduleName + "\" )\n";
        }
        else
        {
            sectionContent += "\nproject( \"" + moduleName + "\" )\n";
        }

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "module name = " + moduleName + " for platform " + platformName );
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleDescriptionSection( const CModuleInfoEntryPtr& moduleInfoEntry  ,
                                          const CORE::CString& consensusModuleName    )
{GUCEF_TRACE;

    // This bit of information is not additive and is in fact mutually exclusive per platform
    // As such we have to encompass each different definition in an if-else
    CORE::CString sectionContent;
    bool platformAdded = false;
    TConstModuleInfoPtrMap moduleTypeMap;
    GetModuleInfoWithUniqueModulesTypes( moduleInfoEntry, moduleTypeMap );
    TConstModuleInfoPtrMap::iterator n = moduleTypeMap.begin();
    while ( n != moduleTypeMap.end() )
    {
        const CORE::CString& platformName = (*n).first;
        const CModuleInfo* moduleInfo = (*n).second;

        if ( platformName != AllPlatforms )
        {
            sectionContent += "configuration( { \"" + platformName.Uppercase() + "\" } )\n" + GeneratePremake5ModuleDescriptionLine( *moduleInfo, consensusModuleName, platformName );
        }
        ++n;
    }

    n = moduleTypeMap.find( AllPlatforms );
    if ( n != moduleTypeMap.end() )
    {
        const CModuleInfo* moduleInfo = (*n).second;

        if ( platformAdded )
        {
            // This module has platform module descriptions which override the AllPlatforms version which we will define here
            sectionContent += "configuration( {} )\n  " + GeneratePremake5ModuleDescriptionLine( *moduleInfo, consensusModuleName, AllPlatforms ) + "\n";
        }
        else
        {
            // This module only has a AllPlatforms description and no deviating description for any platform
            // AllPlatforms means we also support MS windows. On that platform we still want to use the WinMain
            // entry point.
            if ( moduleInfo->moduleType == MODULETYPE_EXECUTABLE )
            {
                sectionContent += "configuration( { \"WIN32\" } )\n" +
                                    GeneratePremake5ModuleDescriptionLine( *moduleInfo, consensusModuleName, "win32" ) +
                                  "configuration( { \"NOT WIN32\" } )\n  " +
                                    GeneratePremake5ModuleDescriptionLine( *moduleInfo, consensusModuleName, AllPlatforms );
            }
            else
            {
                sectionContent += GeneratePremake5ModuleDescriptionLine( *moduleInfo, consensusModuleName, AllPlatforms );
            }
        }
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleLanguageSection( const CModuleInfoEntryPtr& moduleInfoEntry ,
                                       const CORE::CString& consensusName         )
{GUCEF_TRACE;

    CORE::CString sectionContent;

    CORE::CString allPlatformsLanguage;
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        allPlatformsLanguage = GetLanguageForModule( (*i).second );
        sectionContent = "\nconfiguration( {} )\nlanguage( \"" + allPlatformsLanguage + "\" )\n";
    }

    i = moduleInfoEntry->modulesPerPlatform.begin();
    while ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;

        if ( platformName != AllPlatforms )
        {
            CORE::CString language;

            // We have to combine the language of the platform independent code with the platform specific code to
            // throw the widest net required between them
            if ( !allPlatformsLanguage.IsNULLOrEmpty() )
            {
                if ( "C#" == allPlatformsLanguage )
                {
                    language = "C#";
                }
                else
                if ( "C++" == allPlatformsLanguage )
                {
                    language = "C++";
                }
                else
                if ( "C" == allPlatformsLanguage )
                {
                    CORE::CString platformLanguage = GetLanguageForModule( (*i).second );
                    if ( "C++" == platformLanguage )
                    {
                        language = "C++";
                    }
                }
            }
            else
            {
                language = GetLanguageForModule( (*i).second );
            }

            // No need to specify something that did not change, reduce clutter
            if ( !language.IsNULLOrEmpty() && language != allPlatformsLanguage )
            {
                sectionContent += "\nconfiguration( { \"" + platformName.Uppercase() + "\" } )\nlanguage( \"" + language + "\" )\n";
            }
        }
        ++i;
    }
    return sectionContent + '\n';
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleTargetNameLine( const CModuleInfo& moduleInfo     ,
                                      const CORE::CString& moduleName   ,
                                      const CORE::CString& platformName )
{GUCEF_TRACE;

    if ( !moduleInfo.linkerSettings.targetName.IsNULLOrEmpty() )
    {
        // Name is the target name unless you want something else, so no need to specify unless they are different
        if ( moduleInfo.linkerSettings.targetName != moduleInfo.name )
        {
            if ( platformName != AllPlatforms )
            {
                return "\nconfiguration( { \"" + platformName.Uppercase() + "\" } )\ntargetname( \"" + moduleInfo.linkerSettings.targetName + "\" )\n";
            }
            return "\nconfiguration( {} )\ntargetname( \"" + moduleInfo.linkerSettings.targetName + "\" )\n";
        }
    }
    return CORE::CString();
} 

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleInfoSection( const CProjectInfo& projectInfo            ,
                                   const CModuleInfoEntryPtr& moduleInfoEntry ,
                                   const CORE::CString& premakeOutputDir      ,
                                   const CORE::CString&  premakeTargetDir     )
{GUCEF_TRACE;

    CORE::CString consensusName = GetConsensusModuleName( moduleInfoEntry );

    // First we define the module name which can differ per platform
    CORE::CString sectionContent = GeneratePremake5ModuleNameSection( moduleInfoEntry );

    // Set the output path for the premake5 generated files
    if ( !premakeOutputDir.IsNULLOrEmpty() )
    {
        // Check to see if an environment variable is desired
        if ( 0 == premakeOutputDir.HasSubstr( "ENVVAR:", true ) )
        {
            // The path specified is actually a directive to use the given environment variable
            CORE::CString premakeOutputDirEnvVar = premakeOutputDir.CutChars( 7, true );
            sectionContent += "\nconfiguration( {} )\n  location( os.getenv( \"" + premakeOutputDirEnvVar + "\" ) )\n";
        }
        else
        {
            CORE::CString pathToOutputDir = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->rootDir, premakeOutputDir );
            pathToOutputDir = pathToOutputDir.ReplaceChar( '\\', '/' );
            sectionContent += "\nconfiguration( {} )\n  location( \"" + pathToOutputDir + "\" )\n";
        }
    }

    // Set the target path for the premake5 based builds
    if ( !premakeTargetDir.IsNULLOrEmpty() )
    {
        // Check to see if an environment variable is desired
        if ( 0 == premakeTargetDir.HasSubstr( "ENVVAR:", true ) )
        {
            // The path specified is actually a directive to use the given environment variable
            CORE::CString premakeTargetDirEnvVar = premakeTargetDir.CutChars( 7, true );
            sectionContent += "\nconfiguration( {} )\n  targetdir( os.getenv( \"" + premakeTargetDirEnvVar + "\" ) )\n";
        }
        else
        {
            CORE::CString pathToOutputDir = CORE::GetRelativePathToOtherPathRoot( moduleInfoEntry->rootDir, premakeTargetDir );
            sectionContent += "\nconfiguration( {} )\n  targetdir( \"" + pathToOutputDir + "\" )\n";
        }
    }

    sectionContent += GeneratePremake5ModuleLanguageSection( moduleInfoEntry, consensusName );

    // Add the module description which says what type of module this is
    sectionContent += GeneratePremake5ModuleDescriptionSection( moduleInfoEntry, consensusName );

    // Add module info which is additive meaning AllPlatforms info can be
    // supplemented with it and it is not mutually exclusive
    TModuleInfoMap::const_iterator i = moduleInfoEntry->modulesPerPlatform.find( AllPlatforms );
    if ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;
        const CModuleInfo& moduleInfo = (*i).second;

        // Generate the different instructions for all platforms (if any exist)
        CORE::CString moduleDependenciesStr = GeneratePremake5ModuleDependenciesLine( projectInfo, moduleInfo, consensusName, platformName );
        CORE::CString moduleLinkingStr = GeneratePremake5ModuleLinkerLine( moduleInfo, consensusName, platformName );
        CORE::CString moduleDefinesStr = GeneratePremake5ModuleDefinesLine( moduleInfo, consensusName, platformName );
        CORE::CString moduleTargetNameStr = GeneratePremake5ModuleTargetNameLine( moduleInfo, consensusName, platformName );

        // make sure we actually have any instructions
        if ( !moduleDependenciesStr.IsNULLOrEmpty() ||
             !moduleLinkingStr.IsNULLOrEmpty()      ||
             !moduleDefinesStr.IsNULLOrEmpty()      ||
             !moduleTargetNameStr.IsNULLOrEmpty()    )
        {
            sectionContent += ( moduleDependenciesStr + moduleLinkingStr + moduleDefinesStr + moduleTargetNameStr );
        }
    }

    // Now do the same for all adative platform specific info
    i = moduleInfoEntry->modulesPerPlatform.begin();
    while ( i != moduleInfoEntry->modulesPerPlatform.end() )
    {
        const CORE::CString& platformName = (*i).first;

        if ( platformName != AllPlatforms )
        {
            const CModuleInfo& moduleInfo = (*i).second;

            // Generate the different instructions for this platform (if any exist)
            CORE::CString moduleDependenciesStr = GeneratePremake5ModuleDependenciesLine( projectInfo, moduleInfo, consensusName, platformName );
            CORE::CString moduleLinkingStr = GeneratePremake5ModuleLinkerLine( moduleInfo, consensusName, platformName );
            CORE::CString moduleDefinesStr = GeneratePremake5ModuleDefinesLine( moduleInfo, consensusName, platformName );
            CORE::CString moduleTargetNameStr = GeneratePremake5ModuleTargetNameLine( moduleInfo, consensusName, platformName );

            // Encompass inside an if section if we have any instructions
            if ( !moduleDependenciesStr.IsNULLOrEmpty() ||
                 !moduleLinkingStr.IsNULLOrEmpty()      ||
                 !moduleDefinesStr.IsNULLOrEmpty()      ||
                 !moduleTargetNameStr.IsNULLOrEmpty()    )
            {
                CORE::CString platformSection;

                platformSection.Clear();// = "\nconfiguration( { \"" + platformName.Uppercase() + "\" } )\n";

                if ( !moduleDependenciesStr.IsNULLOrEmpty() )
                {
                    platformSection += "  " + moduleDependenciesStr;
                }
                if ( !moduleLinkingStr.IsNULLOrEmpty() )
                {
                    platformSection += "  " + moduleLinkingStr;
                }
                if ( !moduleDefinesStr.IsNULLOrEmpty() )
                {
                    platformSection += "  " + moduleDefinesStr;
                }
                if ( !moduleTargetNameStr.IsNULLOrEmpty() )
                {
                    platformSection += "  " + moduleTargetNameStr;
                }

                // Now that we finished generating the section for this platform add it to the entire info content section
                sectionContent += platformSection;
            }
        }
        ++i;
    }

    return sectionContent;
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ModuleFileContent( const CProjectInfo& projectInfo            ,
                                   const CORE::CString& premakeOutputDir      ,
                                   const CORE::CString& premakeTargetDir      ,
                                   const CModuleInfoEntryPtr& moduleInfoEntry ,
                                   bool addCompileDate = false                )
{GUCEF_TRACE;

    // Set file header comment section
    CORE::CString fileContent = GetPremake5FileHeader( addCompileDate );

    // Determine the general consensus module name
    CORE::CString consensusModuleName = GetConsensusModuleName( moduleInfoEntry );

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Generating Premake5 content for module " + consensusModuleName );

    // Set project name comment section
    fileContent += "\n-- Configuration for module: " + consensusModuleName + "\n\n";

    // the module description, dependencies, definitions, etc.
    fileContent += GeneratePremake5ModuleInfoSection( projectInfo, moduleInfoEntry, premakeOutputDir, premakeTargetDir );

    // Add all the general include files
    fileContent += GeneratePremake5FileIncludeSection( moduleInfoEntry, consensusModuleName );

    // Add all the source files
    fileContent += GeneratePremake5FileSrcSection( moduleInfoEntry, consensusModuleName );

    // Add all platform files, headers and source
    fileContent += GeneratePremake5FilePlatformFilesSection( moduleInfoEntry );

    CORE::CString additionFileContent ;//= LoadPremake5AdditionFileFromDisk( moduleInfoEntry );
    if ( !additionFileContent.IsNULLOrEmpty() )
    {
        fileContent += GeneratePremake5AutoGenertedSeperator( true );
        fileContent += additionFileContent;
        fileContent += GeneratePremake5AutoGenertedSeperator( false );
    }

    // Add all the include directories for this module
    fileContent += GeneratePremake5ModuleIncludesSection( moduleInfoEntry );

    // Add the templated addition section which is halfway between a completely manually defined
    // addition and completely autogenerated content
    fileContent += GeneratePremake5TemplatedAdditionSection( moduleInfoEntry );

    // We are done generating the content for the CMake file
    return fileContent;
}

/*---------------------------------------------------------------------------*/

void
WritePremake5ModuleFilesToDisk( const CProjectInfo& projectInfo       ,
                                const CORE::CString& premakeOutputDir ,
                                const CORE::CString& premakeTargetDir ,
                                const CORE::CString& logFilename      ,
                                bool addCompileDate = false           )
{GUCEF_TRACE;

    // Write all the premake5 files
    TModuleInfoEntryPtrVector::const_iterator i = projectInfo.modules.begin();
    while ( i != projectInfo.modules.end() )
    {
        const CModuleInfoEntryPtr& moduleInfoEntry = (*i);
        TModuleType allPlatformsType = GetModuleType( moduleInfoEntry, AllPlatforms );
        if ( ( MODULETYPE_HEADER_INCLUDE_LOCATION != allPlatformsType )    &&
             ( MODULETYPE_HEADER_INTEGRATE_LOCATION != allPlatformsType )  &&
             ( MODULETYPE_CODE_INTEGRATE_LOCATION != allPlatformsType )    )
        {
            CORE::CString fileContent = GeneratePremake5ModuleFileContent( projectInfo, premakeOutputDir, premakeTargetDir, moduleInfoEntry, addCompileDate );
            if ( logFilename.Length() > 0 )
            {
                fileContent += "\n-- Generator logfile can be found at: " + logFilename;
            }

            CORE::CString pathToPremake5ModuleFile = moduleInfoEntry->rootDir;
            CORE::AppendToPath( pathToPremake5ModuleFile, "premake5.lua" );

            if ( CORE::WriteStringAsTextFile( pathToPremake5ModuleFile, fileContent ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Created premake5.lua file for project dir: " + moduleInfoEntry->rootDir );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to write premake5.lua file content to disk at path " + moduleInfoEntry->rootDir );
            }
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Skipping premake5.lua generation for module of type \"HeaderIncludeLocation\"" );
        }
        ++i;
    }
}

/*---------------------------------------------------------------------------*/

CORE::CString
GeneratePremake5ProjectFileContent( const CProjectInfo& projectInfo              ,
                                    const CORE::CString& projectName             ,
                                    const CORE::CString& targetName              ,
                                    const TProjectTargetInfoMap& targetPlatforms ,
                                    const CORE::CString& outputDir               ,
                                    const CORE::CString& premakeOutputDir        )
{
    CORE::CString fileContent = GetPremake5FileHeader( false );

    // Define the start and name of a new workspace (aka ProjectGen project)
    fileContent += "\n\nworkspace( \"" + projectName + "\" )\n\n";

    // Generate the section which defines all configurations available for this Premake5 workspace
    // @TODO

    // Generate the section which defines all platforms available for this Premake5 workspace
    CORE::CString platformsSection = "  platforms( {";
    bool first = true;
    TProjectTargetInfoMap::const_iterator i = targetPlatforms.begin();
    while ( i != targetPlatforms.end() )
    {
        if ( first )
        {
            platformsSection += " \"" + (*i).first.Uppercase() + "\"";
            first = false;
        }
        else
        {
            platformsSection += ", \"" + (*i).first.Uppercase() + "\"";
        }
        ++i;
    }
    platformsSection += " } )\n\n";

    fileContent += platformsSection;

    // Set the output path for the premake5 generated files
    if ( !premakeOutputDir.IsNULLOrEmpty() )
    {
        // Check to see if an environment variable is desired
        if ( 0 == premakeOutputDir.HasSubstr( "ENVVAR:", true ) )
        {
            // The path specified is actually a directive to use the given environment variable
            CORE::CString premakeOutputDirEnvVar = premakeOutputDir.CutChars( 7, true );
            fileContent += "  location( os.getenv( \"" + premakeOutputDirEnvVar + "\" ) )\n\n";
        }
        else
        {
            CORE::CString pathToOutputDir = CORE::GetRelativePathToOtherPathRoot( outputDir, premakeOutputDir );
            fileContent += "  location( \"" + pathToOutputDir + "\" )\n\n";
        }
    }

    // Add the module includes per platform filter
    CORE::CString moduleIncludeListSection = "  --\n  -- Includes for all modules in the solution:\n  --\n";
    i = targetPlatforms.begin();
    while ( i != targetPlatforms.end() )
    {
        moduleIncludeListSection += "\nfilter \"" + (*i).first.Uppercase() + "\"\n";
        const TProjectTargetInfo& targetInfo = (*i).second;

        TModuleInfoEntryPtrSet::const_iterator n = targetInfo.modules.begin();
        while ( n != targetInfo.modules.end() )
        {
            const CModuleInfoEntryPtr& moduleInfo = (*n);
            if ( HasIndependentModuleType( moduleInfo->modulesPerPlatform ) )
            {
                CORE::CString pathToModuleDir = CORE::GetRelativePathToOtherPathRoot( outputDir, moduleInfo->rootDir );

                if ( 0 == pathToModuleDir.HasSubstr( "ENVVAR:", true ) )
                {
                    // The path specified is actually a directive to use the given environment variable
                    CORE::CString premakeOutputDirEnvVar = premakeOutputDir.CutChars( 7, true );
                    fileContent += "  location( os.getenv( \"" + premakeOutputDirEnvVar + "\" ) )\n\n";
                }
                pathToModuleDir = pathToModuleDir.ReplaceChar( '\\', '/' );
                moduleIncludeListSection += "  include( \"" + pathToModuleDir + "\" )\n";
            }
            ++n;
        }

        ++i;
    }    
   
    fileContent += moduleIncludeListSection;

    return fileContent;
}

/*---------------------------------------------------------------------------*/

void
WritePremake5ProjectFileToDisk( const CProjectInfo& projectInfo              ,
                                const CORE::CString& projectName             ,
                                const CORE::CString& targetName              ,
                                const TProjectTargetInfoMap& targetPlatforms ,
                                const CORE::CString& outputDir               ,
                                const CORE::CString& targetsOutputDir        ,
                                const CORE::CString& logFilename             ,
                                bool addCompileDate = false                  )
{
    CORE::CString fileContent = GeneratePremake5ProjectFileContent( projectInfo, 
                                                                    projectName, 
                                                                    targetName, 
                                                                    targetPlatforms, 
                                                                    outputDir, 
                                                                    targetsOutputDir );
    if ( logFilename.Length() > 0 )
    {
        fileContent += "\n-- Generator logfile can be found at: " + logFilename;
    }

    CORE::CString targetOutputDir = CORE::CombinePath( CORE::RelativePath( targetsOutputDir ), projectName );
    CORE::CString pathToPremake5ProjectFile = CORE::CombinePath( targetOutputDir, "premake5.lua" );
    if ( CORE::CreateDirs( targetOutputDir ) )
    {
        if ( CORE::WriteStringAsTextFile( pathToPremake5ProjectFile, fileContent ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Created ProjectInfo premake5.lua file for project dir: " + outputDir );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WritePremake5ProjectFileToDisk: Failed to write ProjectInfo premake5.lua file content to disk at path " + outputDir );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WritePremake5ProjectFileToDisk: Failed to create dirs for path: " + targetOutputDir );
    }
}

/*--------------------------------------------------------------------------*/

void
WritePremake5TargetsToDisk( const CProjectInfo& projectInfo         ,
                            const TProjectTargetInfoMapMap& targets ,
                            const CORE::CString& outputDir          ,
                            const CORE::CString& targetsOutputDir   ,
                            bool addCompileDate                     ,
                            bool splitPremakeTargets                ,
                            bool addLogfilePathToOutput             ,
                            const CORE::CString& logfilePath        )  
{GUCEF_TRACE;

    CORE::CString logfile;
    if ( addLogfilePathToOutput )
        logfile = logfilePath;    
    
    if ( splitPremakeTargets )
    {
        TProjectTargetInfoMapMap::const_iterator t = targets.begin();
        while ( t != targets.end() )
        {
            CORE::CString targetName = GetConsensusTargetName( (*t).second );
            if ( targetName.IsNULLOrEmpty() )
                targetName = (*t).first;

            WritePremake5ProjectFileToDisk( projectInfo      ,
                                            (*t).first       ,
                                            targetName       ,
                                            (*t).second      ,   
                                            outputDir        ,
                                            targetsOutputDir ,
                                            logfile          ,
                                            addCompileDate   );
            ++t;
        }
    }
    else
    {
        TProjectTargetInfoMapMap::const_iterator t = targets.find( projectInfo.projectName );
        if ( t != targets.end() )
        {
            WritePremake5ProjectFileToDisk( projectInfo             ,
                                            projectInfo.projectName ,
                                            projectInfo.projectName ,
                                            (*t).second             ,
                                            outputDir               ,
                                            outputDir               ,
                                            logfile                 ,
                                            addCompileDate          );
        }
    }
}

/*--------------------------------------------------------------------------*/

CPremake5ProjectGenerator::CPremake5ProjectGenerator( void )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CPremake5ProjectGenerator::~CPremake5ProjectGenerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPremake5ProjectGenerator::GenerateProject( const CProjectInfo& projectInfo      ,
                                            const CORE::CString& outputDir       ,
                                            bool addGeneratorCompileTimeToOutput ,
                                            const CORE::CValueList& params       )
{GUCEF_TRACE;

    // Prepare by loading up templates if any where provided
    CORE::CString templateDir = params.GetValueAlways( "premake5gen:TemplateDir" );
    if ( !templateDir.IsNULLOrEmpty() )
    {
        LoadPremake5FileAdditionTemplates( templateDir );
    }

    CORE::CString premakeOutputDir = params.GetValueAlways( "premake5gen:PM5OutputDir" );
    CORE::CString premakeTargetDir = params.GetValueAlways( "premake5gen:PM5TargetDir" );
    bool splitTargets = CORE::StringToBool( params.GetValueAlways( "premake5gen:SplitTargets" ), true );
    bool treatTagsAsTargets = CORE::StringToBool( params.GetValueAlways( "TreatTagsAsTargets" ), true );

    CORE::CString logfilePath;
    bool addLogfilePathToOutput = CORE::StringToBool( params.GetValueAlways( "writeLogLocationToOutput" ) );
    if ( addLogfilePathToOutput )
    {
        logfilePath = params.GetValueAlways( "logfile" );
    }

    // Write the gathered info to disk in premake5 format
    WritePremake5ModuleFilesToDisk( projectInfo, premakeOutputDir, premakeTargetDir, logfilePath, addGeneratorCompileTimeToOutput );

    // Now we tie the different modules together into different Premake5 projects as targets
    // This next step is especially usefull for large codebases where various projects are intertwined but
    // someone who obtained the code repository might only be interested in a much smaller subset
    // This functionality allows this audiance to only have to deal with the smaller crosssection

    CORE::CString targetsOutputDir = params.GetValueAlways( "premake5gen:TargetsDir" );
    if ( targetsOutputDir.IsNULLOrEmpty() )
        targetsOutputDir = outputDir;    
    targetsOutputDir = CORE::RelativePath( targetsOutputDir, true );
    if ( !CORE::CreateDirs( targetsOutputDir ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Failed to create Premake5 project targets output folder: " + targetsOutputDir );

    TProjectTargetInfoMapMap targets;
    SplitProjectPerTarget( projectInfo, targets, treatTagsAsTargets, true ); 

    WritePremake5TargetsToDisk( projectInfo                     , 
                                targets                         ,
                                outputDir                       , 
                                targetsOutputDir                ,
                                addGeneratorCompileTimeToOutput ,
                                splitTargets                    ,
                                addLogfilePathToOutput          ,
                                logfilePath                     );

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

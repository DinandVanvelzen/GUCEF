/*
 *  ProjectGen-VSImporter: Plugin for ProjectGen to import from Visual Studio files
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#include "ProjectGenVSImporter_CDirPreprocessor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {
namespace VSIMPORTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDirPreprocessor::CDirPreprocessor( void )
    : CIDirPreprocessor() ,
      m_xmlCodec()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDirPreprocessor::CDirPreprocessor( const CDirPreprocessor& src )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDirPreprocessor::~CDirPreprocessor()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDirPreprocessor&
CDirPreprocessor::operator=( const CDirPreprocessor& src )
{GUCEF_TRACE;

    return *this;
}

/*--------------------------------------------------------------------------*/

bool
CDirPreprocessor::InitXmlParser( void )
{GUCEF_TRACE;

    if ( m_xmlCodec.IsNULL() )
    {
        CORE::CDStoreCodecRegistry* registry = &CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
        if ( registry->TryLookup( "XML", m_xmlCodec, false ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Requested for data storage codec for xml file, succesfully obtained codec" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Requested for data storage codec for xml file, no such codec is available" );
            CORE::ShowErrorMessage( "Missing codec support",
                                    "Request for data storage codec for xml file but such codec is available!" );
        }
    }
    return !m_xmlCodec.IsNULL();
}

/*--------------------------------------------------------------------------*/

bool
CDirPreprocessor::FindProjectFiles( const CORE::CString& path ,
                                    TStringList& list         )
{GUCEF_TRACE;

    struct CORE::SDI_Data* di = CORE::DI_First_Dir_Entry( path.C_String() );
    if ( NULL != di )
    {
        do
        {
            if ( 0 != DI_Is_It_A_File( di ) )
            {
                CORE::CString name = DI_Name( di );                
                if ( "vcxproj" == CORE::ExtractFileExtention( name ).Lowercase() )
                {
                    list.push_back( name );
                }
            }
        }
        while ( 0 != CORE::DI_Next_Dir_Entry( di ) );
        CORE::DI_Cleanup( di );
    }

    return !list.empty();
}

/*--------------------------------------------------------------------------*/

PROJECTGEN::TModuleType
CDirPreprocessor::StudioConfigurationTypeToModuleType( const CORE::CString& configType )
{GUCEF_TRACE;

    if ( "StaticLibrary" == configType )
    {
        return PROJECTGEN::MODULETYPE_STATIC_LIBRARY;
    }
    else
    if ( "Application" == configType )
    {
        return PROJECTGEN::MODULETYPE_EXECUTABLE; 
    }
    else
    if ( "DynamicLibrary" == configType )
    {
        return PROJECTGEN::MODULETYPE_SHARED_LIBRARY;
    }
    return PROJECTGEN::MODULETYPE_UNKNOWN;
}

/*--------------------------------------------------------------------------*/

void
CDirPreprocessor::ParseGlobalVars( const CORE::CDataNode& projectData ,
                                   CORE::CStringMap& globals          )
{GUCEF_TRACE;

    CORE::CDataNode::TConstDataNodeSet nodes = projectData.FindChildrenOfType( "PropertyGroup", true );
    CORE::CDataNode::TConstDataNodeSet::const_iterator n = nodes.begin();
    while ( n != nodes.end() )
    {
        CORE::CString labelType = (*n)->GetAttributeValue( "Label" );
        if ( "Globals" == labelType )
        {
            CORE::CDataNode::const_iterator m = (*n)->Begin();
            while ( m != (*n)->End() )
            {
                globals[ (*m)->GetName() ] = (*m)->GetValue();                
                ++m;
            }
        }
        ++n;
    }
}

/*--------------------------------------------------------------------------*/

void
CDirPreprocessor::ParseVisualStudioVariables( const CORE::CString& testStr ,
                                              TStringSet& vars             )
{GUCEF_TRACE;

    CORE::Int32 i=0;
    CORE::Int32 subStrStart = -1;

    do
    {
        subStrStart = testStr.HasSubstr( "$(", i, true );
        if ( subStrStart >= 0 )
        {
            CORE::Int32 subStrEnd = testStr.HasChar( ')', i+2, true );
            if ( i+2 < subStrEnd )
            {
                vars.insert( testStr.SubstrFromRange( i+2, subStrEnd ) );
                i = subStrEnd + 1;
            }
        }

    }
    while ( subStrStart >= 0 );
}

/*--------------------------------------------------------------------------*/

CORE::CString
CDirPreprocessor::ReplaceVisualStudioVariables( const CORE::CString& testStr          ,
                                                const CORE::CStringMap& globals       ,
                                                bool replaceRemainderWithEnvVarLookup )
{GUCEF_TRACE;

    CORE::CString resultStr = testStr;
    
    CORE::CStringMap::const_iterator i = globals.begin();
    while ( i != globals.end() )
    {
        CORE::CString varStr = "$(" + (*i).first + ')';        
        resultStr = resultStr.ReplaceSubstr( varStr, (*i).second );
        
        ++i;
    }

    if ( replaceRemainderWithEnvVarLookup )
    {
        // remaining variables are likely to be environment variables
        TStringSet remainingVars;
        ParseVisualStudioVariables( resultStr, remainingVars );

        TStringSet::iterator n = remainingVars.begin();
        while ( n != remainingVars.end() )
        {       
            //CORE::CString envVarValue = CORE::GUCEFGetEnv( (*n).C_String() );        
            //if ( !envVarValue.IsNULLOrEmpty() )
            //{
            //    CORE::CString varStr = "$(" + (*n) + ')';
            //    resultStr = resultStr.ReplaceSubstr( varStr, envVarValue );
            //}

            // Let the generator put in lookup code instead of resolving the variable at this time
            resultStr = resultStr.ReplaceSubstr( "$(" + (*n) + ')', "$ENVVAR:" + (*n) + "$" );
            resultStr = resultStr.ReplaceSubstr( "%(" + (*n) + ')', "$ENVVAR:" + (*n) + "$" );
        
            ++n;
        }
    }
    return resultStr;
}

/*--------------------------------------------------------------------------*/

CString
CDirPreprocessor::ExtractFilename( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::Int32 dotIndex = path.HasChar( '.', false );
    if ( 0 <= dotIndex )
    {
        return CORE::ExtractFilename( path.CutChars( path.Length() - dotIndex, false ) );
    }    

    return CORE::ExtractFilename( path );
}

/*--------------------------------------------------------------------------*/

bool
CDirPreprocessor::ProccessProjectFiles( const CORE::CString& path             ,
                                        TStringList& list                     ,
                                        const CORE::CString& buildConfigToUse )
{GUCEF_TRACE;
    
    CORE::UInt32 errorCount = 0;
    
    TStringList::iterator i = list.begin();
    while ( i != list.end() )
    {
        CORE::CString projectFilePath = CORE::CombinePath( path, (*i) );
        CORE::CDataNode rootNode;
        if( !m_xmlCodec->BuildDataTree( &rootNode, projectFilePath ) ) { ++errorCount; continue; }

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Loaded project file from: " + projectFilePath );
        
        PROJECTGEN::CModuleInfoEntryPtr moduleEntry = PROJECTGEN::CModuleInfoEntry::CreateSharedObj();
        moduleEntry->rootDir = path;

        PROJECTGEN::CModuleInfoPtr moduleInfo = moduleEntry->FindModuleInfoForPlatform( "all" );
        if ( !moduleInfo.IsNULL() )
            moduleInfo->Clear();
        
        // First parse the globals so we can resolve variables in other sections
        // The $(ProjectName) var is actually derived from the filename so we handle it seperatly
        CORE::CStringMap globals;
        globals[ "ProjectName" ] = ExtractFilename( (*i) );
        ParseGlobalVars( rootNode, globals );
        
        CORE::CDataNode::TDataNodeSet nodes = rootNode.FindChildrenOfType( "PropertyGroup", true );
        CORE::CDataNode::TDataNodeSet::iterator n = nodes.begin();
        while ( n != nodes.end() )
        {
            CORE::CString labelType = (*n)->GetAttributeValue( "Label" );
            if ( "Globals" == labelType )
            {
                CORE::CString rootName = (*n)->GetChildValueByName( "RootNamespace" );
                if ( !rootName.IsNULLOrEmpty() )
                {                    
                    moduleInfo->name = rootName;
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined project name to be: " + rootName );
                }
            }
            else
            if ( "Configuration" == labelType )
            {
                CORE::CString cTypeStr = (*n)->GetChildValueByName( "ConfigurationType" );
                if ( !cTypeStr.IsNULLOrEmpty() )
                {
                    PROJECTGEN::TModuleType configurationType = StudioConfigurationTypeToModuleType( cTypeStr );
                    if ( PROJECTGEN::MODULETYPE_UNDEFINED == moduleInfo->moduleType )
                    {
                        moduleInfo->moduleType = configurationType;
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Determined project module type to be: " + ModuleTypeToString( configurationType ) );
                    }
                    else
                    if ( PROJECTGEN::MODULETYPE_UNKNOWN != configurationType && PROJECTGEN::MODULETYPE_UNKNOWN == moduleInfo->moduleType )
                    {
                        moduleInfo->moduleType = configurationType;
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Determined project module type to be: " + ModuleTypeToString( configurationType ) );
                    }
                }                
            }
            else
            if ( labelType.IsNULLOrEmpty() )
            {
                CORE::CDataNode::TDataNodeSet targetNodes = rootNode.FindChildrenOfType( "TargetName", true );
                CORE::CDataNode::TDataNodeSet::iterator m = targetNodes.begin();
                while ( m != targetNodes.end() )
                {
                    if ( buildConfigToUse == (*m)->GetAttributeValue( "Condition" ) )
                    {
                        moduleInfo->linkerSettings.SetTargetName( ReplaceVisualStudioVariables( (*m)->GetValue(), globals, false ) );
                    }
                    ++m;
                }
            }
            ++n;
        }
       
        nodes = rootNode.FindChildrenOfType( "ItemDefinitionGroup", true );
        n = nodes.begin();
        while ( n != nodes.end() )
        {
            if ( buildConfigToUse == (*n)->GetAttributeValue( "Condition" ) )
            {
                CORE::CDataNode* clCompileNode = (*n)->FindChild( "ClCompile" );
                if ( NULL != clCompileNode )
                {                
                    PROJECTGEN::TStringSet newDefines = PROJECTGEN::StringVectorToStringSet( clCompileNode->GetChildValueByName( "PreprocessorDefinitions" ).AsString().ParseElements( ';', false ) );
                    PROJECTGEN::TStringSet::iterator m = newDefines.begin();
                    while ( m != newDefines.end() ) 
                    {
                        moduleInfo->preprocessorSettings.AddDefine( ReplaceVisualStudioVariables( (*m), globals, true ) ); 
                        ++m;
                    }

                    PROJECTGEN::TStringSet additionalIncludeDirs = PROJECTGEN::StringVectorToStringSet( clCompileNode->GetChildValueByName( "AdditionalIncludeDirectories" ).AsString().ParseElements( ';', false ) );
                    m = additionalIncludeDirs.begin();
                    while ( m != additionalIncludeDirs.end() ) 
                    {
                        moduleInfo->AddIncludeDir( ReplaceVisualStudioVariables( (*m), globals, true ) );
                        ++m;
                    }
                }
                CORE::CDataNode* linkNode = (*n)->FindChild( "Link" );
                if ( NULL != linkNode )
                {
                    //PROJECTGEN::TStringSet newLibDirs = PROJECTGEN::StringVectorToStringSet( (*n)->GetChildValueByName( "AdditionalLibraryDirectories" ).ParseElements( ';', false ) );
                    //moduleInfo.linkerSettings.linkedLibraries[ ];

                    // Sometimes despite all the other variables project files have an explicit output filename
                    // In such a case we should use that name as the target name
                    CORE::CString outputFilename = linkNode->GetChildValueByName( "OutputFile" );
                    if ( !outputFilename.IsNULLOrEmpty() )
                    {
                        // Resolve variables (if any) and strip path (if any)
                        outputFilename = ReplaceVisualStudioVariables( outputFilename, globals, true );
                        outputFilename = outputFilename.ReplaceChar( '\\', '/' ).SubstrToChar( '/', false );
                        
                        // Strip the file extension (if any)
                        CORE::Int32 dotIndex = outputFilename.HasChar( '.', false );
                        if ( dotIndex >= 0 ) outputFilename = outputFilename.SubstrToIndex( dotIndex, true );
                        
                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Since the given module has an explicit Output filename set (\"" + outputFilename + "\") we will use said filename as the target name instead of \"" + moduleInfo->linkerSettings.GetTargetName() + "\"" );                        
                        moduleInfo->linkerSettings.SetTargetName( outputFilename );
                    }

                    PROJECTGEN::TStringVector additionalDependencies = linkNode->GetChildValueByName( "AdditionalDependencies" ).AsString().ParseElements( ';', false );
                    if ( !additionalDependencies.empty() )
                    {
                        PROJECTGEN::TStringVector::iterator m = additionalDependencies.begin();
                        while ( m != additionalDependencies.end() )
                        {
                            CORE::CString libraryName = ReplaceVisualStudioVariables( (*m), globals, true );
                            CLinkedLibrarySettingsPtr linkedLibrary;
                            if ( moduleInfo->linkerSettings.TryGetLinkedLibrary( libraryName, linkedLibrary, true ) )
                                linkedLibrary->SetModuleType( PROJECTGEN::MODULETYPE_UNKNOWN );
                            ++m;
                        }
                    }                
                }
                break;
            }
            ++n;
        }

        nodes = rootNode.FindChildrenOfType( "ItemGroup", true );
        n = nodes.begin();
        while ( n != nodes.end() )
        {
            CORE::CDataNode::TDataNodeSet fileNodes = (*n)->FindChildrenOfType( "ClInclude", true );
            CORE::CDataNode::TDataNodeSet::iterator m = fileNodes.begin();
            while ( m != fileNodes.end() )
            {
                // VS uses paths relative to the project file
                CORE::CString vsRelPath = (*m)->GetAttributeValue( "Include" );
                if ( 0 != vsRelPath.Length() )
                {
                    CORE::CString includeFilename = CORE::ExtractFilename( vsRelPath );
                    vsRelPath = vsRelPath.CutChars( includeFilename.Length(), false );
                    CORE::CString relPath = ReplaceVisualStudioVariables( vsRelPath, globals, true );

                    // We only want to explicitly specify files that are outside the project directory 
                    // sub structure. Files which are in the sub-structure will be auto-processed by the regular tooling code
                    // We want to keep the module definition file minimalistic and not add clutter for stuff that can be auto-resolved
                    // We cannot remove paths with variables however since we have no idea whether the file is located in the sub-folders
                    CORE::CString pathOnDisk = CORE::RelativePath( CORE::CombinePath( path, relPath ) );
                    if ( !relPath.IsNULLOrEmpty() && ( ( vsRelPath != relPath ) || !CORE::IsPathValid( pathOnDisk ) || -1 == pathOnDisk.HasSubstr( path, 0, true ) ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Explicitly adding include file to module definition because it does not appear to be in a sub-dir: " + relPath );
                        moduleInfo->AddIncludeFile( relPath, includeFilename );
                    }
                }
                ++m;
            }

            fileNodes = (*n)->FindChildrenOfType( "ClCompile", true );
            m = fileNodes.begin();
            while ( m != fileNodes.end() )
            {
                // VS uses paths relative to the project file
                CORE::CString vsRelPath = (*m)->GetAttributeValue( "Include" );
                if ( 0 != vsRelPath.Length() )
                {
                    CORE::CString sourceFilename = CORE::ExtractFilename( vsRelPath );
                    vsRelPath = vsRelPath.CutChars( sourceFilename.Length(), false );
                    CORE::CString relPath = ReplaceVisualStudioVariables( vsRelPath, globals, true );

                    // We only want to explicitly specify files that are outside the project directory 
                    // sub structure. Files which are in the sub-structure will be auto-processed by the regular tooling code
                    // We want to keep the module definition file minimalistic and not add clutter for stuff that can be auto-resolved
                    // We cannot remove paths with variables however since we have no idea whether the file is located in the sub-folders
                    CORE::CString pathOnDisk = CORE::RelativePath( CORE::CombinePath( path, relPath ) );
                    if ( !relPath.IsNULLOrEmpty() && ( ( vsRelPath != relPath ) || !CORE::IsPathValid( pathOnDisk ) || -1 == pathOnDisk.HasSubstr( path, 0, true ) ) )
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Explicitly adding source file to module definition because it does not appear to be in a sub-dir: " + relPath );
                        moduleInfo->AddSourceFile( relPath, sourceFilename );
                    }
                }
                ++m;
            }

            fileNodes = (*n)->FindChildrenOfType( "ProjectReference", true );
            m = fileNodes.begin();
            while ( m != fileNodes.end() )
            {
                CORE::CString relPath = (*m)->GetAttributeValue( "Include" );
                if ( 0 != relPath.Length() )
                {
                    CORE::CString moduleDependency = CORE::ExtractFilename( relPath );
                    CORE::Int32 dotIndex = moduleDependency.HasChar( '.', false );
                    if ( 0 <= dotIndex ) moduleDependency = moduleDependency.CutChars( moduleDependency.Length() - dotIndex, false );
                    moduleDependency = ReplaceVisualStudioVariables( moduleDependency, globals, true );

                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Module has dependency on: " + moduleDependency );
                    moduleInfo->AddNameOfDependency( moduleDependency );
                }
                ++m;
            }
            
            fileNodes = (*n)->FindChildrenOfType( "Reference", true );
            m = fileNodes.begin();
            while ( m != fileNodes.end() )
            {
                // VS uses paths relative to the project file
                CORE::CString vsRelPath = (*m)->GetAttributeValue( "Include" );
                if ( 0 != vsRelPath.Length() )
                {
                    // For now whenever we encounter a vcxproj that has references we will assume that this is a C++CLI project
                    moduleInfo->compilerSettings.AddUsedLanguage( "C++CLI" );

                    CORE::CString referenceFilename = CORE::ExtractFilename( vsRelPath );
                    vsRelPath = vsRelPath.CutChars( referenceFilename.Length(), false );
                    CORE::CString relPath = ReplaceVisualStudioVariables( vsRelPath, globals, true );

                    CLinkedLibrarySettingsPtr linkedLibrary;
                    if ( moduleInfo->linkerSettings.TryGetLinkedLibrary( referenceFilename, linkedLibrary, true ) )
                        linkedLibrary->SetModuleType( PROJECTGEN::MODULETYPE_REFERENCE_LIBRARY );

                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "The module has a managed reference to " + referenceFilename );
                }
                ++m;
            }            

            ++n;
        }

        if ( moduleInfo->name.IsNULLOrEmpty() )
        {
            CORE::Int32 dotIndex = (*i).HasChar( '.', false );
            if ( 0 <= dotIndex )
            {
                moduleInfo->name = (*i).CutChars( (*i).Length() - dotIndex, false );
            }
            else
            {
                moduleInfo->name = ExtractFilename (*i);
            }
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Module name could not be determined from xml data, using project filename as module name: " + moduleInfo->name );
        }
        if ( !moduleInfo->linkerSettings.GetTargetName().IsNULLOrEmpty() )
        {
            // Don't retain duplicate info: Only retain target name if it differs from the module name
            if ( moduleInfo->name == moduleInfo->linkerSettings.GetTargetName() )
            {
                moduleInfo->linkerSettings.SetTargetName( CORE::CString::Empty );
            }
        }

        CORE::CString moduleInfoFilePath = CORE::CombinePath( path, "ModuleInfo.xml" );

        moduleEntry->metadata.lastEditBy = "ProjectGenVSImporter";
        PROJECTGEN::SerializeModuleInfo( moduleEntry, moduleInfoFilePath );

        ++i;
    }

    return 0 == errorCount;
}

/*--------------------------------------------------------------------------*/
    
bool
CDirPreprocessor::ProccessDir( const CORE::CString& path )
{GUCEF_TRACE;

    // Find and and all project files in the dir
    TStringList projectFileList;
    if ( !FindProjectFiles( path, projectFileList ) ) return true;

    // Since we found one or more,.. we will need the xml parser
    if ( !InitXmlParser() ) return false;
    
    CORE::CString buildConfigToUse = "'$(Configuration)|$(Platform)'=='Release|Win32'";
    return ProccessProjectFiles( path, projectFileList, buildConfigToUse );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VSIMPORTER */
}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

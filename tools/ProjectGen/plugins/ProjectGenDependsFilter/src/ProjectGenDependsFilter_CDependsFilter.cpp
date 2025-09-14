/*
 *  ProjectGen-DependsFilter: Plugin for ProjectGen to leverage depends tool output
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "ProjectGenDependsFilter_CDependsFilter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {
namespace DEPFILTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDependsFilter::CDependsFilter( void )
    : CIProjectPreprocessor()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDependsFilter::CDependsFilter( const CDependsFilter& src )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDependsFilter::~CDependsFilter()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/
    
CDependsFilter&
CDependsFilter::operator=( const CDependsFilter& src )
{GUCEF_TRACE;

    return *this;
}

/*--------------------------------------------------------------------------*/

CORE::CString::StringSet
CDependsFilter::GetListOfModules( const CORE::CString::StringVector& dependsCsvFiles ,
                                  const CORE::CString::StringVector& binarySrcDirs   )
{GUCEF_TRACE;

    TStringSet modules;
    TStringVector::const_iterator i = dependsCsvFiles.begin();
    while ( i != dependsCsvFiles.end() )
    {
        CORE::CString csvContent;
        if ( CORE::LoadTextFileAsString( CORE::RelativePath( (*i) ), csvContent, true, "\n" ) )
        {
            TStringVector lines = csvContent.ParseElements( '\n', false );
            csvContent.Clear();

	        TStringVector::iterator n = lines.begin();
            
            // We need to skip the first line as its a legend
            if ( n != lines.end() ) ++n;
	        
            while ( n != lines.end() )
	        {
		        CORE::Int32 firstCommaPos = (*n).HasChar( ',', 0, true );
		        if ( -1 != firstCommaPos )
		        {
			        CORE::Int32 secondCommaPos = (*n).HasChar( ',', firstCommaPos+1, true );
			        if ( -1 != secondCommaPos )
			        {	
				        // Name also has quotes around it, lets strip those
				        CORE::Int32 nameLength = (secondCommaPos-1) - (firstCommaPos+2);
				        if ( nameLength > 0 )
				        {
					        CORE::CString moduleName = (*n).SubstrFromRange( firstCommaPos+2, firstCommaPos+2+nameLength );

                            // Strip the extention from the file name
                            Int32 dotIndex = moduleName.HasChar( '.', false );
                            moduleName = moduleName.SubstrToIndex( dotIndex, true );
                            
                            // For easy searches lets make the names lowercase.
                            // Depends is a MS Windows tool so case sensitivity is not an issue anyway
                            moduleName = moduleName.Lowercase();

					        modules.insert( moduleName );

					        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Found dependency module with name \"" + moduleName + "\"" );
				        }
			        }
		        }
		        ++n;
	        }
	        lines.clear();
        }        
        ++i;
    }

    // Check if we want to apply additional filtering using source binary dirs
    // Depends also spits out O/S dependencies etc which you might not want added
    if ( !binarySrcDirs.empty() )
    {
        TStringSet eraseList;
        TStringSet::const_iterator n = modules.begin();
        while ( n != modules.end() )
        {        
            bool moduleLocated = false;
            TStringVector::const_iterator i = binarySrcDirs.begin();
            while ( i != binarySrcDirs.end() )
            {
                CORE::CString testPath = CORE::CombinePath( CORE::RelativePath( (*i) ), (*n) );
                if ( CORE::FileExists( testPath + ".dll" ) )
                {
                    moduleLocated = true;
                    break;
                }
                else // Although bad practice some people link against the exports from executables :( so we have to support it
                if ( CORE::FileExists( testPath + ".exe" ) )
                {
                    moduleLocated = true;
                    break;
                }
                ++i;
            }

            if ( !moduleLocated )
            {
                eraseList.insert( (*n) );
            }
            ++n;
        }

        n = eraseList.begin();
        while ( n != eraseList.end() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Dropping dependency module with name \"" + (*n) + "\" because it's not found in any of the binary source dirs given" );
            modules.erase( (*n) );
            ++n;
        }
    }

    return modules;
}

/*--------------------------------------------------------------------------*/
    
bool
CDependsFilter::ProccessProjects( CProjectInfo& projectInfo      ,
                                  const CORE::CString& outputDir ,
                                  const CORE::CValueList& params )
{GUCEF_TRACE;
   
    CORE::CString filterFileStr = params.GetValueAlways( "DependsFilter:DependsOutput" );
    TStringVector dependsCsvFiles = filterFileStr.ParseElements( ';', false );

    CORE::CString binarySrcDirsStr = params.GetValueAlways( "DependsFilter:BinarySrcDirs" );
    TStringVector binarySrcDirs = binarySrcDirsStr.ParseElements( ';', false );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Executing Depends filter on the given project info. There are " + CORE::ToString( dependsCsvFiles.size() ) + " csv files given" );
    
    // Obtain a list of all modules from the Depends generated csv files.
    TStringSet modules = GetListOfModules( dependsCsvFiles, binarySrcDirs );

    // Mark all the modules from the project which are not in the modules list for deletion
    // Note that not all module types are filtered as they are not compiled into binaries for which Depends can provide a check
    TStringSet deleteList;
    TStringToModuleInfoEntryPtrMap& moduleInfoList = projectInfo.modules;
    TStringToModuleInfoEntryPtrMap::iterator i = moduleInfoList.begin(); 
    while ( i != moduleInfoList.end() )
    {
        CString targetName = GetModuleTargetName( (*i).second, "win32", true );
        
        // we will check using the target name if the module has one
        // Keep in mind that Depends would be using the target name.
        // If no target name is defines we use the module name
        TModuleType moduleType = (*i).second->GetModuleType( "win32" );
        if ( moduleType == MODULETYPE_SHARED_LIBRARY    ||
             moduleType == MODULETYPE_EXECUTABLE        ||
             moduleType == MODULETYPE_REFERENCE_LIBRARY  )
        {        
            TStringSet::iterator n = modules.find( targetName.Lowercase() );
            if ( n == modules.end() )
            {
                // The given module is not in the list of modules we obtained from depends
                // as such we should filter it out
                deleteList.insert( (*i).second->GetAbsolutePathToModuleRootDir() + ':' + targetName );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Skipping Depends check for module with target name \"" + targetName + "\" since it's type is not checkable via Depends" );
        }
        ++i;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Based on the Depends filter there are now " + CORE::ToString( deleteList.size() ) + " modules listed for deletion" );

    if ( !deleteList.empty() )
    {
        TStringToModuleInfoEntryPtrMap::iterator i = moduleInfoList.begin(); 
        while ( i != moduleInfoList.end() )
        {
            CString targetName = GetModuleTargetName( (*i).second, "win32", true );
            TStringSet::iterator n = deleteList.find( (*i).second->GetAbsolutePathToModuleRootDir() + ':' + targetName );
            if ( n != deleteList.end() )
            {
                moduleInfoList.erase( i );
                i = moduleInfoList.begin();

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Filtered out module with target name: " + targetName );
            }
            ++i;
        }

        // Since we deleted modules we should reindex the build order for the modules to remove gaps
        // @TODO
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DEPFILTER */
}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

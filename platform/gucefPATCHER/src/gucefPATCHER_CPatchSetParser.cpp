/*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#include <assert.h>
#include <map>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefPATCHER_CPatchSetParser.h"	/* definition of this class */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchSetParser::CPatchSetParser( void )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CPatchSetParser::CPatchSetParser( const CPatchSetParser& src )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CPatchSetParser::~CPatchSetParser()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CPatchSetParser&
CPatchSetParser::operator=( const CPatchSetParser& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ValidateAndParseFileLocEntries( const CORE::CDataNode& patchSetFileNode ,
                                                 TFileEntry& fileEntry                   ,
                                                 bool mustHaveLocationEntries            ) const
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchSetParser: Parsing patch set" );

    const CORE::CDataNode* locationEntry( GUCEF_NULL );
    CORE::CDataNode::const_iterator i = patchSetFileNode.ConstBegin();
    while ( i != patchSetFileNode.ConstEnd() )
    {
        locationEntry = (*i);
        if ( locationEntry->GetName().Equals( "FileLocation", false ) )
        {
            // Validate that the file location has a URL property
            const CORE::CDataNode::TKeyValuePair* attData = locationEntry->GetAttribute( "URL" );
            if ( attData != GUCEF_NULL )
            {
                TFileLocation fileLocation;
                fileLocation.URL = attData->second;

                // Check for an optional codec property
                const CORE::CDataNode::TKeyValuePair* attData = locationEntry->GetAttribute( "Codec" );
                if ( attData != GUCEF_NULL )
                {
                    fileLocation.codec = attData->second;

                    // Check for an optional codec parameter property
                    const CORE::CDataNode::TKeyValuePair* attData = locationEntry->GetAttribute( "CodecParams" );
                    if ( attData != GUCEF_NULL )
                    {
                        fileLocation.codecParams = attData->second;
                    }
                }

                // Add the file location to our list
                fileEntry.fileLocations.push_back( fileLocation );
            }
            else
            {
                return false;
            }
        }

        ++i;
    }

    if ( mustHaveLocationEntries )
    {
        // A file entry must have at least 1 file location entry
        if ( fileEntry.fileLocations.size() >= 1 )
        {
            return true;
        }

        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ValidateAndParseFileEntry( const CORE::CDataNode& patchSetFileNode ,
                                            TFileEntry& fileEntry                   ,
                                            bool mustHaveLocationEntries            ) const
{GUCEF_TRACE;

    // Validate that the file has a name property
    const CORE::CDataNode::TKeyValuePair* attData = patchSetFileNode.GetAttribute( "Name" );
    if ( attData != GUCEF_NULL )
    {
        // We found our filename
        const CORE::CString& filename = attData->second;

        // Validate that the file has a size property
        const CORE::CDataNode::TKeyValuePair* attData = patchSetFileNode.GetAttribute( "Size" );
        if ( attData != GUCEF_NULL )
        {
            // We found our file's size
            const CORE::CString& fileSizeStr = attData->second;
            UInt64 fileSize = CORE::StringToInt64( fileSizeStr );

            // Validate that the file has a hash property
            const CORE::CDataNode::TKeyValuePair* attData = patchSetFileNode.GetAttribute( "Hash" );
            if ( attData != GUCEF_NULL )
            {
                const CORE::CString& fileHash = attData->second;

                // Our file entry has everything we expected
                // We will now parse the file location entries for this file
                if ( ValidateAndParseFileLocEntries( patchSetFileNode        ,
                                                     fileEntry               ,
                                                     mustHaveLocationEntries ) )
                {
                    // Success,.. the file and it's locations have been parsed
                    fileEntry.hash = fileHash;
                    fileEntry.name = filename;
                    fileEntry.sizeInBytes = fileSize;

                    return true;
                }
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ValidateAndParseDirEntry( const CORE::CDataNode& patchSetDirNode ,
                                           TDirEntry& dirEntry                    ) const
{GUCEF_TRACE;

    // Validate that the directory has a name property
    const CORE::CDataNode::TKeyValuePair* attData = patchSetDirNode.GetAttribute( "Name" );
    if ( attData != GUCEF_NULL )
    {
        // We found our directory name
        const CORE::CString& dirName = attData->second;

        // Validate that the directory has a total size property
        const CORE::CDataNode::TKeyValuePair* attData = patchSetDirNode.GetAttribute( "TotalSize" );
        if ( attData != GUCEF_NULL )
        {
            // We found our directory's total size
            const CORE::CString& dirTotalSizeStr = attData->second;
            UInt64 totalDirSize = CORE::StringToUInt64( dirTotalSizeStr );

            // Validate that the directory has a hash property
            const CORE::CDataNode::TKeyValuePair* attData = patchSetDirNode.GetAttribute( "Hash" );
            if ( attData != GUCEF_NULL )
            {
                // Our directory entry has everything we expected
                // It can now be added to the patch set
                dirEntry.hash = attData->second;
                dirEntry.name = dirName;
                dirEntry.sizeInBytes = totalDirSize;

                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ParseAndWalkDirTree( const CORE::CDataNode& patchSetDirNode ,
                                      TDirEntry& parentDir                   ) const
{
    // The directory info has been validated and parsed
    // We now iterate the directory entries
    const CORE::CDataNode* patchSetDirEntry( GUCEF_NULL );
    CORE::CDataNode::const_iterator i = patchSetDirNode.ConstBegin();
    while ( i != patchSetDirNode.ConstEnd() )
    {
        patchSetDirEntry = (*i);
        if ( patchSetDirEntry->GetName().Equals( "Dir", false ) )
        {
            TDirEntry dirEntry;
            if ( ValidateAndParseDirEntry( *patchSetDirEntry ,
                                           dirEntry          ) )
            {
                // Add the sub-directory
                parentDir.subDirs.push_back( dirEntry );
                TDirEntry& addedDir = parentDir.subDirs.back();

                // Parse and walk the sub-dir
                if ( !ParseAndWalkDirTree( *patchSetDirEntry ,
                                           addedDir          ) )
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        if ( patchSetDirEntry->GetName().Equals( "File", false ) )
        {
            // Validate the file entry
            TFileEntry fileEntry;
            if ( ValidateAndParseFileEntry( *patchSetDirEntry ,
                                            fileEntry         ) )
            {
                // Add the file to the directory
                parentDir.files.push_back( fileEntry );
            }
            else
            {
                return false;
            }
        }

        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ParseTopLevelDir( const CORE::CDataNode& patchSetDirNode ,
                                   TPatchSet& patchSet                    ) const
{GUCEF_TRACE;

    TDirEntry dirEntry;
    if ( ValidateAndParseDirEntry( patchSetDirNode ,
                                   dirEntry        ) )
    {
        if ( ParseAndWalkDirTree( patchSetDirNode ,
                                  dirEntry        ) )
        {
            patchSet.push_back( dirEntry );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ParsePatchSet( const CORE::CDataNode& patchSetData ,
                                TPatchSet& patchSet                 ) const
{GUCEF_TRACE;

    const CORE::CDataNode* patchSetNode = patchSetData.Find( "PatchSet" );

    if ( patchSetNode != GUCEF_NULL )
    {
        // We found our patch set, now we go trough all top-level directories
        const CORE::CDataNode* patchSetEntry( GUCEF_NULL );
        CORE::CDataNode::const_iterator i = patchSetNode->ConstBegin();
        while ( i != patchSetNode->ConstEnd() )
        {
            patchSetEntry = (*i);
            if ( patchSetEntry->GetName().Equals( "Dir", false ) )
            {
                // Parse the directory
                if ( !ParseTopLevelDir( *patchSetEntry ,
                                        patchSet       ) )
                {
                    // Malformed patch set data structure found
                    return false;
                }
            }

            ++i;
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CPatchSetParser::SerializeFileEntry( const TFileEntry& fileEntry ,
                                     CORE::CDataNode& parentNode ) const
{GUCEF_TRACE;

    // Now we add the files
    CORE::CDataNode fileNode;
    fileNode.SetName( "File" );

    fileNode.SetAttribute( "Name", fileEntry.name );
    fileNode.SetAttribute( "Hash", fileEntry.hash );
    fileNode.SetAttribute( "Size", CORE::ToString( fileEntry.sizeInBytes ) );

    const TFileLocations& locList = fileEntry.fileLocations;
    const TFileLocation* fileLoc = GUCEF_NULL;
    CORE::CDataNode fileLocNode;
    fileLocNode.SetName( "FileLocation" );
    for ( UInt32 n=0; n<locList.size(); ++n )
    {
        fileLoc = &locList[ n ];
        fileLocNode.SetAttribute( "URL", fileLoc->URL );
        fileLocNode.SetAttribute( "Codec", fileLoc->codec );
        fileLocNode.SetAttribute( "CodecParams", fileLoc->codecParams );

        fileNode.AddChild( fileLocNode );
    }

    parentNode.AddChild( fileNode );
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ParseAndWalkDirTree( const TDirEntry& patchSetDir   ,
                                      CORE::CDataNode& parentDirNode ) const
{GUCEF_TRACE;

    // Set all attributes of this dir node
    CORE::CDataNode newNode;
    newNode.SetName( "Dir" );
    newNode.SetAttribute( "Name", patchSetDir.name );
    newNode.SetAttribute( "TotalSize", CORE::ToString( patchSetDir.sizeInBytes ) );
    newNode.SetAttribute( "Hash", patchSetDir.hash );

    // Recursively add all sub-directories
    for ( UInt32 i=0; i<patchSetDir.subDirs.size(); ++i )
    {
        if ( !ParseAndWalkDirTree( patchSetDir.subDirs[ i ] ,
                                   newNode                  ) )
        {
            return false;
        }
    }

    // Now we add the files
    CORE::CDataNode fileNode;
    fileNode.SetName( "File" );
    for ( UInt32 i=0; i<patchSetDir.files.size(); ++i )
    {
        SerializeFileEntry( patchSetDir.files[ i ], newNode );
    }

    // Now we copy our entire 'newNode' tree below the parent node
    parentDirNode.AddChild( newNode );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchSetParser::ParsePatchSet( const TPatchSet& patchSetData ,
                                CORE::CDataNode& patchSet     ) const
{GUCEF_TRACE;

    patchSet.SetName( "PatchSet" );
    patchSet.DelSubTree();
    patchSet.ClearAttributes();

    for ( UInt32 i=0; i<patchSetData.size(); ++i )
    {
        const TDirEntry& dirEntry = patchSetData[ i ];
        if ( !ParseAndWalkDirTree( dirEntry ,
                                   patchSet ) )
        {
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

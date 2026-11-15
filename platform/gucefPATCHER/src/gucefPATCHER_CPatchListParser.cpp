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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#include "gucefPATCHER_CPatchListParser.h"

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

CPatchListParser::CPatchListParser( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchListParser::~CPatchListParser()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPatchListParser::ParsePatchList( const CORE::CDataNode& patchListData ,
                                  TPatchList& patchList                ) const
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchListParser(" + CORE::PointerToString( this ) + "): Parsing patch list" );

    const CORE::CDataNode* patchListNode = patchListData.Find( "PatchList" );

    if ( patchListNode != GUCEF_NULL )
    {
        const CORE::CDataNode* patchListEntry( GUCEF_NULL );
        CORE::CDataNode::const_iterator i = patchListNode->ConstBegin();
        while ( i != patchListNode->ConstEnd() )
        {
            patchListEntry = (*i);
            if ( patchListEntry->GetName().Equals( "PatchSetEntry", false ) )
            {
                // Validate that the patch set has a name property
                const CORE::CDataNode::TKeyValuePair* attData = patchListEntry->GetAttribute( "Name" );
                if ( attData != GUCEF_NULL )
                {
                    // A name was found, we can continue on to parse the patch set's locations
                    CORE::CString patchSetName = attData->second;

                    const CORE::CDataNode* patchSetLocEntry( GUCEF_NULL );
                    CORE::CDataNode::const_iterator n = patchListEntry->ConstBegin();
                    while ( n != patchListEntry->ConstEnd() )
                    {
                        // Get the information we need from each location entry
                        patchSetLocEntry = (*n);
                        if ( patchSetLocEntry->GetName().Equals( "Location", false ) )
                        {
                            // Get the URL of the patch set location
                            attData = patchSetLocEntry->GetAttribute( "URL" );
                            if ( attData != GUCEF_NULL )
                            {
                                // We have a URL
                                CORE::CString patchSetLocURL = attData->second;

                                // Get the codec name for the patch set data
                                attData = patchSetLocEntry->GetAttribute( "Codec" );
                                if ( attData != GUCEF_NULL )
                                {
                                    // We have the codec name
                                    CORE::CString patchSetDataCodec = attData->second;

                                    // We have all the info we need for this location
                                    // we can now add the location to the list
                                    TPatchSetLocation location;
                                    location.codec = patchSetDataCodec;
                                    location.URL = patchSetLocURL;
                                    TPatchSetLocations& locationList = patchList[ patchSetName ];
                                    locationList.push_back( location );
                                }
                            }
                        }

                        ++n;
                    }
                }
            }

            ++i;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace PATCHER */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

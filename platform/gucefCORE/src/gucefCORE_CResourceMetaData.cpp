/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CResourceMetaData.h"          

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CResourceMetaData::CResourceMetaData( void )
    : name()
    , hasName( false )
    , version()
    , mimeType()
    , hasMimeType( false )
    , hasVersion( false )
    , creationDateTime(  )
    , hasCreationDateTime( false )
    , modifiedDateTime()
    , hasModifiedDateTime( false )
    , lastAccessedDateTime()
    , hasLastAccessedDateTime( false )
    , resourceSizeInBytes( 0 )
    , hasResourceSizeInBytes( false )
    , resourceExists( false )
    , hasIsReadOnly( false )
    , isReadOnly( false )
    , hasIsHidden( false )
    , isHidden( false )
    , hasIsSystemResource( false )
    , isSystemResource( false )
    , hasIsArchive( false )
    , isArchive( false )
    , hasIsCompressed( false )
    , isCompressed( false )
    , hasIsEncrypted( false )
    , isEncrypted( false )
    , hasIsEncoded( false )
    , isEncoded( false )
    , hasIsTemporary( false )
    , isTemporary( false )
    , hasIsOffline( false )
    , isOffline( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CResourceMetaData::CResourceMetaData( const CResourceMetaData& src )
    : name( src.name )
    , hasName( src.hasName )
    , version( src.version )
    , hasVersion( src.hasVersion )
    , mimeType( src.mimeType )
    , hasMimeType( src.hasMimeType )
    , creationDateTime( src.creationDateTime )
    , hasCreationDateTime( src.hasCreationDateTime )
    , modifiedDateTime( src.modifiedDateTime )
    , hasModifiedDateTime( src.hasModifiedDateTime )
    , lastAccessedDateTime( src.lastAccessedDateTime )
    , hasLastAccessedDateTime( src.hasLastAccessedDateTime )
    , resourceSizeInBytes( src.resourceSizeInBytes )
    , hasResourceSizeInBytes( src.hasResourceSizeInBytes )
    , resourceExists( src.resourceExists )
    , hasIsReadOnly( src.hasIsReadOnly )
    , isReadOnly( src.isReadOnly )
    , hasIsHidden( src.hasIsHidden )
    , isHidden( src.isHidden )
    , hasIsSystemResource( src.hasIsSystemResource )
    , isSystemResource( src.isSystemResource )
    , hasIsArchive( src.hasIsArchive )
    , isArchive( src.isArchive )
    , hasIsCompressed( src.hasIsCompressed )
    , isCompressed( src.isCompressed )
    , hasIsEncrypted( src.hasIsEncrypted )
    , isEncrypted( src.isEncrypted )
    , hasIsEncoded( src.hasIsEncoded )
    , isEncoded( src.isEncoded )
    , hasIsTemporary( src.hasIsTemporary )
    , isTemporary( src.isTemporary )
    , hasIsOffline( src.hasIsOffline )
    , isOffline( src.isOffline )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CResourceMetaData::Clear( void )
{GUCEF_TRACE;

    name.Clear();
    hasName = false;
    version.Clear();
    hasVersion = false;
    mimeType.Clear();
    hasMimeType = false;
    creationDateTime.Clear();
    hasCreationDateTime = false;
    modifiedDateTime.Clear();
    hasModifiedDateTime = false;
    lastAccessedDateTime.Clear();
    hasLastAccessedDateTime = false;
    resourceSizeInBytes = 0;
    hasResourceSizeInBytes = false;

    hasIsReadOnly = false;
    isReadOnly = false;
    hasIsHidden = false;
    isHidden = false;
    hasIsSystemResource = false;
    isSystemResource = false;
    hasIsArchive = false;
    isArchive = false;
    hasIsCompressed = false;
    isCompressed = false;
    hasIsEncrypted = false;
    isEncrypted = false;
    hasIsEncoded = false;
    isEncoded = false;
    hasIsTemporary = false;
    isTemporary = false;
    hasIsOffline = false;
    isOffline = false;

    resourceExists = false;
}

/*-------------------------------------------------------------------------*/

bool
CResourceMetaData::GetEarliestUtcDt( CORE::CDateTime& earliest ) const
{GUCEF_TRACE;

    if ( hasCreationDateTime && hasModifiedDateTime && hasLastAccessedDateTime )
    {
        if ( creationDateTime > modifiedDateTime )
        {
            if ( modifiedDateTime > lastAccessedDateTime )
            {
                earliest = lastAccessedDateTime.ToUTC();
                return true;
            }
            else
            {
                earliest = modifiedDateTime.ToUTC();
                return true;
            }
        }
        else
        {
            if ( creationDateTime > lastAccessedDateTime )
            {
                earliest = lastAccessedDateTime.ToUTC();
                return true;
            }
            else
            {
                earliest = creationDateTime.ToUTC();
                return true;
            }
        }
    }
    else
    if ( hasCreationDateTime && hasModifiedDateTime )
    {
        if ( creationDateTime > modifiedDateTime )
        {
            earliest = modifiedDateTime.ToUTC();
            return true;
        }
        else
        {
            earliest = creationDateTime.ToUTC();
            return true;
        }
    }
    else 
    if ( hasCreationDateTime )
    {
        earliest = creationDateTime.ToUTC();
        return true;
    }
    else 
    if ( hasModifiedDateTime )
    {
        earliest = modifiedDateTime.ToUTC();
        return true;
    }
    else 
    if ( hasLastAccessedDateTime )
    {
        earliest = lastAccessedDateTime.ToUTC();
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

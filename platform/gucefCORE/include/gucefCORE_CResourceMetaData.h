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

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#define GUCEF_CORE_CRESOURCEMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"          
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"   
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CResourceMetaData
{
    public:

    CString name;
    bool hasName;
    CString version;
    bool hasVersion;
    CString mimeType;
    bool hasMimeType;
    CDateTime creationDateTime;
    bool hasCreationDateTime;
    CDateTime modifiedDateTime;
    bool hasModifiedDateTime;
    CDateTime lastAccessedDateTime;
    bool hasLastAccessedDateTime;
    UInt64 resourceSizeInBytes;
    bool hasResourceSizeInBytes;

    bool hasIsReadOnly;
    bool isReadOnly;
    bool hasIsHidden;
    bool isHidden;
    bool hasIsSystemResource;
    bool isSystemResource;
    bool hasIsArchive;
    bool isArchive;
    bool hasIsCompressed;
    bool isCompressed;
    bool hasIsEncrypted;
    bool isEncrypted;
    bool hasIsEncoded;
    bool isEncoded;
    bool hasIsTemporary;
    bool isTemporary;
    bool hasIsOffline;
    bool isOffline;

    bool resourceExists;

    CResourceMetaData( void );
    CResourceMetaData( const CResourceMetaData& src );

    void Clear( void );

    bool GetEarliestUtcDt( CORE::CDateTime& earliest ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVERSION_H
#define GUCEF_CORE_CVERSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/**
 *  
 */
class GUCEF_CORE_PUBLIC_CPP CVersion
{
    public:

    UInt16 GetMajor( void ) const;
    UInt16 GetMinor( void ) const;
    UInt16 GetPatch( void ) const;
    UInt16 GetRelease( void ) const;

    void SetMajor( UInt16 major );
    void SetMinor( UInt16 minor );
    void SetPatch( UInt16 patch );
    void SetRelease( UInt16 release );

    const TVersion& GetCStyleVersion( void ) const;
    void SetCStyleVersion( const TVersion& version );
    
    CVersion( UInt16 major = 0   , 
              UInt16 minor = 0   , 
              UInt16 patch = 0   ,  
              UInt16 release = 0 );

    CVersion( const CVersion& src );

    CVersion( const TVersion& version );

    virtual ~CVersion();

    void Clear( void );

    bool operator<( const CVersion& other ) const;
    bool operator>( const CVersion& other ) const;
    bool operator==( const CVersion& other ) const;

    CVersion& operator=( const CVersion& other );
    CVersion& operator=( const TVersion& other );

    bool FromString( const CString& versionStr );

    CString ToString( bool includeMinor = true   ,
                      bool includePatch = true   ,
                      bool includeRelease = true ) const;

    operator CString() const;

    bool IsAllZero( void ) const;

    private:

    TVersion m_version;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVERSION_H ? */


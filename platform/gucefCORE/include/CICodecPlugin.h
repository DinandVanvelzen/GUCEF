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

#ifndef GUCEF_CORE_CICODECPLUGIN_H
#define GUCEF_CORE_CICODECPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  Interface class for plugin's that provide a number of codec's
 *
 *  Note that the CTCodecPluginItem template can be used by the class
 *  decending from this class to specify it's codec's delegation to the
 *  plugin module with ease. This allows you to use a more generic class
 *  as the utility interface and base class for the given codec family.
 */
class GUCEF_CORE_PUBLIC_CPP CICodecPlugin : public CIPlugin
{
    public:

    typedef CTSharedPtr< CICodec, MT::CMutex > CCodecPtr;
    typedef GUCEF::map< CString, CCodecPtr > CCodecFamilySet;
    typedef GUCEF::map< CString, CCodecFamilySet > CCodecSet;
    typedef GUCEF::vector< CString > CCodecFamilyList;
    typedef GUCEF::map< CString, CCodecFamilyList > CCodecList;

    public:

    CICodecPlugin( void );

    virtual ~CICodecPlugin();

    virtual bool GetCodecs( CCodecSet& codecSet ) = 0;

    virtual bool GetCodecList( CCodecList& codecList ) = 0;

    virtual bool Encode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   ) = 0;

    virtual bool Decode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   ) = 0;

    private:

    CICodecPlugin( const CICodecPlugin& src );
    CICodecPlugin& operator=( const CICodecPlugin& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CICODECPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 15-12-2006 :
        - Dinand: Designed and implemented this class.

-----------------------------------------------------------------------------*/

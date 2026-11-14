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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_PATCHER_CPATCHSETGENERATOR_H
#define GUCEF_PATCHER_CPATCHSETGENERATOR_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H  ? */

#ifndef GUCEF_PATCHER_CPATCHSETPARSER_H
#include "gucefPATCHER_CPatchSetParser.h"
#define GUCEF_PATCHER_CPATCHSETPARSER_H
#endif /* GUCEF_PATCHER_CPATCHSETPARSER_H ? */

#ifndef GUCEF_PATCHER_MACROS_H
#include "gucefPATCHER_macros.h"
#define GUCEF_PATCHER_MACROS_H
#endif /* GUCEF_PATCHER_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEFPATCHER_EXPORT_CPP CPatchSetGenerator
{
    public:

    typedef CPatchSetParser::TFileLocation TFileLocation;
    typedef CPatchSetParser::TFileEntry TFileEntry;
    typedef CPatchSetParser::TDirEntry TDirEntry;
    typedef CPatchSetParser::TPatchSet TPatchSet;
    typedef std::set< CORE::CString > TStringSet;
    
    public :
    
    CPatchSetGenerator( void );
    ~CPatchSetGenerator();
    
    bool GeneratePatchSet( const CORE::CString& localRoot             ,
                           const CORE::CString& URLRoot               ,
                           TPatchSet& patchSet                        ,
                           const TStringSet* dirsToIgnore = GUCEF_NULL      ,
                           const TStringSet* fileTypesToIgnore = GUCEF_NULL ) const;

    bool GeneratePatchSet( const CORE::CString& localRoot             ,
                           const CORE::CString& URLRoot               ,
                           CORE::CDataNode& patchSet                  ,
                           const TStringSet* dirsToIgnore = GUCEF_NULL      ,
                           const TStringSet* fileTypesToIgnore = GUCEF_NULL ) const;

    bool GeneratePatchSet( const CORE::CString& localRoot             ,
                           const CORE::CString& URLRoot               ,
                           const CORE::CString& storageCodec          ,
                           CORE::CIOAccess& patchSetStorage           ,
                           const TStringSet* dirsToIgnore = GUCEF_NULL      ,
                           const TStringSet* fileTypesToIgnore = GUCEF_NULL ) const;
                           
    private:
    
    CPatchSetGenerator( const CPatchSetGenerator& src );
    CPatchSetGenerator& operator=( const CPatchSetGenerator& src );
    
    bool GeneratePatchSet( const CORE::CString& localRoot             ,
                           const CORE::CString& URLRoot               ,
                           TDirEntry& currentDir                      ,
                           const TStringSet* dirsToIgnore = GUCEF_NULL      ,
                           const TStringSet* fileTypesToIgnore = GUCEF_NULL ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHER_CPATCHSETGENERATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 30-12-2006 :
        - Dinand: Initial version

-----------------------------------------------------------------------------*/
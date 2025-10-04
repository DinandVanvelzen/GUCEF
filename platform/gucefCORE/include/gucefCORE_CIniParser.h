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

#ifndef GUCEF_CORE_CINIPARSER_H
#define GUCEF_CORE_CINIPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"     /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

class GUCEF_CORE_PUBLIC_CPP CIniParser
{
    public:
    
    struct SIniSection
    {
        CString sectionName;
        CValueList sectionData;
    };
    typedef struct SIniSection TIniSection;
    
    typedef GUCEF::vector< TIniSection > TIniData;
    
    CIniParser( void );
    
    CIniParser( const CIniParser& src );
    
    virtual ~CIniParser();
    
    CIniParser& operator=( const CIniParser& src );
    
    bool SaveTo( CDataNode& rootNode ) const;
    
    bool SaveTo( CIOAccess& fileAccess ) const;
    
    bool SaveTo( CString& outputIniString ) const;
    
    bool LoadFrom( const CDataNode& rootNode );
    
    bool LoadFrom( CIOAccess& fileAccess );
    
    bool LoadFrom( const CString& iniText );
    
    void Clear( void );
    
    TIniData& GetData( void );
    
    const TIniData& GetData( void ) const;

    private:

    bool LoadFrom( const CDataNode& node       ,
                   TIniSection* currentSection ,
                   bool& foundASection         );
    
    static bool IsCharIndexWithinQuotes( const CString& testString  , 
                                         UInt32 charIndex           ,
                                         Int32& quotationStartIndex ,
                                         Int32& quotationEndIndex   );

    static Int32 FindIndexOfNonQuotedEquals( const CString& testString );
    
    static CString StripQuotation( const CString& testString );

    static bool HasChildWithValueOrAttribs( const CDataNode& node );

    private:
    
    TIniData m_iniData;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CINIPARSER_H */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-09-2005 :
        - initial version

---------------------------------------------------------------------------*/

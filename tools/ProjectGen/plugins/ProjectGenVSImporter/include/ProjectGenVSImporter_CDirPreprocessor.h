/*
 *  ProjectGenerator: Tool to generate module/project files
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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

#ifndef PROJECTGENVSIMPORTER_CDIRPROCESSOR_H
#define PROJECTGENVSIMPORTER_CDIRPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CIDIRPREPROCESSOR_H
#include "gucefProjectGen_CIDirPreprocessor.h"
#define GUCEF_PROJECTGEN_CIDIRPREPROCESSOR_H
#endif /* GUCEF_PROJECTGEN_CIDIRPREPROCESSOR_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CDirPreprocessor : public PROJECTGEN::CIDirPreprocessor
{
    public:
    
    CDirPreprocessor( void );
    
    CDirPreprocessor( const CDirPreprocessor& src );
    
    virtual ~CDirPreprocessor();
    
    CDirPreprocessor& operator=( const CDirPreprocessor& src );
    
    virtual bool ProccessDir( const CORE::CString& path );

    private:

    typedef std::vector< CORE::CString > TStringList;

    bool InitXmlParser( void );

    bool FindProjectFiles( const CORE::CString& path, TStringList& list );

    bool ProccessProjectFiles( const CORE::CString& path             ,
                               TStringList& list                     ,
                               const CORE::CString& buildConfigToUse );

    static PROJECTGEN::TModuleType StudioConfigurationTypeToModuleType( const CORE::CString& configType );

    static void ParseGlobalVars( const CORE::CDataNode& projectData ,
                                 CORE::CStringMap& globals          );

    static CORE::CString ReplaceVisualStudioVariables( const CORE::CString& path             ,
                                                       const CORE::CStringMap& globals       ,
                                                       bool replaceRemainderWithEnvVarLookup );

    static void ParseVisualStudioVariables( const CORE::CString& testStr ,
                                            TStringSet& vars             );

    static CORE::CString ExtractFilename( const CORE::CString& path );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr m_xmlCodec;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VSIMPORTER */
}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PROJECTGENVSIMPORTER_CDIRPROCESSOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 06-06-2014 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
/*
 *  ProjectGen-VSImporter: Plugin for ProjectGen to import from Visual Studio files
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

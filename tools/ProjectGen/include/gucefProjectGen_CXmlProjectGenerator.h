/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
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

#ifndef GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H
#define GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#include "gucefProjectGen_CIProjectGenerator.h"
#define GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H ? */

#ifndef GUCEF_PROJECTGEN_MACROS_H
#include "gucefProjectGen_macros.h"
#define GUCEF_PROJECTGEN_MACROS_H
#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CXmlProjectGenerator : public CIProjectGenerator
{
    public:
    
    CXmlProjectGenerator( void );
    
    virtual ~CXmlProjectGenerator();

    virtual bool GetCapabilities( CProjectGeneratorCapabilities& capabilities ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool GenerateProject( const CProjectInfo& projectInfo      ,
                                  const CORE::CString& outputDir       ,
                                  bool addGeneratorCompileTimeToOutput ,
                                  const CORE::CValueList& params       ) GUCEF_VIRTUAL_OVERRIDE;
                                  
    private:
    
    CXmlProjectGenerator( const CXmlProjectGenerator& src );
    
    CXmlProjectGenerator& operator=( const CXmlProjectGenerator& src );

    CProjectGeneratorCapabilities m_capabilities;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CXMLPROJECTGENERATOR_H ? */

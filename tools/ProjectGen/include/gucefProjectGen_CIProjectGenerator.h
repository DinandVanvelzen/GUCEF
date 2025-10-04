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

#ifndef GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H
#define GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_PROJECTGEN_MACROS_H
#include "gucefProjectGen_macros.h"
#define GUCEF_PROJECTGEN_MACROS_H
#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

#ifndef GUCEF_PROJECTGEN_CPROJECTINFO_H
#include "gucefProjectGen_CProjectInfo.h"
#define GUCEF_PROJECTGEN_CPROJECTINFO_H
#endif /* GUCEF_PROJECTGEN_CPROJECTINFO_H ? */

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

class GUCEF_PROJECTGEN_PUBLIC_CPP CProjectGeneratorCapabilities
{
    public:

    virtual bool IsPlatformSpecific( void ) const;

    virtual bool IsPlatformAgnostic( void ) const;

    void AddSupportedPlatform( const CORE::CString& platform );

    void SetSupportedPlatforms( const CORE::CStringSet& platforms );

    const CORE::CStringSet& GetSupportedPlatforms( void ) const;

    CProjectGeneratorCapabilities( void );

    CProjectGeneratorCapabilities( const CProjectGeneratorCapabilities& src );

    virtual ~CProjectGeneratorCapabilities();

    CProjectGeneratorCapabilities& operator=( const CProjectGeneratorCapabilities& src );

    private:

    CORE::CStringSet m_supportedPlatforms;
};

/*-------------------------------------------------------------------------*/

class GUCEF_PROJECTGEN_PUBLIC_CPP CIProjectGenerator
{
    public:
    
    CIProjectGenerator( void );
    
    CIProjectGenerator( const CIProjectGenerator& src );
    
    virtual ~CIProjectGenerator();
    
    CIProjectGenerator& operator=( const CIProjectGenerator& src );

    virtual bool GetCapabilities( CProjectGeneratorCapabilities& capabilities ) const = 0;
    
    virtual bool GenerateProject( const CProjectInfo& projectInfo      ,
                                  const CORE::CString& outputDir       ,
                                  bool addGeneratorCompileTimeToOutput ,
                                  const CORE::CValueList& params       ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PROJECTGEN_CIPROJECTGENERATOR_H ? */


/*
 *  ProjectGen-DependsFilter: Plugin for ProjectGen to leverage depends tool output
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

#ifndef PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H
#define PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H

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

#ifndef GUCEF_PROJECTGEN_CIPROJECTPREPROCESSOR_H
#include "gucefProjectGen_CIProjectPreprocessor.h"
#define GUCEF_PROJECTGEN_CIPROJECTPREPROCESSOR_H
#endif /* GUCEF_PROJECTGEN_CIPROJECTPREPROCESSOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {
namespace DEPFILTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_HIDDEN CDependsFilter : public PROJECTGEN::CIProjectPreprocessor
{
    public:
    
    CDependsFilter( void );
    
    CDependsFilter( const CDependsFilter& src );
    
    virtual ~CDependsFilter();
    
    CDependsFilter& operator=( const CDependsFilter& src );
    
    virtual bool ProccessProjects( CProjectInfo& projectInfo      ,
                                   const CORE::CString& outputDir ,
                                   const CORE::CValueList& params );

    private:
    
    CORE::CString::StringSet GetListOfModules( const CORE::CString::StringVector& dependsCsvFiles ,
                                               const CORE::CString::StringVector& binarySrcDirs   );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DEPFILTER */
}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PROJECTGENDEPENDSFILTER_CDEPENDSFILTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 06-06-2014 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/

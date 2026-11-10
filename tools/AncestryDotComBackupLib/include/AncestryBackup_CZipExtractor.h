/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef ANCESTRYBACKUP_CZIPEXTRACTOR_H
#define ANCESTRYBACKUP_CZIPEXTRACTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Takes a file out of a zip file, through the virtual file system and its archive plugin (libarchive): the plugin is
 *  the only place that knows about the zip format, and so about the third party library. Ancestry delivers the GEDCOM
 *  export as a zip file.
 */
class CZipExtractor
{
    public:

    /** Can zip files be opened: is the zip plugin loaded? */
    static bool IsAvailable( void );

    /**
     *  Extracts the first file in the zip whose name ends with the given text, for example ".ged"
     *  @param zipPath          the zip file
     *  @param extension        what the name of the file has to end with, not case sensitive
     *  @param destinationDir   where the file is put, under the name it has in the zip
     *  @param extractedPath    receives the path of the file that was written
     */
    static bool ExtractFirst( const CORE::CString& zipPath       ,
                              const CORE::CString& extension     ,
                              const CORE::CString& destinationDir ,
                              CORE::CString& extractedPath       );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CZIPEXTRACTOR_H ? */

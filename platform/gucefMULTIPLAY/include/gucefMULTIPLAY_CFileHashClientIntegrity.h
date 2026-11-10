/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H
#define GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#include "gucefMULTIPLAY_CIClientIntegrity.h"
#define GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H
#endif /* GUCEF_MULTIPLAY_CICLIENTINTEGRITY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The built-in CIClientIntegrity: the SHA-256 (lowercase hex) of the files it is given, typically the executable and
 *  the data files of the game, by the name they were added with. A file that cannot be read is reported as "missing".
 *  The files are hashed once, at the first report, and again after a change of the files.
 *
 *  Config: files (paths separated by ';', resolved with CORE::RelativePath())
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CFileHashClientIntegrity : public CIClientIntegrity ,
                                                            public CORE::CIConfigurable
{
    public:

    static const CORE::CString MissingFileValue;    /**< "missing" */

    CFileHashClientIntegrity( void );

    virtual ~CFileHashClientIntegrity();

    /** Adds a file to the report, under the name given or else under the path as given */
    void AddFile( const CORE::CString& path, const CORE::CString& name = CORE::CString::Empty );

    void ClearFiles( void );

    virtual bool GetIntegrityReport( CORE::CValueList& report ) GUCEF_VIRTUAL_OVERRIDE;

    /** The SHA-256 of a file in lowercase hex, false when it cannot be read */
    static bool HashFile( const CORE::CString& path, CORE::CString& hash );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CFileHashClientIntegrity( const CFileHashClientIntegrity& src );
    CFileHashClientIntegrity& operator=( const CFileHashClientIntegrity& src );

    private:

    CORE::CString::StringVector m_paths;
    CORE::CString::StringVector m_names;
    CORE::CValueList m_report;
    bool m_hashed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CFILEHASHCLIENTINTEGRITY_H ? */

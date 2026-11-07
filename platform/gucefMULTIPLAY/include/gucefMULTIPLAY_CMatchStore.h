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

#ifndef GUCEF_MULTIPLAY_CMATCHSTORE_H
#define GUCEF_MULTIPLAY_CMATCHSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHENTRY_H
#include "gucefMULTIPLAY_CMatchEntry.h"
#define GUCEF_MULTIPLAY_CMATCHENTRY_H
#endif /* GUCEF_MULTIPLAY_CMATCHENTRY_H ? */

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
 *  Keeps the match entries on disk, one data node file per match in a directory. The file name is the match id
 *  and the extension of the data store codec ("json" by default), nothing from outside ever becomes part of a path.
 *
 *  A record is written to a temporary file next to its place, read back, and only then moved over the old record:
 *  a crash at any moment leaves either the complete old or the complete new record. A temporary file that is
 *  left over (the crash happened before the move) is never read as a record; it is removed when the store is
 *  loaded. Save() does the two steps, WriteTemporary() and Commit() are the steps themselves.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchStore
{
    public:

    static const UInt32 MaxRecordFileSize = 1024 * 1024;       /**< records above this are not read */

    CMatchStore( void );

    ~CMatchStore();

    void SetDirectory( const CORE::CString& directory );

    const CORE::CString& GetDirectory( void ) const;

    /** The name of the data store codec: "json", "xml", ... */
    void SetCodecName( const CORE::CString& codecName );

    const CORE::CString& GetCodecName( void ) const;

    /** The file of a record; empty when the match id is not valid or no directory is set */
    CORE::CString GetRecordPath( const CORE::CString& matchId ) const;

    CORE::CString GetTemporaryPath( const CORE::CString& matchId ) const;

    /** Writes the record to its temporary file and reads it back; the record on disk is untouched */
    bool WriteTemporary( const CMatchEntry& entry );

    /** Moves the temporary file of the match over its record */
    bool Commit( const CORE::CString& matchId );

    /** WriteTemporary() and Commit() */
    bool Save( const CMatchEntry& entry );

    bool Load( const CORE::CString& matchId, CMatchEntry& entry ) const;

    /**
     *  Reads every record in the directory
     *
     *  @param skipped receives the number of files that are not valid records (they are left alone)
     *  @return false when the directory cannot be read
     */
    bool LoadAll( TMatchEntryVector& entries, UInt32* skipped = GUCEF_NULL ) const;

    /** Removes temporary files that were left behind */
    UInt32 RemoveLeftoverTemporaries( void ) const;

    bool Remove( const CORE::CString& matchId );

    private:

    CMatchStore( const CMatchStore& src );
    CMatchStore& operator=( const CMatchStore& src );

    private:

    CORE::CString m_directory;
    CORE::CString m_codecName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHSTORE_H ? */

/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H
#define GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#include "gucefGUI_CIGridPreferenceStore.h"
#define GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#endif /* GUCEF_GUI_CIGRIDPREFERENCESTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Keeps the preferences of grids in a directory of the user: one small file per grid, named after the id of the grid
 *  and the extension of the DStore codec it is written with.
 *
 *  - The directory is resolved with CORE::RelativePath(), so "$APPCONFIGDIR$/MyApp/GridPreferences" is the settings
 *    directory of the user ($XDG_CONFIG_HOME or ~/.config on Linux, Roaming AppData on Windows). It is made when the
 *    first record is saved.
 *  - The file name is the id with every byte outside A-Z a-z 0-9 . _ - written as %XX (a leading dot too); a name that
 *    is a reserved device name of Windows gets a '_'; a long id is cut and the hash of the whole id appended. So no id
 *    reaches outside the directory.
 *  - The codec is a DStore codec by name; empty (the default) takes "json" when that codec is registered and else the
 *    built in binary "dnc". A record is looked for in the file of that codec, then in the file of the other one, each
 *    read with the codec of its extension, so a change of the loaded codec plugins does not lose it.
 *  - A record is encoded in memory and written with CORE::WriteFileAtomically(): a crash leaves the old or the new file,
 *    never a part of one. Files larger than MaxRecordBytes are not read.
 *  - A file that cannot be decoded, has another root or no version is not loaded (a warning is logged); the next save
 *    replaces it.
 */
class GUCEF_GUI_PUBLIC_CPP CLocalGridPreferenceStore : public CIGridPreferenceStore
{
    public:

    static const CString JsonCodecName;
    static const CString BinaryCodecName;

    /** A record file larger than this is not read */
    static const UInt32 MaxRecordBytes;

    /** The length the file name of an id is cut at, before the hash is appended */
    static const UInt32 MaxFileNameBytes;

    explicit CLocalGridPreferenceStore( const CString& directory, const CString& codecName = CString::Empty );

    virtual ~CLocalGridPreferenceStore() GUCEF_VIRTUAL_OVERRIDE;

    /** The directory as given */
    const CString& GetDirectory( void ) const;

    /** The directory with its variables resolved */
    CString GetResolvedDirectory( void ) const;

    /** The codec asked for; empty for json when registered, else dnc */
    void SetCodecName( const CString& codecName );

    const CString& GetCodecName( void ) const;

    /** The codec a save uses now */
    CString GetEffectiveCodecName( void ) const;

    /** The file a save of the id writes now */
    CString GetRecordPath( const CString& gridId ) const;

    /** The file name of an id, without the extension */
    static CString GetRecordFileName( const CString& gridId );

    virtual bool Load( const CString& gridId, CGridPreferences& preferences ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Save( const CString& gridId, const CGridPreferences& preferences ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Remove( const CString& gridId ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The codecs a record is looked for with, the one a save uses first */
    CString::StringVector GetCandidateCodecs( void ) const;

    bool LoadFile( const CString& path, const CString& codecName, CGridPreferences& preferences ) const;

    CLocalGridPreferenceStore( const CLocalGridPreferenceStore& src );
    CLocalGridPreferenceStore& operator=( const CLocalGridPreferenceStore& src );

    private:

    CString m_directory;
    CString m_codecName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H ? */

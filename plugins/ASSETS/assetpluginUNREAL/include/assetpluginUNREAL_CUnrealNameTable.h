/*
 *  assetpluginUNREAL: The name table of a package
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The names of a package. On disk a name is an index into this table plus a number; a number above zero is shown
 *  as a "_N" suffix with N one less than the number, as the engine does.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealNameTable
{
    public:

    CUnrealNameTable( void );

    void Clear( void );

    /** Reads count names (each a string and two 16 bit hashes) from the reader position */
    bool Read( CUnrealArchiveReader& reader, UInt32 count, const CUnrealReaderLimits& limits );

    /** Adds a name and returns its index (also for the test writers and the class readers) */
    UInt32 Add( const CORE::CString& name );

    UInt32 GetCount( void ) const;

    const CORE::CString& Get( UInt32 index ) const;

    /** The index of a name by its text, -1 when absent */
    Int32 Find( const CORE::CString& name ) const;

    /** Reads an index and a number from the reader and resolves them; an index outside the table is an error of the reader */
    bool ReadName( CUnrealArchiveReader& reader, CORE::CString& name ) const;

    /** Same, without building the text: the index and the number as stored */
    bool ReadNameRef( CUnrealArchiveReader& reader, UInt32& index, Int32& number ) const;

    /** The text of an index and a number as ReadName() gives it */
    CORE::CString Format( UInt32 index, Int32 number ) const;

    private:

    typedef GUCEF::vector< CORE::CString > TNameVector;

    TNameVector m_names;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

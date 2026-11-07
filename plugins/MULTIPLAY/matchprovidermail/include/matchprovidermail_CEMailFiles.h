/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The files of the provider. Everything is written to a temporary file in the same directory, synced and moved over the
 *  target, so that a crash leaves the complete old or the complete new content. Reads are bounded. File names are always
 *  made by the provider from match ids (checked) and counters, never from anything that came in mail.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailFiles
{
    public:

    /** Writes the bytes to the path through a temporary file and a rename; the directory is made when needed */
    static bool WriteAtomic( const CORE::CString& path, const void* data, UInt32 size );

    /** Reads a whole file of at most maxBytes; false when it is missing, unreadable or larger */
    static bool ReadBounded( const CORE::CString& path, UInt32 maxBytes, CORE::CDynamicBuffer& bytes );

    /** A record stored as JSON through the data store codec, atomically */
    static bool WriteRecord( const CORE::CString& path, const CORE::CIDataNodeSerializable& record );

    static bool ReadRecord( const CORE::CString& path, CORE::CIDataNodeSerializable& record, UInt32 maxBytes );

    /** The names of the files (not directories) in a directory, in no particular order */
    static void ListFiles( const CORE::CString& directory, CORE::CString::StringVector& names );

    /** The names of the sub directories in a directory */
    static void ListDirectories( const CORE::CString& directory, CORE::CString::StringVector& names );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILFILES_H ? */

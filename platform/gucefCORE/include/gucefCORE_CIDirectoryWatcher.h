/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CIDIRECTORYWATCHER_H
#define GUCEF_CORE_CIDIRECTORYWATCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CDIRECTORYWATCHEREVENTS_H
#include "gucefCORE_CDirectoryWatcherEvents.h"
#define GUCEF_CORE_CDIRECTORYWATCHEREVENTS_H
#endif /* GUCEF_CORE_CDIRECTORYWATCHEREVENTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CIDirectoryWatcher : public CDirectoryWatcherEvents
{
    public:

    class GUCEF_CORE_PUBLIC_CPP CDirWatchOptions
    {
        public:

        bool watchSubTree;
        bool watchForFileCreation;
        bool watchForFileDeletion;
        bool watchForFileRenames;
        bool watchForFileModifications;
        bool watchForDirCreation;
        bool watchForDirDeletion;
        bool watchForDirRenames;
        bool watchForDirModifications;

        CDirWatchOptions( bool watchDefault = true );
        CDirWatchOptions( const CDirWatchOptions& src );
        CDirWatchOptions& operator=( const CDirWatchOptions& src );
    };

    virtual bool AddDirToWatch( const CString& dirToWatch       ,
                                const CDirWatchOptions& options ,
                                bool tryToCreatePathIfNotExists ) = 0;

    virtual bool RemoveDirToWatch( const CString& dirToWatch ) = 0;

    virtual bool RemoveAllWatches( void ) = 0;

    virtual bool GetAllWatchedDirs( CString::StringSet& dirs ) const = 0;

    CIDirectoryWatcher( void );

    CIDirectoryWatcher( const CIDirectoryWatcher& src );

    virtual ~CIDirectoryWatcher();

    CIDirectoryWatcher& operator=( const CIDirectoryWatcher& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIDIRECTORYWATCHER_H ? */

/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H
#define GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H
#include "gucefASSETSTORE_CAssetToolsExposure.h"
#define GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Holds the exposure of the asset tools that is in force and replaces it as a whole when the exposure file changes
 *  (section 11.4): CheckForChange() looks at the time and the size of the file and reads it again when either
 *  changed; Reload() reads it whatever they say. A file that does not read strictly puts the closed defaults in
 *  force, with the reason in GetState().problem and in the log. The file is read from the file system directly, not
 *  through the VFS. Thread safe: the state is copied out under a leaf lock.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolsExposureControl
{
    public:

    CAssetToolsExposureControl( void );

    /** The file to watch; empty: the closed defaults, always. Reads it at once */
    void SetFile( const CORE::CString& path );

    CORE::CString GetFile( void ) const;

    /** Reads the file again; true when the state in force changed */
    bool Reload( void );

    /** Reads the file again when its time or size changed; true when the state in force changed */
    bool CheckForChange( void );

    /** Puts a state in force directly (tests, an admin command); true when it changed */
    bool SetState( const CAssetToolsExposure& state );

    CAssetToolsExposure GetState( void ) const;

    private:

    CAssetToolsExposureControl( const CAssetToolsExposureControl& src );              /**< not implemented */
    CAssetToolsExposureControl& operator=( const CAssetToolsExposureControl& src );   /**< not implemented */

    bool ReadLocked( void );

    mutable MT::CMutex m_lock;
    CORE::CString m_path;
    CAssetToolsExposure m_state;
    bool m_seen;
    bool m_seenExists;
    UInt64 m_seenSize;
    CORE::CDateTime m_seenTime;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H ? */

/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H
#define GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_MACROS_H
#include "vfspluginDRN_macros.h"
#define GUCEF_VFSPLUGIN_DRN_MACROS_H
#endif /* GUCEF_VFSPLUGIN_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Who is told when a file that is open through the handle pool turns out to be another revision on the serving side: the one that keeps what is
 *  known of the share (the generic layer through CDrnBridgeBackend) drops it and tells its observers. Called on the thread that read the file.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CIDrnBridgeChangeListener
{
    public:

    virtual ~CIDrnBridgeChangeListener() {}

    /** The file at the path is not the revision it was opened at ('revision') any more */
    virtual void OnFileChanged( const CORE::CString& path, const CORE::CString& revision ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CIDRNBRIDGECHANGELISTENER_H ? */

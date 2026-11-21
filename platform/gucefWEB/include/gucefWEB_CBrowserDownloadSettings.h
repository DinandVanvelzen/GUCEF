/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H
#define GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The checks of a CBrowserDownloadCapture. Every member is a configuration key of the same name (attributes or child
 *  values of one data node, lists as arrays or comma separated text). A list that is configured replaces the default.
 */
class GUCEF_WEB_PUBLIC_CPP CBrowserDownloadSettings : public CORE::CIConfigurable
{
    public:

    static const UInt32 MinFileNameLength = 16;             /**< the lowest maxFileNameLength that is accepted */

    UInt64 maxBytes;                                        /**< larger downloads are canceled or refused; 0: no maximum. Default 256 MiB */
    UInt64 minBytes;                                        /**< smaller downloads are refused, default 1 */
    CORE::CString::StringVector allowedExtensions;          /**< lower case, without the dot (csv, pdf); empty: any */
    CORE::CString::StringVector allowedMimeTypes;           /**< content types or glob patterns of them (text/csv, or text/x-* as a glob); empty: any */
    bool allowHtml;                                         /**< accept an HTML page, which usually means an error or sign-in page came instead of the file; default false */
    UInt32 maxFileNameLength;                               /**< of the sanitised name, default 100 */

    CBrowserDownloadSettings( void );

    CBrowserDownloadSettings( const CBrowserDownloadSettings& src );

    virtual ~CBrowserDownloadSettings() GUCEF_VIRTUAL_OVERRIDE;

    CBrowserDownloadSettings& operator=( const CBrowserDownloadSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /** False, leaving the settings as they were, for a maxFileNameLength below MinFileNameLength or minBytes above maxBytes */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H ? */

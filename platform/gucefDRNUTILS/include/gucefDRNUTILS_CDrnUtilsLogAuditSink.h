/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H
#define GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#include "gucefDRNUTILS_CIDrnUtilsAuditSink.h"
#define GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H
#endif /* GUCEF_DRNUTILS_CIDRNUTILSAUDITSINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The default audit sink of the hosts: one log line of level LOGLEVEL_IMPORTANT per record */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsLogAuditSink : public CIDrnUtilsAuditSink
{
    public:

    CDrnUtilsLogAuditSink( void );

    virtual ~CDrnUtilsLogAuditSink() GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnAuditRecord( const SDrnUtilsAuditRecord& record ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSLOGAUDITSINK_H ? */

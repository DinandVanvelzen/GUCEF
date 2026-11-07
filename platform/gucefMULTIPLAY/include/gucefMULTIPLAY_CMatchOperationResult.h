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

#ifndef GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H
#define GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#include "gucefMULTIPLAY_CMatchPreview.h"
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#include "gucefMULTIPLAY_CMatchPackage.h"
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */

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

enum EMatchOperationStatus
{
    MATCHOPERATION_OK          = 0 ,    /**< done; for a repeated submission of the same package too */
    MATCHOPERATION_STALE       = 1 ,    /**< the provider has a newer revision than the one expected; preview is the current one */
    MATCHOPERATION_REJECTED    = 2 ,    /**< the provider does not accept it; reason says why */
    MATCHOPERATION_NOT_FOUND   = 3
};
typedef enum EMatchOperationStatus TMatchOperationStatus;

/*-------------------------------------------------------------------------*/

/**
 *  The outcome of a call of a provider that was answered: the data of the future of the call and of
 *  SubmitResultEvent. A call that could not be made or finished at all (no connection) ends its future
 *  as failed instead.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchOperationResult : public CORE::CICloneable
{
    public:

    TMatchOperationStatus status;
    CORE::CString matchId;
    CORE::CString reason;
    CMatchPreview preview;          /**< the current preview of the match, when hasPreview */
    bool hasPreview;
    CMatchPackage package;          /**< the package a Fetch() asked for, when hasPackage */
    bool hasPackage;

    CMatchOperationResult( void );

    CMatchOperationResult( const CMatchOperationResult& src );

    virtual ~CMatchOperationResult();

    CMatchOperationResult& operator=( const CMatchOperationResult& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    /** Gets the result from a finished future of a call, false when the future is not finished successfully or has none */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CMatchOperationResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H ? */

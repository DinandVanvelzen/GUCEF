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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#include "matchprovidermail_CEMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H ? */

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

/** A turn that was received and not applied: kept on disk with the reason */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailRefusal : public CORE::CIDataNodeSerializable
{
    public:

    UInt32 revision;
    CORE::CString sha256;               /**< of the package the preview names */
    CORE::CString code;                 /**< a short stable word: UNKNOWN_SIGNER, NOT_ON_TURN, BROKEN_CHAIN, BAD_PACKAGE, GAME_REJECTED, BAD_EVIDENCE, OLD_REVISION, EQUIVOCATION, NO_VERIFIER */
    CORE::CString reason;               /**< for the player */
    Int64 at;
    CORE::CString file;                 /**< the name of the kept container in the match directory */

    CEMailRefusal( void );
    CEMailRefusal( const CEMailRefusal& src );
    virtual ~CEMailRefusal() GUCEF_VIRTUAL_OVERRIDE;
    CEMailRefusal& operator=( const CEMailRefusal& src );
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CEMailRefusal > TEMailRefusalVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H ? */

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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHSTATE_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHSTATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#include "matchprovidermail_CEMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHCODEC_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H
#include "matchprovidermail_CEMailRefusal.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILREFUSAL_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILEQUIVOCATION_H
#include "matchprovidermail_CEMailEquivocation.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILEQUIVOCATION_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILEQUIVOCATION_H ? */

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
 *  What the provider remembers of one match, written to the file state.json in the directory of the match (the turns are
 *  files of their own, r<revision>.rdxturn). Everything in it was verified when it was put in; the file is not trusted
 *  more than the mail was: it is read with the same limits.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CEMailMatchState : public CORE::CIDataNodeSerializable
{
    public:

    MULTIPLAY::CMatchRecord record;
    bool hasRecord;
    MULTIPLAY::CMatchInfo info;                         /**< the match as known now: the record with the statuses that followed from acceptances, leaves and decisions */
    GUCEF::vector< MULTIPLAY::CMatchAcceptance > acceptances;
    GUCEF::vector< CEMailSignedStatement > resigns;
    GUCEF::vector< MULTIPLAY::CMatchProposal > proposals;
    GUCEF::vector< MULTIPLAY::CMatchVote > votes;
    GUCEF::vector< CORE::CString > appliedDecisions;    /**< digests of the proposals that were carried out */
    UInt32 trustedRevision;                             /**< 0 when the match has no first turn yet */
    GUCEF::map< UInt32, CORE::CString > shaByRevision;  /**< the package SHA-256 of every revision that is trusted */
    TEMailRefusalVector refusals;
    TEMailEquivocationVector equivocations;
    CORE::CString lastMessageId;                        /**< the newest mail of the match, for the threading of the next one */
    CORE::CString::StringVector references;
    GUCEF::map< CORE::CString, Int64 > nudgesSent;      /**< user id -> time */
    GUCEF::map< CORE::CString, Int64 > nudgesReceived;
    GUCEF::map< UInt32, Int64 > resendRequested;        /**< revision -> time of the last request */
    GUCEF::map< CORE::CString, Int64 > resendAnswered;  /**< "revision:address" -> time of the last answer */
    GUCEF::set< UInt32 > deliveredRevisions;
    Int64 lastActivity;
    CORE::CString cursor;

    CEMailMatchState( void );
    CEMailMatchState( const CEMailMatchState& src );
    virtual ~CEMailMatchState() GUCEF_VIRTUAL_OVERRIDE;
    CEMailMatchState& operator=( const CEMailMatchState& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
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

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CEMAILMATCHSTATE_H ? */

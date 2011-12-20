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

#ifndef GUCEF_CORE_CSYSCONSOLE_H
#define GUCEF_CORE_CSYSCONSOLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#include "CISysConsoleCmdHandler.h"
#define GUCEF_CORE_CISYSCONSOLECMDHANDLER_H
#endif /* GUCEF_CORE_CISYSCONSOLECMDHANDLER_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"     /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/*
 *      Forward declaration of classes used.
 */
class CSysConsoleClient;

/*-------------------------------------------------------------------------*/

/**
 *      Central hub for system console command interaction.
 *      system console clients can use the system console to
 *      iterate the command listing and manipulate it.
 *      handlers can then process commands with there parameters
 *      as sent by system console clients.
 *
 *      Think of it as a DOS-console/Bash but instead of a storage
 *      medium you can move trough a tree of system commands.
 */
class GUCEF_CORE_PUBLIC_CPP CSysConsole : public CIConfigurable         ,
                                          public CISysConsoleCmdHandler
{
    public:

    static CSysConsole* Instance( void );

    bool RegisterCmd( const CString& path                ,
                      const CString& command             ,
                      const std::vector< CString >& args ,
                      CISysConsoleCmdHandler* cmdhandler );

    void UnregisterCmd( const CString& path    ,
                        const CString& command );

    bool RegisterAlias( const CString& aliasname ,
                        const CString& path      ,
                        const CString& function  );

    bool UnregisterAlias( const CString& aliasname ,
                          const CString& path      ,
                          const CString& function  );

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @return wheter storing the tree was successfull
     */
    virtual bool SaveConfig( CDataNode& tree );

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CDataNode& treeroot );

    public:

    struct SCmdChannel;
    struct SFunctionHook;

    private:
    friend class CSysConsoleClient;

    void LeaveDir( CSysConsoleClient* client );

    bool EnterDir( CSysConsoleClient* client ,
                   const CString& dirname );

    bool JumpTo( CSysConsoleClient* client ,
                 const CString& path );

    bool Execute( CSysConsoleClient* client             ,
                  const CString& funcname               ,
                  const std::vector< CString >& arglist ,
                  std::vector< CString >& resultdata    );

    std::vector< CString > GetDirList( const CSysConsoleClient* client ) const;

    std::vector< CString > GetCmdList( const CSysConsoleClient* client ) const;

    void InitClient( CSysConsoleClient* client );

    void UnregClient( CSysConsoleClient* client );

    private:
    friend class CGUCEFCOREModule;

    static void Deinstance( void );

    private:

    struct SAliasData
    {
        CString path;
        CString function;
    };
    typedef struct SAliasData TAliasData;
    typedef std::map< CString, TAliasData > TAliasList;

    CSysConsole( void );
    CSysConsole( const CSysConsole& src );
    ~CSysConsole();
    CSysConsole& operator=( const CSysConsole& src );

    struct SCmdChannel* FindChannel( struct SCmdChannel* curchannel ,
                                     const CString& name            );

    struct SCmdChannel* BuildTree( struct SCmdChannel* curchannel ,
                                   const CString& path            );

    void DelTree( struct SCmdChannel* tree );

    struct SCmdChannel* WalkTree( struct SCmdChannel* curchannel ,
                                  const CString& path            ,
                                  CString& leftover              );

    struct SFunctionHook* FindFunction( const struct SCmdChannel* curchannel ,
                                        const CString& functionname          );

    TAliasData* FindAliasFunction( const CString& aliasname );

    bool OnSysConsoleCommand( const CString& path                ,
                              const CString& command             ,
                              const std::vector< CString >& args ,
                              std::vector< CString >& resultdata );

    struct SCmdChannel* _root;
    TAliasList _aliases;
    MT::CMutex _datalock;

    static CSysConsole* _instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSYSCONSOLE_H */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-05-2005 :
        - Applied file layout standard

--------------------------------------------------------------------------*/

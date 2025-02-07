--------------------------------------------------------------------
-- This file was automatically generated by ProjectGenerator
-- which is tooling part the build system designed for GUCEF
--     (Galaxy Unlimited Framework)
-- For the latest info, see http://www.VanvelzenSoftware.com/
--
-- The contents of this file are placed in the public domain. Feel
-- free to make use of it in any way you like.
--------------------------------------------------------------------
--

-- Configuration for module: redisinfo


project( "redisinfo" )

configuration( {} )
  location( os.getenv( "PM4OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM4TARGETDIR" ) )

configuration( {} )
language( "C++" )

configuration( { "WIN32" } )


configuration( { WIN32 } )
kind( "WindowedApp" )
configuration( { "NOT WIN32" } )
  

configuration( {} )
kind( "ConsoleApp" )

configuration( {} )
links( { "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB", "hiredis", "redis-plus-plus" } )
links( { "gucefCOM", "gucefCOMCORE", "gucefCORE", "gucefMT", "gucefVFS", "gucefWEB", "hiredis", "redis-plus-plus" } )


configuration( {} )
vpaths { ["Headers"] = { "**.h", "**.hpp", "**.hxx" } }
files( {
  "include/crc16.h",
  "include/redisinfo.h",
  "include/redisinfo_CRedisClusterKeyCache.h",
  "include/redisinfo_CRedisClusterKeyCacheUpdateTask.h",
  "include/redisinfo_CRedisClusterKeyPrunerTask.h",
  "include/redisinfo_RedisClusterNodeCmds.h"
 } )



configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "src/crc16.cpp",
  "src/main.cpp",
  "src/redisinfo.cpp",
  "src/redisinfo_CRedisClusterKeyCache.cpp",
  "src/redisinfo_CRedisClusterKeyCacheUpdateTask.cpp",
  "src/redisinfo_CRedisClusterKeyPrunerTask.cpp"
 } )


configuration( {} )
includedirs( { "../../common/include", "../../dependencies/hiredis", "../../dependencies/redis-plus-plus/../", "../../dependencies/redis-plus-plus/src/sw/../", "../../dependencies/redis-plus-plus/src/sw/redis++", "../../dependencies/redis-plus-plus/src/sw/redis++/cxx11", "../../dependencies/redis-plus-plus/src/sw/redis++/cxx11/sw", "../../dependencies/redis-plus-plus/src/sw/redis++/cxx11/sw/redis++", "../../dependencies/redis-plus-plus/src/sw/redis++/no_tls", "../../dependencies/redis-plus-plus/src/sw/redis++/no_tls/sw", "../../dependencies/redis-plus-plus/src/sw/redis++/no_tls/sw/redis++", "../../dependencies/redis-plus-plus/src/sw/redis++/patterns", "../../platform/gucefCOM/include", "../../platform/gucefCOMCORE/include", "../../platform/gucefCORE/include", "../../platform/gucefCORE/include/c_api", "../../platform/gucefMT/include", "../../platform/gucefVFS/include", "../../platform/gucefWEB/include", "include" } )

configuration( { "ANDROID" } )
includedirs( { "../../dependencies/hiredis", "../../platform/gucefCORE/include/android" } )

configuration( { "ARDUINO" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "EMSCRIPTEN" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "GLX" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "GTK" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "IOS" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "LINUX32" } )
includedirs( { "../../dependencies/hiredis", "../../platform/gucefCOMCORE/include/linux", "../../platform/gucefCORE/include/linux" } )

configuration( { "LINUX64" } )
includedirs( { "../../dependencies/hiredis", "../../platform/gucefCOMCORE/include/linux", "../../platform/gucefCORE/include/linux" } )

configuration( { "NACL" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "OSX" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "POSIX" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "SDL" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "SYMBIAN" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "UNIX" } )
includedirs( { "../../dependencies/hiredis" } )

configuration( { "WIN32" } )
includedirs( { "../../dependencies/hiredis", "../../platform/gucefCOMCORE/include/mswin", "../../platform/gucefCORE/include/mswin", "../../platform/gucefMT/include/mswin" } )

configuration( { "WIN64" } )
includedirs( { "../../dependencies/hiredis", "../../platform/gucefCOMCORE/include/mswin", "../../platform/gucefCORE/include/mswin", "../../platform/gucefMT/include/mswin" } )

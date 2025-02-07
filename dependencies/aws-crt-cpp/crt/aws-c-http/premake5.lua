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

-- Configuration for module: aws-c-http


project( "aws-c-http" )

configuration( {} )
  location( os.getenv( "PM5OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM5TARGETDIR" ) )

configuration( {} )
language( "C" )



configuration( {} )
kind( "StaticLib" )

configuration( {} )
links( { "aws-c-common", "aws-c-compression", "aws-c-io" } )
links( { "aws-c-common", "aws-c-compression", "aws-c-io" } )


configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "source/connection.c",
  "source/connection_manager.c",
  "source/connection_monitor.c",
  "source/h1_connection.c",
  "source/h1_decoder.c",
  "source/h1_encoder.c",
  "source/h1_stream.c",
  "source/h2_connection.c",
  "source/h2_decoder.c",
  "source/h2_frames.c",
  "source/h2_stream.c",
  "source/hpack.c",
  "source/hpack_decoder.c",
  "source/hpack_encoder.c",
  "source/hpack_huffman_static.c",
  "source/http.c",
  "source/http2_stream_manager.c",
  "source/proxy_connection.c",
  "source/proxy_strategy.c",
  "source/random_access_set.c",
  "source/request_response.c",
  "source/statistics.c",
  "source/strutil.c",
  "source/websocket.c",
  "source/websocket_bootstrap.c",
  "source/websocket_decoder.c",
  "source/websocket_encoder.c"
 } )


configuration( {} )
includedirs( { "../aws-c-cal/include", "../aws-c-common/include", "../aws-c-compression/include", "../aws-c-io/include", "../aws-c-io/source", "include" } )

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

-- Configuration for module: gucefCOMCORE


project( "gucefCOMCORE" )

configuration( {} )
  location( os.getenv( "PM5OUTPUTDIR" ) )

configuration( {} )
  targetdir( os.getenv( "PM5TARGETDIR" ) )

configuration( {} )
language( "C++" )



configuration( {} )
kind( "SharedLib" )

configuration( {} )
links( { "gucefCORE", "gucefMT" } )
links( { "gucefCORE", "gucefMT" } )


configuration( {} )
defines( { "GUCEF_COMCORE_BUILD_MODULE" } )
  links( { "Advapi32.lib", "Iphlpapi.lib", "Ws2_32.lib" } )
  

configuration( { WIN32 } )
defines( { "_WINSOCK_DEPRECATED_NO_WARNINGS" } )
  links( { "Advapi32.lib", "Iphlpapi.lib", "Ws2_32.lib" } )
  

configuration( { WIN64 } )
defines( { "_WINSOCK_DEPRECATED_NO_WARNINGS" } )


configuration( {} )
vpaths { ["Headers"] = { "**.h", "**.hpp", "**.hxx" } }
files( {
  "include/CCom.h",
  "include/CGUCEFCOMCOREModule.h",
  "include/CHostAddress.h",
  "include/CPing.h",
  "include/CSocket.h",
  "include/CTCPClientSocket.h",
  "include/CTCPConnection.h",
  "include/CTCPServerConnection.h",
  "include/CTCPServerSocket.h",
  "include/CUDPChannel.h",
  "include/CUDPMasterSocket.h",
  "include/CUDPSocket.h",
  "include/dvwinsock.h",
  "include/gucefCOMCORE.h",
  "include/gucefCOMCORE_CComCoreGlobal.h",
  "include/gucefCOMCORE_CDiscoveryManager.h",
  "include/gucefCOMCORE_CDnsCache.h",
  "include/gucefCOMCORE_CDnsCacheEntry.h",
  "include/gucefCOMCORE_CDnsCacheRefreshTaskConsumer.h",
  "include/gucefCOMCORE_CDnsResolver.h",
  "include/gucefCOMCORE_CGeoLocationLookupService.h",
  "include/gucefCOMCORE_CICommunicationInterface.h",
  "include/gucefCOMCORE_CIDiscovery.h",
  "include/gucefCOMCORE_CIDiscoveryCapability.h",
  "include/gucefCOMCORE_CIDiscoveryClient.h",
  "include/gucefCOMCORE_CIDiscoveryEntry.h",
  "include/gucefCOMCORE_CIGeoLocationLookupServiceProvider.h",
  "include/gucefCOMCORE_CIMessage.h",
  "include/gucefCOMCORE_CIMessageTransformer.h",
  "include/gucefCOMCORE_CINetworkInterface.h",
  "include/gucefCOMCORE_CIPv4Address.h",
  "include/gucefCOMCORE_CIPv4Info.h",
  "include/gucefCOMCORE_CIPv6Address.h",
  "include/gucefCOMCORE_CIPv6Info.h",
  "include/gucefCOMCORE_CMessageSerializer.h",
  "include/gucefCOMCORE_CPingTaskConsumer.h",
  "include/gucefCOMCORE_CSerialPort.h",
  "include/gucefCOMCORE_ETypes.h",
  "include/gucefCOMCORE_config.h",
  "include/gucefCOMCORE_macros.h",
  "include/socketerrors.h",
  "include/socketutils.h"
 } )



configuration( {} )
vpaths { ["Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
files( {
  "src/CCom.cpp",
  "src/CGUCEFCOMCOREModule.cpp",
  "src/CHostAddress.cpp",
  "src/CPing.cpp",
  "src/CSocket.cpp",
  "src/CTCPClientSocket.cpp",
  "src/CTCPConnection.cpp",
  "src/CTCPServerConnection.cpp",
  "src/CTCPServerSocket.cpp",
  "src/CUDPChannel.cpp",
  "src/CUDPMasterSocket.cpp",
  "src/CUDPSocket.cpp",
  "src/dvwinsock.c",
  "src/gucefCOMCORE.cpp",
  "src/gucefCOMCORE_CComCoreGlobal.cpp",
  "src/gucefCOMCORE_CDiscoveryManager.cpp",
  "src/gucefCOMCORE_CDnsCache.cpp",
  "src/gucefCOMCORE_CDnsCacheEntry.cpp",
  "src/gucefCOMCORE_CDnsCacheRefreshTaskConsumer.cpp",
  "src/gucefCOMCORE_CDnsResolver.cpp",
  "src/gucefCOMCORE_CGeoLocationLookupService.cpp",
  "src/gucefCOMCORE_CICommunicationInterface.cpp",
  "src/gucefCOMCORE_CIDiscovery.cpp",
  "src/gucefCOMCORE_CIDiscoveryCapability.cpp",
  "src/gucefCOMCORE_CIDiscoveryClient.cpp",
  "src/gucefCOMCORE_CIDiscoveryEntry.cpp",
  "src/gucefCOMCORE_CIGeoLocationLookupServiceProvider.cpp",
  "src/gucefCOMCORE_CIMessage.cpp",
  "src/gucefCOMCORE_CIMessageTransformer.cpp",
  "src/gucefCOMCORE_CINetworkInterface.cpp",
  "src/gucefCOMCORE_CIPv4Address.cpp",
  "src/gucefCOMCORE_CIPv4Info.cpp",
  "src/gucefCOMCORE_CIPv6Address.cpp",
  "src/gucefCOMCORE_CIPv6Info.cpp",
  "src/gucefCOMCORE_CMessageSerializer.cpp",
  "src/gucefCOMCORE_CPingTaskConsumer.cpp",
  "src/gucefCOMCORE_CSerialPort.cpp",
  "src/socketutils.cpp"
 } )



configuration( { "LINUX32" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/linux/gucefCOMCORE_CLinuxNetworkInterface.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/linux/gucefCOMCORE_CLinuxNetworkInterface.cpp"
    } )



configuration( { "LINUX64" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/linux/gucefCOMCORE_CLinuxNetworkInterface.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/linux/gucefCOMCORE_CLinuxNetworkInterface.cpp"
    } )



configuration( { "WIN32" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/mswin/gucefCOMCORE_CWin32NetworkInterface.h",
      "include/mswin/gucefCOMCORE_CWin32SerialPort.h",
      "include/mswin/gucefCOMCORE_IphlpapiApi.h",
      "include/mswin/gucefCOMCORE_icmpApi.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/mswin/gucefCOMCORE_CWin32NetworkInterface.cpp",
      "src/mswin/gucefCOMCORE_CWin32SerialPort.cpp",
      "src/mswin/gucefCOMCORE_IphlpapiApi.cpp",
      "src/mswin/gucefCOMCORE_icmpApi.cpp"
    } )



configuration( { "WIN64" } )
    vpaths { ["Platform Headers"] = { "**.h", "**.hpp", "**.hxx" } }
    files( {
      "include/mswin/gucefCOMCORE_CWin32NetworkInterface.h",
      "include/mswin/gucefCOMCORE_CWin32SerialPort.h",
      "include/mswin/gucefCOMCORE_IphlpapiApi.h",
      "include/mswin/gucefCOMCORE_icmpApi.h"
    } )

    vpaths { ["Platform Source"] = { "**.c", "**.cpp", "**.cs", "**.asm" } }
    files( {
      "src/mswin/gucefCOMCORE_CWin32NetworkInterface.cpp",
      "src/mswin/gucefCOMCORE_CWin32SerialPort.cpp",
      "src/mswin/gucefCOMCORE_IphlpapiApi.cpp",
      "src/mswin/gucefCOMCORE_icmpApi.cpp"
    } )


configuration( {} )
includedirs( { "../../common/include", "../gucefCORE/include", "../gucefCORE/include/c_api", "../gucefMT/include", "include" } )

configuration( { "ANDROID" } )
includedirs( { "../gucefCORE/include/android" } )

configuration( { "LINUX32" } )
includedirs( { "../gucefCORE/include/linux", "include/linux" } )

configuration( { "LINUX64" } )
includedirs( { "../gucefCORE/include/linux", "include/linux" } )

configuration( { "WIN32" } )
includedirs( { "../gucefCORE/include/mswin", "../gucefMT/include/mswin", "include/mswin" } )

configuration( { "WIN64" } )
includedirs( { "../gucefCORE/include/mswin", "../gucefMT/include/mswin", "include/mswin" } )


***Versions used***

miniupnpc: is release version 1.4
MyGui: is version 3.2.0
zzlib version is: 0.13.49
zlib version is: 1.3.2 from https://zlib.net (local edits marked "DV edit" in zconf.h)
minizip is the contrib/minizip (with ZIP64) of zlib 1.3.2, only the zip, unzip and ioapi sources
libarchive is version 3.8.9 from https://github.com/libarchive/libarchive/releases (signature verified), configured for zlib only, see libarchive/libarchive/config.h
FreeImage version is: 3110
freetype is actually Freetype2 which is version 2.6.3
pcre is version 8.37
CEGUI is version 0.8.3
libiconv is version 1.14
libpng is version 1.6.26
libmng is version 2.0.3
libtiff is version 4.0.7
libsquish is version 1.14
OpenEXR is version 2.2.0
glew is version 1.13
glm is version 0.9.6
LUA is version 5.3.4
Little-CMS is version >=2.8 HEAD from https://github.com/mm2/Little-CMS
DevIL is version 1.8.0, see https://github.com/DentonW/DevIL
toluapp is a LUA 5.3 patched version from : https://github.com/waltervn/toluapp
hiredis is version 1.0.2 from https://github.com/redis/hiredis
redis-plus-plus is version 1.3.9 from https://github.com/sewenew/redis-plus-plus
librdkafka is version 2.3.0 from https://github.com/confluentinc/librdkafka/releases/tag/v2.3.0
OIS is at version 1.3 and can be obtained from http://sourceforge.net/projects/wgois
json-parser is at version 1.1.0 and can be obtained from https://github.com/udp/json-parser
zookeeper-client-c is from Zookeeper version 3.5.6 source tar dated 2019-10-16 00:35
curl is release version 8.23.0 from https://github.com/curl/curl/releases (signature verified), TLS via system OpenSSL on Linux and Schannel on Windows, see lib/curl_config.h
stbrumme-hash-library is version 8 from https://github.com/stbrumme/hash-library obtained source dated 2022-01-22
Protocol Buffers is at release v21.12 from https://github.com/protocolbuffers/protobuf/releases/tag/v21.12
aws-cpp-sdk-core is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-s3 is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-sqs is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-sns is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-secretsmanager is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-core/s3/sns/sqs/secretsmanager: local edit marked "DV edit" in the *_EXPORTS.h headers, on non-Windows the API macros export symbols explicitly since our builds hide them by default
aws-crt-cpp (aws-checksums): local edit marked "DV edit" in source/intel/asm/crc32c_sse42_asm.c, GCC without optimization uses the software CRC since its inline assembly does not compile unoptimized
aws-lambda-runtime is version 0.2.7 from GitHub @ https://github.com/awslabs/aws-lambda-cpp/releases/tag/v0.2.7
libyaml is in-flight version 0.2.6 from GitHub @ https://github.com/yaml/libyaml obtained source dated 2023-04-23
opentelemetry-cpp is version 1.21.0 from GitHub
RmlUI is version 6.2 from GitHub @ https://github.com/mikke89/RmlUi/releases/tag/6.2
jasper: local edit marked "DV edit" in src/libjasper/include/jasper/jas_config.h, the values of a POSIX system instead of cmake template lines
tiff: local edit marked "DV edit" in libtiff/tif_config.h, io.h and setmode() only on Windows, unistd.h and snprintf() elsewhere
openexr: local edits marked "DV edit" in IlmBase/config.windows/IlmBaseConfig.h and OpenEXR/config.windows/OpenEXRConfig.h, pthreads, POSIX semaphores and sysconf() on non-Windows. The Win32 thread sources are only part of the build on Windows (IlmBase/IlmThread/ProjectGenInstructions.xml)
FreeImage: local edits marked "DV edit": LibJXR/image/sys/ansi.h (byteswap declaration and wchar.h for ANSI platforms), LibRawLite/internal/dcraw_common.cpp (powf64 renamed dcraw_powf64 since glibc has a function by that name), LibJXR/jxrgluelib/JXRGlueJxr.c (wchar.h). The processor specific WebP code for other architectures is excluded in LibWebP/src/dsp/ProjectGenInstructions.xml
DevIL: the Linux config include/IL/linux/config.h was renamed DevILconfig.h like the Windows one and had its Windows specifics removed (marked "DV edit"). The older il_exr.c and il_utx.c, which are duplicates of the .cpp versions, are excluded in src-IL/src/ProjectGenExcludeList.txt
Ogre: local edit marked "DV edit" in OgreMain/include/OgrePlatform.h, [[deprecated]] without the empty parentheses newer compilers refuse
RmlUi: local edits marked "DV edit" in Include/RmlUi/Core/RmlUiMath.h (the types header is included last, since the templates of the vector and matrix types use what the math header declares), Matrix4.h/Vector2.h/Vector4.h (include the renamed RmlUiMath.h) and Source/Core/FontEngineDefault/FreeTypeInterface.cpp (compatibility with the older FreeType which is part of this code base)
MyGui: local edit marked "DV edit" in MyGUIEngine/include/MyGUI_Prerequest.h, nullptr is only emulated for compilers from before C++11
cegui: local edit marked "DV edit" in cegui/include/CEGUI/Config.h, glibc iconv takes a non-const input buffer
libogg is version 1.3.6 from https://downloads.xiph.org/releases/ogg/libogg-1.3.6.tar.xz (sha256 of the tarball 5c8253428e181840cd20d41f3ca16557a9cc04bad4a3d04cce84808677fa1061, matches the xiph SHA256SUMS), only the library sources and public headers. include/ogg/config_types.h is the hand written stand-in for the file configure or CMake generates from config_types.h.in (marked "DV edit")
libvorbis is version 1.3.7 from https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz (sha256 of the tarball b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b, matches the xiph SHA256SUMS), unmodified lib/ sources of libvorbis, libvorbisfile and libvorbisenc (encoder kept) with the public headers, without the tools, tests and examples
opusfile is version 0.12 from https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz (sha256 of the tarball 118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b, matches the xiph SHA256SUMS), unmodified decoder sources and opusfile.h, built against the libopus of the opus module. The HTTP URL streaming (http.c, wincerts.c, winerrno.h, opusurl) is left out so OpenSSL is not needed
dr_libs is commit 35b5c392cc15456736c440a68dbe073178bbffe0 of https://github.com/mackron/dr_libs, only dr_mp3.h (v0.7.4, sha256 f5c8020eeebb79877184432efdd2651f24869f5bb8f379d45450fd0aed1ee915) and dr_flac.h (v0.13.4, sha256 b8078eeb5d523502eb7964508cd7ec8450bc79d383bec90aa17fd9e2fefd9c24) plus the LICENSE, unmodified

Note for MS Windows builds: 
- to build the DirectInput dependent code you must have the DirectX SDK installed on your system.
Such external binary dependencies are managed via the ModuleInfo.xml under /dependencies/external
- Note that for some of the dependencies you need to have the Windows SDK installed. Said SDK needs to be of the correct recent version to avoid build issues.
Incorrect older SDKs on Windows can cause build issues due to certain APIs not being available because of NTDDI_VERSION based exclusions.

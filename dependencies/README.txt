
***Versions used***

miniupnpc: is release version 1.4
libRocket: is version 1.2.1
MyGui: is version 3.2.0
zzlib version is: 0.13.49
zlib version is: 1.2.3
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
curl is release version 7.69.1 from GitHub
stbrumme-hash-library is version 8 from https://github.com/stbrumme/hash-library obtained source dated 2022-01-22
Protocol Buffers is at release v21.12 from https://github.com/protocolbuffers/protobuf/releases/tag/v21.12
aws-cpp-sdk-core is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-s3 is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-sqs is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-sns is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-cpp-sdk-secretsmanager is version 1.10.51 from GitHub (Jan 13th 2023) @ https://github.com/aws/aws-sdk-cpp/releases/tag/1.10.51
aws-lambda-runtime is version 0.2.7 from GitHub @ https://github.com/awslabs/aws-lambda-cpp/releases/tag/v0.2.7
libyaml is in-flight version 0.2.6 from GitHub @ https://github.com/yaml/libyaml obtained source dated 2023-04-23
opentelemetry-cpp is version 1.21.0 from GitHub
RmlUI is version 6.2 from GitHub @ https://github.com/mikke89/RmlUi/releases/tag/6.2

Note for MS Windows builds: 
- to build the DirectInput dependent code you must have the DirectX SDK installed on your system.
Such external binary dependencies are managed via the ModuleInfo.xml under /dependencies/external
- Note that for some of the dependencies you need to have the Windows SDK installed. Said SDK needs to be of the correct recent version to avoid build issues.
Incorrect older SDKs on Windows can cause build issues due to certain APIs not being available because of NTDDI_VERSION based exclusions.

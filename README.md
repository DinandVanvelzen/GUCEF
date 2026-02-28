# GUCEF

This is the main mono-repo for the Galaxy Unlimited Platform.
The software contained herein is a collection of libraries and derived services and applications all fairly generic in nature.
The overarching purpose of these libraries is to act as a C++ toolchest with which to rapidly develop applications and services.
Some such tools/applications/services are included in the same mono-repo as well.

Note that the main platform modules intentionally do not mandate newer versions of C++ to allow them to be used in situations where no modern compiler is available. 
Any use of newer language features tends to be handled via macros for that reason such that the code can still compile using older compilers.

## About the name
GUCEF stood for "Galaxy Unlimited Client Engine Framework" at one time. It was part of a multi-tier stack that had independent GUCE and GU software stack tiers.
GUCE stood for "Galaxy Unlimited Client Engine".
GUC stood for "Galaxy Unlimited Client".
GU is still the abbreviation for "Galaxy Unlimited".
At this time GUCEF and GUCE have merged together and some parts of GUC and GU have been incorporated as well.
Instead of being client-side focused at lot of the new development has focused on supporting services and generally back end development.
At this time the generic code in this repo is often refered to as the Galaxy Unlimited Platform (GUP).

## A bit of history
Throughout the years various codebases were created and lessons learned  by the author. Slowly but surely a common core of code emerged that started growing organically.
The original common core code base was partially lost a couple times due to data rentention issues and youngster backup practices, further constrained by a lack of financial resources.
Various repositories were merged together, combining various partial backups to create the first version of the GUCEF repo. This first version found its public home on SourceForce.
Originally CVS was used as the version control system, followed by SVN and later followed by Git. When the move to Git was contemplated a new home for the open source repo was also considered.
Due to growing open-source community popularity GitHub won the day as the new home for GUCEF and some related repos where it remains today.

## About the author
Aside from the dependencies almost all code in this repository was written by Dinand Vanvelzen. 
Dinand started programming as a child during the 1980s, writing assembly code for the MSX which was a Philips designed take on a PC concept.
Later on during the 1990s RAD programming principles became the new love afair via Borland Delphi followed by Borland C++ Builder.
At this juncture the author had embarked upon professsional education in the dark arts of software engineering. 
A dual strategy was persued between application programming and back-end/lower-level programming which were distinct degrees at the time.
During this time the codebase you find in this repository continued to grow and evolve as concepts blended and evolved.
During the 2000s the author's main focus was on visualization, mostly 3D graphics both professsionally and as an intellectual curiosity making its way into this repository.
Migration to the United States and starting a family slowed new development to a trickle for a good number of years before beginning to pick back up once the personal situation stabilized.
It is during this phase that youthful idealism started to give way to pragmatism. Professionally the interventional heathcare domain gave way to enterprise healthcare which in turn later gave way to the financial sector.
Subsequent new development has pragmatically focused on common needs in the author's professional life with the incidental oddball projects mixed in as a means to an end.

## mono-repo breakdown
- /platform : this has the main platform modules having both concrete portable functionality and interfaces for utilizing additional functionality via plugins
- /plugins : this has various plugins that can optionally be used to extend platform capabilities
- /projects : this has scripts and the like for generating build projects
- /common/bin : this has binary output of projects
- /dependencies : As the name suggests this folder has the various dependency libraries used by plugins, never by platform libraries directly.
- /tests : As the name suggests, this hold some dedicated test code

## How to generate a project for my IDE
The main method through which IDE project/solution files are supported is via CMake. 
The CMake files themselves are almost all auto-generated via a tool included in this repo called 'ProjectGenerator'.
ProjectGenerator performs an analysis of the files in the repo with minimal ModuleInfo.xml files to identify modules, from there it derives all needed paths and generates the needed files.
This same tool can output middleware files for other toolchains as well, not just CMake. 
There are also premake 4 and premake 5 and android make files included but they are not updated as frequently at the moment so your milage will vary on those.

In most cases you want to go to /projects/CMake first to generate the relevant IDE files.
In that folder you will see various scripts with long names. At some point this will be refactored per the TODO list but for now the scheme is as follows:

As an example you have a script file like 'RunCMake_Shared_CodeBlocks_Unix_Debug_pubsub2pubsub.sh' 
- RunCMake_ : This first segment is just to set it apart from other files in the folder for historical reasons, ignore.
- Shared_ : This segment is mostly related to CMake where the default module build method passed will be 'dynamic' (dll/so) vs static (.lib/.a)
- CodeBlocks_ : This segment has the name of the IDE for which you wish to generate project files, in this example the Code Blocks IDE
- Unix_ : This segment denotes the target platform. Some IDEs support multiple target platforms.
- Debug_ : This segment may not be present for all scripts, it depends on the capabilities of the generator. For some generators the project files themselves need to be created from the start for either debug or release builds. This is not needed in the case of say Visual Studio
- pubsub2pubsub.sh : This segment denotes the main target of the logical view on the entire mono-repo. In this case the service pubsub2pubsub.

In other words the format is:
'RunCMake_Shared/Static_IDE to use_OS Target_Debug/Release_Target name.Executable script extension' 

On Windows when you run the above script and similar it will also trigger a run of the ProjectGenerator.
This is merely intended to make the flow more fool-proof since usually the commited CMake files are already up-to-date and thus this step can usually be x-ed out and skipped.
Following this potential step CMake is invoked to take care of the generation of the build files.
Such files are writting to  /common/bin. This repo follows the thinking that temporary output files should not be co-mingled with code.
If you get into trouble for whatever reason with stale/bad output files (disk issue?) one can always just delete the entire folder structure under /common/bin

## Contained Platform Libraries
- gucefMT: Library with multithreaded programming primitives. Will likely be folded into gucefCORE at some point.
- gucefCORE: Library with functionality that is core to the way functionality build on the platform operates such as the plugin/logging/metrics/eventing/etc systems
- gucefIMAGE: Optional library used when building software that utilizes digital images
- gucefVFS: Optional library used when building software that performs more complex file system style I/O beyond just basic access. VFS = Virtual File System.
- gucefCOMCORE: Optional library used when building software that performs basic networking
- gucefCOM: Optional library used when building software that performs networking and uses fairly industry standard functionality such as StatsD
- gucefWEB: Optional library used when building software that performs Internet/Web style networking
- gucefPUBSUB: Optional library used when building software that uses publish/subscribe concepts
- gucefGUI: Optional library used when a GUI is needed. Main focus is on GUIs within the context of rendering not OS native GUIs
- gucefINPUT: Optional library used when support for human input devices is needed.
- gucefLOADER: Optional library used when dynamically loading multiple GUCEF platform versions dynamically 
- gucefPATCHER: Optional library used when support is needed for auto-patching functionality vs an end-user manually downloading updates
- gucefMATH: Optional library used when performing math calculations via OO concepts
- gucefKAITAI: Optional library used when interpretating binary formats leveraging the Kaitai schema to do so

## Contained Platform Plugins
- dstorepluginPARSIFALXML: gucefCORE plugin: Adds DStore (DataNode) codec support for XML. Writing via GUCEF code and reading via the ParsifalXML dependency library
- dstorepluginJSONPARSER: gucefCORE plugin: Adds DStore (DataNode) codec support for JSON. Uses the json-parser and json-builder dependency libraries
- dstorepluginYAML: gucefCORE plugin: Adds DStore (DataNode) codec support for YAML. Uses the libyaml dependency library
- codecspluginZLIB: gucefCORE plugin: Adds codec support for zlib/gzip/deflate/crc32/adler32
- codecspluginSTBRUMMEHASH: gucefCORE plugin: Adds codec support for CRC32/MD5/SHA1/SHA3/SHA256/Keccak
- imgpluginDEVIL: gucefIMAGE plugin: Adds digital image management functionality sourced from the DEVIL image dependency library. Supports various image codecs.
- imgpluginFLIC: gucefIMAGE plugin: Adds image codec support for the FLIC format
- imgpluginFreeImage: gucefIMAGE plugin: Adds digital image management functionality sourced from the FreeImage image dependency library. Supports various image codecs.
- imgpluginITV: gucefIMAGE plugin: Adds image codec support for the reverse engineered ITV format
- vfspluginAWSS3: gucefVFS plugin: Adds VFS backend for mounting and using AWS S3 as part of the virtual filesystem
- vfspluginDVP: gucefVFS plugin: Obsolete VFS backend for mounting and using DVP archive files as part of the virtual filesystem
- vfspluginITV: gucefVFS plugin: VFS backend for mounting and using reverse engineered ITV archive files as part of the virtual filesystem
- vfspluginVP: gucefVFS plugin: VFS backend for mounting and using 'Violation Pack' archive files as part of the virtual filesystem such as used by the games Decent and FreeSpace
- vfspluginZIP: gucefVFS plugin: VFS backend for mounting and using zlib compatible archive files as part of the virtual filesystem such as .gz and .zip
- inputdriverDIRECTINPUT8: gucefINPUT plugin: Adds an input driver for input device interaction via Microsoft DirectInput 8
- inputdriverMSWINMSG: gucefINPUT plugin: Adds an input driver for input device interaction via Microsoft Windows Win32 API messages
- inputdriverNANDROID: gucefINPUT plugin: Adds an input driver for input device interaction via native Android APIs
- inputdriverOIS: gucefINPUT plugin: Adds an input driver for input device interaction via the OIS dependency library
- inputdriverXWINMSG: gucefINPUT plugin: Adds an input driver for input device interaction via the X-Windowing messaging system for Linux
- pubsubpluginAWSSNS: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for AWS's SNS service. Uses the AWS C++ SDK.
- pubsubpluginAWSSQS: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for AWS's SQS service. Uses the AWS C++ SDK.
- pubsubpluginKAFKA: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for Kafka streams. Uses the RdKafka dependency library.
- pubsubpluginMSMQ: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for MSMQ. Uses the Windows Operating system MSMQ subsystem which needs to be installed.
- pubsubpluginREDISCLUSTER: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for Redis streams. Uses redis++ and hiredis dependencies.
- pubsubpluginSTORAGE: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend that allows easy interaction with the VFS and its capabilities
- pubsubpluginUDP: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for UDP pub/sub via unicast or multicast
- pubsubpluginWEB: gucefPUBSUB plugin: Adds a pub-sub concept compatible backend for Web concepts like HTTP/REST/WebSockets
- ProjectGenDependsFilter: ProjectGen plugin: Allows filtering of libraries in a repo based on a depends.exe tool output report
- ProjectGenVSImporter: ProjectGen plugin: Imports Visual Studio project files to generate a ModuleInfo.xml starting point for a new repo
- comcorepluginDBL: gucefCOMCORE plugin which attempts to provide information from the reverse engineered Myricom DBL network driver
- comcorepluginGEOOSM: gucefCOMCORE plugin which adds Open Street Maps based geo location lookup capabilities

## Contained Services
- UdpViaTCP: Bridge service which funnels UDP traffic over a TCP segment. Will be superseded by pubsub2pubsub.
- udp2kafka: Adapter service which allows ingress of UDP traffic into Kafka streams. Will be superseded by pubsub2pubsub.
- udp2redis: Adapter service which allows ingress of UDP traffic into Redis streams for non-clustered redis. Will be superseded by pubsub2pubsub.
- udp2rediscluster: Adapter service which allows ingress of UDP traffic into Redis streams for clustered redis. Will be superseded by pubsub2pubsub.
- ProcessMetrics: Agent service which obtains metrics for a target process. Intended as targeted light-weight alternative to high-frequency performance counters and the like
- redisinfo: Monitoring service which uses the Redis protocol to obtain information about Redis clusters and exposes such information as metrics to broadcast
- pubsub2storage: Adapter service which translates more generally between messaging paradigms and storage. Will be superseded by pubsub2pubsub.
- pubsub2pubsub: Adapter service which translates more generally between different messaging paradigms. Will supersede many of the other adapter services.
- FilePusher: Agent service which monitors the file system for certain files and pushes them to a VFS destination for example AWS S3.
- FileSorter: Agent service which automatically sorts files. Handy for initial sort for digital picture and video archive for example.
- GucefLogService: Service which can accept GUCEF platform logs streamed via network connection. Aimed at situations where we cannot have/access local logs
- ServerPortExtender: Service used to reverse inbound/outbound connection initiation for an application server port thus bypassing egress only restrictions
- UdpTransformer: Service which takes ingress UDP and performs a simplistic transform followed by a retransmission
- MsmqMetrics: Agent service which gathers information on MSMQ queues and subsequently gathers and transmits metrics for said queues

## Contained Tools
- ProjectGenerator: Tool to auto generate various project/module files and different logical views of the same mono-repo
- DCSBruteInstaller: Tool to leverage a combo of some hardware and this software to brute force crack the codes in a DCS home alarm panel
- itvExporter: Reverse engineered export tool for assets contained in a classic game
- GucefArchiver: Basic archiver tool that leverages the VFS and its plugins
- HDFiller: As the name suggests, it fills your hard drive. Intended for test scenarios.
- PubSubStorageTool: Tool to manipulate storage files produced by the pubsub storage plugin

## Contained Misc Libraries
- MemoryLeakFinder: Dynamically loaded platform helper library to track down memory leaks aided by GUCEF platform functionality
- ProjectGen: Library that holds all the logic of the ProjectGenerator tool. Its supports plugins to expand functionality.

## TODO list
- IN PROGRESS: Performance improvements for ProjectGenerator
- IN PROGRESS: Add web assembly support via emscripten
- IN PROGRESS: Add support for 'static' plugins (needed for monolithic WASM builds)

- Short term: Add basic websocket support native to the platform
- Short term: Add proper parameter support for codecs
- Short term: Add StringView support
- Short term: Complete initial pass at pubsub2pubsub 'web' plugin
- Short term: Complete initial pass at pubsub2pubsub 'aws sns' plugin
- Short term: Complete initial pass at pubsub2pubsub 'aws sqs' plugin
- Short term: Add config driven CodecChain class
- Mid term: Add gRPC plugin based on API registry and data driven codecs
- Mid term: Add JSON RPC plugin based on API registry and data driven codecs
- Mid term: Fix the GitHub CI integration. The auto build trigger link is broken.
- Mid term: Add DataNode schema system
- Mid term: Add config driven message transform engine
- Mid term: Add AWS Lambda support
- Mid term: Use StringView instead of String for all static string references
- Mid term: Turn ProjectGenerator into something that can run as a background service
- Mid term: Complete GitHub Actions support via ProjectGenerator
- Mid term: Add GitLab CI support via ProjectGenerator
- Mid term: Complete renaming platform source files to all have module prefix
- Mid term: Remove dead code as part of a larger revitalization effort of projects contained within the mono-repo. Determine which other apps contained herein is worth saving.
- Mid term: Recreate the platform console client functionality
- Mid term: Add native secure socket support for the supported platforms
- Mid term: Add OTEL support to gucefCOM
- Long term: rename gucefCOM to gucefCOMM
- Long term: Add Jenkins CI support via ProjectGen's CI Helper backend
- One day: Deprecate/undo the classic MFC style 'C' prefix for classes. This would be a huge change breaking everything which is why it has not occured yet. 
- Ongoing: Evaluate need to update various dependencies considering plethora of constraints
- Ongoing: Refine logging/metrics
- Ongoing: Add more javadoc format (doxygen compatible) documentation

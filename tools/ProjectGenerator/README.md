# GUCEF ProjectGenerator

A powerful, cross-platform project file generator designed for large-scale mono-repositories. ProjectGenerator is part of the GUCEF (Galaxy Unlimited Framework) ecosystem and serves as a unified build system abstraction layer.

## Overview

ProjectGenerator is an intelligent build system generator that transforms minimal XML module descriptions into comprehensive project files for multiple build systems and IDEs. Born from the necessity to manage complex mono-repositories, it automates the tedious aspects of build system maintenance while providing flexibility for various development workflows.

### Key Philosophy
- **Minimal Input, Maximum Output**: Describe your module with just a name and dependencies - let the tool figure out the rest
- **Build System Agnostic**: Generate files for multiple build systems from a single source of truth
- **Mono-Repo Optimized**: Built specifically to handle the complexities of large, multi-project repositories
- **CI/CD Ready**: Includes intelligent CI pipeline generation with change-based build triggering

## Features

### Intelligent Project Analysis
- **Automatic File Discovery**: Scans directory structures to identify source files, headers, and resources
- **License Detection**: Automatically determines project licenses from copyright headers
- **Semantic Versioning**: Integrates with semVer files when present
- **Dependency Resolution**: Simple name-based dependency references with automatic path resolution

### Multi-Target Generation
Supports **7 different output generators**:

| Generator | Description | Use Case |
|-----------|-------------|----------|
| **XML** | Project information dump | Caching, middleware integration |
| **CMake** | CMake build files | Cross-platform C++ builds |
| **Premake4** | Premake 4 project files | IDE project generation |
| **Premake5** | Premake 5 project files | IDE project generation |
| **Android Make** | Android.mk makefiles | Android NDK development |
| **Arduino CLI** | Arduino IDE projects | Microcontroller development |
| **CI Helper** | CI/CD automation files | GitHub Actions, GitLab CI |

### Advanced CI/CD Integration
- **Smart Build Triggering**: Only builds targets affected by code changes
- **GitHub Actions Workflows**: Auto-generates complete CI pipelines
- **Glob Pattern Generation**: Creates file watch patterns for change detection
- **Multi-Platform Support**: Handles Windows, Linux, and cross-compilation scenarios

### Extensible Architecture
- **Preprocessor System**: Custom data manipulation before generation
- **Plugin Support**: Visual Studio project import, dependency filtering
- **Template System**: Customizable output with macro replacements
- **Caching Mechanism**: Speeds up subsequent runs with project info caching

## Module Definition (ModuleInfo.xml)

The heart of ProjectGenerator is the `ModuleInfo.xml` file that describes each module in your project. This file should be placed in the root directory of each 'module' you wish to define.

### Supported Module Types

| Module Type | Description | Generated Artifacts |
|-------------|-------------|-------------------|
| `Executable` | Applications and console programs | `.exe`, binary executables |
| `SharedLibrary` | Dynamic libraries | `.dll`, `.so`, `.dylib` |
| `StaticLibrary` | Static libraries | `.lib`, `.a` |
| `HeaderIncludeLocation` | Header-only libraries | Referenced by dependents as include path only |
| `CodeIntegrateLocation` | Source code for inclusion | Code integration into another module |
| `HeaderIntegrateLocation` | Headers for inclusion | Header integration into another module |
| `ReferenceLibrary` | .NET/C# libraries | Managed references |
| `BinaryPackage` | Pre-compiled packages | Package definitions |

### Real GUCEF Examples

#### Simple Executable (from ArchiveDiff tool)
```xml
<ModuleInfoEntry>
    <Module Name="ArchiveDiff" Type="Executable" Platform="mswin;linux" Tags="tools">
        <Dependencies>
            <Dependency Name="gucefMT" />
            <Dependency Name="gucefCORE" />
            <Dependency Name="gucefPATCHER" />
            <Dependency Name="ArchiveDiffLib" />
        </Dependencies>
        <Linker>
            <Dependency Name="gucefMT" />
            <Dependency Name="gucefCORE" />
            <Dependency Name="gucefPATCHER" />
            <Dependency Name="ArchiveDiffLib" />
        </Linker>
        <Preprocessor>
            <Define String="ARCHIVEDIFF_BUILD_MODULE" />
        </Preprocessor>
    </Module>
</ModuleInfoEntry>
```

#### Simple Header-Only Library
```xml
<ModuleInfoEntry>
    <Module Name="gucefHeaders" Type="HeaderIncludeLocation" Platform="All" Tags="platform">
    </Module>
</ModuleInfoEntry>
```

#### Advanced Multi-Platform Library (from gucefCORE)
```xml
<ModuleInfoEntries>
<ModuleInfoEntry>
    <ModuleMetaData>
        <License>APACHEv2</License>
        <SemVer>0.1.0.0</SemVer>
        <DescriptionHeadline>GUCEF module providing O/S abstraction and generic solutions</DescriptionHeadline>
        <Authors>
            <Author>Dinand Vanvelzen</Author>
        </Authors>
        <Maintainers>
            <Maintainer>Dinand Vanvelzen</Maintainer>
        </Maintainers>
    </ModuleMetaData>
    <Module Name="gucefCORE" Type="SharedLibrary" Platform="All" Tags="platform">
        <Dependencies>
            <Dependency Name="gucefMT" />
        </Dependencies>
        <Linker>
            <Dependency Name="gucefMT" />
        </Linker>
        <Preprocessor>
            <Define String="GUCEF_CORE_BUILD_MODULE" />
        </Preprocessor>
    </Module>
    <Module Platform="mswin">
        <Linker>
            <Dependency Name="Ws2_32.lib" />
            <Dependency Name="PowrProf.lib" />
        </Linker>
    </Module>  
    <Module Platform="Linux">
        <Linker>
            <Dependency Name="dl" />
            <Dependency Name="rt" />
        </Linker>
    </Module>
    <Module Platform="Android">
        <Linker>
            <Dependency Name="dl" />
        </Linker>
    </Module>
</ModuleInfoEntry>
</ModuleInfoEntries>
```

#### Application with Dependencies (from gucefPATCHERAPP)
```xml
<ModuleInfoEntry>
    <Module Name="gucefPATCHERAPP" Type="SharedLibrary" Platform="All">
        <Dependencies>
            <Dependency Name="gucefHeaders" />
            <Dependency Name="gucefMT" />
            <Dependency Name="gucefCORE" />
            <Dependency Name="gucefCOMCORE" />
            <Dependency Name="gucefCOM" />
            <Dependency Name="gucefPATCHER" />
            <Dependency Name="gucefINPUT" />
            <Dependency Name="gucefGUI" />
            <Dependency Name="gucefIMAGE" />
        </Dependencies>
        <Linker>
            <Dependency Name="gucefMT" />
            <Dependency Name="gucefCORE" />
            <Dependency Name="gucefCOMCORE" />
            <Dependency Name="gucefCOM" />
            <Dependency Name="gucefPATCHER" />
            <Dependency Name="gucefINPUT" />
            <Dependency Name="gucefGUI" />
            <Dependency Name="gucefIMAGE" />
        </Linker>
        <Preprocessor>
            <Define String="GUCEF_PATCHERAPP_BUILD_MODULE" />
        </Preprocessor>
    </Module>
</ModuleInfoEntry>
```

### Key XML Structure Elements

| Element | Description | Usage |
|---------|-------------|-------|
| `<ModuleInfoEntry>` | Root container for module definition | Single module |
| `<ModuleInfoEntries>` | Root XML node for multiple ModuleInfoEntry sections | Name is arbitrary but per XML we must have a singular root element |
| `<ModuleMetaData>` | Metadata section | License, version, authors |
| `<Module>` | Core module definition | Name, Type, Platform, Tags |
| `<Dependencies>` | Build-time dependencies | Required modules |
| `<RuntimeDependencies>` | run-time dependencies | Inclusion of plugins in output project |
| `<Linker>` | Link-time dependencies | Libraries to link |
| `<Preprocessor>` | Compiler definitions | #define statements |

### Module Attributes

| Attribute | Description | Values |
|-----------|-------------|--------|
| `Name` | Module identifier | Any valid identifier |
| `Type` | Module type | See supported types above |
| `Platform` | Target platforms | `All`, `mswin`, `linux`, `Android`, etc. |
| `Tags` | Classification tags | `platform`, `tools`, `app`, etc. |

### Platform Support

GUCEF ProjectGenerator supports platform-specific configurations:

| Platform | Description | Common Uses |
|----------|-------------|-------------|
| `All` | Cross-platform code | Default implementation |
| `mswin` | Microsoft Windows | MS Windows-specific code |
| `win32` | 32-bit Microsoft Windows | MS Windows-specific code |
| `win64` | 64-bit Microsoft Windows | MS Windows-specific code |
| `linux` | Linux systems | Linux based distribution specific code |
| `linux32` | 32-bit Linux systems | Linux based distribution specific code |
| `linux64` | 64-bit Linux systems | Linux based distribution specific code |
| `android` | Android with support for native Linux code | Android NDK |
| `android32` | 32-bit Android with support for native Linux code | Linux based distribution specific code |
| `android64` | 64-bit Android with support for native Linux code | Linux based distribution specific code |

Note that mswin is an alias for win32 and win64, specifying both at the same time.
The same exists for linux and android, where linux is an alias for linux32 and linux64, and android is an alias for android32 and android64.

## Configuration Files

### Directory Processing Control

ProjectGenerator supports several configuration files to control how directories and files are processed:

#### ProjectGenExcludeList.txt

A simple text file listing directories and files to ignore during processing. Supports wildcard patterns.
# Comments start with #
temp/
*.tmp
build*/
.git
.svn
node_modules/
*.bak

## Quick Start

### Basic Module Definition
Create a simple XML file describing your module:
```xml
<ModuleInfoEntry>
    <Module Name="MyLibrary" Type="SharedLibrary" Platform="All" Tags="myproject">
        <Dependencies>
            <Dependency Name="gucefCORE" />
        </Dependencies>
        <Linker>
            <Dependency Name="gucefCORE" />
        </Linker>
        <Preprocessor>
            <Define String="MYLIBRARY_BUILD_MODULE" />
        </Preprocessor>
    </Module>
</ModuleInfoEntry>
```

### Running ProjectGenerator # Generate all supported formats
ProjectGenerator.exe

### Running ProjectGenerator
```bash

# Specify config file to use
ProjectGenerator.exe *ConfigPath=C:\example\GUCEF\projects\ProjectGenerator.ini*

```

### Generated Output Structure

output/
??? targets/                    # Individual target files
?   ??? MyLibrary/
?   ?   ??? CMakeLists.txt
??? .github/workflows/          # GitHub Actions
?   ??? MyLibrary.yml
??? Project.xml                 # Cached project info

## Command Line Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `rootDir` | Source directory to scan | Current directory |
| `ConfigPath` | Path where a config file can be found | Current working directory etc |
| `MinimalLogLevel` | Logging verbosity (0-6) | `2` |
| `envOverridesFile` | Path to a text file which includes overrides for environment variables for use in restrictive environments | Not used by default |
| `outputDir` | Directory where the log file will be written. Independent of ProjectGenerator output | Current working directory |
| `projectName` | Name of the overall project, the overarching name of the body of work | Auto-detected: root dir name |
| `useProjectInfoCache` | Directory where the log file will be written. Independent of ProjectGenerator output | `false` |
| `generators` | Semicolon-separated list of generators | `xml` |
| `dirsToIgnore` | Semicolon-separated ignore list | Empty |


### Generator-Specific Parameters

#### CMake Generator
- `cmakegen:TargetsDir` - Custom targets output directory
- `cmakegen:GenerateTargetsOnly` - Only generate target files

#### CI Helper Generator  
- `cihelpergen:GenerateGithubActionsWorkflowTargetsYml` - Generate GitHub workflows
- `cihelpergen:GithubActionsWorkflowsUseSelfHostedRunners` - Use self-hosted runners
- `cihelpergen:GenerateGlobPatternPathListPerTarget` - Generate glob patterns

#### Premake Generators
- `premake4gen:TemplateDir` - Template directory for Premake4
- `premake5gen:TemplateDir` - Template directory for Premake5
- `premake4gen:PM4OutputDir` - Premake4 output directory
- `premake5gen:PM5OutputDir` - Premake5 output directory

## Architecture

### Core Components

ProjectGenerator
??? Info Gatherers
?   ??? CDirCrawlingProjectInfoGatherer  # Scans filesystem
?   ??? Visual Studio Importer           # Imports .vcxproj files
??? Preprocessors
?   ??? Dependency Filter               # Filters based on binary deps
?   ??? Visual Studio Preprocessor     # Converts VS projects
??? Generators
    ??? CXmlProjectGenerator           # XML output
    ??? CCMakeProjectGenerator         # CMake files
    ??? CPremake4/5ProjectGenerator    # Premake files
    ??? CAndroidMakefileGenerator      # Android makefiles
    ??? CArduinoCLIGenerator          # Arduino projects
    ??? CCIHelperGenerator            # CI/CD automation

### Processing Pipeline

1. **Configuration Loading**: Loads settings from INI files and command line
2. **Information Gathering**: Scans directories and analyzes source code
3. **Preprocessing**: Applies filters and transformations
4. **Generation**: Creates output files using selected generators
5. **Caching**: Stores project info for faster subsequent runs

## Use Cases

### Large Mono-Repository Management
Perfect for repositories with hundreds of modules where maintaining individual build files becomes impractical.

### Cross-Platform Development
Generate build files for multiple platforms and IDEs from a single source of truth.

### CI/CD Optimization
Automatically generate CI pipelines that only build affected components, dramatically reducing build times.

### Legacy Project Modernization
Use preprocessors to import existing Visual Studio projects and modernize build systems.

### Embedded Development
Arduino CLI generator creates proper directory structures for microcontroller development.

## Advanced Configuration

### Custom Templates
CMake and Premake generators support custom templates with macro replacement:

# Template file: SharedLibrary.cmake.template
```cmake
project(${PROJECT_NAME})
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${INCLUDE_DIRS})
```

### Environment Overrides
Support for environment variable overrides from file:
#### env_overrides.txt
```text
CUSTOM_BUILD_PATH=/path/to/custom/builds
SPECIAL_COMPILER_FLAGS=-O3 -march=native
```

### Platform-Specific Configurations
```xml
<Module Name="PlatformSpecific" Type="SharedLibrary" Platform="All">
    <Dependencies>
        <Dependency Name="gucefCORE" />
    </Dependencies>
    <Linker>
        <Dependency Name="gucefCORE" />
    </Linker>
</Module>
<Module Platform="mswin">
    <Linker>
        <Dependency Name="user32.lib" />
        <Dependency Name="kernel32.lib" />
    </Linker>
</Module>
<Module Platform="linux">
    <Linker>
        <Dependency Name="dl" />
        <Dependency Name="pthread" />
    </Linker>
</Module>
```

## Contributing

ProjectGenerator is one of the oldest surviving projects in the GUCEF ecosystem dating back to the early 2000s and contains battle-tested code handling numerous edge cases. The codebase uses a more functional, C-like style that is being gradually modernized.

### Development Notes
- **Edge Case Handling**: The code accounts for many repository oddities - consider this when making changes
- **Performance Focus**: Current work focuses on optimizing generation time for large repositories
- **Backward Compatibility**: Maintain compatibility with existing project configurations

### Building
ProjectGenerator uses its own tooling for building:# Generate build files
ProjectGenerator.exe *generators=cmake*

# Build with CMake
cd build
cmake --build .

## License

This library is free software; you can redistribute it and/or modify it under the terms of the Apache v2 license.

## Related Projects

ProjectGenerator is part of the larger GUCEF (Galaxy Unlimited Framework) ecosystem:
- **gucefCORE**: Core utilities and platform abstraction
- **gucefMT**: Multi-threading support
- **Build System**: Complete build automation for large C++ projects

---

**Copyright (C) Dinand Vanvelzen**

*ProjectGenerator: Reducing the complexity of build system management for large-scale C++ development.*
## Usage/Examples

```javascript
import Component from 'my-project'

function App() {
  return <Component />
}
```


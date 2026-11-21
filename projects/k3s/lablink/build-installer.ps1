# ---------------------------------------------------------------------------
# Makes the Windows setup program of gucefLabLink (gucefLabLink-<version>-setup.exe):
# builds gucefLabLink from this checkout, as install.ps1 does, and packs the
# program, its DLLs, install.ps1 and uninstall.ps1 with CPack and NSIS
# (windows-installer\CMakeLists.txt). The setup program is run as an
# administrator on the machines of the lab and installs the Windows service
# gucefLabLink, see README.md.
#
# Usage (a Developer PowerShell; an administrator is not needed):
#   .\build-installer.ps1 [-Prebuilt DIR] [-OutDir DIR] [-Version X.Y.Z]
#   -Prebuilt DIR  pack the built files in DIR (gucefLabLink.exe and its DLLs,
#                  as a Windows build puts them in its bin directory) instead
#                  of building
#   -OutDir DIR    where the setup program goes; default:
#                  common\bin\lablink-windows-installer of this checkout
#   -Version X.Y.Z the version of the package; default: that of the program
# Needs cmake, and for building ninja and the compiler of Visual Studio, in the
# PATH. NSIS (makensis.exe, https://nsis.sourceforge.io, winget install
# NSIS.NSIS) makes the setup program; without it a ZIP is made.
# ---------------------------------------------------------------------------

param(
    [string] $Prebuilt = "",
    [string] $OutDir = "",
    [string] $Version = ""
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$GucefRoot = Split-Path -Parent ( Split-Path -Parent ( Split-Path -Parent $ScriptDir ) )
$InstallerSource = Join-Path $ScriptDir "windows-installer"
$Work = Join-Path $env:LOCALAPPDATA "gucef-lablink-installer"
if ( $OutDir -eq "" ) { $OutDir = Join-Path $GucefRoot "common\bin\lablink-windows-installer" }

function Fail( [string] $message )
{
    Write-Host "ERROR: $message"
    exit 1
}

if ( $null -eq ( Get-Command "cmake" -ErrorAction SilentlyContinue ) ) { Fail "cmake is not in the PATH." }

if ( $Prebuilt -ne "" )
{
    $BinDir = ( Resolve-Path -LiteralPath $Prebuilt ).Path
}
else
{
    foreach ( $tool in "ninja", "cl" )
    {
        if ( $null -eq ( Get-Command $tool -ErrorAction SilentlyContinue ) )
        {
            Fail "$tool is not in the PATH: run this from a Developer PowerShell of Visual Studio, or give -Prebuilt DIR."
        }
    }
    $source = Join-Path $GucefRoot "projects\CMake\targets\GUCEF_exe_gucefLabLink"
    if ( -not ( Test-Path -LiteralPath $source ) ) { Fail "$source is not there: run this from a checkout, or give -Prebuilt DIR." }
    New-Item -ItemType Directory -Force -Path $Work | Out-Null
    $AppDir = Join-Path $Work "app"
    $buildLog = Join-Path $Work "build.log"
    Remove-Item -LiteralPath $buildLog -Force -ErrorAction SilentlyContinue
    Write-Host "Building gucefLabLink from $GucefRoot (log: $buildLog)"
    & cmake -S $source -B $AppDir -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON *>> $buildLog
    if ( $LASTEXITCODE -eq 0 ) { & cmake --build $AppDir *>> $buildLog }
    if ( $LASTEXITCODE -ne 0 )
    {
        Select-String -LiteralPath $buildLog -Pattern "error" | Select-Object -First 20 | ForEach-Object { Write-Host $_.Line }
        Fail "the build failed, see $buildLog."
    }
    $BinDir = Join-Path $AppDir "bin"
}
if ( -not ( Test-Path -LiteralPath ( Join-Path $BinDir "gucefLabLink.exe" ) ) ) { Fail "$BinDir\gucefLabLink.exe is not there." }

New-Item -ItemType Directory -Force -Path $Work, $OutDir | Out-Null
$PackageDir = Join-Path $Work "package"
Remove-Item -LiteralPath $PackageDir -Recurse -Force -ErrorAction SilentlyContinue
$arguments = @( "-S", $InstallerSource, "-B", $PackageDir, "-G", "Ninja", "-DLABLINK_BIN_DIR=$BinDir" )
if ( $Version -ne "" ) { $arguments += "-DLABLINK_VERSION=$Version" }
& cmake @arguments
if ( $LASTEXITCODE -ne 0 ) { Fail "the installer project could not be configured." }
& cpack --config ( Join-Path $PackageDir "CPackConfig.cmake" ) -B $OutDir
if ( $LASTEXITCODE -ne 0 ) { Fail "cpack failed." }

Write-Host ""
Write-Host "Made:"
Get-ChildItem -LiteralPath $OutDir -Filter "gucefLabLink-*" -File | Sort-Object LastWriteTime -Descending | Select-Object -First 2 | ForEach-Object { Write-Host ( "  " + $_.FullName ) }
Write-Host "Run the setup program as an administrator on the machine; /SECRET=<lab secret> links it without approval on the cluster."
exit 0

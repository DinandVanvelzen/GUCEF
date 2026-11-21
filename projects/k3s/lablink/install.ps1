# ---------------------------------------------------------------------------
# Installs gucefLabLink on this Windows machine and runs it as the Windows
# service gucefLabLink (see tools/gucefLabLink/README.md), the counterpart of
# install.sh on Linux:
#
#   - the machine finds the cluster, links to it with the lab secret or after
#     approval on the cluster, and reports to the hub
#   - the files: the program in %ProgramFiles%\gucef-lablink\<version>, reached
#     through the junction current; the state directory
#     %ProgramData%\gucef-lablink (lab secret, identity, logs), which only
#     SYSTEM and the Administrators can access; the settings of the service in
#     %ProgramData%\gucef-lablink\lablink.conf
#   - the service: automatic start, restarted when it fails, registered with
#     sc.exe and started
#   - the firewall: the discovery ports (and on a cluster the ports of the lab
#     link) open for the lab networks only
#
# Joining the cluster as a k3s node and setting up Claude Code are not done on
# Windows: LAB_JOIN and LAB_CLAUDE_USER do not apply here.
#
# Idempotent: a re-run installs and restarts only when the program or the
# settings differ.
#
# Usage (an elevated PowerShell):
#   .\install.ps1 [-Secret SECRET] [-SkipTests] [-Prebuilt DIR] [-Upgrade] [-NoFirewall]
#   -Secret SECRET  the lab secret the cluster printed (or the environment
#                   variable GUCEF_LAB_SECRET); without it the machine
#                   asks the cluster for access and waits until it is approved
#                   there: sudo gucefLabLink --approve <name>
#   -SkipTests      install without running the tests
#   -Prebuilt DIR   install the built files in DIR (gucefLabLink.exe and its
#                   DLLs, as a Windows build puts them in its bin directory)
#                   instead of building
#   -Upgrade        only when the service is installed already: install the new
#                   files and keep lablink.conf
#   -NoFirewall     leave the firewall as it is
# Settings: config.env next to this script, the same file as for install.sh
# (see config.env.example). Building needs cmake, ninja and the compiler of
# Visual Studio in the PATH (a Developer PowerShell).
# ---------------------------------------------------------------------------

param(
    [string] $Secret = "",
    [switch] $SkipTests,
    [string] $Prebuilt = "",
    [switch] $Upgrade,
    [switch] $NoFirewall
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

# the secret may also come in the environment variable GUCEF_LAB_SECRET (the installer uses it: not on a command line)
if ( $Secret -eq "" -and $env:GUCEF_LAB_SECRET ) { $Secret = $env:GUCEF_LAB_SECRET }

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$K3sDir = Split-Path -Parent $ScriptDir
$GucefRoot = Split-Path -Parent (Split-Path -Parent $K3sDir)
$Service = "gucefLabLink"
$StateDir = Join-Path $env:ProgramData "gucef-lablink"
$ConfigFile = Join-Path $StateDir "lablink.conf"
$LogFile = Join-Path $StateDir "logs\gucefLabLink.log"
$BuildDir = Join-Path $env:ProgramData "gucef-lablink-build"     # outside the state directory
$InstallRoot = Join-Path $env:ProgramFiles "gucef-lablink"
$KeepVersions = 3
$SystemSid = "*S-1-5-18"
$AdministratorsSid = "*S-1-5-32-544"
$FirewallGroup = "gucef-lab"
$script:LastOutput = ""
$TestBinDir = ""

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Fail( [string] $message )
{
    Write-Host "ERROR: $message"
    exit 1
}

# Runs a program and waits for it; the output goes to the files, the exit code is returned.
# gucefLabLink is a Windows application, which a console does not wait for unless it is started this way.
function Invoke-Program( [string] $program, [string] $arguments, [string] $workingDir = "" )
{
    $out = [System.IO.Path]::GetTempFileName()
    $err = [System.IO.Path]::GetTempFileName()
    try
    {
        $start = @{ FilePath = $program; Wait = $true; PassThru = $true; NoNewWindow = $true;
                    RedirectStandardOutput = $out; RedirectStandardError = $err }
        if ( $arguments -ne "" ) { $start.ArgumentList = $arguments }
        if ( $workingDir -ne "" ) { $start.WorkingDirectory = $workingDir }
        $process = Start-Process @start
        $script:LastOutput = ( Get-Content -LiteralPath $out -Raw -ErrorAction SilentlyContinue ) + ( Get-Content -LiteralPath $err -Raw -ErrorAction SilentlyContinue )
        return $process.ExitCode
    }
    finally
    {
        Remove-Item -LiteralPath $out, $err -Force -ErrorAction SilentlyContinue
    }
}

# A Windows command line argument: quoted when it holds spaces or quotes, as the C runtime splits it again
function Quote( [string] $argument )
{
    if ( $argument -ne "" -and $argument -notmatch '[\s"]' ) { return $argument }
    $escaped = [regex]::Replace( $argument, '(\\*)"', { param( $m ) $m.Groups[ 1 ].Value + $m.Groups[ 1 ].Value + '\"' } )
    $escaped = [regex]::Replace( $escaped, '(\\+)$', { param( $m ) $m.Groups[ 1 ].Value + $m.Groups[ 1 ].Value } )
    return '"' + $escaped + '"'
}

# KEY=value lines of config.env; comments and the quotes around a value are left out
function Read-Settings( [string] $path )
{
    $settings = @{}
    foreach ( $line in Get-Content -LiteralPath $path )
    {
        $text = $line.Trim()
        if ( $text -eq "" -or $text.StartsWith( "#" ) ) { continue }
        if ( $text -match '^([A-Z_][A-Z0-9_]*)=(.*)$' )
        {
            $value = $Matches[ 2 ].Trim()
            if ( $value.Length -ge 2 -and ( ( $value.StartsWith( '"' ) -and $value.EndsWith( '"' ) ) -or ( $value.StartsWith( "'" ) -and $value.EndsWith( "'" ) ) ) )
            {
                $value = $value.Substring( 1, $value.Length - 2 )
            }
            $settings[ $Matches[ 1 ] ] = $value
        }
    }
    return $settings
}

function Get-Setting( [hashtable] $settings, [string] $name, [string] $default )
{
    if ( $settings.ContainsKey( $name ) -and $settings[ $name ] -ne "" ) { return $settings[ $name ] }
    return $default
}

# The network of an address, as CIDR
function Get-NetworkCidr( [string] $address, [int] $prefixLength )
{
    $bytes = ( [System.Net.IPAddress]::Parse( $address ) ).GetAddressBytes()
    for ( $i = 0; $i -lt 4; $i++ )
    {
        $bits = [Math]::Max( 0, [Math]::Min( 8, $prefixLength - 8 * $i ) )
        $mask = ( 0xFF -shl ( 8 - $bits ) ) -band 0xFF
        $bytes[ $i ] = $bytes[ $i ] -band $mask
    }
    return ( New-Object System.Net.IPAddress( ,$bytes ) ).ToString() + "/" + $prefixLength
}

# The networks of the interfaces of this machine, not those of WSL, Hyper-V, VPNs and link-local addresses
function Get-LocalNetworks
{
    $networks = @()
    foreach ( $ip in Get-NetIPAddress -AddressFamily IPv4 -ErrorAction SilentlyContinue )
    {
        if ( $ip.InterfaceAlias -match '^(vEthernet|Loopback|Tailscale|Bluetooth|ZeroTier)' ) { continue }
        if ( $ip.IPAddress -like "127.*" -or $ip.IPAddress -like "169.254.*" ) { continue }
        $networks += Get-NetworkCidr $ip.IPAddress $ip.PrefixLength
    }
    return @( $networks | Sort-Object -Unique )
}

# Owner the Administrators, access for SYSTEM and the Administrators only, passed on to what is made in it
function Protect-StateDirectory( [string] $path )
{
    New-Item -ItemType Directory -Force -Path $path | Out-Null
    & icacls.exe $path /inheritance:r /grant:r "${SystemSid}:(OI)(CI)F" "${AdministratorsSid}:(OI)(CI)F" /Q | Out-Null
    if ( $LASTEXITCODE -ne 0 ) { Fail "the access of $path cannot be limited to SYSTEM and the Administrators" }
    & icacls.exe $path /setowner $AdministratorsSid /Q | Out-Null
    if ( $LASTEXITCODE -ne 0 ) { Fail "the owner of $path cannot be set" }
}

# The SHA-256 of the files of a directory, their names included: the version of an installation
function Get-DirectoryVersion( [string] $path )
{
    $root = ( Resolve-Path -LiteralPath $path ).Path.TrimEnd( '\' )
    $lines = Get-ChildItem -LiteralPath $root -Recurse -File | Sort-Object FullName | ForEach-Object {
        ( Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName ).Hash + "  " + $_.FullName.Substring( $root.Length + 1 )
    }
    $sha = [System.Security.Cryptography.SHA256]::Create()
    $digest = $sha.ComputeHash( [System.Text.Encoding]::UTF8.GetBytes( ( $lines -join "`n" ) ) )
    return ( -join ( $digest | ForEach-Object { $_.ToString( "x2" ) } ) ).Substring( 0, 12 )
}

# sc.exe with its arguments as one command line, so that the quotes of binPath= reach it as they are
function Invoke-Sc( [string] $arguments )
{
    return Invoke-Program "sc.exe" $arguments
}

# ---------------------------------------------------------------------------
# Checks and settings
# ---------------------------------------------------------------------------

$principal = New-Object Security.Principal.WindowsPrincipal( [Security.Principal.WindowsIdentity]::GetCurrent() )
if ( -not $principal.IsInRole( [Security.Principal.WindowsBuiltInRole]::Administrator ) )
{
    Fail "run this from an elevated PowerShell (Run as administrator): it installs a Windows service."
}
$existing = Get-Service -Name $Service -ErrorAction SilentlyContinue
if ( $Upgrade -and ( $null -eq $existing -or -not ( Test-Path -LiteralPath $ConfigFile ) ) )
{
    exit 0
}

$SettingsFile = Join-Path $ScriptDir "config.env"
$checkout = Test-Path -LiteralPath ( Join-Path $GucefRoot "projects\CMake\targets\GUCEF_exe_gucefLabLink_TestApp" )
if ( -not $checkout -and $Prebuilt -eq "" )
{
    Fail "there is no checkout around this script: give the built files with -Prebuilt DIR."
}
if ( $Prebuilt -ne "" -and -not ( Test-Path -LiteralPath ( Join-Path $Prebuilt "gucefLabLink.exe" ) ) )
{
    Fail "$Prebuilt\gucefLabLink.exe is not there."
}
if ( -not ( Test-Path -LiteralPath $SettingsFile ) )
{
    Copy-Item -LiteralPath ( Join-Path $ScriptDir "config.env.example" ) -Destination $SettingsFile
    Write-Host "Created $SettingsFile (all settings commented out, defaults apply)."
}
$settings = Read-Settings $SettingsFile

# no k3s server runs on Windows: a machine, unless the settings say otherwise
$Role = Get-Setting $settings "LAB_ROLE" "machine"
if ( $Role -ne "machine" -and $Role -ne "cluster" ) { Fail "LAB_ROLE is machine or cluster, not $Role" }
$Allow = @( ( Get-Setting $settings "LAB_ALLOW" "" ) -split '\s+' | Where-Object { $_ -ne "" } )
if ( $Allow.Count -eq 0 ) { $Allow = Get-LocalNetworks }
$Mdns = ( Get-Setting $settings "LAB_MDNS" "true" ) -eq "true"
$Broadcast = ( Get-Setting $settings "LAB_BROADCAST" "true" ) -eq "true"
$ClusterName = Get-Setting $settings "LAB_CLUSTER_NAME" $env:COMPUTERNAME.ToLowerInvariant()
$CiRegistry = Get-Setting $settings "LAB_CI_REGISTRY" ""
$CiGatewayUrl = Get-Setting $settings "LAB_CI_GATEWAY_URL" ""
$Ci = ( Get-Setting $settings "LAB_CI" "true" ) -eq "true"

# ---------------------------------------------------------------------------
# Build, or the built files
# ---------------------------------------------------------------------------

$Stage = Join-Path $InstallRoot ( ".stage." + [Guid]::NewGuid().ToString( "N" ).Substring( 0, 8 ) )
New-Item -ItemType Directory -Force -Path $InstallRoot, $Stage | Out-Null
try
{
    if ( $Prebuilt -ne "" )
    {
        $BinDir = $Prebuilt
        $SkipTests = $true
    }
    else
    {
        foreach ( $tool in "cmake", "ninja", "cl" )
        {
            if ( $null -eq ( Get-Command $tool -ErrorAction SilentlyContinue ) )
            {
                Fail "$tool is not in the PATH: run this from a Developer PowerShell of Visual Studio, or give -Prebuilt DIR."
            }
        }
        New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
        $buildLog = Join-Path $BuildDir "build.log"
        Write-Host "Building gucefLabLink from $GucefRoot"
        # the program with the plugins it loads; the tests in a build of their own
        $builds = @( @{ Target = "GUCEF_exe_gucefLabLink"; Dir = ( Join-Path $BuildDir "app" ) } )
        if ( -not $SkipTests ) { $builds += @{ Target = "GUCEF_exe_gucefLabLink_TestApp"; Dir = ( Join-Path $BuildDir "test" ) } }
        Remove-Item -LiteralPath $buildLog -Force -ErrorAction SilentlyContinue
        foreach ( $build in $builds )
        {
            $source = Join-Path $GucefRoot ( "projects\CMake\targets\" + $build.Target )
            & cmake -S $source -B $build.Dir -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON *>> $buildLog
            if ( $LASTEXITCODE -eq 0 ) { & cmake --build $build.Dir *>> $buildLog }
            if ( $LASTEXITCODE -ne 0 )
            {
                Select-String -LiteralPath $buildLog -Pattern "error" | Select-Object -First 20 | ForEach-Object { Write-Host $_.Line }
                Fail "the build failed, see $buildLog. The installed version stays."
            }
        }
        $BinDir = Join-Path $BuildDir "app\bin"
        $TestBinDir = Join-Path $BuildDir "test\bin"
    }

    # the program and the libraries next to it; not the test application
    Copy-Item -LiteralPath ( Join-Path $BinDir "gucefLabLink.exe" ) -Destination $Stage
    Get-ChildItem -LiteralPath $BinDir -Filter "*.dll" | Copy-Item -Destination $Stage
    Copy-Item -LiteralPath ( Join-Path $ScriptDir "README.md" ) -Destination $Stage
    if ( $checkout ) { Copy-Item -LiteralPath ( Join-Path $GucefRoot "tools\gucefLabLink\README.md" ) -Destination ( Join-Path $Stage "gucefLabLink.md" ) }

    $Version = Get-DirectoryVersion $Stage
    $Current = Join-Path $InstallRoot "current"
    $CurrentVersion = ""
    if ( Test-Path -LiteralPath $Current ) { $CurrentVersion = Split-Path -Leaf ( [string] ( Get-Item -LiteralPath $Current ).Target ) }

    if ( $Version -eq $CurrentVersion )
    {
        Write-Host "Version $Version is installed already."
    }
    else
    {
        if ( -not $SkipTests )
        {
            Write-Host "Testing version $Version"
            $env:GUCEF_CONSOLE_LOGGER = "none"
            # the program next to the tests, for the tests that run it
            Copy-Item -Path ( Join-Path $Stage "*" ) -Destination $TestBinDir -Force
            $testExit = Invoke-Program ( Join-Path $TestBinDir "gucefLabLink_TestApp.exe" ) "" $TestBinDir
            if ( $testExit -ne 0 )
            {
                $results = Join-Path $TestBinDir "gucefLabLink_TestApp_Results.xml"
                if ( Test-Path -LiteralPath $results )
                {
                    ( [xml] ( Get-Content -LiteralPath $results -Raw ) ).SelectNodes( "//testcase[failure]" ) | ForEach-Object { Write-Host ( "  failed: " + $_.name ) }
                }
                Fail "the tests failed (exit code $testExit), see $TestBinDir\gucefLabLink_TestApp_Log.txt. The installed version stays."
            }
            Write-Host "Tests passed."
        }
        $VersionDir = Join-Path $InstallRoot $Version
        if ( Test-Path -LiteralPath $VersionDir ) { Remove-Item -LiteralPath $VersionDir -Recurse -Force }
        Move-Item -LiteralPath $Stage -Destination $VersionDir
        # the junction the service starts from; the running program keeps the files of its own version
        if ( Test-Path -LiteralPath $Current ) { ( Get-Item -LiteralPath $Current ).Delete() }
        New-Item -ItemType Junction -Path $Current -Target $VersionDir | Out-Null
        Write-Host "Installed version $Version in $VersionDir"
        Get-ChildItem -LiteralPath $InstallRoot -Directory | Where-Object { $_.Name -ne "current" -and -not $_.Name.StartsWith( "." ) -and $_.Name -ne $Version } |
            Sort-Object LastWriteTime -Descending | Select-Object -Skip ( $KeepVersions - 1 ) | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    }
}
finally
{
    if ( Test-Path -LiteralPath $Stage ) { Remove-Item -LiteralPath $Stage -Recurse -Force -ErrorAction SilentlyContinue }
}
$Program = Join-Path $InstallRoot "current\gucefLabLink.exe"

# ---------------------------------------------------------------------------
# The state directory, the settings of the service and the lab secret
# ---------------------------------------------------------------------------

Protect-StateDirectory $StateDir
New-Item -ItemType Directory -Force -Path ( Join-Path $StateDir "logs" ) | Out-Null

$lines = @( "# Settings of the service gucefLabLink, written by projects/k3s/lablink/install.ps1 from config.env.",
            "# The options of the command line, one per line (tools/gucefLabLink/README.md).",
            "role = $Role",
            "state-dir = `"$StateDir`"" )
foreach ( $range in $Allow ) { $lines += "allow = $range" }
if ( -not $Mdns ) { $lines += "no-mdns" }
if ( -not $Broadcast ) { $lines += "no-broadcast" }
if ( $Role -eq "cluster" ) { $lines += "cluster-name = $ClusterName" }
# the CI configuration (tools/gucefLabLink/README.md, CI configuration): the registry and gateway of a hub, or no-ci on a machine
if ( $Role -eq "cluster" -and $CiRegistry -ne "" ) { $lines += "ci-registry = `"$CiRegistry`"" }
if ( $Role -eq "cluster" -and $CiGatewayUrl -ne "" ) { $lines += "ci-gateway-url = $CiGatewayUrl" }
if ( $Role -eq "machine" -and -not $Ci ) { $lines += "no-ci" }
$NewConfig = ( $lines -join "`r`n" ) + "`r`n"

$ConfigChanged = $false
if ( -not $Upgrade )
{
    $OldConfig = ""
    if ( Test-Path -LiteralPath $ConfigFile ) { $OldConfig = [System.IO.File]::ReadAllText( $ConfigFile ) }
    if ( $OldConfig -ne $NewConfig )
    {
        [System.IO.File]::WriteAllText( $ConfigFile, $NewConfig, ( New-Object System.Text.UTF8Encoding( $false ) ) )
        $ConfigChanged = $true
    }
}
# the service reads only a file of the Administrators that nobody else can change
& icacls.exe $ConfigFile /inheritance:r /grant:r "${SystemSid}:F" "${AdministratorsSid}:F" /Q | Out-Null
& icacls.exe $ConfigFile /setowner $AdministratorsSid /Q | Out-Null

$ConfigArgument = "--config " + ( Quote $ConfigFile )
$SecretShown = ""
if ( $Upgrade )
{
}
elseif ( $Role -eq "cluster" )
{
    if ( 0 -ne ( Invoke-Program $Program "--console $ConfigArgument --init" ) )
    {
        Write-Host $script:LastOutput
        Fail "the lab secret could not be made."
    }
    $SecretShown = ( Get-Content -LiteralPath ( Join-Path $StateDir "secret" ) -Raw ).Trim()
}
elseif ( $Secret -ne "" )
{
    if ( 0 -ne ( Invoke-Program $Program ( "--console $ConfigArgument --set-secret " + ( Quote $Secret ) ) ) )
    {
        Write-Host ( ( $script:LastOutput -split "`n" ) | Where-Object { $_ -match "ERROR|gucefLabLink:" } | Select-Object -Last 5 )
        Fail "the lab secret could not be stored."
    }
}

# ---------------------------------------------------------------------------
# Firewall: the ports the role needs, for the lab networks only
# ---------------------------------------------------------------------------

if ( -not $NoFirewall )
{
    Get-NetFirewallRule -Group $FirewallGroup -ErrorAction SilentlyContinue | Remove-NetFirewallRule
    $remote = @( $Allow )
    if ( $remote.Count -eq 0 ) { $remote = @( "LocalSubnet" ) }
    # discovery: mDNS and the UDP broadcasts of the cluster
    New-NetFirewallRule -DisplayName "gucef-lab discovery (UDP-In)" -Group $FirewallGroup -Direction Inbound -Action Allow -Profile Any `
                        -Protocol UDP -LocalPort 5353, 47812 -RemoteAddress $remote | Out-Null
    if ( $Role -eq "cluster" )
    {
        New-NetFirewallRule -DisplayName "gucef-lab link (TCP-In)" -Group $FirewallGroup -Direction Inbound -Action Allow -Profile Any `
                            -Protocol TCP -LocalPort 47810, 47811 -RemoteAddress $remote | Out-Null
    }
    Write-Host ( "Firewall: the $Role ports open to " + ( $remote -join " " ) )
}

# ---------------------------------------------------------------------------
# Service
# ---------------------------------------------------------------------------

$BinPath = ( Quote $Program ) + " " + $ConfigArgument
$ScBinPath = '"' + $BinPath.Replace( '"', '\"' ) + '"'
$ServiceChanged = $false
if ( $null -eq $existing )
{
    if ( 0 -ne ( Invoke-Sc "create $Service binPath= $ScBinPath start= auto DisplayName= `"GUCEF lab link`"" ) )
    {
        Write-Host $script:LastOutput
        Fail "the service $Service could not be made."
    }
    $ServiceChanged = $true
}
else
{
    $registered = ( Get-ItemProperty -LiteralPath "HKLM:\SYSTEM\CurrentControlSet\Services\$Service" -Name ImagePath ).ImagePath
    if ( $registered -ne $BinPath )
    {
        if ( 0 -ne ( Invoke-Sc "config $Service binPath= $ScBinPath start= auto" ) )
        {
            Write-Host $script:LastOutput
            Fail "the service $Service could not be changed."
        }
        $ServiceChanged = $true
    }
}
Invoke-Sc "description $Service `"Links this machine to the k3s cluster of the home network (tools/gucefLabLink)`"" | Out-Null
# restarted after 10, 30 and 60 seconds when it fails, also when it ends with an error code of its own
Invoke-Sc "failure $Service reset= 86400 actions= restart/10000/restart/30000/restart/60000" | Out-Null
Invoke-Sc "failureflag $Service 1" | Out-Null

$service = Get-Service -Name $Service
if ( $service.Status -ne "Running" )
{
    Start-Service -Name $Service
}
elseif ( $Version -ne $CurrentVersion -or $ConfigChanged -or $ServiceChanged )
{
    Restart-Service -Name $Service
}
try
{
    ( Get-Service -Name $Service ).WaitForStatus( "Running", ( New-TimeSpan -Seconds 60 ) )
}
catch
{
    Fail "$Service does not run. See $LogFile and the System event log (source Service Control Manager)."
}

Write-Host ""
Write-Host ( "gucefLabLink $Version runs as $Role, for " + ( $( if ( $Allow.Count -gt 0 ) { $Allow -join " " } else { "every address" } ) ) + "." )
if ( $Role -eq "cluster" -and $SecretShown -ne "" )
{
    Write-Host "  The lab secret, for machines that link without approval:"
    Write-Host "    $SecretShown"
}
else
{
    Write-Host "  It finds the cluster within a minute; its log: $LogFile"
    if ( $Secret -eq "" -and -not ( Test-Path -LiteralPath ( Join-Path $StateDir "secret" ) ) )
    {
        Write-Host ( "  Without the lab secret it waits for approval on the cluster: sudo gucefLabLink --approve " + $env:COMPUTERNAME.ToLowerInvariant() )
    }
}
exit 0

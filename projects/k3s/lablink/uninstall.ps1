# ---------------------------------------------------------------------------
# Removes the Windows service gucefLabLink from this machine, the counterpart
# of uninstall.sh on Linux: the service, the installed versions in
# %ProgramFiles%\gucef-lablink, the firewall rules of install.ps1 (group
# gucef-lab) and its build directory. %ProgramData%\gucef-lablink (the lab
# secret, the identity of the machine, lablink.conf and the logs) is kept
# unless -Purge is passed.
#
# Usage (an elevated PowerShell): .\uninstall.ps1 [-Purge]
# ---------------------------------------------------------------------------

param(
    [switch] $Purge
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$Service = "gucefLabLink"
$StateDir = Join-Path $env:ProgramData "gucef-lablink"
$BuildDir = Join-Path $env:ProgramData "gucef-lablink-build"
$InstallRoot = Join-Path $env:ProgramFiles "gucef-lablink"
$FirewallGroup = "gucef-lab"

$principal = New-Object Security.Principal.WindowsPrincipal( [Security.Principal.WindowsIdentity]::GetCurrent() )
if ( -not $principal.IsInRole( [Security.Principal.WindowsBuiltInRole]::Administrator ) )
{
    Write-Host "ERROR: run this from an elevated PowerShell (Run as administrator)."
    exit 1
}

$existing = Get-Service -Name $Service -ErrorAction SilentlyContinue
if ( $null -ne $existing )
{
    if ( $existing.Status -ne "Stopped" )
    {
        Stop-Service -Name $Service -Force -ErrorAction SilentlyContinue
        try { ( Get-Service -Name $Service ).WaitForStatus( "Stopped", ( New-TimeSpan -Seconds 60 ) ) } catch { }
    }
    & sc.exe delete $Service | Out-Null
    if ( $LASTEXITCODE -ne 0 )
    {
        Write-Host "ERROR: the service $Service could not be removed (sc.exe delete: $LASTEXITCODE)."
        exit 1
    }
}
Get-NetFirewallRule -Group $FirewallGroup -ErrorAction SilentlyContinue | Remove-NetFirewallRule
$current = Join-Path $InstallRoot "current"
if ( Test-Path -LiteralPath $current ) { ( Get-Item -LiteralPath $current ).Delete() }
foreach ( $directory in $InstallRoot, $BuildDir )
{
    if ( Test-Path -LiteralPath $directory ) { Remove-Item -LiteralPath $directory -Recurse -Force }
}
Write-Host "Removed the service $Service."

if ( $Purge )
{
    if ( Test-Path -LiteralPath $StateDir ) { Remove-Item -LiteralPath $StateDir -Recurse -Force }
    Write-Host "Removed $StateDir."
}
else
{
    Write-Host "Kept $StateDir (lab secret, identity, lablink.conf, logs). -Purge removes it."
}
exit 0

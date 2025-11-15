# Generate-XSD.ps1
# PowerShell script to generate an XSD schema from an XML file using xsd.exe
# This script auto-discovers xsd.exe in common SDK install paths
#
# Example usage:
# .\Generate-XSD.ps1 -XmlPath "C:\data\large.xml" -OutputDir "C:\schemas"

param(
    [Parameter(Mandatory = $true)]
    [string]$XmlPath,

    [Parameter(Mandatory = $false)]
    [string]$OutputDir = "."
)

# Ensure the XML file exists
if (-not (Test-Path $XmlPath)) {
    Write-Error "The XML file '$XmlPath' does not exist."
    exit 1
}

# Resolve full paths
$XmlFullPath = (Resolve-Path $XmlPath).Path
$OutputFullPath = (Resolve-Path $OutputDir).Path

# Candidate root paths to search
$candidateRoots = @(
    "${env:ProgramFiles(x86)}\Microsoft SDKs\Windows",
    "${env:ProgramFiles}\Microsoft SDKs\Windows",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio",
    "${env:ProgramFiles}\Microsoft Visual Studio"
)

# Search for xsd.exe recursively
$xsdExe = $null
foreach ($root in $candidateRoots) {
    if (Test-Path $root) {
        $found = Get-ChildItem -Path $root -Recurse -Filter "xsd.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $xsdExe = $found.FullName
            break
        }
    }
}

if (-not $xsdExe) {
    Write-Error "Could not locate xsd.exe. Please ensure the .NET SDK or Visual Studio tools are installed."
    exit 1
}

Write-Host "Using xsd.exe at: $xsdExe"
Write-Host "Generating XSD from XML..."

& $xsdExe $XmlFullPath /outputdir:$OutputFullPath

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ XSD successfully generated in $OutputFullPath"
} else {
    Write-Error "xsd.exe failed with exit code $LASTEXITCODE"
}

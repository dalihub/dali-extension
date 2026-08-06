[CmdletBinding()]
param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Debug",
  [string]$VcpkgRoot = "",
  [switch]$Clean,
  [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"
$ExtensionRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$WorkspaceRoot = Split-Path -Parent $ExtensionRoot
$WindowsDependenciesRoot = Join-Path $WorkspaceRoot "windows-dependencies"
$CommonScript = Join-Path $WindowsDependenciesRoot "vcpkg-script\dali-build-common.ps1"
if(-not (Test-Path -LiteralPath $CommonScript))
{
  throw "dali-build-common.ps1 not found: $CommonScript"
}
. $CommonScript

$Context = New-DaliBuildContext `
  -WindowsDependenciesRoot $WindowsDependenciesRoot `
  -VcpkgRoot $VcpkgRoot
Initialize-DaliBuildEnvironment -Context $Context

$ConfigurationRoot = Join-Path $Context.SdkRoot $Configuration.ToLowerInvariant()
$CorePackage = Join-Path $Context.InstallPrefix "share\dali2-core"
$AdaptorPackage = Join-Path $Context.InstallPrefix "share\dali2-adaptor"
Assert-DaliPaths -Paths @(
  (Join-Path $CorePackage "dali2-core-config.cmake"),
  (Join-Path $AdaptorPackage "dali2-adaptor-config.cmake")
) -Description "DALi package; build dali-core and dali-adaptor first"
Assert-DaliPaths -Paths @(
  (Join-Path $ConfigurationRoot "include\LWEWebView.h"),
  (Join-Path $ConfigurationRoot "lib\Starfish.lib"),
  (Join-Path $ConfigurationRoot "bin\Starfish.dll")
) -Description "$Configuration LWE SDK; run windows-dependencies\install.ps1 first"

$LweConfigFile = Join-Path $ConfigurationRoot "bin\.starfish-config"
if(Test-Path -LiteralPath $LweConfigFile)
{
  $LweBuiltConfiguration = (Get-Content -LiteralPath $LweConfigFile -Raw).Trim()
  if($LweBuiltConfiguration -ne $Configuration)
  {
    throw "Starfish configuration mismatch: expected $Configuration, found $LweBuiltConfiguration."
  }
}

$Arguments = (Get-DaliCommonCMakeArguments -Context $Context -Configuration $Configuration) + @(
  "-Ddali2-core_DIR=$CorePackage",
  "-Ddali2-adaptor_DIR=$AdaptorPackage",
  "-DLWE_INCLUDE_DIR=$(Join-Path $ConfigurationRoot 'include')",
  "-DSTARFISH_LIBRARY=$(Join-Path $ConfigurationRoot 'lib\Starfish.lib')",
  "-DSTARFISH_RUNTIME=$(Join-Path $ConfigurationRoot 'bin\Starfish.dll')"
)

Invoke-DaliCMakeProject `
  -Name "dali-extension LWE plugin ($Configuration)" `
  -SourceDirectory $PSScriptRoot `
  -BuildDirectory (Join-Path $Context.BuildRoot "dali-extension\lwe-web-engine-plugin") `
  -ConfigureArguments $Arguments `
  -Clean:$Clean `
  -Jobs $Jobs

$InstalledPlugin = Join-Path $Context.InstallPrefix "bin\dali2-web-engine-lwe-plugin.dll"
$InstalledStarfish = Join-Path $Context.InstallPrefix "bin\Starfish.dll"
Assert-DaliPaths -Paths @($InstalledPlugin, $InstalledStarfish) -Description "Installed LWE runtime"
Write-Host "`nLWE web-engine plugin ($Configuration) installed in $InstalledPlugin" -ForegroundColor Green

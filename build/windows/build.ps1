[CmdletBinding()]
param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Debug",
  [string]$VcpkgRoot = "",
  [switch]$Clean,
  [switch]$SkipLWEInstall,
  [switch]$RendererTests,
  [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"
$ExtensionRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$WorkspaceRoot = Split-Path -Parent $ExtensionRoot
$WindowsDependenciesRoot = $env:DALI_WINDOWS_DEPENDENCIES_ROOT
if(-not $WindowsDependenciesRoot)
{
  $WindowsDependenciesRoot = Join-Path $WorkspaceRoot "windows-dependencies"
}
$CommonScript = Join-Path $WindowsDependenciesRoot "vcpkg-script\dali-build-common.ps1"
if(-not (Test-Path -LiteralPath $CommonScript))
{
  throw "dali-build-common.ps1 not found: $CommonScript"
}
. $CommonScript

$EnvironmentConfiguration = $env:DALI_CONFIGURATION
if($EnvironmentConfiguration -and $EnvironmentConfiguration -ne $Configuration)
{
  throw "Requested $Configuration build does not match the $EnvironmentConfiguration DALi environment. Re-run the matching dali-env\setenv.ps1."
}
$SdkRoot = $env:DALI_WINDOWS_SDK_ROOT
$InstallPrefix = $env:DALI_PREFIX
if(-not $VcpkgRoot)
{
  $VcpkgRoot = $env:VCPKG_ROOT
}
$Context = New-DaliBuildContext `
  -WindowsDependenciesRoot $WindowsDependenciesRoot `
  -VcpkgRoot $VcpkgRoot `
  -InstallPrefix $InstallPrefix `
  -SdkRoot $SdkRoot
Initialize-DaliBuildEnvironment -Context $Context

$ConfigurationName = $Configuration.ToLowerInvariant()
$ConfigurationRoot = Join-Path $Context.SdkRoot $ConfigurationName
$SdkStarfishRuntime = Join-Path $ConfigurationRoot "bin\Starfish.dll"
$InstalledStarfish = Join-Path $Context.InstallPrefix "bin\Starfish.dll"
$CorePackage = Join-Path $Context.InstallPrefix "share\dali2-core"
$AdaptorPackage = Join-Path $Context.InstallPrefix "share\dali2-adaptor"
Assert-DaliPaths -Paths @(
  (Join-Path $CorePackage "dali2-core-config.cmake"),
  (Join-Path $AdaptorPackage "dali2-adaptor-config.cmake")
) -Description "DALi package; build dali-core and dali-adaptor first"
$RequiredStarfishPaths = @(
  (Join-Path $ConfigurationRoot "include\LWEWebView.h"),
  (Join-Path $ConfigurationRoot "include\LWEWorker.h"),
  (Join-Path $ConfigurationRoot "include\PlatformIntegrationData.h"),
  (Join-Path $ConfigurationRoot "lib\Starfish.lib"),
  $SdkStarfishRuntime,
  $InstalledStarfish
)
if(-not $SkipLWEInstall)
{
  $StarfishInstaller = Join-Path $WindowsDependenciesRoot "install-lwe.ps1"
  if(-not (Test-Path -LiteralPath $StarfishInstaller -PathType Leaf))
  {
    throw "Starfish installer is missing: $StarfishInstaller"
  }
  $StarfishAssetName = "windows_build_windows-dali-x64-$ConfigurationName.zip"
  $StarfishReleaseUrl = "https://github.sec.samsung.net/lws/lwe_rel/releases/latest/download/$StarfishAssetName"
  Write-Host "Installing Starfish from the latest $Configuration release." -ForegroundColor Yellow
  & $StarfishInstaller `
    $StarfishReleaseUrl `
    -Configuration $Configuration `
    -SdkRoot $Context.SdkRoot `
    -DaliPrefix $Context.InstallPrefix `
    -ForceDownload
}
Assert-DaliPaths `
  -Paths $RequiredStarfishPaths `
  -Description "$Configuration Starfish SDK and runtime"
$SdkStarfishHash = (Get-FileHash -LiteralPath $SdkStarfishRuntime -Algorithm SHA256).Hash
$InstalledStarfishHash = (Get-FileHash -LiteralPath $InstalledStarfish -Algorithm SHA256).Hash
if($SdkStarfishHash -ne $InstalledStarfishHash)
{
  throw "Starfish.dll differs between the Windows SDK and DALi runtime directories."
}

$LWEConfigFile = Join-Path $ConfigurationRoot "bin\.starfish-config"
if(Test-Path -LiteralPath $LWEConfigFile)
{
  $LWEConfig = (Get-Content -LiteralPath $LWEConfigFile -Raw).Trim()
  $LWEBuiltConfiguration = $LWEConfig

  # Older SDKs stored only the configuration name. Current SDKs use JSON for
  # build provenance, which is validated by windows-dependencies rather than
  # coupled to dali-extension.
  try
  {
    $LWEBuild = $LWEConfig | ConvertFrom-Json
    $ConfigurationProperty = $LWEBuild.PSObject.Properties["configuration"]
    if($ConfigurationProperty)
    {
      $LWEBuiltConfiguration = [string]$ConfigurationProperty.Value
    }
  }
  catch
  {
    # A plain Debug or Release marker is the legacy format.
  }

  if($LWEBuiltConfiguration -ne $Configuration)
  {
    throw "Starfish configuration mismatch: expected $Configuration, found $LWEBuiltConfiguration."
  }
}

$Arguments = (Get-DaliCommonCMakeArguments -Context $Context -Configuration $Configuration) + @(
  "-DDALI_LWE_BUILD_RENDERER_TESTS=$($RendererTests.IsPresent)",
  "-Ddali2-core_DIR=$CorePackage",
  "-Ddali2-adaptor_DIR=$AdaptorPackage",
  "-DLWE_INCLUDE_DIR=$(Join-Path $ConfigurationRoot 'include')",
  "-DSTARFISH_LIBRARY=$(Join-Path $ConfigurationRoot 'lib\Starfish.lib')"
)

Invoke-DaliCMakeProject `
  -Name "dali-extension LWE plugin ($Configuration)" `
  -SourceDirectory $PSScriptRoot `
  -BuildDirectory (Join-Path $Context.BuildRoot "dali-extension\lwe-web-engine-plugin") `
  -ConfigureArguments $Arguments `
  -Clean:$Clean `
  -Jobs $Jobs

$InstalledPlugin = Join-Path $Context.InstallPrefix "bin\dali2-web-engine-lwe-plugin.dll"
Assert-DaliPaths -Paths @(
  $InstalledPlugin,
  $InstalledStarfish,
  (Join-Path $Context.InstallPrefix "bin\libEGL.dll"),
  (Join-Path $Context.InstallPrefix "bin\libGLESv2.dll")
) -Description "Installed LWE ANGLE runtime"
Write-Host "`nLWE web-engine plugin ($Configuration) installed in $InstalledPlugin" -ForegroundColor Green

if($RendererTests)
{
  Invoke-DaliNative -Step "LWE ANGLE renderer tests" -Command "ctest.exe" -Arguments @(
    "--test-dir", (Join-Path $Context.BuildRoot "dali-extension\lwe-web-engine-plugin"),
    "--output-on-failure"
  )
}

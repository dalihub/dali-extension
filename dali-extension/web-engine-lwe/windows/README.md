# Windows LWE WebEngine plugin

This directory contains only the Windows rendering backend for DALi's LWE
`WebEnginePlugin`. Engine behavior, DALi callback handling, settings, context,
and error mapping are shared with the Tizen plugin from `../common`.

The plugin is owned and versioned by `dali-extension`, while the Starfish SDK
binaries are supplied by the internal `windows-dependencies` SDK.

Build `dali-core` and `dali-adaptor` first, then run:

```powershell
.\build\windows\build.ps1 -Configuration Debug -Clean
```

The resulting `dali2-web-engine-lwe-plugin.dll` is installed in
`<workspace>\dali-env\bin` together with the matching `Starfish.dll`. Debug
and Release builds use only the corresponding `WindowsDependenciesSDK\debug`
or `WindowsDependenciesSDK\release` tree.

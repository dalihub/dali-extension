# Windows LWE WebEngine plugin

This directory contains only the Windows rendering backend for DALi's LWE
`WebEnginePlugin`. Engine behavior, DALi callback handling, settings, context,
and error mapping are shared with the Tizen plugin from `../common`.

The plugin is owned and versioned by `dali-extension`. By default, its Windows
build script installs the configuration-specific asset from the latest
published Starfish release before every build.

The Windows backend requires an ANGLE-enabled Starfish SDK and creates
Starfish with `WebContainer::CreateGL()`. It provides EGL/OpenGL ES 2 renderer
callbacks backed by an RGBA8/stencil8 pbuffer. ANGLE uses D3D11 hardware, with
WARP as a fallback when initialization fails. A hidden 1x1 window supplies a
private native display for each view; rendering does not use a window swapchain.
Surface resizing happens on the LWE render thread before drawing the next frame.

Completed RGBA frames are read back from the pbuffer, converted to top-down row
order, and uploaded through DALi's `NativeImage` CPU-buffer path. This remains
a CPU copy: using ANGLE in both engines does not establish texture sharing.
The existing Windows adaptor implementation of `DevelNativeImage::SetPixels`
is sufficient; no shared-image integration header or companion adaptor change
is required. Frame delivery keeps only the latest pending CPU image when DALi's
event thread falls behind.
Destroying the LWE container synchronously releases its rendering callbacks
before the backend destroys its EGL resources.

Use matching Debug/Release Starfish and vcpkg ANGLE builds from the same
`windows-dependencies` SDK as DALi. An older desktop-OpenGL Starfish binary is
not supported. The plugin links `unofficial::angle::libEGL` and
`unofficial::angle::libGLESv2` and installs their selected configuration's DLLs
alongside `Starfish.dll`. The optional `.starfish-config` marker checks build
configuration only; SDK provenance remains the SDK provider's responsibility.

Build `dali-core` and `dali-adaptor` first, then run:

```powershell
.\build\windows\build.ps1 -Configuration Debug -Clean
```

To reuse an existing Starfish installation without downloading or installing
the latest published asset, run:

```powershell
.\build\windows\build.ps1 -Configuration Debug -SkipLWEInstall
```

The latest release must contain `windows_build_windows-dali-x64-debug.zip` and
`windows_build_windows-dali-x64-release.zip`. The build script selects the
asset matching `-Configuration` and bypasses the mutable latest-URL download
cache.
When installation is skipped, all required headers, libraries and DLLs must
already exist, and the SDK and `dali-env` copies of `Starfish.dll` must match.

The resulting `dali2-web-engine-lwe-plugin.dll` is installed in
`<workspace>\dali-env\bin` together with the matching `Starfish.dll`. Debug
and Release builds use only the corresponding `WindowsDependenciesSDK\debug`
or `WindowsDependenciesSDK\release` tree.

To build and run the ANGLE renderer smoke test as well:

```powershell
.\build\windows\build.ps1 -Configuration Debug -Clean -RendererTests
```

The test checks RGBA readback and row order, GL state restoration, pending
resizes, zero-size surfaces, shared textures, multiple views, and shutdown /
reinitialization. Run it on Windows with the matching SDK DLLs available; the
build script selects their runtime paths. This test does not exercise LWE page
rendering or DALi's final upload. Also verify a page containing text, images,
Canvas/WebGL, resize the WebView, and repeatedly open/close multiple WebViews in
a DALi application using the ANGLE-enabled Starfish SDK.

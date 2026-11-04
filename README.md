# How to build lottie extenstion for Ubuntu

## Prerequisites
### Environment setup
- Need environment created using dali_env script in dali-core repository

### Dali
- Install Dali libraries with reference to dali-core/README.md
    - https://review.tizen.org/gerrit/#/admin/projects/platform/core/uifw/dali-core
    - https://review.tizen.org/gerrit/#/admin/projects/platform/core/uifw/dali-adaptor
    - https://review.tizen.org/gerrit/#/admin/projects/platform/core/uifw/dali-toolkit

### rLottie
```sh
git clone https://github.com/Samsung/rlottie.git
```
```sh
cmake -DCMAKE_INSTALL_PREFIX=$DESKTOP_PREFIX -DLIB_INSTALL_DIR=$DESKTOP_PREFIX/lib
make install -j
```
<br/>

## Build & Install
```sh
cd build/ubuntu
cmake -DCMAKE_INSTALL_PREFIX=$DESKTOP_PREFIX
make install -j
```


# Ubuntu LWE web engine

The Ubuntu plugin renders Starfish into a private X11/EGL Pbuffer, reads RGBA
frames into CPU memory, and exposes a buffer-backed `NativeImageInterface` to
DALi. DALi uploads the latest frame on its render thread. No TBM, GBM or dma-buf
support is required. This path includes GPU readback and texture upload; it does
not provide zero-copy or direct rendering.

## Prerequisites

- Matching DALi core/adaptor headers and libraries, available through pkg-config.
  A plugin built against a different `WebEnginePlugin` vtable is not compatible.
- X11, EGL and OpenGL ES 2 development packages and an accessible X11 display.
- A matching Starfish header/library build providing `LWEWebView.h`,
  `InitializeOption::PreferSeparateThread` and `WebContainer::CreateGL`.
  Build both its public API shared library and implementation library before
  configuring the plugin.

## Build only the LWE plugin

With the usual DALi `setenv` sourced, enter the plugin build directory:

```sh
cd build/ubuntu/web-engine-lwe
cmake . \
  -DLWE_INCLUDE_DIR=/path/to/starfish/inc \
  -DLWE_LIBRARY=/path/to/starfish/out/release/lib/libStarfish.so \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$DESKTOP_PREFIX"
make -j6
make install
```

`LWE_INCLUDE_DIR` is the directory containing `LWEWebView.h`.
`LWE_LIBRARY` is the full path to the matching public API shared library file,
not its directory. Adjust both paths for your Starfish build.
The plugin installs into `$DESKTOP_PREFIX/lib` as
`libdali2-web-engine-lwe-plugin.so` with the default library directory setting.

When building from `build/ubuntu`, pass `-DENABLE_WEB_ENGINE_LWE=ON` together
with the same `LWE_INCLUDE_DIR`, `LWE_LIBRARY` and installation options.
The enable option is not needed when building directly from `web-engine-lwe`.
Its other plugins retain their existing dependencies.
To build only LWE through that entry point, also pass
`-DENABLE_VECTOR_ANIMATION_RENDERER=OFF -DENABLE_ICU=OFF -DENABLE_IMAGE_LOADER=OFF`.
All three existing plugins remain enabled by default.

## Runtime

Starfish and its dependent shared libraries must be available to the dynamic
loader. The plugin installation does not copy the Starfish SDK or its libraries.
For a Starfish source-tree build:

```sh
export LD_LIBRARY_PATH="/path/to/starfish/out/release/lib:$DESKTOP_PREFIX/lib:${LD_LIBRARY_PATH:-}"
```

Use the native-image WebView rendering mode. Each WebView owns a separate EGL
display/context; LWE uses a dedicated thread and sends notifications back to
DALi's event thread. The latest completed frame replaces an older pending frame.
Resizing replaces the buffer image when the new frame arrives. RGBA row order,
alpha and graphics resource recreation are handled by the Ubuntu backend.

The Ubuntu backend calls `Adaptor::RequestProcessEventsAndUpdate()` after
publishing a frame and before notifying the consumer, matching the Tizen
backend. This wakes DALi even when the UI reuses its existing native-image
visual; no dali-ui render-scheduling changes are required.

If an NVIDIA X11 desktop shows approximately one-second presentation stalls,
check the time spent inside the driver's `eglSwapBuffers`. During local
validation, setting `__GL_SYNC_TO_VBLANK=0` for the sample process removed those
stalls. This is a diagnostic runtime option; the plugin does not change the
application's VBlank configuration.

## Tests

From `build/ubuntu/web-engine-lwe`, after configuring as above:

```sh
cmake . -DDALI_LWE_BUILD_TESTS=ON
make -j6
ctest --output-on-failure
```

Tests require an accessible X11 display. The renderer test checks readback,
row orientation, repeated texture uploads, resize, shared contexts and resource
recreation. The plugin test starts a hidden DALi application, loads offline HTML
in two engines, verifies uploaded pixels, changes a page with JavaScript, resizes,
destroys and recreates an engine, and destroys an engine inside a frame callback.

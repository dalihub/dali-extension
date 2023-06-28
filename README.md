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

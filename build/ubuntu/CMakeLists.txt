CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)
CMAKE_POLICY(SET CMP0012 NEW) # Prevent dereferencing of OFF/ON as variables
IF(POLICY CMP0177)
  CMAKE_POLICY(SET CMP0177 NEW) # install() DESTINATION paths are normalized.
ENDIF()

# API major version (Not DALi release version). It names the plugins
# (dali<major>-*-plugin) and selects dali<major>-core, -adaptor and -toolkit.
# The 2.x names are available as links through DALI_LEGACY_ALIASES (see
# dali-legacy-aliases.cmake).
SET(DALI_API_MAJOR 3)
SET(name "dali${DALI_API_MAJOR}-extension")
SET(core_name "dali${DALI_API_MAJOR}-core")
SET(adaptor_name "dali${DALI_API_MAJOR}-adaptor")
SET(toolkit_name "dali${DALI_API_MAJOR}-toolkit")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/dali-legacy-aliases.cmake)

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_CXX_STANDARD 17)
PROJECT(${name})
# Enable shadow errors for GNU builds; validate other compilers separately.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  ADD_COMPILE_OPTIONS(-Werror=shadow)
ENDIF()

SET(PKG_NAME ${name})

OPTION(ENABLE_DEBUG         "Enable Debug"  OFF)
OPTION(ENABLE_PKG_CONFIGURE "Use pkgconfig" ON)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  SET( ENABLE_DEBUG ON )
ENDIF()

# from root/build/tizen, get back to root
SET(ROOT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Make sure the path is absolute
GET_FILENAME_COMPONENT(ROOT_SRC_DIR ${ROOT_SRC_DIR} ABSOLUTE)

OPTION(ENABLE_VECTOR_ANIMATION_RENDERER "Build the rlottie plugin" ON)
OPTION(ENABLE_ICU "Build the ICU plugin" ON)
OPTION(ENABLE_IMAGE_LOADER "Build image loader plugins" ON)

IF(ENABLE_VECTOR_ANIMATION_RENDERER)
  ADD_SUBDIRECTORY(vector-animation-renderer)
ENDIF()
IF(ENABLE_ICU)
  ADD_SUBDIRECTORY(icu)
ENDIF()
IF(ENABLE_IMAGE_LOADER)
  ADD_SUBDIRECTORY(image-loader)
ENDIF()

OPTION(ENABLE_WEB_ENGINE_LWE "Build the LWE web engine plugin" OFF)
IF(ENABLE_WEB_ENGINE_LWE)
  IF(DALI_LWE_BUILD_TESTS)
    ENABLE_TESTING()
  ENDIF()
  ADD_SUBDIRECTORY(web-engine-lwe)
ENDIF()

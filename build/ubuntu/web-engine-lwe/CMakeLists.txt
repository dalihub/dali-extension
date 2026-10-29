cmake_minimum_required(VERSION 3.16)
project(dali2-web-engine-lwe-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(DALI REQUIRED IMPORTED_TARGET dali2-core dali2-adaptor)
pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl glesv2 x11)
find_path(LWE_INCLUDE_DIR LWEWebView.h DOC "Directory containing the Starfish LWEWebView.h header")
find_library(LWE_LIBRARY NAMES lightweight-web-engine Starfish DOC "Path to the Starfish public API shared library")
if(NOT LWE_INCLUDE_DIR OR NOT LWE_LIBRARY)
  message(FATAL_ERROR "Starfish SDK not found. Set LWE_INCLUDE_DIR to the header directory and LWE_LIBRARY to the public API shared library file.")
endif()

get_filename_component(PLUGIN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../dali-extension/web-engine-lwe" ABSOLUTE)
set(COMMON_DIR "${PLUGIN_ROOT}/common")
set(UBUNTU_DIR "${PLUGIN_ROOT}/ubuntu")
add_library(${PROJECT_NAME} SHARED
  ${COMMON_DIR}/web-engine-lwe.cpp
  ${COMMON_DIR}/web-engine-lwe-back-forward-list.cpp
  ${COMMON_DIR}/web-engine-lwe-context.cpp
  ${COMMON_DIR}/web-engine-lwe-cookie-manager.cpp
  ${COMMON_DIR}/web-engine-lwe-input.cpp
  ${COMMON_DIR}/web-engine-lwe-load-error.cpp
  ${COMMON_DIR}/web-engine-lwe-plugin.cpp
  ${COMMON_DIR}/web-engine-lwe-settings.cpp
  ${UBUNTU_DIR}/web-engine-lwe-backend-ubuntu.cpp
  ${UBUNTU_DIR}/web-engine-lwe-egl-renderer.cpp
  ${UBUNTU_DIR}/web-engine-lwe-buffer-image.cpp
)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${LWE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::DALI PkgConfig::EGL Threads::Threads ${LWE_LIBRARY})
target_compile_definitions(${PROJECT_NAME} PRIVATE HIDE_DALI_INTERNALS)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-z,defs)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter -fvisibility=hidden)

include(GNUInstallDirs)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

option(DALI_LWE_BUILD_TESTS "Build Ubuntu EGL and frame upload integration tests" OFF)
if(DALI_LWE_BUILD_TESTS)
  enable_testing()
  add_executable(lwe-ubuntu-renderer-test
    tests/renderer-test.cpp
    ${UBUNTU_DIR}/web-engine-lwe-egl-renderer.cpp
    ${UBUNTU_DIR}/web-engine-lwe-buffer-image.cpp
  )
  target_include_directories(lwe-ubuntu-renderer-test PRIVATE ${UBUNTU_DIR})
  target_link_libraries(lwe-ubuntu-renderer-test PRIVATE PkgConfig::DALI PkgConfig::EGL Threads::Threads)
  target_compile_options(lwe-ubuntu-renderer-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME lwe-ubuntu-renderer COMMAND lwe-ubuntu-renderer-test)
  add_executable(lwe-ubuntu-plugin-test tests/plugin-test.cpp ${UBUNTU_DIR}/web-engine-lwe-egl-renderer.cpp)
  target_include_directories(lwe-ubuntu-plugin-test PRIVATE ${UBUNTU_DIR})
  target_include_directories(lwe-ubuntu-plugin-test SYSTEM PRIVATE ${LWE_INCLUDE_DIR})
  target_link_libraries(lwe-ubuntu-plugin-test PRIVATE ${PROJECT_NAME} PkgConfig::DALI PkgConfig::EGL ${LWE_LIBRARY})
  target_compile_options(lwe-ubuntu-plugin-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME lwe-ubuntu-plugin COMMAND lwe-ubuntu-plugin-test)
  set_tests_properties(lwe-ubuntu-plugin PROPERTIES TIMEOUT 30)
endif()

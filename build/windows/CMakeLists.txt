cmake_minimum_required(VERSION 3.16)

project(dali2-web-engine-lwe-plugin-win LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dali2-core CONFIG REQUIRED)
find_package(dali2-adaptor CONFIG REQUIRED)
find_package(unofficial-angle CONFIG REQUIRED)
find_path(LWE_INCLUDE_DIR NAMES LWEWebView.h REQUIRED)
find_library(STARFISH_LIBRARY NAMES Starfish REQUIRED)

set(PLUGIN_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../dali-extension/web-engine-lwe")
set(PLUGIN_COMMON_DIR "${PLUGIN_ROOT_DIR}/common")
set(PLUGIN_WINDOWS_DIR "${PLUGIN_ROOT_DIR}/windows")
add_library(${PROJECT_NAME} SHARED
  ${PLUGIN_COMMON_DIR}/web-engine-lwe.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-back-forward-list.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-context.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-cookie-manager.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-input.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-load-error.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-plugin.cpp
  ${PLUGIN_COMMON_DIR}/web-engine-lwe-settings.cpp
  ${PLUGIN_WINDOWS_DIR}/web-engine-lwe-backend-win.cpp
  ${PLUGIN_WINDOWS_DIR}/web-engine-lwe-angle-renderer.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "dali2-web-engine-lwe-plugin"
  PREFIX ""
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PLUGIN_COMMON_DIR}
  ${PLUGIN_WINDOWS_DIR}
)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${LWE_INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE /W3 /vmg /utf-8 /wd4251 /wd4275)
target_compile_definitions(${PROJECT_NAME} PRIVATE __restrict__=__restrict)
target_link_libraries(${PROJECT_NAME} PRIVATE
  dali2-adaptor::dali2-adaptor
  dali2-core::dali2-core
  ${STARFISH_LIBRARY}
  gdi32
  unofficial::angle::libEGL
  unofficial::angle::libGLESv2
  user32
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Use the same configuration-specific ANGLE DLLs as DALi and the Starfish SDK.
install(FILES
  "$<TARGET_FILE:unofficial::angle::libEGL>"
  "$<TARGET_FILE:unofficial::angle::libGLESv2>"
  DESTINATION bin
)

option(DALI_LWE_BUILD_RENDERER_TESTS "Build the ANGLE renderer smoke test" OFF)
if(DALI_LWE_BUILD_RENDERER_TESTS)
  enable_testing()
  add_executable(lwe-angle-renderer-test
    tests/angle-renderer-test.cpp
    ${PLUGIN_WINDOWS_DIR}/web-engine-lwe-angle-renderer.cpp
  )
  target_include_directories(lwe-angle-renderer-test PRIVATE ${PLUGIN_WINDOWS_DIR})
  target_compile_options(lwe-angle-renderer-test PRIVATE /W3 /vmg /utf-8)
  target_link_libraries(lwe-angle-renderer-test PRIVATE
    dali2-core::dali2-core
    unofficial::angle::libEGL
    unofficial::angle::libGLESv2
    gdi32
    user32
  )
  add_test(NAME lwe-angle-renderer COMMAND lwe-angle-renderer-test)
endif()

cmake_minimum_required(VERSION 3.16)

project(dali2-web-engine-lwe-plugin-win LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dali2-core CONFIG REQUIRED)
find_package(dali2-adaptor CONFIG REQUIRED)
find_path(LWE_INCLUDE_DIR NAMES LWEWebView.h REQUIRED)
find_library(STARFISH_LIBRARY NAMES Starfish REQUIRED)
find_file(STARFISH_RUNTIME NAMES Starfish.dll REQUIRED)

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
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES "${STARFISH_RUNTIME}" DESTINATION bin)

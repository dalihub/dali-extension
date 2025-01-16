CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)
CMAKE_POLICY(SET CMP0012 NEW) # Prevent dereferencing of OFF/ON as variables

SET(name "dali2-extension")

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_CXX_STANDARD 17)
PROJECT(${name})
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

ADD_SUBDIRECTORY(vector-animation-renderer)
ADD_SUBDIRECTORY(icu)

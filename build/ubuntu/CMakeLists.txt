CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)
CMAKE_POLICY(SET CMP0012 NEW) # Prevent dereferencing of OFF/ON as variables

OPTION(ENABLE_DEBUG "Enable Debug" OFF)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  SET( ENABLE_DEBUG ON )
ENDIF()

# from root/build/tizen, get back to root
SET(ROOT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Make sure the path is absolute
GET_FILENAME_COMPONENT(ROOT_SRC_DIR ${ROOT_SRC_DIR} ABSOLUTE)

ADD_SUBDIRECTORY(vector-animation-renderer)

SET(name "dali2-vector-animation-renderer-plugin")

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_CXX_STANDARD 17)
PROJECT(${name})
SET(PKG_NAME ${name})

SET(GCC_COMPILER_VERSION_REQUIRED "6")
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_COMPILER_VERSION_REQUIRED)
    MESSAGE(FATAL_ERROR "The GCC required compiler version is " ${GCC_COMPILER_VERSION_REQUIRED})
  ENDIF()
ENDIF()

IF( ENABLE_PKG_CONFIGURE )
  FIND_PACKAGE( PkgConfig REQUIRED )
  PKG_CHECK_MODULES(DALICORE REQUIRED dali2-core)
  PKG_CHECK_MODULES(DALIADAPTOR REQUIRED dali2-adaptor)
  PKG_CHECK_MODULES(DALITOOLKIT REQUIRED dali2-toolkit)
  PKG_CHECK_MODULES(RLOTTIE REQUIRED rlottie)
ENDIF()

# Deployment folder should come from spec file or command line:
SET( PREFIX ${CMAKE_INSTALL_PREFIX})
SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX})

# Set up the include dir
SET( INCLUDE_DIR $ENV{includedir} )
IF( NOT INCLUDE_DIR )
  SET( INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR} )
ENDIF()
IF( NOT INCLUDE_DIR )
  SET( INCLUDE_DIR ${PREFIX}/include )
ENDIF()

# Set up the lib dir
SET( LIB_DIR $ENV{libdir} )
IF( NOT LIB_DIR )
  SET( LIB_DIR ${CMAKE_INSTALL_LIBDIR} )
ENDIF()
IF( NOT LIB_DIR )
  SET( LIB_DIR ${PREFIX}/lib )
ENDIF()

# Set up the bin dir
SET( BIN_DIR $ENV{bindir} )
IF( NOT BIN_DIR )
  SET( BIN_DIR ${CMAKE_INSTALL_BINDIR} )
ENDIF()
IF( NOT BIN_DIR )
  SET( BIN_DIR ${PREFIX}/bin )
ENDIF()

IF( ENABLE_DEBUG )
  MESSAGE( STATUS "CMAKE_BUILD_TYPE: " Debug )
  ADD_DEFINITIONS( "-DDEBUG_ENABLED" )
  SET( ENABLE_EXPORTALL ON )
ELSE()
  MESSAGE( STATUS "CMAKE_BUILD_TYPE: " Release )
ENDIF()

IF( ENABLE_TRACE )
  ADD_DEFINITIONS("-DTRACE_ENABLED")
ENDIF()

# Hijack the information of tizen version here
ADD_DEFINITIONS("-DOVER_TIZEN_VERSION_9")

INCLUDE(CheckCXXCompilerFlag)

ADD_COMPILE_OPTIONS( -Werror -Wall -Wextra -Wno-unused-parameter -Wfloat-equal )
CHECK_CXX_COMPILER_FLAG(-Wno-class-memaccess HAVE_NO_CLASS_MEMACCESS)
IF (HAVE_NO_CLASS_MEMACCESS)
  ADD_COMPILE_OPTIONS( -Wno-class-memaccess )
ENDIF()

CHECK_CXX_COMPILER_FLAG(-Wno-cast-function-type HAVE_NO_CAST_FUNCTION_TYPE)
IF (HAVE_NO_CAST_FUNCTION_TYPE)
  ADD_COMPILE_OPTIONS( -Wno-cast-function-type )
ENDIF()

IF( NOT ${ENABLE_EXPORTALL} )
  ADD_COMPILE_OPTIONS( "-fvisibility=hidden" )
ENDIF()

INCLUDE_DIRECTORIES(
  ${ROOT_SRC_DIR}
  ${DALICORE_INCLUDE_DIRS}
  ${DALIADAPTOR_INCLUDE_DIRS}
  ${INCLUDE_DIR}
)

SET(SOURCE_DIR "${ROOT_SRC_DIR}/dali-extension/vector-animation-renderer")

SET(SOURCES "")

INCLUDE( ${ROOT_SRC_DIR}/dali-extension/vector-animation-renderer/file-ubuntu.list )

SET(LIBTYPE SHARED)
IF(DEFINED STATIC)
  SET(LIBTYPE STATIC)
ENDIF()

ADD_LIBRARY( ${name} ${LIBTYPE} ${SOURCES} )

SET(OPTIONAL_LIBS ${OPTIONAL_LIBS} -lpthread)

TARGET_LINK_LIBRARIES( ${name}
  ${OPTIONAL_LIBS}
  ${DALICORE_LDFLAGS}
  ${DALIADAPTOR_LDFLAGS}
  ${RLOTTIE_LDFLAGS}
)

IF( INSTALL_CMAKE_MODULES )
  IF( ENABLE_DEBUG )
    SET( BIN_DIR "${BIN_DIR}/debug" )
    SET( LIB_DIR "${LIB_DIR}/debug" )
  ENDIF()

  # Install the library files.
  INSTALL( TARGETS ${name}
    EXPORT ${name}-targets
    LIBRARY DESTINATION ${LIB_DIR}
    ARCHIVE DESTINATION ${LIB_DIR}
    RUNTIME DESTINATION ${BIN_DIR}
  )

  # Install the cmake modules.
  INSTALL(
    EXPORT ${name}-targets
    NAMESPACE ${name}::
    FILE ${name}-targets.cmake
    DESTINATION share/${name}
  )

  FILE( WRITE ${CMAKE_CURRENT_BINARY_DIR}/${name}-config.cmake "
    include(CMakeFindDependencyMacro)
    include(\${CMAKE_CURRENT_LIST_DIR}/${name}-targets.cmake)
  ")
  INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}-config.cmake DESTINATION share/${name} )

ELSE()
  # Install the library so file and symlinks.
  INSTALL( TARGETS ${name} DESTINATION ${LIB_DIR} )
ENDIF()

# Configuration Messages
MESSAGE( STATUS "Configuration:\n" )
MESSAGE( STATUS "Prefix:                " ${PREFIX} )
MESSAGE( STATUS "Lib Dir:               " ${LIB_DIR} )
MESSAGE( STATUS "Bin Dir:               " ${BIN_DIR} )
MESSAGE( STATUS "Include Dir:           " ${INCLUDE_DIR} )
MESSAGE( STATUS "Debug build:           " ${ENABLE_DEBUG} )
MESSAGE( STATUS "Export all symbols:    " ${ENABLE_EXPORTALL} )
MESSAGE( STATUS "Trace:                 " ${ENABLE_TRACE} )
MESSAGE( STATUS "Use pkg configure:     " ${ENABLE_PKG_CONFIGURE} )
MESSAGE( STATUS "CXXFLAGS:              " ${CMAKE_CXX_FLAGS} )
MESSAGE( STATUS "LDFLAGS:               " ${CMAKE_SHARED_LINKER_FLAGS_INIT}${CMAKE_SHARED_LINKER_FLAGS} )

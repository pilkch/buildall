# Set the minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the project name
PROJECT(buildall)

SET(CMAKE_SOURCE_DIR ./../library/)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

SET(PROJECT_DIRECTORY "./")

SET(LIBRARY_INCLUDE "${CMAKE_SOURCE_DIR}/include/")
SET(LIBRARY_SRC "${CMAKE_SOURCE_DIR}/src/")


# Debug build
SET(CMAKE_BUILD_TYPE Debug)

# Release build
#SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
#SET(CMAKE_C_FLAGS_RELEASE "-O3")
#ADD_DEFINITIONS("-DNDEBUG")


# Add definitions, compiler switches, etc.
ADD_DEFINITIONS("-Wall -W -Wextra -Werror -Wformat -Wformat-y2k -Winit-self -Wstack-protector -Wunknown-pragmas -Wundef -Wcast-qual -Wwrite-strings -Wno-unused-parameter -Wpointer-arith -Wno-switch -Woverloaded-virtual -Wno-stack-protector -Wmissing-include-dirs -Wuninitialized -Wc++0x-compat -std=c++0x -ffor-scope -O2")

ADD_DEFINITIONS("-DBUILDALL")
ADD_DEFINITIONS("-DBUILD_HTML_LOG")
ADD_DEFINITIONS("-DBUILD_SPITFIRE_UNITTEST")

ADD_DEFINITIONS("-DSPITFIRE_APPLICATION_NAME=\"\\\"Buildall\\\"\"")
ADD_DEFINITIONS("-DSPITFIRE_APPLICATION_NAME_LWR=\"\\\"buildall\\\"\"")
ADD_DEFINITIONS("-DBUILD_APPLICATION_VERSION_STRING=\"\\\"0.1\\\"\"")

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  ADD_DEFINITIONS("-D__LINUX__")
ENDIF()
IF(WIN32)
  ADD_DEFINITIONS("-DNOMINMAX")
ENDIF()

INCLUDE_DIRECTORIES(${LIBRARY_INCLUDE})


# Files from library directory

MACRO(PREFIX_PATHS SOURCE_DIRECTORY SOURCE_FILES)
MESSAGE(STATUS "PREFIX_PATHS dir=${SOURCE_DIRECTORY}, files=${SOURCE_FILES}, ${ARGN}")
MESSAGE(a=${SOURCE_FILES})
  SET(OUTPUT_FILES "")
  SET(a ${OUTPUT_FILES})
  SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILES}")
  FOREACH(SOURCE_FILE ${ARGN})
    MESSAGE(b=${SOURCE_FILE})
    SET(a ${OUTPUT_FILES})
    SET(OUTPUT_FILES ${a} "${SOURCE_DIRECTORY}${SOURCE_FILE}")
  ENDFOREACH()
MESSAGE(d=${OUTPUT_FILES})
ENDMACRO(PREFIX_PATHS)




SET(LIBRARY_SPITFIRE_SOURCE_DIRECTORY spitfire/)
SET(LIBRARY_SPITFIRE_SOURCE_FILES
spitfire.cpp platform/pipe.cpp
communication/http.cpp communication/network.cpp
storage/document.cpp storage/file.cpp storage/filesystem.cpp storage/json.cpp storage/xml.cpp
util/datetime.cpp util/log.cpp util/cConsoleApplication.cpp util/string.cpp util/thread.cpp util/unittest.cpp)

IF(WIN32)
  SET(LIBRARY_SPITFIRE_SOURCE_FILES ${LIBRARY_SPITFIRE_SOURCE_FILES})
ENDIF()

PREFIX_PATHS(${LIBRARY_SPITFIRE_SOURCE_DIRECTORY} ${LIBRARY_SPITFIRE_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY libtrashmm/)
SET(LIBRARY_LIBTRASHMM_SOURCE_FILES
libtrashmm.cpp)

PREFIX_PATHS(${LIBRARY_LIBTRASHMM_SOURCE_DIRECTORY} ${LIBRARY_LIBTRASHMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_LIBXDGMM_SOURCE_DIRECTORY libxdgmm/)
SET(LIBRARY_LIBXDGMM_SOURCE_FILES
libxdgmm.cpp)

PREFIX_PATHS(${LIBRARY_LIBXDGMM_SOURCE_DIRECTORY} ${LIBRARY_LIBXDGMM_SOURCE_FILES})
SET(OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES ${OUTPUT_FILES})



SET(LIBRARY_SOURCE_FILES ${OUTPUT_LIBRARY_SPITFIRE_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBTRASHMM_SOURCE_FILES} ${OUTPUT_LIBRARY_LIBXDGMM_SOURCE_FILES})
PREFIX_PATHS(${LIBRARY_SRC} ${LIBRARY_SOURCE_FILES})
SET(OUTPUT_LIBRARY_SOURCE_FILES ${OUTPUT_FILES})
#MESSAGE(final=${OUTPUT_LIBRARY_SOURCE_FILES})



#list all source files here
ADD_EXECUTABLE(${PROJECT_NAME} main.cpp ${OUTPUT_LIBRARY_SOURCE_FILES} ${PROJECT_SOURCE_FILES})


FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})


SET(LIBRARIES
  XDG-basedir
#  GLU
#  SDL
#  SDL_image
#  SDL_ttf
)


# Some of the libraries have different names than their Find*.cmake name
SET(LIBRARIES_LINKED
  boost_iostreams
  boost_filesystem
  boost_locale
  boost_system
  pthread
  xdg-basedir
)


FOREACH(LIBRARY_FILE ${LIBRARIES})
  Find_Package(${LIBRARY_FILE} REQUIRED)
ENDFOREACH(LIBRARY_FILE)

#need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES_LINKED} ${Boost_LIBRARIES})


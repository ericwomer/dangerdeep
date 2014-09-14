cmake_minimum_required(VERSION 2.8)

project(dangerdeep)
# set(CMAKE_BUILD_TYPE "Debug")
if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

option(USE_UNITY "Enable unity builds ON|OFF(default)" OFF) # OFF is the default until I fix the scope issues with vars.
option(USE_CLANG "Build Dangerdeep with clang ON|OFF(default) " OFF) # OFF is the default

if(${USE_CLANG} STREQUAL "OFF")
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall -std=c++11 -march=native -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -std=c++11 -march=native -g -O2")
	set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++11 -g -Og -DDEBUG")

	set(CMAKE_LINKER_FLAGS "--export-dynamic")
elseif(${USE_CLANG} STREQUAL "ON")
	set(CMAKE_C_COMPILER             "/usr/bin/clang")
	set(CMAKE_C_FLAGS                "-Wall -std=c99")
	set(CMAKE_C_FLAGS_DEBUG          "-g")
	set(CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

	set(CMAKE_CXX_COMPILER             "/usr/bin/clang++")
	set(CMAKE_CXX_FLAGS                "-Wall -std=c++11 ")
	set(CMAKE_CXX_FLAGS_DEBUG          "-g")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
endif(${USE_CLANG} STREQUAL "OFF")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#------------------------------------

find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
	include_directories(${SDL2_INCLUDE_DIR})
    message(STATUS "SDL2_INCLUDE_DIR:       ${SDL2_INCLUDE_DIR}")
    message(STATUS "SDL2_LIBRARY:           ${SDL2_LIBRARY}")
elseif(NOT_SDL2_FOUND)
    message(FATAL_ERROR "Could not find SDL2")
endif(SDL2_FOUND)

#------------------------------------

find_package(SDL2_mixer)
if(SDL2_MIXER_FOUND)
	include_directories(${SDL2_MIXER_INCLUDE_DIRS})
    message(STATUS "SDL2_MIXER_INCLUDE_DIRS:       ${SDL2_MIXER_INCLUDE_DIRS}")
    message(STATUS "SDL2_MIXER_LIBRARY:           ${SDL2_MIXER_LIBRARIES}")
elseif(NOT_SDL2_MIXER_FOUND)
    message(FATAL_ERROR "Could not find SDL2_mixer")
endif(SDL2_MIXER_FOUND)

#------------------------------------

find_package(SDL2_image)
if(SDL2_IMAGE_FOUND)
	include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    message(STATUS "SDL2_IMAGE_INCLUDE_DIRS:       ${SDL2_IMAGE_INCLUDE_DIRS}")
    message(STATUS "SDL2_IMAGE_LIBRARY:           ${SDL2_IMAGE_LIBRARIES}")
elseif(NOT_SDL2_IMAGE_FOUND)
    message(FATAL_ERROR "Could not find SDL2_image")
endif(SDL2_IMAGE_FOUND)

#------------------------------------

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
    message(STATUS "OPENGL_INCLUDE_DIR:     ${OPENGL_INCLUDE_DIR}")
    message(STATUS "OPENGL_LIBRARY:         ${OPENGL_LIBRARIES}")
elseif(NOT_OPENGL_FOUND)
    message(FATAL_ERROR "Could not find OpenGL")
endif(OPENGL_FOUND)

#------------------------------------

find_package(BZip2)
if(BZIP2_FOUND)
	include_directories(${BZIP2_INCLUDE_DIR})
    message(STATUS "BZIP2_INCLUDE_DIR:        ${BZIP2_INCLUDE_DIR}")
    message(STATUS "BZIP2_LIBRARY:            ${BZIP2_LIBRARIES}")
elseif(NOT_BZIP2_FOUND)
    message(FATAL_ERROR "Could not find BZip2")
endif(BZIP2_FOUND)

#------------------------------------

find_package(X11)
if(X11_FOUND)
	include_directories(${X11_INCLUDE_DIR})
    message(STATUS "X11_INCLUDE_DIR:        ${X11_INCLUDE_DIR}")
    message(STATUS "X11_LIBRARY:            ${X11_LIBRARIES}")
elseif(NOT_X11_FOUND)
    message(FATAL_ERROR "Could not find X11")
endif(X11_FOUND)

#------------------------------------

find_package(FFTW)
if(FFTW_FOUND)
	include_directories(${FFTW_INCLUDE_DIR})
    message(STATUS "FFTW_INCLUDE_DIR:        ${FFTW_INCLUDE_DIR}")
    message(STATUS "FFTW_LIBRARY:            ${FFTW_LIBRARIES}")
elseif(NOT_FFTW_FOUND)
    message(FATAL_ERROR "Could not find FFTW")
endif(FFTW_FOUND)

#------------------------------------

find_package(TinyXML)
if(TINYXML_FOUND)
	include_directories(${TINYXML_INCLUDE_DIR})
    message(STATUS "TINYXML_INCLUDE_DIR:        ${TINYXML_INCLUDE_DIR}")
	message(STATUS "TINYXML_LIBRAIES:            ${TINYXML_LIBRARY}")
elseif(NOT_TINYXML_FOUND)
    message(FATAL_ERROR "Could not find TinyXML")
endif(TINYXML_FOUND)

#------------------------------------

#------------------------------------
# !Rake: Do platform check to see where datadir needs
# to point to then set DATADIR to datadir (overly complicated)
set(datadir "/usr/local/share/dangerdeep/") # default to linux for now
set(version 0.3.900)
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/system_defines.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/system_defines.h"
)

set(SYS_INC "${CMAKE_CURRENT_BINARY_DIR}/system_defines.h")
## end system_defines.h
#------------------------------------

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin)

# include_directories(.)

# include_directories(${INC_DIRS})
# get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# message("INCLUDE_DIRECTORIES: ${dirs}")

# Needed since everyone codes #include <someglheader.h> instead of
# #include <GL/someglheader.h>
include_directories("/usr/include/GL")
include_directories("${CMAKE_BINARY_DIR}")

set(LIBS
	${OPENGL_LIBRARY}
	${SDL2_LIBRARY}
	${SDL2_IMAGE_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${BZIP2_LIBRARIES}
	${FFTW_LIBRARIES}
	${X11_LIBRARIES}
	${TINYXML_LIBRARIES})

# message("LIBS:  ${LIBS}")

add_subdirectory(src)

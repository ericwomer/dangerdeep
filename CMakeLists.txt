cmake_minimum_required(VERSION 3.10)

project(dangerdeep LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/modules/PrecompiledHeaders.cmake)

option(USE_UNITY "Enable unity builds" OFF)
option(USE_CLANG "Build with Clang instead of GCC" OFF)

# Silenciar deprecación de std::auto_ptr (migración a unique_ptr pendiente)
set(DEPRECATION_FLAG "-Wno-deprecated-declarations")

if(NOT USE_CLANG)
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall ${DEPRECATION_FLAG} -march=native -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall ${DEPRECATION_FLAG} -march=native -g -O2")
	set(CMAKE_CXX_FLAGS_DEBUG "-Wall ${DEPRECATION_FLAG} -g -Og -DDEBUG")
	set(CMAKE_LINKER_FLAGS "--export-dynamic")
else()
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang++")
	set(CMAKE_C_FLAGS "-Wall -std=c99")
	set(CMAKE_C_FLAGS_DEBUG "-g")
	set(CMAKE_C_FLAGS_RELEASE "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS "-Wall ${DEPRECATION_FLAG}")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#------------------------------------

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
message(STATUS "SDL2_INCLUDE_DIR:       ${SDL2_INCLUDE_DIR}")
message(STATUS "SDL2_LIBRARY:           ${SDL2_LIBRARY}")

#------------------------------------

find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
message(STATUS "SDL2_MIXER_LIBRARY:     ${SDL2_MIXER_LIBRARIES}")

#------------------------------------

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
message(STATUS "SDL2_IMAGE_LIBRARY:     ${SDL2_IMAGE_LIBRARIES}")

#------------------------------------

set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
if(OPENGL_INCLUDE_DIR)
  include_directories("${OPENGL_INCLUDE_DIR}/GL")
else()
  include_directories("/usr/include/GL")
endif()
message(STATUS "OPENGL_LIBRARY:         ${OPENGL_LIBRARIES}")

#------------------------------------

find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIR})
message(STATUS "BZIP2_LIBRARY:          ${BZIP2_LIBRARIES}")

#------------------------------------

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
message(STATUS "X11_LIBRARY:            ${X11_LIBRARIES}")

#------------------------------------

find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})
message(STATUS "FFTW_LIBRARY:           ${FFTW_LIBRARIES}")

#------------------------------------

find_package(TinyXML REQUIRED)
include_directories(${TINYXML_INCLUDE_DIR})
message(STATUS "TINYXML_INCLUDE_DIR:    ${TINYXML_INCLUDE_DIR}")
message(STATUS "TINYXML_LIBRARIES:      ${TINYXML_LIBRARIES}")

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

# Salida de configure_file (system_defines.h)
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

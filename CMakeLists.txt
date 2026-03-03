cmake_minimum_required(VERSION 3.20)

# ccache (opcional): acelera recompilaciones reutilizando objetos ya compilados
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
	set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
	message(STATUS "ccache: activado (${CCACHE_PROGRAM})")
else()
	message(STATUS "ccache: no encontrado (instala ccache para recompilaciones más rápidas)")
endif()

project(dangerdeep LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/modules/PrecompiledHeaders.cmake)

option(USE_UNITY "Enable unity builds" OFF)
option(USE_CLANG "Build with Clang instead of GCC" OFF)
option(BUILD_ASAN "Build with AddressSanitizer + LeakSanitizer (detección fugas/errores memoria)" OFF)
option(BUILD_VALGRIND_FRIENDLY "Build sin -march=native (compatible con Valgrind, evita 'Instrucción ilegal')" OFF)
option(BUILD_UNIT_TESTS "Build tests unitarios (ptrlist_test, mutex_test, parser_test)" OFF)
option(BUILD_COVERAGE "Build con cobertura de código (gcov/lcov, requiere Debug)" OFF)

if(NOT USE_CLANG)
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
	set(MARCH_FLAG "-march=native")
	if(BUILD_VALGRIND_FRIENDLY)
		set(MARCH_FLAG "-march=x86-64")
		message(STATUS "Build compatible con Valgrind (sin -march=native)")
	endif()
	set(CMAKE_CXX_FLAGS_RELEASE "-Wall ${MARCH_FLAG} -O3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall ${MARCH_FLAG} -g -O2")
	set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -Og -DDEBUG")
	set(CMAKE_LINKER_FLAGS "--export-dynamic")
else()
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang++")
	set(CMAKE_C_FLAGS "-Wall -std=c99")
	set(CMAKE_C_FLAGS_DEBUG "-g")
	set(CMAKE_C_FLAGS_RELEASE "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS "-Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
	message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
endif()

if(BUILD_ASAN)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "ASan requiere Debug" FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
	message(STATUS "Build con AddressSanitizer + LeakSanitizer (detección fugas de memoria)")
endif()

if(BUILD_COVERAGE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Coverage requiere Debug" FORCE)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
	message(STATUS "Build con cobertura de código (gcov, branch coverage)")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

#------------------------------------

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
message(STATUS "SDL2_INCLUDE_DIR:       ${SDL2_INCLUDE_DIR}")
message(STATUS "SDL2_LIBRARY:           ${SDL2_LIBRARY}")
if(DEFINED SDL2_VERSION)
	message(STATUS "SDL2_VERSION:           ${SDL2_VERSION}")
endif()

#------------------------------------

find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
message(STATUS "SDL2_MIXER_LIBRARY:     ${SDL2_MIXER_LIBRARIES}")
if(DEFINED SDL2_MIXER_VERSION_STRING)
	message(STATUS "SDL2_MIXER_VERSION:     ${SDL2_MIXER_VERSION_STRING}")
endif()

#------------------------------------

find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
message(STATUS "SDL2_IMAGE_LIBRARY:     ${SDL2_IMAGE_LIBRARIES}")
if(DEFINED SDL2_IMAGE_VERSION_STRING)
	message(STATUS "SDL2_IMAGE_VERSION:     ${SDL2_IMAGE_VERSION_STRING}")
endif()

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
if(DEFINED TINYXML_VERSION)
	message(STATUS "TINYXML_VERSION:        ${TINYXML_VERSION}")
endif()

#------------------------------------

#------------------------------------
# Por defecto usar data/ del repo (desarrollo). Para instalación: -Ddatadir=/usr/local/share/dangerdeep/
if(NOT DEFINED datadir)
  set(datadir "${CMAKE_SOURCE_DIR}/data/")
endif()
if(NOT IS_ABSOLUTE "${datadir}")
  get_filename_component(datadir "${datadir}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
endif()
if(NOT datadir MATCHES "/$")
  set(datadir "${datadir}/")
endif()
message(STATUS "DATADIR (default): ${datadir}")
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

if(BUILD_UNIT_TESTS)
	enable_testing()
endif()
add_subdirectory(src)

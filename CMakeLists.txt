#---------------------------------------------------------------------------
#
#  bitpit
#
#  Copyright (C) 2015-2019 OPTIMAD engineering Srl
#
#  -------------------------------------------------------------------------
#  License
#  This file is part of bitpit.
#
#  bitpit is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License v3 (LGPL)
#  as published by the Free Software Foundation.
#
#  bitpit is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
#  License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
#
#---------------------------------------------------------------------------*/

#------------------------------------------------------------------------------------#
# CMake settings
#------------------------------------------------------------------------------------#
cmake_minimum_required(VERSION 2.8)

project("bitpit" CXX)

#------------------------------------------------------------------------------------#
# Variables visible to the user
#------------------------------------------------------------------------------------#
set(ENABLE_MPI 0 CACHE BOOL "If set, the program is compiled with MPI support")
set(VERBOSE_MAKE 0 CACHE BOOL "Set appropriate compiler and cmake flags to enable verbose output from compilation")
set(BUILD_SHARED_LIBS 0 CACHE BOOL "Build Shared Libraries")

if(${CMAKE_VERSION} VERSION_EQUAL "3.9.0" OR ${CMAKE_VERSION} VERSION_GREATER "3.9.0")
    set(LTO_STRATEGY "Auto" CACHE STRING "Choose the Link Time Optimization (LTO) strategy, options are: Auto (i.e., optimiziation is enabled only in release build) Enabled Disabled.")
    set_property(CACHE LTO_STRATEGY PROPERTY STRINGS "Auto" "Enabled" "Disabled")
endif()

#------------------------------------------------------------------------------------#
# Functions
#------------------------------------------------------------------------------------#
function(getModuleEnableName MODULE_NAME FLAG_NAME)
	string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
	set(${FLAG_NAME} "BITPIT_MODULE_${UPPER_MODULE_NAME}" PARENT_SCOPE)
endfunction()

function(isModuleEnabled MODULE_NAME ENABLED)
	getModuleEnableName(${MODULE_NAME} ENABLED_VARIABLE)
	if (DEFINED ${ENABLED_VARIABLE})
		set(${ENABLED} ${${ENABLED_VARIABLE}} PARENT_SCOPE)
	else ()
		set(${ENABLED} "FALSE" PARENT_SCOPE)
	endif ()
endfunction()

function(areModulesEnabled MODULE_LIST ENABLED)
	foreach (MODULE_NAME IN LISTS MODULE_LIST)
		isModuleEnabled(${MODULE_NAME} MODULE_ENABLED)
		if (NOT MODULE_ENABLED)
			set(${ENABLED} "FALSE" PARENT_SCOPE)
			return()
		endif()
	endforeach ()

	set(${ENABLED} "TRUE" PARENT_SCOPE)
endfunction()

function(enableModule MODULE_NAME ENABLED)
	set (EXTRA_ARGUMENTS ${ARGN})
	list(LENGTH EXTRA_ARGUMENTS EXTRA_ARGUMENT_COUNT)
	if (${EXTRA_ARGUMENT_COUNT} GREATER 0)
		list(GET EXTRA_ARGUMENTS 0 FORCED)
		if (FORCED)
			set(FORCE_FLAG "FORCE")
		endif()
	endif ()

	if (NOT DEFINED FORCE_FLAG)
		set(FORCE_FLAG "")
	endif ()

	getModuleEnableName(${MODULE_NAME} MODULE_ENABLE_FLAG)
	set(${MODULE_ENABLE_FLAG} ${ENABLED} CACHE BOOL "Request building ${MODULE_NAME} module" ${FORCE_FLAG})
	mark_as_advanced(${MODULE_ENABLE_FLAG})
endfunction()

function(hideModule MODULE_NAME)
	getModuleEnableName(${MODULE_NAME} MODULE_ENABLE_FLAG)
	if (DEFINED ${MODULE_ENABLE_FLAG})
		unset(${MODULE_ENABLE_FLAG} CACHE)
	endif ()
endfunction()

function(addModuleIncludeDirectories MODULE_NAME)
    set (EXTRA_ARGUMENTS ${ARGN})
    list(LENGTH EXTRA_ARGUMENTS EXTRA_ARGUMENT_COUNT)
    if (${EXTRA_ARGUMENT_COUNT} GREATER 0)
        list(GET EXTRA_ARGUMENTS 0 INCLUDE_EXTERNAL)
    else ()
        set(INCLUDE_EXTERNAL "TRUE")
    endif ()

    # External includes
    if (INCLUDE_EXTERNAL)
        include_directories("${BITPIT_EXTERNAL_INCLUDE_DIRS}")
    endif ()

    # Add dependiecies
    string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
    foreach (DEPENDENCY_NAME IN LISTS ${UPPER_MODULE_NAME}_DEPS)
        addModuleIncludeDirectories(${DEPENDENCY_NAME} "FALSE")
    endforeach()
    unset(UPPER_MODULE_NAME)

    # Add module directory
    include_directories("${PROJECT_SOURCE_DIR}/src/${MODULE_NAME}")
endfunction()

#------------------------------------------------------------------------------------#
# MPI
#------------------------------------------------------------------------------------#
if (ENABLE_MPI)
    find_package(MPI)

    include_directories("${MPI_INCLUDE_PATH}")

    if(MPI_C_COMPILE_FLAGS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MPI_C_COMPILE_FLAGS}")
    endif()

    if(MPI_CXX_COMPILE_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
    endif()

    if(MPI_CXX_LINK_FLAGS)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
    endif()
endif()

#------------------------------------------------------------------------------------#
# Compiler settings
#------------------------------------------------------------------------------------#
set (BITPIT_DEFINITIONS_PRIVATE "")
set (BITPIT_DEFINITIONS_PUBLIC "")

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

set (ENABLE_WARNINGS ${VERBOSE_MAKE})
if (VERBOSE_MAKE)
	set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

if (ENABLE_MPI)
	list (APPEND BITPIT_DEFINITIONS_PUBLIC "BITPIT_ENABLE_MPI=1")
else ()
	list (APPEND BITPIT_DEFINITIONS_PUBLIC "BITPIT_ENABLE_MPI=0")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

if (ENABLE_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") 
endif()

if (NOT ("${CMAKE_VERSION}" VERSION_LESS "2.8.12"))
	add_compile_options("-std=c++11")
else ()
	add_definitions("-std=c++11")
endif ()

# Define a preprocessor macro to recognize debug builds
IF(CMAKE_BUILD_TYPE_LOWER MATCHES "debug" OR CMAKE_BUILD_TYPE_LOWER MATCHES "debinfo")
	list (APPEND BITPIT_DEFINITIONS_PRIVATE "BITPIT_ENABLE_DEBUG=1")
else ()
	list (APPEND BITPIT_DEFINITIONS_PRIVATE "BITPIT_ENABLE_DEBUG=0")
endif ()

IF(NOT CMAKE_BUILD_TYPE_LOWER MATCHES "debug")
	list (APPEND BITPIT_DEFINITIONS_PRIVATE "NDEBUG")
endif ()

# Define an alias for building with scalasca
if (ENABLE_MPI)
	SET(C_FLAGS_INSTRUMENT   "-instrument mpicxx")
	SET(CXX_FLAGS_INSTRUMENT "-instrument mpic")
else ()
	SET(C_FLAGS_INSTRUMENT   "")
	SET(CXX_FLAGS_INSTRUMENT "")
endif ()

if (CMAKE_BUILD_TYPE_LOWER MATCHES "scalasca")
	file(WRITE scalasca_c_compiler
"#!/bin/bash
scalasca ${C_FLAGS_INSTRUMENT} \"$@\"
"
	)

	file(WRITE scalasca_cxx_compiler
"#!/bin/bash
scalasca ${C_FLAGS_INSTRUMENT} \"$@\"
"
	)

	file(INSTALL scalasca_cxx_compiler DESTINATION ${PROJECT_BINARY_DIR} PERMISSIONS OWNER_READ OWNER_EXECUTE )
	file(INSTALL scalasca_c_compiler   DESTINATION ${PROJECT_BINARY_DIR} PERMISSIONS OWNER_READ OWNER_EXECUTE )
	file(REMOVE  scalasca_cxx_compiler)
	file(REMOVE  scalasca_c_compiler)

	set(CMAKE_CXX_COMPILER "${PROJECT_BINARY_DIR}/scalasca_cxx_compiler")
	set(CMAKE_C_COMPILER   "${PROJECT_BINARY_DIR}/scalasca_c_compiler")
endif ()

# Check the features supported by the compiler
include(CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("int main() {__builtin_unreachable();}" HAVE___BUILTIN_UNREACHABLE)
if(HAVE___BUILTIN_UNREACHABLE)
    list (APPEND BITPIT_DEFINITIONS_PRIVATE "HAVE___BUILTIN_UNREACHABLE")
endif()

# Add the definitions
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${BITPIT_DEFINITIONS_PRIVATE})
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${BITPIT_DEFINITIONS_PUBLIC})

#------------------------------------------------------------------------------------#
# Link Time Optimization (LTO)
#------------------------------------------------------------------------------------#

# Detect if LTO can be enabled
if(DEFINED LTO_STRATEGY)
    if (NOT "${LTO_STRATEGY}" STREQUAL "Disabled")
        cmake_policy(SET CMP0069 NEW)
        include(CheckIPOSupported)

        if ("${LTO_STRATEGY}" STREQUAL "Enabled")
            if (NOT BUILD_SHARED_LIBS)
                message(FATAL_ERROR "LTO can be enabled only when building a shared library." )
            endif()
            check_ipo_supported()
            set(ENABLE_LTO TRUE)
        elseif ("${LTO_STRATEGY}" STREQUAL "Auto")
            if (BUILD_SHARED_LIBS)
                check_ipo_supported(RESULT LTO_SUPPORTED)
                if (${LTO_SUPPORTED} AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
                    set(ENABLE_LTO TRUE)
                else()
                    set(ENABLE_LTO FALSE)
                endif()
            else()
                set(ENABLE_LTO FALSE)
            endif()
        else()
            set(ENABLE_LTO FALSE)
        endif()
    else()
        set(ENABLE_LTO FALSE)
    endif()
else()
    set(ENABLE_LTO FALSE)
endif()

# Initialize LTO property
function(initialize_lto_property)
    if (${ENABLE_LTO})
        cmake_policy(SET CMP0069 NEW)
    endif()
endfunction()

# Set LTO property for the specified target
function(set_lto_property TARGET_NAME)
    if (${ENABLE_LTO})
        set_target_properties(${TARGET_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

#------------------------------------------------------------------------------------#
# Customized build types
#------------------------------------------------------------------------------------#

# GNU Profiling
SET(CMAKE_CXX_FLAGS_GNUPROFILING "-pg" CACHE STRING
    "Flags used by the C++ compiler during GNU profiling builds." FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_GNUPROFILING)

SET(CMAKE_C_FLAGS_GNUPROFILING "-pg" CACHE STRING
    "Flags used by the C compiler during GNU profiling builds." FORCE)
MARK_AS_ADVANCED(CMAKE_C_FLAGS_GNUPROFILING)

SET(CMAKE_EXE_LINKER_FLAGS_GNUPROFILING "-pg" CACHE STRING
    "Flags used for linking binaries during GNU profiling builds." FORCE)
MARK_AS_ADVANCED(CMAKE_EXE_LINKER_FLAGS_GNUPROFILING)

SET(CMAKE_SHARED_LINKER_FLAGS_GNUPROFILING "-pg" CACHE STRING
    "Flags used by the shared libraries linker during GNU profiling builds." FORCE)
MARK_AS_ADVANCED(CMAKE_SHARED_LINKER_FLAGS_GNUPROFILING)

# Scalasca Profiling
SET(CMAKE_CXX_FLAGS_SCALASCAPROFILING "-O2" CACHE STRING
    "Flags used by the C++ compiler during Scalasca profiling builds." FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_SCALASCAPROFILING)

SET(CMAKE_C_FLAGS_SCALASCAPROFILING "-O2" CACHE STRING
    "Flags used by the C compiler during Scalasca builds." FORCE)
MARK_AS_ADVANCED(CMAKE_C_FLAGS_SCALASCAPROFILING)

SET(CMAKE_EXE_LINKER_FLAGS_SCALASCAPROFILING "" CACHE STRING
    "Flags used for linking binaries during Scalasca builds." FORCE)
MARK_AS_ADVANCED(CMAKE_EXE_LINKER_FLAGS_SCALASCAPROFILING)

SET(CMAKE_SHARED_LINKER_FLAGS_SCALASCAPROFILING "" CACHE STRING
    "Flags used by the shared libraries linker during Scalasca builds." FORCE)
MARK_AS_ADVANCED(CMAKE_SHARED_LINKER_FLAGS_SCALASCAPROFILING)

# Set build type variable
if(NOT CMAKE_BUILD_TYPE)
	# Set default build type to Debug
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel GNUProfiling ScalascaProfiling."
		FORCE)

	# Set the possible values of build type for the GUI
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
	"MinSizeRel" "RelWithDebInfo" "GNUProfiling" "ScalascaProfiling")
endif()

#------------------------------------------------------------------------------------#
# Library version
#------------------------------------------------------------------------------------#
set (VERSION_HEADER_FILENAME "${PROJECT_SOURCE_DIR}/src/common/${CMAKE_PROJECT_NAME}_version.hpp")
string(TOUPPER "${CMAKE_PROJECT_NAME}_VERSION" VERSION_DEFINE_NAME)

file(READ "${VERSION_HEADER_FILENAME}" header)

string(REGEX REPLACE ".*#[ \t]*define[ \t]*${VERSION_DEFINE_NAME}[ \t]*\"([^\n]*)\".*" "\\1" match "${header}")
set(BITPIT_VERSION "${match}")

STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+(-[0-9A-Za-z-]+)?" "\\1" match "${BITPIT_VERSION}")
set(BITPIT_MAJOR_VERSION "${match}")

STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+(-[0-9A-Za-z-]+)?" "\\1" match "${BITPIT_VERSION}")
set(BITPIT_MINOR_VERSION "${match}")

STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)(-[0-9A-Za-z-]+)?" "\\1" match "${BITPIT_VERSION}")
set(BITPIT_PATCH_VERSION "${match}")

STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+-([0-9A-Za-z-]+)" match "${BITPIT_VERSION}")
if (NOT match STREQUAL "")
	STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+-([0-9A-Za-z-]+)" "\\1" match "${BITPIT_VERSION}")
	set(BITPIT_TAG_VERSION "${match}")
else ()
	set(BITPIT_TAG_VERSION "")
endif ()

#------------------------------------------------------------------------------------#
# List of all avilable modules
#------------------------------------------------------------------------------------#
set(BITPIT_MODULE_LIST "")
list(APPEND BITPIT_MODULE_LIST "common")
list(APPEND BITPIT_MODULE_LIST "operators")
list(APPEND BITPIT_MODULE_LIST "containers")
list(APPEND BITPIT_MODULE_LIST "IO")
list(APPEND BITPIT_MODULE_LIST "communications")
list(APPEND BITPIT_MODULE_LIST "LA")
list(APPEND BITPIT_MODULE_LIST "SA")
list(APPEND BITPIT_MODULE_LIST "CG")
list(APPEND BITPIT_MODULE_LIST "PABLO")
list(APPEND BITPIT_MODULE_LIST "patchkernel")
list(APPEND BITPIT_MODULE_LIST "surfunstructured")
list(APPEND BITPIT_MODULE_LIST "volcartesian")
list(APPEND BITPIT_MODULE_LIST "voloctree")
list(APPEND BITPIT_MODULE_LIST "volunstructured")
list(APPEND BITPIT_MODULE_LIST "discretization")
list(APPEND BITPIT_MODULE_LIST "RBF")
list(APPEND BITPIT_MODULE_LIST "levelset")
list(APPEND BITPIT_MODULE_LIST "POD")

#------------------------------------------------------------------------------------#
# Modules dependecies
#------------------------------------------------------------------------------------#
set(OPERATORS_DEPS "common")
set(CONTAINERS_DEPS "common;operators")
set(IO_DEPS "common;operators;containers")
set(COMMUNICATIONS_DEPS "common;containers;IO")
set(LA_DEPS "common;containers;operators;IO")
set(SA_DEPS "common;operators")
set(CG_DEPS "common;operators;SA")
set(PABLO_DEPS "IO;communications")
set(PATCHKERNEL_DEPS "common;operators;containers;IO;communications;SA;CG")
set(SURFUNSTRUCTURED_DEPS "common;patchkernel")
set(VOLCARTESIAN_DEPS "common;patchkernel")
set(VOLOCTREE_DEPS "common;PABLO;patchkernel")
set(VOLUNSTRUCTURED_DEPS "common;patchkernel")
set(RBF_DEPS "operators;IO;SA")
set(DISCRETIZATION_DEPS "common;containers;LA")
set(LEVELSET_DEPS "common;communications;SA;CG;surfunstructured;voloctree;volcartesian")
set(POD_DEPS "IO;common;containers;voloctree")

#------------------------------------------------------------------------------------#
# MPI-only modules
#------------------------------------------------------------------------------------#
set(MPI_ONLY_MODULE_LIST "")
list(APPEND MPI_ONLY_MODULE_LIST "communications")

#------------------------------------------------------------------------------------#
# Check of the dependencies
#------------------------------------------------------------------------------------#

# If building without MPI remove dependecies on MPI-only modules
if (NOT ENABLE_MPI)
    foreach (MODULE_NAME IN LISTS BITPIT_MODULE_LIST)
        string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
        list(LENGTH ${UPPER_MODULE_NAME}_DEPS DEPENDENCY_COUNT)
        if (${DEPENDENCY_COUNT} GREATER 0)
            list(REMOVE_ITEM ${UPPER_MODULE_NAME}_DEPS ${MPI_ONLY_MODULE_LIST})
        endif()
        unset(UPPER_MODULE_NAME)
    endforeach()
endif()

# Initialize module state
foreach (MODULE_NAME IN LISTS BITPIT_MODULE_LIST)
	if ((NOT ENABLE_MPI) AND (";${MPI_ONLY_MODULE_LIST};" MATCHES ";${MODULE_NAME};"))
		hideModule(${MODULE_NAME})
	else ()
		enableModule(${MODULE_NAME} TRUE FALSE)
	endif()
endforeach ()

set(CHECK_DEPS 1)
while (${CHECK_DEPS} EQUAL 1)
	set(CHECK_DEPS 0)
	foreach (MODULE_NAME IN LISTS BITPIT_MODULE_LIST)
		isModuleEnabled(${MODULE_NAME} MODULE_ENABLED)
		if (MODULE_ENABLED)
			string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
			foreach (DEPENDENCY_NAME IN LISTS ${UPPER_MODULE_NAME}_DEPS)
				isModuleEnabled(${DEPENDENCY_NAME} DEPENDENCY_ENABLED)
				if (NOT DEPENDENCY_ENABLED)
					set(CHECK_DEPS 1)
					enableModule(${DEPENDENCY_NAME} TRUE TRUE)
					message("Module '${DEPENDENCY_NAME}' has been enabled because it's needed by module '${MODULE_NAME}'")
				endif ()
			endforeach ()
		endif ()
		unset(MODULE_ENABLE_FLAG)
	endforeach ()
endwhile()
unset(CHECK_DEPS)

# The modules 'common' and 'operators' has to be always built
getModuleEnableName("common" COMMON_MODULE_ENABLED)
unset(${COMMON_MODULE_ENABLED} CACHE)
set(${COMMON_MODULE_ENABLED} 1 CACHE INTERNAL "Request building common module" FORCE)

getModuleEnableName("operators" OPERATORS_MODULE_ENABLED)
unset(${OPERATORS_MODULE_ENABLED} CACHE)
set(${OPERATORS_MODULE_ENABLED} 1 CACHE INTERNAL "Request building operators module" FORCE)

#------------------------------------------------------------------------------------#
# External modules dependecies
#------------------------------------------------------------------------------------#

set(BITPIT_EXTERNAL_DEPENDENCIES "")
set(BITPIT_EXTERNAL_LIBRARIES "")
set(BITPIT_EXTERNAL_INCLUDE_DIRS "")

isModuleEnabled("CG" MODULE_CG_ENABLED)
isModuleEnabled("LA" MODULE_LA_ENABLED)
isModuleEnabled("RBF" MODULE_RBF_ENABLED)
isModuleEnabled("POD" MODULE_POD_ENABLED)
if (MODULE_CG_ENABLED OR MODULE_LA_ENABLED OR MODULE_RBF_ENABLED OR MODULE_POD_ENABLED)
	enable_language(Fortran)
	enable_language(C)

	set(BLAS_VENDOR "All" CACHE STRING "If set, checks only the specified vendor. If not set, checks all the possibilities")
	mark_as_advanced(BLAS_VENDOR)
	set(BLA_VENDOR "${BLAS_VENDOR}")
	set(LAPACKE_STATIC 0 CACHE BOOL "Require LAPACKE static libraries")
	mark_as_advanced(LAPACKE_STATIC)

	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/LAPACKE/cmake")

	find_package(LAPACK REQUIRED)
	find_package(LAPACKE REQUIRED)

	set(LAPACKE_PRIVATE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/LAPACKE/include")

	list (APPEND BITPIT_EXTERNAL_DEPENDENCIES "LAPACK")
	list (APPEND BITPIT_EXTERNAL_DEPENDENCIES "LAPACKE")
	list (APPEND BITPIT_EXTERNAL_LIBRARIES "${LAPACKE_LIBRARIES}" "${LAPACK_LIBRARIES}")
	list (APPEND BITPIT_EXTERNAL_INCLUDE_DIRS "${LAPACKE_PRIVATE_INCLUDE_DIR}" "${LAPACKE_INCLUDE_DIRS}" "${LAPACK_INCLUDE_DIRS}")
endif()

isModuleEnabled("IO" MODULE_IO_ENABLED)
if (MODULE_IO_ENABLED)
    find_package(LibXml2 REQUIRED)

    list (APPEND BITPIT_EXTERNAL_DEPENDENCIES "LibXml2")
    list (APPEND BITPIT_EXTERNAL_LIBRARIES "${LIBXML2_LIBRARIES}")
    list (APPEND BITPIT_EXTERNAL_INCLUDE_DIRS "${LIBXML2_INCLUDE_DIR}")
endif()

isModuleEnabled("LA" MODULE_LA_ENABLED)
if (MODULE_LA_ENABLED)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/PETSc/cmake")

    set(PETSC_DIR "" CACHE PATH "Directory in which PETSc resides")
    set(PETSC_ARCH "arch-linux2-c" CACHE STRING "Build architecture")

    find_package(PETSc REQUIRED)

    list (APPEND BITPIT_EXTERNAL_DEPENDENCIES "PETSc")
    list (APPEND BITPIT_EXTERNAL_LIBRARIES "${PETSC_LIBRARIES}")
    list (APPEND BITPIT_EXTERNAL_INCLUDE_DIRS "${PETSC_INCLUDES}" )
endif()

if (ENABLE_MPI)
    list (APPEND BITPIT_EXTERNAL_DEPENDENCIES "MPI")
    list (APPEND BITPIT_EXTERNAL_LIBRARIES "${MPI_CXX_LIBRARIES}")
    list (APPEND BITPIT_EXTERNAL_INCLUDE_DIRS "${MPI_CXX_INCLUDE_DIRS}" )
endif()

#------------------------------------------------------------------------------------#
# Subdirectories
#------------------------------------------------------------------------------------#

# Main library
add_subdirectory(src)

# Docs
add_subdirectory(doc)

# CMake
add_subdirectory(cmake)

# Examples
add_subdirectory(examples)

# External
add_subdirectory(external)

# Tests
enable_testing()
add_subdirectory(test)

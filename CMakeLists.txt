# Copyright © 2014 Michael Jung
#
# This file is part of Panga.
# 
# Panga is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Panga is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Panga.  If not, see <http://www.gnu.org/licenses/>.

project(panga)

cmake_minimum_required(VERSION 2.8.9)

#Wenn kein Build-Typ ausgewählt: Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#-O3 macht Probleme, daher nur -O2 nutzen
if(CMAKE_BUILD_TYPE MATCHES Release AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
    unset(CMAKE_BUILD_TYPE)
    add_definitions(-O2)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_AUTOMOC ON)

set(Boost_USE_STATIC_LIBS ON)
if(WIN32)
    find_package(Boost COMPONENTS iostreams zlib thread system chrono serialization REQUIRED)
elseif(APPLE)
    find_package(Boost COMPONENTS iostreams thread system chrono serialization REQUIRED)
    find_package(zlib REQUIRED)
else()
    find_package(Boost COMPONENTS iostreams thread system chrono serialization REQUIRED)
endif()

find_package(Qwt REQUIRED)
find_package(Qt5Widgets REQUIRED)

if(NOT CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(Boost COMPONENTS unit_test_framework)
    find_package(GTest)
    find_package(GMock)
    if(Boost_UNIT_TEST_FRAMEWORK_FOUND AND GTEST_FOUND AND GMOCK_FOUND)
        enable_testing()
        set(TEST_LIBRARIES
            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            ${GTEST_LIBRARIES}
            ${GMOCK_LIBRARIES})
        set(TESTING_ENABLED true)
    endif()

    # add a target to generate API documentation with Doxygen
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(
            doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
    endif(DOXYGEN_FOUND)
else()
    set(TESTING_ENABLED false)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(LIBRARIES
    ${QWT_LIBRARY}
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARY}
    )

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE
    -DBOOST_THREAD_USE_LIB
    -DEIGEN_MPL2_ONLY
    )

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -msse2 -static-libgcc -static-libstdc++")
endif(WIN32)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic --std=c++0x")
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11")
endif()

include(GetGitRevisionDescription)
git_describe(GIT_VERSION)
string(REGEX REPLACE "-g[a-f0-9]+" "" GIT_VERSION_SHORT ${GIT_VERSION})

add_subdirectory(src)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Panga is a tool for the evaluation of noble gas data.")
set(CPACK_PACKAGE_VENDOR "Michael Jung, Institute of Environmental Physics, Heidelberg University, Germany")
set(CPACK_PACKAGE_VERSION "${GIT_VERSION_SHORT}")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(CPACK_SYSTEM_NAME "OSX")
    set(CPACK_FILE_NAME "${PROJECT_NAME}")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Michael Jung")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libqwt6, libqtcore4 (>= 4:4.8.0), libqtgui4 (>= 4:4.8.0), libqt4-svg (>= 4:4.8.0), libstdc++6 (>= 4.6), libc6 (>= 2.14), libgcc1 (>= 1:4.1.1)")
elseif(WIN32)
	set(CPACK_GENERATOR "ZIP")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8) #64 bit Windows?
	    set(CPACK_SYSTEM_NAME "win64")
	else()
	    set(CPACK_SYSTEM_NAME "win32")
	endif()
endif()

include(CPack)


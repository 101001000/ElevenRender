cmake_minimum_required(VERSION 3.16)
set(FETCHCONTENT_QUIET FALSE)
set( CMAKE_CXX_STANDARD 17 )
include(FetchContent)

if(NOT DEFINED SYCL_PATH)

	message(STATUS "Getting Intel DPC++ framework...")
	
	FetchContent_Declare(sycl_llvm
		GIT_REPOSITORY  https://github.com/intel/llvm
		GIT_TAG         0f579bae55c48d810e1ed76db29229c854e61d5e
		GIT_PROGRESS TRUE)
	
	FetchContent_MakeAvailable(sycl_llvm)
	
	message(STATUS "Building Intel DPC++ framework...")
	
	execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/_deps/sycl_llvm-src/buildbot/configure.py --cuda)
	execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/_deps/sycl_llvm-src/buildbot/compile.py)
	
	set(SYCL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/_deps/sycl_llvm-src/)

endif(NOT DEFINED SYCL_PATH)


message(STATUS "Setting up clang...")

set(CMAKE_CXX_COMPILER "${SYCL_PATH}/build/bin/clang++.exe" CACHE INTERNAL "C++ compiler" FORCE)
set(CMAKE_CXX_FLAGS  "-W#pragma-messages -Wno-unknown-cuda-version -Wno-enum-constexpr-conversion -Wno-linker-warnings -Wno-macro-redefined -fsycl -fsycl-targets=nvptx64-nvidia-cuda")
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_C_COMPILER "${SYCL_PATH}/build/bin/clang.exe" CACHE INTERNAL "C compiler" FORCE)
set(CMAKE_C_COMPILER_WORKS 1)

project(ElevenRender)



message(STATUS "Checking dependencies...")
find_package(Python3 REQUIRED)
find_package(Git REQUIRED)
# And ninja.

message(STATUS "Setting up Boost...")

set(Boost_COMPILER "-clangw15")
set(Boost_USE_STATIC_LIBS ON)
#set(BOOST_ENABLE_CMAKE ON)
find_package(Boost 1.79.0 COMPONENTS program_options json log REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 


message(STATUS "Getting RapidObj")
message(STATUS ${Boost_INCLUDE_DIRS})

FetchContent_Declare(rapidobj
    GIT_REPOSITORY  https://github.com/guybrush77/rapidobj.git
    GIT_TAG         origin/master)

FetchContent_MakeAvailable(rapidobj)
include_directories(ElevenRender _deps/rapidobj-src/include)


message(STATUS "Getting stb_image.h")

FetchContent_Declare(stb_image
    URL   "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
	DOWNLOAD_NO_EXTRACT TRUE)

FetchContent_MakeAvailable(stb_image)
include_directories(ElevenRender _deps/stb_image-src)

message(STATUS "Getting stb_image_write.h")

FetchContent_Declare(stb_image_write
    URL   "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
	DOWNLOAD_NO_EXTRACT TRUE)

FetchContent_MakeAvailable(stb_image_write)
include_directories(ElevenRender _deps/stb_image_write-src)


message(STATUS "Getting MikkTSpace")

FetchContent_Declare(mikktspace
    GIT_REPOSITORY  https://github.com/mmikk/MikkTSpace.git
	GIT_TAG         origin/master)

FetchContent_MakeAvailable(mikktspace)
include_directories(ElevenRender _deps/mikktspace-src)


message(STATUS "Getting Open Image Denoiser prebuilt")

FetchContent_Declare(oidnzip
	URL "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.3/oidn-1.4.3.x64.vc14.windows.zip"
	DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(oidnzip)
execute_process(WORKING_DIRECTORY _deps/oidnzip-src COMMAND ${CMAKE_COMMAND} -E tar xvf oidn-1.4.3.x64.vc14.windows.zip)


include_directories(_deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/include) 
include_directories(src) 

FILE(GLOB SOURCES src/*.cpp)
FILE(GLOB LAN_SOURCES src/lan/*.cpp)
add_executable(ElevenRender ${SOURCES} ${LAN_SOURCES} _deps/mikktspace-src/mikktspace.c)

#target_link_libraries(ElevenRender PUBLIC Boost::program_options Boost::json Boost::log Boost::asio)

target_link_libraries(ElevenRender ${Boost_LIBRARIES})
target_link_libraries(ElevenRender _deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/lib/OpenImageDenoise)
target_link_libraries(ElevenRender _deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/lib/tbb)



#set(Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost" )
#set(BOOST_ROOT _deps/boost-src CACHE PATH "Boost library path" )

#set(Boost_COMPILER "-vc143")
#set(Boost_NO_BOOST_CMAKE ON)

#set(BOOST_INCLUDE_LIBRARIES program_options json log asio)
#
#FetchContent_Declare(
#  Boost
#  GIT_REPOSITORY https://github.com/boostorg/boost.git
#  GIT_TAG boost-1.79.0
#  GIT_PROGRESS TRUE
#)
#
##
#FetchContent_MakeAvailable(Boost)

#
#execute_process(
#		WORKING_DIRECTORY _deps/boost-src
#		COMMAND cmd /C bootstrap.bat)
#		
#execute_process(
#		WORKING_DIRECTORY _deps/boost-src
#		COMMAND cmd /C b2.exe link=static)	

#set(BOOST_ROOT _deps/boost-src CACHE PATH "Boost library path" )
#set(Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost" )
#
#set(Boost_USE_STATIC_LIBS ON)
##set(Boost_COMPILER "-vc143")
#
##set(BOOST_ENABLE_CMAKE ON)
###set(BOOST_INCLUDE_LIBRARIES program_options json log asio assign)
###
###FetchContent_Declare(
###  Boost
###  GIT_REPOSITORY https://github.com/boostorg/boost.git
###  GIT_TAG boost-1.79.0
###  GIT_PROGRESS TRUE
###)
###
###FetchContent_GetProperties(Boost)
###if(NOT Boost_POPULATED)
###  FetchContent_Populate(Boost)
###endif()
#
## This Clang SYCL version is not compatible with Windows SEH. It's necessary to remove the __try line from the win_mutex.ipp file. I don't really understand the implications of this change, so keep in mind this can break things.
#message(STATUS "Replacing SEH")
#
#STRING(REGEX REPLACE "/" "\\\\" Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}) 
#
#execute_process(
#		COMMAND cmd /C string_replace.bat "__try" "try" \"${Boost_INCLUDE_DIRS}\\boost\\asio\\detail\\impl\\win_mutex.ipp" > \"${Boost_INCLUDE_DIRS}\\boost\\asio\\detail\\impl\\win_mutex2.ipp")
#	
#message(STATUS cmd /C string_replace.bat "__try" "try" \"${Boost_INCLUDE_DIRS}\\boost\\asio\\detail\\impl\\win_mutex.ipp" > \"${Boost_INCLUDE_DIRS}\\boost\\asio\\detail\\impl\\win_mutex2.ipp")
#	
#execute_process(
#		COMMAND cmd /C erase ${Boost_INCLUDE_DIRS}"\\boost\\asio\\detail\\impl\\win_mutex.ipp")
#
#execute_process(
#		COMMAND cmd /C rename ${Boost_INCLUDE_DIRS}"\\boost\\asio\\detail\\impl\\win_mutex2.ipp" "win_mutex.ipp")

## I was not able to build boost with cmake in windows because some tags, so instead I'm just using the scripts provided to build it.
#message(STATUS "Building boost manually")
#
#include(ExternalProject)
#
## Download boost from git and build regex module
#
#
#ExternalProject_Add(Boost_download
#  GIT_REPOSITORY https://github.com/boostorg/boost.git
#  GIT_TAG boost-1.79.0
#  GIT_PROGRESS TRUE
#  BUILD_IN_SOURCE 1
#  UPDATE_COMMAND ""
#  PATCH_COMMAND ""
#  CONFIGURE_COMMAND bootstrap.bat
#  BUILD_COMMAND b2.exe install
#    --without-python
#    --without-mpi
#    --disable-icu
#    --prefix=${CMAKE_BINARY_DIR}/INSTALL
#    --threading=single,multi
#    --link=shared
#    --variant=release
#    -j8
#  INSTALL_COMMAND ""
#  INSTALL_DIR ${CMAKE_BINARY_DIR}/INSTALL
#)
#
#set(Boost_LIBRARY_DIR ${CMAKE_BINARY_DIR}/INSTALL/lib/ )
#set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/INSTALL/include/boost-1_79/ )
#
#add_library(Boost SHARED IMPORTED)
#set_target_properties(Boost PROPERTIES IMPORTED_LOCATION ${Boost_LIBRARY_DIR})
#
#include_directories(${Boost_INCLUDE_DIR})
#
##execute_process(
##		WORKING_DIRECTORY _deps/boost-src
##		COMMAND cmd /C bootstrap.bat)
##		
##execute_process(
##		WORKING_DIRECTORY _deps/boost-src
##		COMMAND cmd /C b2.exe)	
##
##find_package(Boost 1.79.0 COMPONENTS program_options json log REQUIRED)
#
#
##include_directories(ElevenRender ${Boost_INCLUDE_DIRS})
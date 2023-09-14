cmake_minimum_required(VERSION 3.16)
include(FetchContent)
include(CheckCXXSourceCompiles)
Set(FETCHCONTENT_QUIET FALSE)

# SYCL_PATH The path where Clang is installed
# USE_ICPX 
# SYCL_TARGETS
# OFFLOAD_ARCH
# SPIR64_DEVICES
# DOWNLOAD_DPCPP
# CUSTOM_SHADERS


# SANITY CHECKS:
if (NOT DOWNLOAD_DPCPP AND NOT SYCL_PATH)
	message(FATAL_ERROR "SYCL_PATH not set!")
endif()
if (DOWNLOAD_DPCPP AND SYCL_PATH)
	message(FATAL_ERROR "DOWNLOAD_DPCPP and SYCL_PATH cannot be set together")
endif()
if (USE_ICPX AND CUSTOM_SHADERS)
	message(WARNING "ICPX won't support IR code emission, CUSTOM_SHADERS forcibly disabled!")
	set(CUSTOM_SHADERS OFF)
endif()
if (NOT SYCL_TARGETS)
	message(WARNING "No targets selected, ElevenRender requires AOT for the running device, OpenCL cpu targets selected by default")
	set(SYCL_TARGETS "spir64_x86_64")
endif()
string(REPLACE "," ";" SYCL_TARGETS_LIST ${SYCL_TARGETS})
if ("spir64_gen" IN_LIST SYCL_TARGETS_LIST AND NOT SPIR64_DEVICES)
	message(WARNING "No device selected for spir64_gen targets, selected all architectures by default. This option can slow down the compilation at the linking stage.")
	set(SPIR64_DEVICES "bdw,skl,kbl,cfl,apl,bxt,glk,whl,aml,cml,icllp,lkf,ehl,jsl,tgllp,rkl,adl-s,adl-p,adl-n,dg1,acm-g10,ats-m150,dg2-g10,acm-g11,ats-m75,dg2-g11,acm-g12,dg2-g12,pvc-sdv,pvc,gen11,gen12lp,gen8,gen9,xe,xe-hpc,xe-hpg")
endif()
if ("amdgcn-amd-amdhsa" IN_LIST SYCL_TARGETS_LIST AND NOT OFFLOAD_ARCH)
	set(OFFLOAD_ARCH "native")
	message(WARNING "No offload architecture selected for amdgcn-amd-amdhsa targets, selected the native architecture by default")
endif()




# DEPDENDENCY CHECK:
message(STATUS "Checking dependencies...")
find_package(Python3 REQUIRED)
find_package(Git REQUIRED)
if ("nvptx64-nvidia-cuda" IN_LIST SYCL_TARGETS_LIST)
	set(CUPTI_LIBRARY_PATH NOTFOUND)
	if(WIN32)
		set(PATHS "$ENV{PATH}")
		set(LIB_PREFIX "")
		set(LIB_EXTENSION "dll")
		foreach(DIR ${PATHS})
			file(GLOB FOUND_LIBRARY "${DIR}/${LIB_PREFIX}cupti64*.${LIB_EXTENSION}")
			if(FOUND_LIBRARY)
				list(GET FOUND_LIBRARY 0 FIRST_MATCH)
				set(CUPTI_LIBRARY_PATH ${FIRST_MATCH})
				break()
			endif()
		endforeach()
		if(NOT CUPTI_LIBRARY_PATH)
			message(WARNING "Cupti64 not found in the paths! You won't be able to find CUDA devices if you don't set your path to: CUDA_DIR/extras/CUPTI/lib64")
		endif()
	else()
		message(WARNING "Linux CUDA cupti64* checker not implemented")
	endif()
endif()

# DPC++ DOWNLOAD
option(DOWNLOAD_DPCPP "DPC++ download option" OFF)
if(${DOWNLOAD_LLVM})
	set(SYCL_PATH "${CMAKE_BINARY_DIR}/_deps/sycl_llvm-src/build" CACHE STRING "SYCL compiler path")
    message(STATUS "Getting Intel DPC++ framework...")
    FetchContent_Declare(sycl_llvm
                GIT_REPOSITORY  https://github.com/intel/llvm
                GIT_TAG         6977f1aced3ed6a08573fdbdd4f35a5d719c8d98
                GIT_PROGRESS TRUE)

    FetchContent_MakeAvailable(sycl_llvm)
    message(STATUS "Building Intel's DPC++ framework...")
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SYCL_PATH}/../buildbot/configure.py --cuda)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SYCL_PATH}/../buildbot/compile.py)
endif()

# COMPILER SETUP
if(USE_ICPX)
	set(CXX_EXEC_NAME "icpx")
	set(C_EXEC_NAME "icx")
else()
	set(CXX_EXEC_NAME "clang++")
	set(C_EXEC_NAME "clang")
endif()
if(WIN32)
	set(CXX_EXEC_NAME "${CXX_EXEC_NAME}.exe")
	set(C_EXEC_NAME "${C_EXEC_NAME}.exe")
endif()
set(CMAKE_CXX_COMPILER "${SYCL_PATH}/bin/${CXX_EXEC_NAME}" CACHE INTERNAL "C++ compiler" FORCE)
set(CMAKE_CXX_COMPILER_WORKS 1) # TODO: remove this
set(CMAKE_C_COMPILER "${SYCL_PATH}/bin/${C_EXEC_NAME}" CACHE INTERNAL "C compiler" FORCE)
set(CMAKE_C_COMPILER_WORKS 1) # TODO: remove this
#set(CMAKE_RC_COMPILER "${SYCL_PATH}/bin/${C_EXEC_NAME}" CACHE INTERNAL "RC compiler" FORCE)
#set(CMAKE_RC_COMPILER_WORKS 1) # TODO: remove this
set(CMAKE_CXX_FLAGS  "-Wno-unused-command-line-argument -fsycl -fsycl-targets=${SYCL_TARGETS}")
separate_arguments(SHADER_COMPILE_FLAGS UNIX_COMMAND ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xsycl-target-backend=amdgcn-amd-amdhsa --offload-arch=${OFFLOAD_ARCH} -Xsycl-target-backend=spir64_gen -device -Xsycl-target-backend=spir64_gen ${SPIR64_DEVICES}")


project(ElevenRender)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# DEPENDENCIES SETUP:
if(WIN32)
    SET(OIDN_FOLDER "oidn-1.4.3.x64.vc14.windows")
    SET(OIDN_FILE "${OIDN_FOLDER}.zip")
else()
    SET(OIDN_FOLDER "oidn-1.4.3.x86_64.linux")
    SET(OIDN_FILE "${OIDN_FOLDER}.tar.gz")
endif()
FetchContent_Declare(oidnzip
    URL "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.3/${OIDN_FILE}"
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(oidnzip)
message(STATUS "UNZIPPING ${OIDN_FILE}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" "-E" "tar" "xvf" "${OIDN_FILE}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/_deps/oidnzip-src
)

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES program_options json log interprocess)
add_subdirectory(boost EXCLUDE_FROM_ALL)

include_directories(ElevenRender ${CMAKE_CURRENT_SOURCE_DIR}/submodules/rapidobj/include)
include_directories(ElevenRender ${CMAKE_CURRENT_SOURCE_DIR}/submodules/MikkTSpace)
include_directories(ElevenRender ${CMAKE_CURRENT_SOURCE_DIR}/submodules/stb)
include_directories(ElevenRender ${CMAKE_BINARY_DIR}/_deps/oidnzip-src/${OIDN_FOLDER}/include)
include_directories(ElevenRender ${SYCL_PATH}/include/)
include_directories(ElevenRender ${CMAKE_CURRENT_SOURCE_DIR}/src)


FILE(GLOB SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


# CUSTOM SHADING:
option(CUSTOM_SHADERS "Custom shaders option" ON)
if(CUSTOM_SHADERS)
	list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp)
	list(APPEND SOURCES ${CMAKE_BINARY_DIR}/shader.ll)
	set_source_files_properties(${CMAKE_BINARY_DIR}/shader.ll PROPERTIES LANGUAGE CXX)
	add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/shader.ll
		COMMAND ${CMAKE_CXX_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/preprocessor/include ${SHADER_COMPILE_FLAGS} -S -emit-llvm  ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp -o ${CMAKE_BINARY_DIR}/shader.ll
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/function_body_replace.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ ${CMAKE_BINARY_DIR}/shader.ll
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp ${CMAKE_CURRENT_SOURCE_DIR}/function_body_replace.py
		COMMENT "Generating LLVM IR for shader_placeholder.cpp"
	)
endif()


add_executable(ElevenRender ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/submodules/MikkTSpace/mikktspace.c)


# Linking

target_link_directories(ElevenRender PRIVATE ${SYCL_PATH}/lib )
find_library(SYCL_LIBRARY
    NAMES sycl sycl6 sycl7
    PATHS ${SYCL_PATH}/lib
)
if(NOT SYCL_LIBRARY)
    message(FATAL_ERROR "SYCL library not found!")
endif()

target_link_libraries(ElevenRender Boost::program_options Boost::json Boost::log_setup Boost::log Boost::asio Boost::interprocess)
target_link_libraries(ElevenRender ${SYCL_LIBRARY})
if(WIN32)
	target_link_libraries(ElevenRender ${CMAKE_BINARY_DIR}/_deps/oidnzip-src/${OIDN_FOLDER}/lib/OpenImageDenoise.lib)
	target_link_libraries(ElevenRender ${CMAKE_BINARY_DIR}/_deps/oidnzip-src/${OIDN_FOLDER}/lib/tbb.lib)
else()
	target_link_libraries(ElevenRender ${CMAKE_BINARY_DIR}/_deps/oidnzip-src/${OIDN_FOLDER}/lib/libOpenImageDenoise.so)
	target_link_libraries(ElevenRender ${CMAKE_BINARY_DIR}/_deps/oidnzip-src/${OIDN_FOLDER}/lib/libtbb.so)
endif()












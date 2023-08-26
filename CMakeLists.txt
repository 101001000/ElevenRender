cmake_minimum_required(VERSION 3.16)
set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)
include(CheckCXXSourceCompiles)

# The path to the sycl compiler. By default it will download intel's dpc++
set(SYCL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/sycl_llvm-src/build" CACHE STRING "SYCL compiler path")
set(SYCL_TARGETS "spir64_x86_64,nvptx64-nvidia-cuda" CACHE STRING "SYCL compiler targets")

message(STATUS "Checking dependencies...")
find_package(Python3 REQUIRED)
find_package(Git REQUIRED)
# And ninja.

message(STATUS "SYCL compiler path set to: ${SYCL_PATH}")
message(STATUS "SYCL compiler targets set to: ${SYCL_TARGETS}")

if(NOT EXISTS ${SYCL_PATH}/bin/clang++.exe)

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


message(STATUS "Setting up clang...")

# CMake won't detect Clang ABI for the intel's Clang implementation with the -fsycl tag, so some compile features need to be specified manually.
set(CMAKE_CXX_COMPILE_FEATURES cxx_constexpr cxx_std_11 cxx_alias_templates cxx_variadic_templates cxx_decltype)

set(CMAKE_CXX_COMPILER "${SYCL_PATH}/bin/clang++.exe" CACHE INTERNAL "C++ compiler" FORCE)
set(CMAKE_CXX_FLAGS  "-fsycl -fsycl-targets=${SYCL_TARGETS}")
set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_C_COMPILER "${SYCL_PATH}/bin/clang.exe" CACHE INTERNAL "C compiler" FORCE)
set(CMAKE_C_COMPILER_WORKS 1)

project(ElevenRender)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


message(STATUS "Setting up Boost...")
set (BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES program_options json log interprocess)
add_subdirectory(boost EXCLUDE_FROM_ALL)

message(STATUS "Getting RapidObj")
message(STATUS ${Boost_INCLUDE_DIRS})

FetchContent_Declare(rapidobj
    GIT_REPOSITORY  https://github.com/guybrush77/rapidobj.git
    GIT_TAG         origin/master)

FetchContent_MakeAvailable(rapidobj)
include_directories(ElevenRender build/_deps/rapidobj-src/include)


message(STATUS "Getting stb_image.h")

FetchContent_Declare(stb_image
    URL   "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
	DOWNLOAD_NO_EXTRACT TRUE)

FetchContent_MakeAvailable(stb_image)
include_directories(ElevenRender build/_deps/stb_image-src)

message(STATUS "Getting stb_image_write.h")

FetchContent_Declare(stb_image_write
    URL   "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
	DOWNLOAD_NO_EXTRACT TRUE)

FetchContent_MakeAvailable(stb_image_write)
include_directories(ElevenRender build/_deps/stb_image_write-src)


message(STATUS "Getting MikkTSpace")

FetchContent_Declare(mikktspace
    GIT_REPOSITORY  https://github.com/mmikk/MikkTSpace.git
	GIT_TAG         origin/master)

FetchContent_MakeAvailable(mikktspace)
include_directories(ElevenRender build/_deps/mikktspace-src)


message(STATUS "Getting Open Image Denoiser prebuilt")

FetchContent_Declare(oidnzip
	URL "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.3/oidn-1.4.3.x64.vc14.windows.zip"
	DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(oidnzip)
execute_process(WORKING_DIRECTORY build/_deps/oidnzip-src COMMAND ${CMAKE_COMMAND} -E tar xvf oidn-1.4.3.x64.vc14.windows.zip)


include_directories(build/_deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/include) 
include_directories(src)
include_directories(${SYCL_PATH}/include/) 


FILE(GLOB SOURCES src/*.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/shader.ll
    COMMAND ${CMAKE_CXX_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/boost/libs/preprocessor/include -fsycl -fsycl-targets=spir64_x86_64,nvptx64-nvidia-cuda -S -emit-llvm  ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/src/shader.ll 
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/function_body_replace.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ ${CMAKE_CURRENT_SOURCE_DIR}/src/shader.ll
	COMMAND powershell.exe -noninteractive -NoProfile -ExecutionPolicy Bypass -Command \"& {[IO.File]::WriteAllText\('${CMAKE_CURRENT_SOURCE_DIR}\\src\\shader.ll', \([IO.File]::ReadAllText\('${CMAKE_CURRENT_SOURCE_DIR}\\src\\shader.ll'\) -replace \\\"`r`n\\\", \\\"`n\\\" \)\)}\;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_placeholder.cpp
    COMMENT "Generating LLVM IR for shader_placeholder.cpp"
)


set_source_files_properties(src/shader.ll PROPERTIES LANGUAGE CXX)
add_executable(ElevenRender ${SOURCES} build/_deps/mikktspace-src/mikktspace.c src/shader.ll)


target_link_directories(ElevenRender PRIVATE ${SYCL_PATH}/lib ) 

target_link_libraries(ElevenRender Boost::program_options Boost::json Boost::log_setup Boost::log Boost::asio Boost::interprocess)
target_link_libraries(ElevenRender _deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/lib/OpenImageDenoise)
target_link_libraries(ElevenRender _deps/oidnzip-src/oidn-1.4.3.x64.vc14.windows/lib/tbb)

find_library(SYCL_LIBRARY 
    NAMES sycl sycl6 sycl7
    PATHS ${SYCL_PATH}/lib
)

if(NOT SYCL_LIBRARY)
    message(FATAL_ERROR "SYCL library not found!")
endif()

target_link_libraries(ElevenRender ${SYCL_LIBRARY})


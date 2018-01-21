cmake_minimum_required(VERSION 2.6)
project(StereoDepthCL)
set(PROJECT_BRIEF "StereoDepthCL")
set(CMAKE_BUILD_TYPE DEBUG)
set(GCC_COMPILE_FLAGS "-std=c++11 -Wall -O3 -g")
set(GCC_LINK_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

#opencl
set(OPENCL_INC_DIR "C:\\Program Files (x86)\\AMD APP SDK\\3.0\\include" CACHE PATH  "path to opencl include directory")
set(OPENCL_LIB_DIR "C:\\Program Files (x86)\\AMD APP SDK\\3.0\\lib\\x86_64" CACHE PATH "path to opencl lib directory")
include_directories(${OPENCL_INC_DIR})
link_directories(${OPENCL_LIB_DIR})


#Devil
set(DEVIL_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/DevIL/include  CACHE PATH  "path to Devil include directory")
set(DEVIL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/DevIL/  CACHE PATH  "path to Devil include directory")
include_directories(${DEVIL_INC_DIR})
link_directories(${DEVIL_LIB_DIR})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)

